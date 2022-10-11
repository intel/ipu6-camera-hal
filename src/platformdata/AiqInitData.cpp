/*
 * Copyright (C) 2015-2022 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG AiqInitData

#include "AiqInitData.h"

#include <dirent.h>
#include <sys/stat.h>

#include <unordered_map>

#include "AiqUtils.h"
#include "PlatformData.h"
#include "ia_types.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"

using std::string;

namespace icamera {

static const char* CAMERA_AIQD_PATH = "/run/camera/";

AiqData::AiqData(const std::string& fileName, int maxSize) : mDataPtr(nullptr) {
    LOG1("%s, file name %s", __func__, fileName.c_str());

    mFileName = fileName;
    loadFile(fileName, &mData, maxSize);
}

AiqData::~AiqData() {
    LOG1("%s, aiqd file name %s", __func__, mFileName.c_str());
}

ia_binary_data* AiqData::getData() {
    return mDataPtr ? &mData : nullptr;
}

void AiqData::saveData(const ia_binary_data& data) {
    LOG1("%s", __func__);

    if (!mDataPtr || data.size != mData.size) {
        mDataPtr.reset(new char[data.size]);
        mData.size = data.size;
        mData.data = mDataPtr.get();
    }
    MEMCPY_S(mData.data, mData.size, data.data, data.size);

    saveDataToFile(mFileName, &mData);
}

void AiqData::loadFile(const std::string& fileName, ia_binary_data* data, int maxSize) {
    LOG1("%s, file name %s", __func__, fileName.c_str());
    CheckAndLogError(data == nullptr, VOID_VALUE, "data is nullptr");

    // Get file size
    struct stat fileStat;
    CLEAR(fileStat);
    int ret = stat(fileName.c_str(), &fileStat);
    if (ret != 0) {
        LOG1("There is no file %s", fileName.c_str());
        return;
    }

    int64_t usedFileSize = fileStat.st_size;
    if (maxSize > 0 && maxSize < fileStat.st_size) usedFileSize = maxSize;

    // Open file
    FILE* fp = fopen(fileName.c_str(), "rb");
    CheckWarning(fp == nullptr, VOID_VALUE, "Failed to open file %s, error %s", fileName.c_str(),
                 strerror(errno));

    std::unique_ptr<char[]> dataPtr(new char[usedFileSize]);

    // Read data
    size_t readSize = fread(dataPtr.get(), sizeof(char), usedFileSize, fp);
    fclose(fp);

    CheckWarning(readSize != (size_t)usedFileSize, VOID_VALUE, "Failed to read %s, error %s",
                 fileName.c_str(), strerror(errno));

    mDataPtr = std::move(dataPtr);
    data->data = mDataPtr.get();
    data->size = usedFileSize;
    LOG1("%s, file %s, size %d", __func__, fileName.c_str(), data->size);
}

void AiqData::saveDataToFile(const std::string& fileName, const ia_binary_data* data) {
    LOG1("%s", __func__);
    CheckAndLogError(data == nullptr, VOID_VALUE, "data is nullptr");

    // Open file
    FILE* fp = fopen(fileName.c_str(), "wb");
    CheckWarning(fp == nullptr, VOID_VALUE, "Failed to open file %s, error %s", fileName.c_str(),
                 strerror(errno));

    // Write data to file
    size_t writeSize = fwrite(data->data, 1, data->size, fp);
    if (writeSize != data->size) {
        LOGW("Failed to write data %s, error %s", fileName.c_str(), strerror(errno));
        fclose(fp);
        return;
    }

    fflush(fp);
    fclose(fp);

    LOG1("%s, file %s, size %d", __func__, fileName.c_str(), data->size);
}

AiqInitData::AiqInitData(const std::string& sensorName, const std::string& camCfgDir,
                         const std::vector<TuningConfig>& tuningCfg, const std::string& nvmDir,
                         int maxNvmSize, const std::string& camModuleName)
        : mSensorName(sensorName),
          mMaxNvmSize(maxNvmSize),
          mTuningCfg(tuningCfg),
          mNvm(nullptr) {
    LOG1("@%s, mMaxNvmSize:%d", __func__, mMaxNvmSize);

    std::string aiqbNameFromModuleInfo;
    if (nvmDir.length() > 0) {
        mNvmPath = nvmDir;

        if (camModuleName.length() > 0) {
            DIR* dir = opendir(camCfgDir.c_str());
            if (dir) {
                std::string aiqbName("camera_");
                std::string postfix(".aiqb");
                aiqbName.append(camModuleName);
                struct dirent* direntPtr = nullptr;
                while ((direntPtr = readdir(dir)) != nullptr) {
                    if ((strncmp(direntPtr->d_name, aiqbName.c_str(), aiqbName.length()) == 0) &&
                        (strstr(direntPtr->d_name, postfix.c_str()) != nullptr)) {
                        aiqbNameFromModuleInfo.assign(direntPtr->d_name);
                        break;
                    }
                }
                closedir(dir);
            }
        }
    }

    for (auto cfg : mTuningCfg) {
        string aiqbName = cfg.aiqbName;
        aiqbName.append(".aiqb");

        if (!aiqbNameFromModuleInfo.empty()) {
            aiqbName.assign(aiqbNameFromModuleInfo);
        }
        LOGI("aiqb file name %s", aiqbName.c_str());

        if (findConfigFile(camCfgDir, &aiqbName) != OK) {
            LOGE("there is no aiqb file:%s", cfg.aiqbName.c_str());
            return;
        }

        if (mCpf.find(cfg.tuningMode) == mCpf.end()) mCpf[cfg.tuningMode] = new AiqData(aiqbName);
    }

    mMkn = std::unique_ptr<MakerNote>(new MakerNote);
}

AiqInitData::~AiqInitData() {
    LOG1("@%s", __func__);

    for (auto aiqb : mCpf) {
        delete aiqb.second;
    }

    for (auto aiqd : mAiqd) {
        delete aiqd.second;
    }

    delete mNvm;
}

/**
 * findConfigFile
 *
 * Search the path where CPF files are stored
 */
int AiqInitData::findConfigFile(const std::string& camCfgDir, std::string* cpfPathName) {
    CheckAndLogError(!cpfPathName, BAD_VALUE, "@%s, cpfPathName is nullptr", __func__);
    LOG1("@%s, cpfPathName:%s", __func__, cpfPathName->c_str());

    std::vector<string> configFilePath;
    configFilePath.push_back("./");
    configFilePath.push_back(camCfgDir);
    int configFileCount = configFilePath.size();

    string cpfFile;
    for (int i = 0; i < configFileCount; i++) {
        cpfFile.append(configFilePath.at(i));
        cpfFile.append(*cpfPathName);
        struct stat st;
        if (!stat(cpfFile.c_str(), &st)) break;
        cpfFile.clear();
    }

    if (cpfFile.empty()) {
        LOG1("@%s:No CPF file found for %s", __func__, cpfPathName->c_str());
        return NAME_NOT_FOUND;
    }

    *cpfPathName = cpfFile;
    LOG1("@%s:CPF file found %s", __func__, cpfPathName->c_str());
    return OK;
}

int AiqInitData::getCpf(TuningMode mode, ia_binary_data* cpfData) {
    LOG1("@%s mode = %d", __func__, mode);
    CheckAndLogError(cpfData == nullptr, BAD_VALUE, "@%s, cpfData is nullptr", __func__);

    CheckAndLogError(mCpf.find(mode) == mCpf.end(), NO_INIT, "@%s, no aiqb, mode = %d", __func__,
                     mode);

    AiqData* cpf = mCpf[mode];
    CheckAndLogError(cpf == nullptr, NO_INIT, "@%s, cpf is nullptr", __func__);

    auto dataPtr = cpf->getData();
    CheckAndLogError(dataPtr == nullptr, BAD_VALUE, "@%s, cpf->getData() is nullptr", __func__);

    *cpfData = *dataPtr;

    return OK;
}

ia_binary_data* AiqInitData::getNvm(int cameraId, const char* overwrittenFile, int fileSize) {
    const char* nvmFile = mNvmPath.c_str();
    int size = mMaxNvmSize;
    if (overwrittenFile && fileSize) {
        nvmFile = overwrittenFile;
        size = fileSize;
    }
    if (!nvmFile || !size) return nullptr;

    if (!mNvm) {
        LOG2("NVM data for %s is located in %s, size %d", mSensorName.c_str(), nvmFile, size);

        mNvm = new AiqData(nvmFile, size);

        if (CameraDump::isDumpTypeEnable(DUMP_NVM_DATA)) {
            ia_binary_data* nvmData = mNvm->getData();
            if (nvmData && nvmData->data && nvmData->size > 0) {
                BinParam_t bParam;
                bParam.bType = BIN_TYPE_GENERAL;
                bParam.mType = M_NVM;
                bParam.sequence = 0;
                bParam.gParam.appendix = "nvm_data";
                CameraDump::dumpBinary(cameraId, nvmData->data, nvmData->size, &bParam);
            }
        }
    }

    return mNvm->getData();
}

std::string AiqInitData::getAiqdFileNameWithPath(TuningMode mode) {
    std::string aiqdFileName;
    aiqdFileName.append(CAMERA_AIQD_PATH);
    aiqdFileName.append(mSensorName);
    aiqdFileName.append("_");
    aiqdFileName.append(CameraUtils::tuningMode2String(mode));
    aiqdFileName.append(".aiqd");

    return aiqdFileName;
}

ia_binary_data* AiqInitData::getAiqd(TuningMode mode) {
    if (mAiqd.find(mode) == mAiqd.end()) {
        mAiqd[mode] = new AiqData(getAiqdFileNameWithPath(mode));
    }
    AiqData* aiqd = mAiqd[mode];
    CheckAndLogError(!aiqd, nullptr, "@%s, aiqd is nullptr", __func__);

    return aiqd->getData();
}

void AiqInitData::saveAiqd(TuningMode mode, const ia_binary_data& data) {
    if (mAiqd.find(mode) == mAiqd.end()) {
        mAiqd[mode] = new AiqData(getAiqdFileNameWithPath(mode));
    }

    AiqData* aiqd = mAiqd[mode];
    CheckAndLogError(!aiqd, VOID_VALUE, "@%s, aiqd is nullptr", __func__);

    aiqd->saveData(data);

    // After saved aiqd data, the aiqd data should be reloaded next time.
    delete aiqd;
    mAiqd.erase(mode);
}

int AiqInitData::initMakernote(int cameraId, TuningMode tuningMode) {
    return mMkn->init(cameraId, tuningMode);
}

int AiqInitData::deinitMakernote(int cameraId, TuningMode tuningMode) {
    return mMkn->deinit(cameraId, tuningMode);
}

int AiqInitData::saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                   int64_t sequence, TuningMode tuningMode) {
    return mMkn->saveMakernoteData(cameraId, makernoteMode, sequence, tuningMode);
}

void AiqInitData::updateMakernoteTimeStamp(int64_t sequence, uint64_t timestamp) {
    mMkn->updateTimestamp(sequence, timestamp);
}

void AiqInitData::acquireMakernoteData(uint64_t timestamp, Parameters* param) {
    mMkn->acquireMakernoteData(timestamp, param);
}

}  // namespace icamera
