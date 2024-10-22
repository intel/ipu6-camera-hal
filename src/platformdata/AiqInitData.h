/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "CameraMetadata.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "MakerNote.h"

namespace icamera {

class AiqData {
 public:
    explicit AiqData(const std::string& fileName, int maxSize = -1);
    ~AiqData();

    ia_binary_data* getData();
    void saveData(const ia_binary_data& data);

    void loadFile(const std::string& fileName, ia_binary_data* data, int maxSize);
    void saveDataToFile(const std::string& fileName, const ia_binary_data* data);

 private:
    std::string mFileName;
    ia_binary_data mData;
    std::unique_ptr<char[]> mDataPtr;

 private:
    DISALLOW_COPY_AND_ASSIGN(AiqData);
};

/**
 * This class ia a wrapper class which includes CPF data, AIQD data and NVM data.
 */
class AiqInitData {
 public:
    AiqInitData(const std::string& sensorName, const std::string& camCfgDir,
                const std::vector<TuningConfig>& tuningCfg, const std::string& nvmDir,
                int maxNvmSize, const std::string& camModuleName);
    ~AiqInitData();

    // cpf
    int getCpf(TuningMode mode, ia_binary_data* cpfData);

    // aiqd
    ia_binary_data* getAiqd(TuningMode mode);
    void saveAiqd(TuningMode mode, const ia_binary_data& data);

    // nvm
    ia_binary_data* getNvm(int cameraId, const char* overwrittenFile = nullptr, int fileSize = 0);

    // maker note
    int initMakernote(int cameraId, TuningMode tuningMode);
    int deinitMakernote(int cameraId, TuningMode tuningMode);
    int saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode, int64_t sequence,
                          TuningMode tuningMode);
    void* getMakernoteBuf(camera_makernote_mode_t makernoteMode, bool& dump);
    void updateMakernoteTimeStamp(int64_t sequence, uint64_t timestamp);
    void acquireMakernoteData(uint64_t timestamp, Parameters* param);

    std::string getAiqdFileNameWithPath(TuningMode mode);
    int findConfigFile(const std::string& camCfgDir, std::string* cpfPathName);

 private:
    std::string mSensorName;
    std::string mNvmPath;
    int mMaxNvmSize;
    std::vector<TuningConfig> mTuningCfg;

    // cpf
    std::unordered_map<TuningMode, AiqData*> mCpf;

    // nvm
    AiqData* mNvm;

    // aiqd
    std::unordered_map<TuningMode, AiqData*> mAiqd;

    // makernote
    std::unique_ptr<MakerNote> mMkn;

 private:
    DISALLOW_COPY_AND_ASSIGN(AiqInitData);
};

}  // namespace icamera
