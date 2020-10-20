/*
 * Copyright (C) 2015-2020 Intel Corporation
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

#define LOG_TAG "AiqInitData"

#include <sys/stat.h>

#include "iutils/CameraLog.h"
#include "AiqInitData.h"
#include "AiqUtils.h"
#include "PlatformData.h"
#include "ia_types.h"

using std::string;

namespace icamera {

AiqdData::AiqdData(TuningMode tuningMode, const string& sensorName) :
    mDataPtr(nullptr)
{
    CLEAR(mBinaryData);

    mAiqdFileName.append(CAMERA_CACHE_DIR);
    mAiqdFileName.append(sensorName);
    mAiqdFileName.append("_");
    mAiqdFileName.append(CameraUtils::tuningMode2String(tuningMode));
    mAiqdFileName.append(".aiqd");

    LOG1("%s, aiqd file name %s", __func__, mAiqdFileName.c_str());
    loadAiqdFromFile();
};

AiqdData::~AiqdData()
{
    LOG1("%s, aiqd file name %s", __func__, mAiqdFileName.c_str());
}

ia_binary_data* AiqdData::getAiqd()
{
    return mDataPtr ? &mBinaryData : nullptr;
}

void AiqdData::saveAiqd(const ia_binary_data& data)
{
    LOG1("%s", __func__);

    if (!mDataPtr || data.size != mBinaryData.size) {
        mDataPtr.reset(new char[data.size]);
        mBinaryData.size = data.size;
        mBinaryData.data = mDataPtr.get();
    }
    MEMCPY_S(mBinaryData.data, mBinaryData.size, data.data, data.size);

    saveAiqdToFile();
}

void AiqdData::loadAiqdFromFile()
{
    LOG1("%s", __func__);

    // Get file size
    struct stat fileStat;
    CLEAR(fileStat);
    int ret = stat(mAiqdFileName.c_str(), &fileStat);
    if (ret != 0) {
        LOG1("There is no aiqd file %s", mAiqdFileName.c_str());
        return;
    }

    // Opem aiqd file
    FILE* fp = fopen(mAiqdFileName.c_str(), "rb");
    CheckWarning(fp == nullptr, VOID_VALUE, "Failed to open aiqd file %s, error %s",
                 mAiqdFileName.c_str(), strerror(errno));

    std::unique_ptr<char[]> dataPtr(new char[fileStat.st_size]);

    // Read aiqd data
    size_t readSize = fread(dataPtr.get(), sizeof(char), fileStat.st_size, fp);
    fclose(fp);

    CheckWarning(readSize != (size_t)fileStat.st_size, VOID_VALUE,
                 "Failed to read aiqd %s, error %s",
                 mAiqdFileName.c_str(), strerror(errno));

    mDataPtr = move(dataPtr);
    mBinaryData.data = mDataPtr.get();
    mBinaryData.size = fileStat.st_size;
    LOG1("%s, aiqd file %s, size %d", __func__, mAiqdFileName.c_str(), mBinaryData.size);
}

void AiqdData::saveAiqdToFile()
{
    LOG1("%s", __func__);

    // Open aiqd file
    FILE* fp = fopen(mAiqdFileName.c_str(), "wb");
    CheckWarning(fp == nullptr, VOID_VALUE, "Failed to open aiqd file %s, error %s",
                 mAiqdFileName.c_str(), strerror(errno));

    // Write aiqd data to file
    size_t writeSize = fwrite(mBinaryData.data, 1, mBinaryData.size, fp);
    if (writeSize != mBinaryData.size) {
        LOGW("Failed to write aiqd data %s, error %s", mAiqdFileName.c_str(), strerror(errno));
        fclose(fp);
        return;
    }

    fflush(fp);
    fclose(fp);

    LOG1("%s, aiqd file %s, size %d", __func__, mAiqdFileName.c_str(), mBinaryData.size);
}

CpfConf::CpfConf()
{
    mLard = new IntelLard();
    mCmc = std::unique_ptr<IntelCmc>(new IntelCmc());
    CLEAR(mAiq);
    CLEAR(mIsp);
    CLEAR(mOthers);
}

CpfConf::~CpfConf()
{
    delete mLard;
    LOG1("@%s", __func__);
}

int CpfConf::init(const ia_binary_data& cpfData, const LardTagConfig* lardTagCfg)
{
    LOG1("@%s", __func__);

    CheckWarning(mCmc->getCmc(), OK, "cmc has already been init before!");
    CheckError((cpfData.data == nullptr), BAD_VALUE, "Error Initializing CPF configure");

    bool cmcRet = false;
    ia_lard *iaLard = mLard->init(&cpfData);
    if (iaLard) {
        LOG1("AIQB file supported by lard.");
        ia_lard_input_params lardInputParams;
        initLardInputParam(*iaLard, lardTagCfg, &lardInputParams);

        ia_lard_results* lardResults;
        // Run ia_lard, result is nullptr if aiqb file is not supported
        ia_err iaErr = mLard->run(iaLard, &lardInputParams, &lardResults);
        if (lardResults != nullptr) {
            LOG1("ia_lard_run success, using lard to get cmc mode and tuning.");
            cmcRet = mCmc->init(&lardResults->aiqb_cmc_data, nullptr);
            mAiq = lardResults->aiqb_aiq_data;
            mIsp = lardResults->aiqb_isp_data;
            mOthers = lardResults->aiqb_other_data;
        } else {
            LOGE("Fail to run ia_lard, iaErr = %d", iaErr);
        }
        mLard->deinit(iaLard);
    } else {
        LOG1("Lard not supported. The AIQB file may be in old CPF format");
        cmcRet = mCmc->init(&cpfData, nullptr);
        mAiq = cpfData;
        mIsp = cpfData;
        mOthers = cpfData;
    }
    CheckError(!cmcRet, FAILED_TRANSACTION, "Error cmc parser init!");

    return OK;
}

ia_cmc_t* CpfConf::getCmc() const
{
    return mCmc->getCmc();
}

uintptr_t CpfConf::getCmcHandle() const
{
    return mCmc->getCmcHandle();
}

void CpfConf::getIspData(ia_binary_data* ispData)
{
    ispData->data = mIsp.data;
    ispData->size = mIsp.size;
}

void CpfConf::getAiqData(ia_binary_data* aiqData)
{
    aiqData->data = mAiq.data;
    aiqData->size = mAiq.size;
}

void CpfConf::getOtherData(ia_binary_data* otherData)
{
    otherData->data = mOthers.data;
    otherData->size = mOthers.size;
}

void CpfConf::deinit()
{
    mCmc->deinit();
}

void CpfConf::initLardInputParam(const ia_lard& iaLard,
                                 const LardTagConfig* lardTagCfg,
                                 ia_lard_input_params* lardInputParam)
{
    if (!lardTagCfg) {
        lardInputParam->cmc_mode_tag = FOURCC_TO_UL('D','F','L','T');
        lardInputParam->aiq_mode_tag = FOURCC_TO_UL('D','F','L','T');
        lardInputParam->isp_mode_index = FOURCC_TO_UL('D','F','L','T');
        lardInputParam->others_mode_tag = FOURCC_TO_UL('D','F','L','T');
        return;
    }

    unsigned int count = 0;
    const unsigned int *tags = nullptr;

    mLard->getTagList(const_cast<ia_lard*>(&iaLard), FOURCC_TO_UL('L','C','M','C'), &count, &tags);
    lardInputParam->cmc_mode_tag = isTagValid(lardTagCfg->cmcTag, count, tags) ? \
                                   lardTagCfg->cmcTag : FOURCC_TO_UL('D','F','L','T');

    mLard->getTagList(const_cast<ia_lard*>(&iaLard), FOURCC_TO_UL('L','A','I','Q'), &count, &tags);
    lardInputParam->aiq_mode_tag = isTagValid(lardTagCfg->aiqTag, count, tags) ? \
                                   lardTagCfg->aiqTag : FOURCC_TO_UL('D','F','L','T');

    mLard->getTagList(const_cast<ia_lard*>(&iaLard), FOURCC_TO_UL('L','I','S','P'), &count, &tags);
    lardInputParam->isp_mode_index = isTagValid(lardTagCfg->ispTag, count, tags) ? \
                                     lardTagCfg->ispTag : FOURCC_TO_UL('D','F','L','T');

    mLard->getTagList(const_cast<ia_lard*>(&iaLard), FOURCC_TO_UL('L','T','H','R'), &count, &tags);
    lardInputParam->others_mode_tag = isTagValid(lardTagCfg->othersTag, count, tags) ? \
                                      lardTagCfg->othersTag : FOURCC_TO_UL('D','F','L','T');

    LOG1("@%s: The lard tags are: aiq-0x%x, isp-0x%x, cmc-0x%x, others-0x%x", __func__,
        lardInputParam->aiq_mode_tag, lardInputParam->isp_mode_index,
        lardInputParam->cmc_mode_tag, lardInputParam->others_mode_tag);
}

bool CpfConf::isTagValid(unsigned int tag, unsigned int count, const unsigned int* tags)
{
    if (tags != nullptr) {
        for (unsigned int i = 0; i < count; i++) {
            if (tags[i] == tag) return true;
        }
    }
    LOG1("@%s: Tag 0x%x is not valid. Will use DFLT instead.", __func__, tag);
    return false;
}

CpfStore::CpfStore(const std::string& sensorName,
                   const std::string& camCfgDir,
                   const std::vector<TuningConfig>& tuningCfg,
                   const std::vector<LardTagConfig>& lardTagCfg)
{
    LOG1("@%s:Sensor Name = %s", __func__, sensorName.c_str());

    LardTagConfig* oneLardTagCfg = nullptr;

    CLEAR(mCpfConfig);
    for (auto &cfg : tuningCfg) {
        if (mCpfConfig[cfg.tuningMode] != nullptr) {
            continue;
        }

        if (cfg.aiqbName.empty()) {
            LOGE("aiqb name is empty, sensor name %s", sensorName.c_str());
            continue;
        }

        if (mCpfData.find(cfg.aiqbName) == mCpfData.end()) {
            // Obtain the configurations
            if (loadConf(camCfgDir, cfg.aiqbName) != OK) {
                LOGE("load file %s failed, sensor %s", cfg.aiqbName.c_str(), sensorName.c_str());
                continue;
            }
        }

        oneLardTagCfg = nullptr;
        for (size_t i = 0; i < lardTagCfg.size(); i++) {
            if (cfg.tuningMode == lardTagCfg[i].tuningMode) {
                oneLardTagCfg = const_cast<LardTagConfig*>(&lardTagCfg[i]);
                break;
            }
        }

        CpfConf* cpfConf = new CpfConf();

        cpfConf->init(mCpfData[cfg.aiqbName], oneLardTagCfg);
        mCpfConfig[cfg.tuningMode] = cpfConf;
    }
}

CpfStore::~CpfStore()
{
    LOG1("@%s", __func__);
    for (int mode=0; mode<TUNING_MODE_MAX; mode++) {
        if (mCpfConfig[mode]) {
            mCpfConfig[mode]->deinit();
            delete mCpfConfig[mode];
        }
    }
    for (auto &cpfData : mCpfData) {
        if (cpfData.second.data) {
            free(cpfData.second.data);
        }
    }
    mCpfData.clear();
}

/**
 * findConfigFile
 *
 * Search the path where CPF files are stored
*/
int CpfStore::findConfigFile(const std::string& camCfgDir, std::string* cpfPathName)
{
    LOG1("@%s, cpfPathName:%p", __func__, cpfPathName);
    CheckError(!cpfPathName, BAD_VALUE, "@%s, cpfPathName is nullptr", __func__);

    std::vector<string> configFilePath;
    configFilePath.push_back("./");
    configFilePath.push_back(camCfgDir);
    int configFileCount = configFilePath.size();

    string cpfFile;
    for (int i = 0; i < configFileCount; i++) {
        cpfFile.append(configFilePath.at(i));
        cpfFile.append(*cpfPathName);
        struct stat st;
        if (!stat(cpfFile.c_str(), &st))
            break;
        cpfFile.clear();
    }

    if (cpfFile.empty()) {//CPF file not found
        LOG1("@%s:No CPF file found for %s", __func__,cpfPathName->c_str());
        return NAME_NOT_FOUND;
    }

    *cpfPathName = cpfFile;
    LOG1("@%s:CPF file found %s", __func__,cpfPathName->c_str());
    return OK;
}

/**
 * loadConf
 *
 * load the CPF file
*/
int CpfStore::loadConf(const std::string& camCfgDir, const std::string& aiqbName)
{
    LOG1("@%s", __func__);
    int ret = OK;
    const char *suffix = ".aiqb";

    string cpfPathName = aiqbName;
    cpfPathName.append(suffix);
    LOG1("aiqb file name %s", cpfPathName.c_str());

    if (findConfigFile(camCfgDir, &cpfPathName) != OK) {
        LOGE("CpfStore no aiqb file:%s", aiqbName.c_str());
        return NAME_NOT_FOUND;
    }

    LOG1("Opening CPF file \"%s\"", cpfPathName.c_str());
    FILE *file = fopen(cpfPathName.c_str(), "rb");
    CheckError((file == nullptr), NAME_NOT_FOUND, "ERROR in opening CPF file \"%s\": %s!", cpfPathName.c_str(), strerror(errno));
    do {
        int fileSize;
        if ((fseek(file, 0, SEEK_END) < 0) || ((fileSize = ftell(file)) < 0) || (fseek(file, 0, SEEK_SET) < 0)) {
            LOGE("ERROR querying properties of CPF file \"%s\": %s!", cpfPathName.c_str(), strerror(errno));
            ret = BAD_VALUE;
            break;
        }

        mCpfData[aiqbName].data = malloc(fileSize);
        if (!mCpfData[aiqbName].data) {
            LOGE("ERROR no memory in %s!", __func__);
            ret = NO_MEMORY;
            break;
        }

        if (fread(mCpfData[aiqbName].data, fileSize, 1, file) < 1) {
            LOGE("ERROR reading CPF file \"%s\"!", cpfPathName.c_str());
            ret = INVALID_OPERATION;
            break;
        }
        mCpfData[aiqbName].size = fileSize;
    } while (0);

    if (fclose(file)) {
        LOGE("ERROR in closing CPF file \"%s\": %s!", cpfPathName.c_str(), strerror(errno));
    }

    return ret;
}

/**
 * convenience getter for Isp data, Aiq data, cmc data and other data.
 */
int CpfStore::getCpfAndCmc(ia_binary_data* ispData,
                           ia_binary_data* aiqData,
                           ia_binary_data* otherData,
                           uintptr_t* cmcHandle,
                           TuningMode mode,
                           ia_cmc_t** cmcData)
{
    LOG1("@%s mode = %d", __func__, mode);
    CheckError((mCpfConfig[mode] == nullptr), NO_INIT, "@%s, No aiqb init, mode = %d", __func__, mode);
    if (ispData != nullptr)
        mCpfConfig[mode]->getIspData(ispData);
    if (aiqData != nullptr)
        mCpfConfig[mode]->getAiqData(aiqData);
    if (otherData != nullptr)
        mCpfConfig[mode]->getOtherData(otherData);
    if (cmcData) {
        *cmcData = mCpfConfig[mode]->getCmc();
    }
    if (cmcHandle) {
        *cmcHandle = mCpfConfig[mode]->getCmcHandle();
    }

    if (mode == TUNING_MODE_VIDEO_ULL) {
        LOG2("@%s ULL mode, ULL cpf file is used", __func__);
    } else if (mode == TUNING_MODE_VIDEO_CUSTOM_AIC) {
        LOG2("@%s CUSTOM AIC mode, CUSTOM AIC cpf file is used", __func__);
    } else if (mode == TUNING_MODE_VIDEO_LL) {
        LOG2("@%s VIDEO LL mode, VIDEO LL cpf file is used", __func__);
    } else if (mode == TUNING_MODE_VIDEO_REAR_VIEW) {
        LOG2("@%s VIDEO Rear View mode, VIDEO REAR VIEW cpf file is used", __func__);
    } else if (mode == TUNING_MODE_VIDEO_HITCH_VIEW) {
        LOG2("@%s VIDEO Hitch View mode, VIDEO HITCH VIEW cpf file is used", __func__);
    } else {
        LOG2("@%s VIDEO mode, default cpf file is used", __func__);
    }

    return OK;
}

AiqInitData::AiqInitData(const std::string& sensorName,
                         const std::string& camCfgDir,
                         const std::vector<TuningConfig>& tuningCfg,
                         const std::vector<LardTagConfig>& lardTagCfg,
                         const std::string& nvmDir,
                         int maxNvmSize) :
    mSensorName(sensorName),
    mCamCfgDir(camCfgDir),
    mNvmDir(nvmDir),
    mMaxNvmSize(maxNvmSize),
    mTuningCfg(tuningCfg),
    mLardTagCfg(lardTagCfg),
    mCpfStore(nullptr),
    mNvmDataBuf(nullptr),
    mMakerNote(nullptr)
{
    CLEAR(mNvmData);
    mMakerNote = std::unique_ptr<MakerNote>(new MakerNote);
}

AiqInitData::~AiqInitData()
{
    delete mCpfStore;
    for (auto aiqd : mAiqdDataMap) {
        delete aiqd.second;
    }
}

int AiqInitData::getCpfAndCmc(ia_binary_data* ispData,
                              ia_binary_data* aiqData,
                              ia_binary_data* otherData,
                              uintptr_t* cmcHandle,
                              TuningMode mode,
                              ia_cmc_t** cmcData)
{
    if (!mCpfStore) {
        mCpfStore = new CpfStore(mSensorName, mCamCfgDir, mTuningCfg, mLardTagCfg);
    }
    return mCpfStore->getCpfAndCmc(ispData, aiqData, otherData, cmcHandle, mode, cmcData);
}

status_t AiqInitData::loadNvm()
{
    LOG1("@%s", __func__);

    if (mNvmDir.length() == 0) {
        LOG1("NVM dirctory from config is null");
        return UNKNOWN_ERROR;
    }

    string nvmDataPath(NVM_DATA_PATH);
    if (nvmDataPath.back() != '/')
        nvmDataPath.append("/");

    nvmDataPath.append(mNvmDir);
    if (nvmDataPath.back() != '/')
        nvmDataPath.append("/");

    nvmDataPath.append("eeprom");
    LOG2("NVM data for %s is located in %s", mSensorName.c_str(), nvmDataPath.c_str());

    FILE* nvmFile = fopen(nvmDataPath.c_str(), "rb");
    CheckError(!nvmFile, UNKNOWN_ERROR, "Failed to open NVM file: %s", nvmDataPath.c_str());

    fseek(nvmFile, 0, SEEK_END);
    int nvmDataSize = std::min(static_cast<int>(ftell(nvmFile)), mMaxNvmSize);
    fseek(nvmFile, 0, SEEK_SET);

    std::unique_ptr<char[]> nvmData(new char[nvmDataSize]);
    LOG2("NVM data size: %d bytes", nvmDataSize);

    int ret = fread(nvmData.get(), nvmDataSize, 1, nvmFile);
    fclose(nvmFile);
    CheckError(ret == 0, UNKNOWN_ERROR, "Cannot read nvm data");

    mNvmDataBuf = std::move(nvmData);
    mNvmData.data = mNvmDataBuf.get();
    mNvmData.size = nvmDataSize;

    return OK;
}

ia_binary_data* AiqInitData::getNvm()
{
    if (!mNvmData.data || mNvmData.size == 0) {
        loadNvm();
    }

    return mNvmData.data ? &mNvmData : nullptr;
}

ia_binary_data* AiqInitData::getAiqd(TuningMode mode) {
    if (mAiqdDataMap.find(mode) == mAiqdDataMap.end()) {
        mAiqdDataMap[mode] = new AiqdData(mode, mSensorName);
    }
    AiqdData* aiqd = mAiqdDataMap[mode];
    CheckError(!aiqd, nullptr, "@%s, aiqd is nullptr", __func__);

    return aiqd->getAiqd();
}

void AiqInitData::saveAiqd(TuningMode mode, const ia_binary_data& data) {
    if (mAiqdDataMap.find(mode) == mAiqdDataMap.end()) {
        mAiqdDataMap[mode] = new AiqdData(mode, mSensorName);
    }

    AiqdData* aiqd = mAiqdDataMap[mode];
    CheckError(!aiqd, VOID_VALUE, "@%s, aiqd is nullptr", __func__);

    aiqd->saveAiqd(data);
}

void* AiqInitData::getMknHandle(void)
{
    return mMakerNote->getMknHandle();
}

int AiqInitData::saveMakernoteData(camera_makernote_mode_t makernoteMode, int64_t sequence)
{
    return mMakerNote->saveMakernoteData(makernoteMode, sequence);
}

void AiqInitData::updateMakernoteTimeStamp(int64_t sequence, uint64_t timestamp)
{
    mMakerNote->updateTimestamp(sequence, timestamp);
}

void AiqInitData::acquireMakernoteData(uint64_t timestamp, Parameters *param)
{
    mMakerNote->acquireMakernoteData(timestamp, param);
}

}
