/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "CameraMetadata.h"

#include "MakerNote.h"
#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelLard.h"
#include "modules/sandboxing/client/IntelCmc.h"
#else
#include "modules/algowrapper/IntelLard.h"
#include "modules/algowrapper/IntelCmc.h"
#endif

namespace icamera {

#define NVM_DATA_PATH "/sys/bus/i2c/devices/"

/**
 * This class is intended to save/load AIQD data.
 */
class AiqdData
{
public:
    AiqdData(TuningMode tuningMode, const std::string& sensorName);
    ~AiqdData();

    ia_binary_data* getAiqd();
    void saveAiqd(const ia_binary_data& data);

private:
    void loadAiqdFromFile();
    void saveAiqdToFile();

private:
    std::string mAiqdFileName;
    ia_binary_data mBinaryData;
    std::unique_ptr<char[]> mDataPtr;
};

/**
  * The IA data stored
*/
class CpfConf
{
public:
    CpfConf();
    virtual ~CpfConf();

    /**
     * \brief get CMC pointer
     *
     */
    ia_cmc_t* getCmc() const;

    /**
     * \brief get CMC uintptr_t
     *
     */
    uintptr_t getCmcHandle() const;

    /**
     * \brief get ISP data from CPF file
     *
     * \param[out] ia_binary_data* IspData: ISP data
     */
    void getIspData(ia_binary_data* IspData);

    /**
     * \brief get AIQ data from CPF file
     *
     * \param[out] ia_binary_data* AiqData: AIQ data
     */
    void getAiqData(ia_binary_data* AiqData);

    /**
     * \brief get others data from CPF file, including LTM data
     *
     * \param[out] ia_binary_data* otherData: others data
     */
    void getOtherData(ia_binary_data* otherData);

    /**
     * \brief parse CMC/ISP/AIQ/Others from the CPF data
     *
     * Parse the CMC/ISP/AIQ/Others data according to the tuning mode, and init
     * the CMC handler.
     *
     * \param[in] ia_binary_data: CPF data loaded from the AIQB file
     * \param[in] LardTagConfig: lard tag cfg
     *
     * \return OK if init successfully; otherwise non-0 value is returned.
     */
    int init(const ia_binary_data& cpfData, const LardTagConfig* lardTagCfg);

    /**
      * \brief deinit CMC handler.
    */
    void deinit();

private:
    DISALLOW_COPY_AND_ASSIGN(CpfConf);

    void initLardInputParam(const ia_lard& iaLard,
                            const LardTagConfig* lardTagCfg,
                            ia_lard_input_params* lardInputParam);
    bool isTagValid(unsigned int tag, unsigned int count, const unsigned int* tags);

private:
    IntelLard* mLard;
    std::unique_ptr<IntelCmc> mCmc;
    ia_binary_data mAiq;
    ia_binary_data mIsp;
    ia_binary_data mOthers;
};//end CpfConf

/**
  * CPF file operation class
*/
class CpfStore
{
public:
    CpfStore(const std::string& sensorName,
             const std::string& camCfgDir,
             const std::vector<TuningConfig>& tuningCfg,
             const std::vector<LardTagConfig>& lardTagCfg);
    virtual ~CpfStore();

    /**
     * get Isp and Aiq data info
     *
     * \param ispData: return isp data of struct ia_binary_data
     * \param aiqData: return aiq data of struct ia_binary_data
     * \param otherData: return other data of struct ia_binary_data, such as tuning data for LTM
     * \param cmcHandle: return cmc uintptr_t
     * \param mode: Camera Mode
     * \param cmcData: return cmc pointer
     * \return NO_INIT if data not found, return OK if success.
     */
    int getCpfAndCmc(ia_binary_data* ispData,
                     ia_binary_data* aiqData,
                     ia_binary_data* otherData,
                     uintptr_t* cmcHandle,
                     TuningMode mode = TUNING_MODE_VIDEO,
                     ia_cmc_t** cmcData = nullptr);
private:
    DISALLOW_COPY_AND_ASSIGN(CpfStore);

    int findConfigFile(const std::string& camCfgDir, std::string* cpfPathName);
    int loadConf(const std::string& camCfgDir, const std::string& aiqbName);

public:
    CpfConf* mCpfConfig[TUNING_MODE_MAX];
    std::map<std::string, ia_binary_data> mCpfData;

};//end CpfStore

/**
 * This class ia a wrapper class which includes CPF data, AIQD data and NVM data.
 */
class AiqInitData {
 public:
    AiqInitData(const std::string& sensorName,
                const std::string& camCfgDir,
                const std::vector<TuningConfig>& tuningCfg,
                const std::vector<LardTagConfig>& lardTagCfg,
                const std::string& nvmDir,
                int maxNvmSize);
    ~AiqInitData();

    // cpf and cmc
    int getCpfAndCmc(ia_binary_data* ispData,
                     ia_binary_data* aiqData,
                     ia_binary_data* otherData,
                     uintptr_t* cmcHandle,
                     TuningMode mode = TUNING_MODE_VIDEO,
                     ia_cmc_t** cmcData = nullptr);

    // aiqd
    ia_binary_data* getAiqd(TuningMode mode);
    void saveAiqd(TuningMode mode, const ia_binary_data& data);

    // nvm
    ia_binary_data* getNvm();

    // maker note
    void* getMknHandle(void);
    int saveMakernoteData(camera_makernote_mode_t makernoteMode, int64_t sequence);
    void updateMakernoteTimeStamp(int64_t sequence, uint64_t timestamp);
    void acquireMakernoteData(uint64_t timestamp, Parameters *param);

 private:
    status_t loadNvm();

 private:
    std::string mSensorName;
    std::string mCamCfgDir;
    std::string mNvmDir;
    int mMaxNvmSize;
    std::vector<TuningConfig> mTuningCfg;
    std::vector<LardTagConfig> mLardTagCfg;
    CpfStore* mCpfStore;

    // NVM data
    std::unique_ptr <char[]> mNvmDataBuf;
    ia_binary_data mNvmData;

    std::unique_ptr<MakerNote> mMakerNote;

    std::map<TuningMode, AiqdData*> mAiqdDataMap;
};

}
