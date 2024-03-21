/*
 * Copyright (C) 2015-2024 Intel Corporation.
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

#define LOG_TAG AiqUnit

#include <map>
#include <string>
#include <memory>

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "gc/IGraphConfigManager.h"

#include "AiqUnit.h"

namespace icamera {

AiqUnit::AiqUnit(int cameraId, SensorHwCtrl* sensorHw, LensHw* lensHw)
        : mCameraId(cameraId),
          // LOCAL_TONEMAP_S
          mLtm(nullptr),
          // LOCAL_TONEMAP_E
          mAiqUnitState(AIQ_UNIT_NOT_INIT),
          // INTEL_DVS_S
          mDvs(nullptr),
          // INTEL_DVS_S
          mCcaInitialized(false),
          mActiveStreamCount(0) {
    mAiqSetting = new AiqSetting(cameraId);
    mAiqEngine = new AiqEngine(cameraId, sensorHw, lensHw, mAiqSetting);

    // INTEL_DVS_S
    if (PlatformData::isDvsSupported(mCameraId)) {
        mDvs = new Dvs(cameraId);
    }
    // INTEL_DVS_E
    // LOCAL_TONEMAP_S
    if (PlatformData::isLtmEnabled(mCameraId)) {
        mLtm = new Ltm(cameraId);
    }
    // LOCAL_TONEMAP_E
}

AiqUnit::~AiqUnit() {
    if (mAiqUnitState == AIQ_UNIT_START) {
        stop();
    }
    if (mAiqUnitState == AIQ_UNIT_INIT) {
        deinit();
    }

    // LOCAL_TONEMAP_S
    delete mLtm;
    // LOCAL_TONEMAP_E
    // INTEL_DVS_S
    delete mDvs;
    // INTEL_DVS_E
    delete mAiqEngine;
    delete mAiqSetting;
}

int AiqUnit::init() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    int ret = mAiqSetting->init();
    if (ret != OK) {
        mAiqSetting->deinit();
        return ret;
    }

    if (mAiqUnitState == AIQ_UNIT_NOT_INIT) {
        ret = mAiqEngine->init();
        if (ret != OK) {
            mAiqEngine->deinit();
            return ret;
        }

        // LOCAL_TONEMAP_S
        if (mLtm) {
            mLtm->init();
        }
        // LOCAL_TONEMAP_E
    }

    mActiveStreamCount = 0;
    mAiqUnitState = AIQ_UNIT_INIT;

    return OK;
}

int AiqUnit::deinit() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    // LOCAL_TONEMAP_S
    if (mLtm) {
        mLtm->deinit();
    }
    // LOCAL_TONEMAP_E

    mAiqEngine->deinit();
    mAiqSetting->deinit();

    deinitIntelCcaHandle();
    mActiveStreamCount = 0;
    mAiqUnitState = AIQ_UNIT_NOT_INIT;

    return OK;
}

int AiqUnit::configure(const stream_config_t* streamList) {
    CheckAndLogError(streamList == nullptr, BAD_VALUE, "streamList is nullptr");

    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    if (mAiqUnitState != AIQ_UNIT_INIT && mAiqUnitState != AIQ_UNIT_STOP &&
        mAiqUnitState != AIQ_UNIT_CONFIGURED) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    int ret = mAiqSetting->configure(streamList);
    CheckAndLogError(ret != OK, ret, "configure AIQ settings error: %d", ret);

    ret = mAiqEngine->configure();
    CheckAndLogError(ret != OK, ret, "configure AIQ engine error: %d", ret);

    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);
    ret = initIntelCcaHandle(configModes);
    CheckAndLogError(ret < 0, BAD_VALUE, "@%s failed to create intel cca handle", __func__);

    mAiqUnitState = AIQ_UNIT_CONFIGURED;
    return OK;
}

void AiqUnit::resetIntelCcaHandle(const std::vector<ConfigMode>& configModes) {
    bool reinit = false;
    if ((PlatformData::supportUpdateTuning(mCameraId) || PlatformData::isDvsSupported(mCameraId)) &&
        !configModes.empty()) {
        std::shared_ptr<IGraphConfig> graphConfig =
            IGraphConfigManager::getInstance(mCameraId)->getGraphConfig(configModes[0]);
        if (graphConfig != nullptr) {
            std::vector<int32_t> streamIds;
            graphConfig->graphGetStreamIds(streamIds);

            if (streamIds.size() != mActiveStreamCount) {
                LOG1("%s, the pipe count(%zu) changed, need to re-init CCA", __func__,
                     streamIds.size());
                mActiveStreamCount = streamIds.size();
                reinit = true;
            }
        }
    }

    if (!mTuningModes.empty()) {
        for (const auto& cfg : configModes) {
            TuningMode tuningMode;
            int ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tuningMode);
            if (ret == OK) {
                bool match = false;
                for (auto mode : mTuningModes) {
                    if (tuningMode == mode) {
                        match = true;
                        break;
                    }
                }
                if (!match) {
                    LOG1("%s, tuning mode changed from %d to %d", __func__, mTuningModes[0],
                         tuningMode);
                    reinit = true;
                    break;
                }
            }
        }
    }

    if (reinit) deinitIntelCcaHandle();
}

int AiqUnit::initIntelCcaHandle(const std::vector<ConfigMode>& configModes) {
    resetIntelCcaHandle(configModes);

    if (mCcaInitialized) {
        // INTEL_DVS_S
        if (mDvs) {
            for (auto& cfg : configModes) {
                std::vector<int32_t> streamIds;
                std::shared_ptr<IGraphConfig> graphConfig =
                    IGraphConfigManager::getInstance(mCameraId)->getGraphConfig(cfg);
                if (graphConfig != nullptr) {
                    graphConfig->graphGetStreamIds(streamIds);
                }
                DvsConfig dvsConfig;
                dvsConfig.gdcConfigs.count = streamIds.size();
                for (size_t i = 0; i < streamIds.size(); ++i) {
                    dvsConfig.gdcConfigs.ids[i] = streamIds[i];
                }
                int ret = mDvs->configure(cfg, &dvsConfig);
                CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, configure DVS error", __func__);

                TuningMode tuningMode;
                ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tuningMode);
                CheckAndLogError(ret != OK, ret, "Failed to get tuningMode, cfg: %d", cfg);
                IntelCca* intelCca = IntelCca::getInstance(mCameraId, tuningMode);
                CheckAndLogError(!intelCca, UNKNOWN_ERROR, "Failed to get cca. mode:%d cameraId:%d",
                                 tuningMode, mCameraId);
                cca::cca_dvs_init_param dvsInitParam = {dvsConfig.zoomRatio, dvsConfig.outputType};
                ia_err iaErr = intelCca->reconfigDvs(dvsInitParam, dvsConfig.gdcConfigs);
                CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR, "Failed to reconfig DVS %d",
                                 iaErr);
            }
        }
        // INTEL_DVS_E
        return OK;
    }

    LOG1("<id%d>@%s", mCameraId, __func__);
    mTuningModes.clear();
    for (auto& cfg : configModes) {
        TuningMode tuningMode;
        int ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tuningMode);
        CheckAndLogError(ret != OK, ret, "%s: Failed to get tuningMode, cfg: %d", __func__, cfg);

        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->init", 1);

        // Initialize cca_cpf data
        ia_binary_data cpfData;
        cca::cca_init_params params = {};
        ret = PlatformData::getCpf(mCameraId, tuningMode, &cpfData);
        if (ret == OK && cpfData.data) {
            CheckAndLogError(cpfData.size > cca::MAX_CPF_LEN, UNKNOWN_ERROR,
                             "%s, AIQB buffer is too small cpfData:%d > MAX_CPF_LEN:%d", __func__,
                             cpfData.size, cca::MAX_CPF_LEN);
            MEMCPY_S(params.aiq_cpf.buf, cca::MAX_CPF_LEN, cpfData.data, cpfData.size);
            params.aiq_cpf.size = cpfData.size;
        }

        // Initialize cca_nvm data
        ia_binary_data* nvmData = PlatformData::getNvm(mCameraId);
        if (nvmData) {
            CheckAndLogError(nvmData->size > cca::MAX_NVM_LEN, UNKNOWN_ERROR,
                             "%s, NVM buffer is too small: nvmData:%d  MAX_NVM_LEN:%d", __func__,
                             nvmData->size, cca::MAX_NVM_LEN);
            MEMCPY_S(params.aiq_nvm.buf, cca::MAX_NVM_LEN, nvmData->data, nvmData->size);
            params.aiq_nvm.size = nvmData->size;
        }

        // Initialize cca_aiqd data
        ia_binary_data* aiqdData = PlatformData::getAiqd(mCameraId, tuningMode);
        if (aiqdData) {
            CheckAndLogError(aiqdData->size > cca::MAX_AIQD_LEN, UNKNOWN_ERROR,
                             "%s, AIQD buffer is too small aiqdData:%d > MAX_AIQD_LEN:%d", __func__,
                             aiqdData->size, cca::MAX_AIQD_LEN);
            MEMCPY_S(params.aiq_aiqd.buf, cca::MAX_AIQD_LEN, aiqdData->data, aiqdData->size);
            params.aiq_aiqd.size = aiqdData->size;
        }

        SensorFrameParams sensorParam = {};
        ret = PlatformData::calculateFrameParams(mCameraId, sensorParam);
        CheckAndLogError(ret != OK, ret, "%s: Failed to calculate frame params", __func__);
        AiqUtils::convertToAiqFrameParam(sensorParam, params.frameParams);

        params.frameUse = ia_aiq_frame_use_video;
        params.aiqStorageLen = MAX_SETTING_COUNT;
        // handle AE delay in AiqEngine
        params.aecFrameDelay = 0;

        // Initialize functions which need to be started
        params.bitmap = cca::CCA_MODULE_AE | cca::CCA_MODULE_AWB | cca::CCA_MODULE_PA |
                        cca::CCA_MODULE_SA | cca::CCA_MODULE_GBCE | cca::CCA_MODULE_LARD;
        if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
            params.bitmap |= cca::CCA_MODULE_AF;
        }

        std::shared_ptr<IGraphConfig> graphConfig =
            IGraphConfigManager::getInstance(mCameraId)->getGraphConfig(cfg);

        // LOCAL_TONEMAP_S
        bool hasLtm = PlatformData::isLtmEnabled(mCameraId);
        // HDR_FEATURE_S
        if (PlatformData::isEnableHDR(mCameraId) &&
            !PlatformData::isMultiExposureCase(mCameraId, tuningMode)) {
            hasLtm = false;
        }
        // HDR_FEATURE_E

        // DOL_FEATURE_S
        hasLtm |= (PlatformData::isDolShortEnabled(mCameraId) ||
                   PlatformData::isDolMediumEnabled(mCameraId));
        // DOL_FEATURE_E
        if (hasLtm && mLtm) {
            params.bitmap |= cca::CCA_MODULE_LTM;
            ret = mLtm->configure(configModes, graphConfig, VIDEO_STREAM_ID);
            CheckAndLogError(ret != OK, ret, "configure LTM engine error: %d", ret);
        }
        // LOCAL_TONEMAP_E

        // INTEL_DVS_S
        if (mDvs) {
            std::vector<int32_t> streamIds;
            DvsConfig dvsConfig;
            if (graphConfig != nullptr) {
                graphConfig->graphGetStreamIds(streamIds);
            }
            dvsConfig.gdcConfigs.count = streamIds.size();
            for (size_t i = 0; i < streamIds.size(); ++i) {
                dvsConfig.gdcConfigs.ids[i] = streamIds[i];
            }
            ret = mDvs->configure(cfg, &dvsConfig);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, configure DVS error", __func__);
            params.bitmap |= cca::CCA_MODULE_DVS;
            params.dvsOutputType = dvsConfig.outputType;
            params.dvsZoomRatio = dvsConfig.zoomRatio;
            params.enableVideoStablization = dvsConfig.enableDvs;
            params.gdcConfigs = dvsConfig.gdcConfigs;
        }
        // INTEL_DVS_E

        // DOL_FEATURE_S
        // Initialize Bcomp params
        if (PlatformData::isDolShortEnabled(mCameraId) ||
            PlatformData::isDolMediumEnabled(mCameraId)) {
            // Parse the DOL mode and CG ratio from sensor mode config
            if (graphConfig != nullptr) {
                std::string dol_mode_name;
                graphConfig->getDolInfo(params.conversionGainRatio, dol_mode_name);
                std::map<std::string, ia_bcomp_dol_mode_t> dolModeNameMap;
                dolModeNameMap["DOL_MODE_2_3_FRAME"] = ia_bcomp_dol_two_or_three_frame;
                dolModeNameMap["DOL_MODE_DCG"] = ia_bcomp_dol_dcg;
                dolModeNameMap["DOL_MODE_COMBINED_VERY_SHORT"] = ia_bcomp_dol_combined_very_short;
                dolModeNameMap["DOL_MODE_DCG_VERY_SHORT"] = ia_bcomp_dol_dcg_very_short;
                if (dolModeNameMap.count(dol_mode_name)) {
                    params.dolMode = dolModeNameMap[dol_mode_name];
                }
            }
            LOG2("conversionGainRatio: %f, dolMode: %d", params.conversionGainRatio,
                 params.dolMode);
            params.bitmap = params.bitmap | cca::CCA_MODULE_BCOM;
        } else if (PlatformData::getSensorAeEnable(mCameraId)) {
            params.conversionGainRatio = 1;
            params.dolMode = ia_bcomp_linear_hdr_mode;
            LOG2("WA: conversionGainRatio: %f, dolMode: %d", params.conversionGainRatio,
                 params.dolMode);
            params.bitmap = params.bitmap | cca::CCA_MODULE_BCOM;
        }
        // DOL_FEATURE_E

        if (PlatformData::supportUpdateTuning(mCameraId)) {
            if (graphConfig != nullptr) {
                std::vector<int32_t> streamIds;
                graphConfig->graphGetStreamIds(streamIds);
                params.aic_stream_ids.count = streamIds.size();
                CheckAndLogError(streamIds.size() > cca::MAX_STREAM_NUM, UNKNOWN_ERROR,
                                 "%s, Too many streams: %zu in graph", __func__, streamIds.size());
                for (size_t i = 0; i < streamIds.size(); ++i) {
                    params.aic_stream_ids.ids[i] = streamIds[i];
                }
            }
        }
        IntelCca* intelCca = IntelCca::getInstance(mCameraId, tuningMode);
        CheckAndLogError(!intelCca, UNKNOWN_ERROR, "Failed to get cca. mode:%d cameraId:%d",
                         tuningMode, mCameraId);
        ia_err iaErr = intelCca->init(params);
        if (iaErr == ia_err_none) {
            mTuningModes.push_back(tuningMode);
        } else {
            LOGE("%s, init IntelCca fails. mode:%d cameraId:%d", __func__, tuningMode, mCameraId);
            IntelCca::releaseInstance(mCameraId, tuningMode);
            return UNKNOWN_ERROR;
        }

        ret = PlatformData::initMakernote(mCameraId, tuningMode);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, PlatformData::initMakernote fails",
                         __func__);
    }

    mCcaInitialized = true;
    return OK;
}

void AiqUnit::deinitIntelCcaHandle() {
    if (!mCcaInitialized) return;

    LOG1("<id%d>@%s", mCameraId, __func__);
    for (auto& mode : mTuningModes) {
        IntelCca* intelCca = IntelCca::getInstance(mCameraId, mode);
        CheckAndLogError(!intelCca, VOID_VALUE, "%s, Failed to get cca: mode(%d), cameraId(%d)",
                         __func__, mode, mCameraId);

        if (PlatformData::isAiqdEnabled(mCameraId)) {
            cca::cca_aiqd aiqd = {};
            ia_err iaErr = intelCca->getAiqd(&aiqd);
            if (AiqUtils::convertError(iaErr) == OK) {
                ia_binary_data data = {aiqd.buf, static_cast<unsigned int>(aiqd.size)};
                PlatformData::saveAiqd(mCameraId, mode, data);
            } else {
                LOGW("@%s, failed to get aiqd data, iaErr %d", __func__, iaErr);
            }
        }

        int ret = PlatformData::deinitMakernote(mCameraId, mode);
        if (ret != OK) {
            LOGE("@%s, PlatformData::deinitMakernote fails", __func__);
        }

        intelCca->deinit();
        IntelCca::releaseInstance(mCameraId, mode);
    }

    mCcaInitialized = false;
}

int AiqUnit::start() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    if (mAiqUnitState != AIQ_UNIT_CONFIGURED && mAiqUnitState != AIQ_UNIT_STOP) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    // LOCAL_TONEMAP_S
    if (mLtm) {
        mLtm->start();
    }
    // LOCAL_TONEMAP_E
    int ret = mAiqEngine->startEngine();
    if (ret == OK) {
        mAiqUnitState = AIQ_UNIT_START;
    }

    return OK;
}

int AiqUnit::stop() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    if (mAiqUnitState == AIQ_UNIT_START) {
        mAiqEngine->stopEngine();
        // LOCAL_TONEMAP_S
        if (mLtm) {
            mLtm->stop();
        }
        // LOCAL_TONEMAP_E
    }

    mAiqUnitState = AIQ_UNIT_STOP;

    return OK;
}

int AiqUnit::run3A(long requestId, int64_t applyingSeq, int64_t* effectSeq) {
    AutoMutex l(mAiqUnitLock);
    TRACE_LOG_PROCESS("AiqUnit", "run3A");

    if (mAiqUnitState != AIQ_UNIT_START) {
        LOGW("%s: AIQ is not started: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    int ret = mAiqEngine->run3A(requestId, applyingSeq, effectSeq);
    CheckAndLogError(ret != OK, ret, "run 3A failed.");

    return OK;
}

std::vector<EventListener*> AiqUnit::getSofEventListener() {
    AutoMutex l(mAiqUnitLock);
    std::vector<EventListener*> eventListenerList;
    eventListenerList.push_back(mAiqEngine->getSofEventListener());
    return eventListenerList;
}

std::vector<EventListener*> AiqUnit::getStatsEventListener() {
    AutoMutex l(mAiqUnitLock);

    std::vector<EventListener*> eventListenerList;
    // LOCAL_TONEMAP_S
    if (mLtm) {
        eventListenerList.push_back(mLtm);
    }
    // LOCAL_TONEMAP_E

    return eventListenerList;
}

// INTEL_DVS_S
std::vector<EventListener*> AiqUnit::getDVSEventListener() {
    AutoMutex l(mAiqUnitLock);
    std::vector<EventListener*> eventListenerList;
    if (mDvs) eventListenerList.push_back(mDvs);
    return eventListenerList;
}
// INTEL_DVS_E

// PRIVACY_MODE_S
EventSource* AiqUnit::get3AReadyEventSource() {
    AutoMutex l(mAiqUnitLock);
    return PlatformData::getSupportPrivacy(mCameraId) == AE_BASED_PRIVACY_MODE ? mAiqEngine :
                                                                                 nullptr;
}
// PRIVACY_MODE_E

int AiqUnit::setParameters(const Parameters& params) {
    AutoMutex l(mAiqUnitLock);

    return mAiqSetting->setParameters(params);
}

void AiqUnit::dumpCcaInitParam(const cca::cca_init_params params) {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(AiqUnit), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("aiqStorageLen:%d", params.aiqStorageLen);
    LOG3("aecFrameDelay:%d", params.aecFrameDelay);
    LOG3("horizontal_crop_offset:%d", params.frameParams.horizontal_crop_offset);
    LOG3("vertical_crop_offset:%d", params.frameParams.vertical_crop_offset);
    LOG3("cropped_image_width:%d", params.frameParams.cropped_image_width);
    LOG3("cropped_image_height:%d", params.frameParams.cropped_image_height);
    LOG3("horizontal_scaling_numerator:%d", params.frameParams.horizontal_scaling_numerator);
    LOG3("horizontal_scaling_denominator:%d", params.frameParams.horizontal_scaling_denominator);
    LOG3("vertical_scaling_numerator:%d", params.frameParams.vertical_scaling_numerator);
    LOG3("vertical_scaling_denominator:%d", params.frameParams.vertical_scaling_denominator);
}

} /* namespace icamera */
