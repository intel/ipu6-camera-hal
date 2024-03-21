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

#define LOG_TAG PlatformData

#include "PlatformData.h"

#include <math.h>
#include <sys/sysinfo.h>

#include <memory>
#include <vector>

#include "CameraParser.h"
#include "iutils/CameraLog.h"
#include "ParameterHelper.h"
#include "PolicyParser.h"

#include "gc/GraphConfigManager.h"

using std::string;
using std::vector;

namespace icamera {
PlatformData* PlatformData::sInstance = nullptr;
Mutex PlatformData::sLock;

PlatformData* PlatformData::getInstance() {
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new PlatformData();
    }

    return sInstance;
}

void PlatformData::releaseInstance() {
    AutoMutex lock(sLock);
    LOG1("@%s", __func__);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

PlatformData::PlatformData() {
    LOG1("@%s", __func__);
    MediaControl* mc = MediaControl::getInstance();
    if (mc) {
        mc->initEntities();
    }

    CameraParser CameraParser(mc, &mStaticCfg);
    PolicyParser PolicyParser(&mStaticCfg);
}

PlatformData::~PlatformData() {
    LOG1("@%s", __func__);

    releaseGraphConfigNodes();

    MediaControl* mc = MediaControl::getInstance();
    if (mc) {
        mc->clearEntities();
        MediaControl::releaseInstance();
    }

    for (size_t i = 0; i < mAiqInitData.size(); i++) {
        delete mAiqInitData[i];
    }

    mAiqInitData.clear();
}

int PlatformData::init() {
    LOG2("@%s", __func__);

    parseGraphFromXmlFile();

    StaticCfg* staticCfg = &(getInstance()->mStaticCfg);
    for (size_t i = 0; i < staticCfg->mCameras.size(); i++) {
        const std::string& camModuleName = staticCfg->mCameras[i].mCamModuleName;
        AiqInitData* aiqInitData = new AiqInitData(
            staticCfg->mCameras[i].sensorName, getCameraCfgPath(),
            staticCfg->mCameras[i].mSupportedTuningConfig, staticCfg->mCameras[i].mNvmDirectory,
            staticCfg->mCameras[i].mMaxNvmDataSize, camModuleName, i);
        getInstance()->mAiqInitData.push_back(aiqInitData);

        staticCfg->getModuleInfoFromCmc(i);

        // Overwrite staticCfg with CameraModuleInfo in sensor xml
        if (!camModuleName.empty() &&
            staticCfg->mCameras[i].mCameraModuleInfoMap.find(camModuleName) !=
                staticCfg->mCameras[i].mCameraModuleInfoMap.end()) {
            ParameterHelper::merge(staticCfg->mCameras[i].mCameraModuleInfoMap[camModuleName],
                                   &staticCfg->mCameras[i].mCapability);
        }

        // HDR_FEATURE_S
        if (isEnableHDR(i)) {
            /* If sensor enable HDR replace media format to full range*/
            bool ret = updateMediaFormat(i, false);
            if (ret) {
                LOG1("%s, Using full range media format for HDR sensor %s", __func__,
                     getSensorName(i));
            }
        } else {
            bool ret = updateMediaFormat(i, true);
            if (ret) {
                LOG1("%s, Using narrow mode media format for sensor %s", __func__,
                     getSensorName(i));
            }
        }
        // HDR_FEATURE_E
    }

    return OK;
}

void PlatformData::StaticCfg::getModuleInfoFromCmc(int cameraId) {
    CameraInfo& info = mCameras[cameraId];

    if (info.mSupportedTuningConfig.empty()) return;

    // Get default tuning mode and cpf data to update some static capabilities
    TuningMode tuningMode = info.mSupportedTuningConfig[0].tuningMode;
    ia_binary_data cpfData;
    int ret = PlatformData::getCpf(cameraId, tuningMode, &cpfData);
    CheckWarning(ret != OK || !cpfData.data || cpfData.size > cca::MAX_CPF_LEN, VOID_VALUE,
                 "%s, AIQB error data %p size %d (max %d), ret %d", __func__, cpfData.data,
                 cpfData.size, cca::MAX_CPF_LEN, ret);

    cca::cca_cmc cmc;
    cca::cca_cpf* cpf = new cca::cca_cpf;
    cpf->size = cpfData.size;
    MEMCPY_S(cpf->buf, cca::MAX_CPF_LEN, cpfData.data, cpfData.size);

    ia_err iaRet = IntelCca::getInstance(cameraId, tuningMode)->getCMC(&cmc, cpf);
    delete cpf;
    IntelCca::releaseInstance(cameraId, tuningMode);
    CheckWarning(iaRet != ia_err_none, VOID_VALUE, "Get cmc data failed");

    LOG1("%s: base iso %d, dg [%4.2f, %4.2f], ag [%4.2f, %4.2f], from aiqb", __func__, cmc.base_iso,
         cmc.min_dg, cmc.max_dg, cmc.min_ag, cmc.max_ag);
    LOG1("%s: focal_len %d, min_fd %d, ap %d", __func__, cmc.optics.effect_focal_length,
         cmc.optics.min_focus_distance, cmc.lut_apertures);
    // HDR_FEATURE_S
    mCameras[cameraId].mMediaFormat = static_cast<ia_media_format>(cmc.media_format);
    LOG1("%s: media_format %d", __func__, cmc.media_format);
    // HDR_FEATURE_E
    int32_t maxAg = cmc.base_iso * cmc.max_ag;
    const CameraMetadata caps = ParameterHelper::getMetadata(info.mCapability);
    CameraMetadata meta;
    uint32_t tag = CAMERA_SENSOR_MAX_ANALOG_SENSITIVITY;
    icamera_metadata_ro_entry entry = caps.find(tag);
    if (!entry.count) {
        meta.update(tag, &maxAg, 1);
    }

    tag = CAMERA_SENSOR_INFO_SENSITIVITY_RANGE;
    entry = caps.find(tag);
    if (!entry.count) {
        int32_t maxDg = static_cast<int32_t>(cmc.max_dg);
        int32_t range[] = {cmc.base_iso, maxAg * maxDg};
        meta.update(tag, range, ARRAY_SIZE(range));
    }

    if (!meta.isEmpty()) {
        ParameterHelper::merge(meta, &info.mCapability);
    }
}

/**
 * Read graph descriptor and settings from configuration files.
 *
 * The resulting graphs represend all possible graphs for given sensor, and
 * they are stored in capinfo structure.
 */
void PlatformData::parseGraphFromXmlFile() {
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();

    // Assuming that PSL section from profiles is already parsed, and number
    // of cameras is known.
    graphConfig->addCustomKeyMap();
    for (size_t i = 0; i < getInstance()->mStaticCfg.mCameras.size(); ++i) {
        const string& fileName = getInstance()->mStaticCfg.mCameras[i].mGraphSettingsFile;
        if (fileName.empty()) {
            continue;
        }

        LOG2("Using graph setting file:%s for camera:%zu", fileName.c_str(), i);
        int ret = graphConfig->parse(i, fileName.c_str());
        CheckAndLogError(ret != OK, VOID_VALUE, "Could not read graph config file for camera %zu",
                         i);
    }
}

int PlatformData::queryGraphSettings(int cameraId, const stream_config_t* streamList) {
    if (PlatformData::getGraphConfigNodes(cameraId)) {
        IGraphConfigManager* gcInstance = IGraphConfigManager::getInstance(cameraId);
        if (gcInstance != nullptr && OK != gcInstance->queryGraphSettings(streamList)) {
            LOG2("@%s Failed to queryGraphSettings cameraId: %d", __func__, cameraId);
            return NO_ENTRY;
        }
    }
    return OK;
}

int PlatformData::getEdgeNrSetting(int cameraId, float totalGain, float hdrRatio,
                                   TuningMode mode, EdgeNrSetting& setting) {
    LOG2("%s, tuningmode %d, totalGain %f, hdrRatio %f", __func__, mode, totalGain, hdrRatio);
    const StaticCfg::CameraInfo& pCam = getInstance()->mStaticCfg.mCameras[cameraId];

    auto totalMap = pCam.mTotalGainHdrRatioToEdgeNrMap.find(mode);
    if (totalMap == pCam.mTotalGainHdrRatioToEdgeNrMap.end()) return NAME_NOT_FOUND;

    std::map<float, std::map<float, EdgeNrSetting>> l1SettingMap;
    // found the l1 value
    auto l1Map = totalMap->second.equal_range(totalGain);
    if (l1Map.second == totalMap->second.begin()) {
        l1SettingMap[l1Map.second->first] = l1Map.second->second;
    } else if (l1Map.first == totalMap->second.end()) {
        --l1Map.first;
        l1SettingMap[l1Map.first->first] = l1Map.first->second;
    } else if (l1Map.first != l1Map.second) {
        l1SettingMap[l1Map.first->first] = l1Map.first->second;
    } else {
        --l1Map.first;
        l1SettingMap[l1Map.first->first] = l1Map.first->second;
        l1SettingMap[l1Map.second->first] = l1Map.second->second;
    }

    auto interpolation = [](float v, float low, float up, const EdgeNrSetting& l,
                            const EdgeNrSetting& u) {
        EdgeNrSetting result = l;
        result.edgeStrength +=
            static_cast<int8_t>((u.edgeStrength - l.edgeStrength) * (v - low) / (up - low) + 0.5);
        result.nrStrength +=
            static_cast<int8_t>((u.nrStrength - l.nrStrength) * (v - low) / (up - low) + 0.5);
        return result;
    };

    for (auto& it : l1SettingMap) {
        std::map<float, EdgeNrSetting> l2SettingMap;
        // found the l2 values
        auto l2Map = it.second.equal_range(hdrRatio);
        if (l2Map.second == it.second.begin()) {
            l2SettingMap[l2Map.second->first] = l2Map.second->second;
        } else if (l2Map.first == it.second.end()) {
            --l2Map.first;
            l2SettingMap[l2Map.first->first] = l2Map.first->second;
        } else if (l2Map.first != l2Map.second) {
            l2SettingMap[l2Map.first->first] = l2Map.first->second;
        } else {
            --l2Map.first;
            l2SettingMap[l2Map.first->first] = interpolation(hdrRatio, l2Map.first->first,
                l2Map.second->first, l2Map.first->second, l2Map.second->second);
        }

        it.second = l2SettingMap;
    }

    if (l1SettingMap.size() == 1) {
        setting = l1SettingMap.begin()->second.begin()->second;
        return OK;
    } else if (l1SettingMap.size() == 2) {
        auto it = l1SettingMap.end();
        --it;
        setting = interpolation(totalGain, l1SettingMap.begin()->first, it->first,
            l1SettingMap.begin()->second.begin()->second, it->second.begin()->second);
        return OK;
    }

    return NAME_NOT_FOUND;
}

void PlatformData::releaseGraphConfigNodes() {
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();
    graphConfig->releaseGraphNodes();
    for (size_t i = 0; i < mStaticCfg.mCameras.size(); ++i) {
        IGraphConfigManager::releaseInstance(i);
    }
}

int PlatformData::getModuleInfo(int cameraId, std::string& moduleId, std::string& sensorId) {
    const StaticCfg::CameraInfo& pCam = getInstance()->mStaticCfg.mCameras[cameraId];

    if (pCam.mModuleId.empty() || pCam.mSensorId.empty()) return NAME_NOT_FOUND;

    moduleId = pCam.mModuleId;
    sensorId = pCam.mSensorId;

    return OK;
}

const char* PlatformData::getSensorName(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorName.c_str();
}

void PlatformData::setBoardName(const std::string& boardName) {
    getInstance()->mStaticCfg.mBoardName = boardName;
}

bool PlatformData::isHDRnetTuningUsed(int cameraId, bool& boardConfig) {
    auto& boards = getInstance()->mStaticCfg.mCameras[cameraId].mDisableHDRnetBoards;
    auto& boardName = getInstance()->mStaticCfg.mBoardName;

    if (!boards.empty()) boardConfig = true;

    if (boardName.empty()) return true;

    for (auto& board : boards) {
        LOG2("mBoardName %s, board %s", boardName.c_str(), board.c_str());
        if (board == boardName) {
            return false;
        }
    }

    return true;
}

const char* PlatformData::getSensorDescription(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorDescription.c_str();
}

const char* PlatformData::getLensName(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensName.c_str();
}

int PlatformData::getLensHwType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensHwType;
}

void PlatformData::setSensorMode(int cameraId, SensorMode sensorMode) {
    // Only change sensor mode when binning mode supported
    if (!PlatformData::isBinningModeSupport(cameraId)) return;
    getInstance()->mStaticCfg.mCameras[cameraId].mSensorMode = sensorMode;
}

SensorMode PlatformData::getSensorMode(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorMode;
}

bool PlatformData::isBinningModeSupport(int cameraId) {
    auto pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
    for (auto& cfg : pCam->mSupportedTuningConfig) {
        if (cfg.tuningMode == TUNING_MODE_VIDEO_BINNING) {
            return true;
        }
    }

    return false;
}

int PlatformData::getSensitivityRangeByTuningMode(int cameraId, TuningMode mode,
                                                  SensitivityRange& range) {
    auto& map = getInstance()->mStaticCfg.mCameras[cameraId].mTuningModeToSensitivityMap;
    if (map.find(mode) != map.end()) {
        range = map[mode];
        return OK;
    }

    return NAME_NOT_FOUND;
}

bool PlatformData::isPdafEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnablePdaf;
}

bool PlatformData::getSensorAwbEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorAwb;
}

bool PlatformData::getSensorAeEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorAe;
}

bool PlatformData::getRunIspAlways(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mRunIspAlways;
}

int PlatformData::getDVSType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDVSType;
}

bool PlatformData::getISYSCompression(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mISYSCompression;
}

bool PlatformData::getPSACompression(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPSACompression;
}

bool PlatformData::getOFSCompression(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mOFSCompression;
}

int PlatformData::getCITMaxMargin(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mCITMaxMargin;
}

bool PlatformData::isEnableAIQ(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAIQ;
}

int PlatformData::getAiqRunningInterval(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mAiqRunningInterval;
}

bool PlatformData::isEnableMkn(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableMkn;
}

float PlatformData::getAlgoRunningRate(int algo, int cameraId) {
    PlatformData::StaticCfg::CameraInfo* pCam = &getInstance()->mStaticCfg.mCameras[cameraId];

    if (pCam->mAlgoRunningRateMap.find(algo) != pCam->mAlgoRunningRateMap.end()) {
        return pCam->mAlgoRunningRateMap[algo];
    }

    return 0.0;
}

bool PlatformData::isStatsRunningRateSupport(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mStatsRunningRate;
}

bool PlatformData::isEnableLtmThread(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableLtmThread;
}

bool PlatformData::isFaceDetectionSupported(int cameraId) {
    Parameters* source = &(getInstance()->mStaticCfg.mCameras[cameraId].mCapability);
    const icamera::CameraMetadata& meta = icamera::ParameterHelper::getMetadata(*source);
    auto entry = meta.find(CAMERA_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        if (entry.data.u8[i] != CAMERA_STATISTICS_FACE_DETECT_MODE_OFF) return true;
    }

    return false;
}

bool PlatformData::isSchedulerEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSchedulerEnabled;
}

bool PlatformData::isFaceAeEnabled(int cameraId) {
    return (isFaceDetectionSupported(cameraId) &&
            getInstance()->mStaticCfg.mCameras[cameraId].mFaceAeEnabled);
}

int PlatformData::faceEngineVendor(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineVendor;
}

int PlatformData::faceEngineRunningInterval(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningInterval;
}

int PlatformData::faceEngineRunningIntervalNoFace(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningIntervalNoFace;
}

bool PlatformData::isFaceEngineSyncRunning(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningSync;
}

bool PlatformData::isIPUSupportFD(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineByIPU;
}

unsigned int PlatformData::getMaxFaceDetectionNumber(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxFaceDetectionNumber;
}

bool PlatformData::isDvsSupported(int cameraId) {
    camera_video_stabilization_list_t videoStabilizationList;
    Parameters* param = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;
    param->getSupportedVideoStabilizationMode(videoStabilizationList);

    bool supported = false;
    for (auto it : videoStabilizationList) {
        if (it == VIDEO_STABILIZATION_MODE_ON) {
            supported = true;
            break;
        }
    }

    const icamera::CameraMetadata& meta = icamera::ParameterHelper::getMetadata(*param);
    auto entry = meta.find(CAMERA_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
    if (entry.count > 0 && *entry.data.f > 1) supported = true;

    return supported;
}

bool PlatformData::psysAlignWithSof(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysAlignWithSof;
}

bool PlatformData::psysBundleWithAic(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysBundleWithAic;
}

bool PlatformData::swProcessingAlignWithIsp(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSwProcessingAlignWithIsp;
}

bool PlatformData::isUsingSensorDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseSensorDigitalGain;
}

bool PlatformData::isUsingIspDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseIspDigitalGain;
}

bool PlatformData::isNeedToPreRegisterBuffer(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mNeedPreRegisterBuffers;
}

// FRAME_SYNC_S
bool PlatformData::isEnableFrameSyncCheck(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFrameSyncCheckEnabled;
}
// FRAME_SYNC_E

bool PlatformData::isEnableDefog(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableLtmDefog;
}

int PlatformData::getExposureNum(int cameraId, bool multiExposure) {
    if (multiExposure) {
        return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureNum;
    }

    int exposureNum = 1;
    // DOL_FEATURE_S
    if (PlatformData::isDolShortEnabled(cameraId)) exposureNum++;

    if (PlatformData::isDolMediumEnabled(cameraId)) exposureNum++;
    // DOL_FEATURE_E

    return exposureNum;
}

bool PlatformData::isLtmEnabled(int cameraId) {
    // HDR_FEATURE_S
    if (isEnableHDR(cameraId)) {
        return true;
    }
    // HDR_FEATURE_E

    return getInstance()->mStaticCfg.mCameras[cameraId].mLtmEnabled;
}

ia_media_format PlatformData::getMediaFormat(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMediaFormat;
}

// HDR_FEATURE_S
bool PlatformData::updateMediaFormat(int cameraId, bool isNarrow) {
    ia_media_format tuning_media_format =
        getInstance()->mStaticCfg.mCameras[cameraId].mMediaFormat;
    ia_media_format media_format = tuning_media_format;
    switch (tuning_media_format) {
    case media_format_legacy:
        media_format = media_format_legacy;
        break;
    case media_format_srgb_jpeg:
        media_format = media_format_srgb_jpeg;
        break;
    case media_format_linear_mono:
        media_format = media_format_linear_mono;
        break;
    case media_format_custom:
        media_format = media_format_custom;
        break;
    case media_format_bt601_8b:
        media_format = isNarrow ? media_format_bt601_8b_narrow : media_format_bt601_8b;
        break;
    case media_format_bt709_8b:
        media_format = isNarrow ? media_format_bt709_8b_narrow : media_format_bt709_8b;
        break;
    case media_format_bt709_10b:
        media_format = isNarrow ? media_format_bt709_10b_narrow : media_format_bt709_10b;
        break;
    case media_format_bt2020_10b:
        media_format = isNarrow ? media_format_bt2020_10b_narrow : media_format_bt2020_10b;
        break;
    case media_format_bt2100_10b:
        media_format = isNarrow ? media_format_bt2100_10b_narrow : media_format_bt2100_10b;
        break;
    case media_format_bt2100_10b_cl:
        media_format = isNarrow ? media_format_bt2100_10b_cl_narrow : media_format_bt2100_10b_cl;
        break;
    case media_format_bt2020_12b:
        media_format = isNarrow ? media_format_bt2020_12b_narrow : media_format_bt2020_12b;
        break;
    case media_format_bt2100_12b:
        media_format = isNarrow ? media_format_bt2100_12b_narrow : media_format_bt2100_12b;
        break;
    case media_format_bt2100_12b_cl:
        media_format = isNarrow ? media_format_bt2100_12b_cl_narrow : media_format_bt2100_12b_cl;
        break;
    default:
        LOGE("invalid media format, default value used.");
        return false;
        break;
    }
    LOGI("%s, media format in tuning: %d, media format for aic %d.", __func__, tuning_media_format,
         media_format);
    getInstance()->mStaticCfg.mCameras[cameraId].mMediaFormat = media_format;
    return true;
}

bool PlatformData::isEnableHDR(int cameraId) {
    return (getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureType !=
            SENSOR_EXPOSURE_SINGLE);
}

int PlatformData::getHDRStatsInputBitDepth(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mHdrStatsInputBitDepth;
}

int PlatformData::getHDRStatsOutputBitDepth(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mHdrStatsOutputBitDepth;
}

int PlatformData::isUseFixedHDRExposureInfo(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseFixedHdrExposureInfo;
}
// HDR_FEATURE_E

bool PlatformData::isMultiExposureCase(int cameraId, TuningMode tuningMode) {
    // HDR_FEATURE_S
    if (tuningMode == TUNING_MODE_VIDEO_HDR || tuningMode == TUNING_MODE_VIDEO_HDR2 ||
        tuningMode == TUNING_MODE_VIDEO_HLC) {
        return true;
    } else if (getSensorAeEnable(cameraId)) {
        return true;
    }
    // HDR_FEATURE_E

    return false;
}

int PlatformData::getSensorExposureType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureType;
}

int PlatformData::getSensorGainType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorGainType;
}

bool PlatformData::isSkipFrameOnSTR2MMIOErr(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSkipFrameV4L2Error;
}

unsigned int PlatformData::getInitialSkipFrame(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mInitialSkipFrame;
}

unsigned int PlatformData::getInitialPendingFrame(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mInitialPendingFrame;
}

unsigned int PlatformData::getMaxRawDataNum(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxRawDataNum;
}

bool PlatformData::getTopBottomReverse(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mTopBottomReverse;
}

bool PlatformData::isPsysContinueStats(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysContinueStats;
}

unsigned int PlatformData::getPreferredBufQSize(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPreferredBufQSize;
}

int PlatformData::getLtmGainLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mLtmGainLag;
}

int PlatformData::getMaxSensorDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxSensorDigitalGain;
}

SensorDgType PlatformData::sensorDigitalGainType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorDgType;
}

int PlatformData::getDigitalGainLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDigitalGainLag;
}

int PlatformData::getExposureLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mExposureLag;
}

int PlatformData::getAnalogGainLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mAnalogGainLag;
}

PolicyConfig* PlatformData::getExecutorPolicyConfig(const std::set<int>& graphIds) {
    PlatformData::StaticCfg* cfg = &getInstance()->mStaticCfg;

    size_t i = 0;
    size_t graphCount = graphIds.size();
    PolicyConfig* pCfg = nullptr;
    for (i = 0; i < cfg->mPolicyConfig.size(); i++) {
        PolicyConfig& policy = cfg->mPolicyConfig[i];
        // Previous platforms only support cfg with one graph id.
        // Find cfg according to the first graphId for them
        if (!graphIds.empty() && (*policy.graphIds.begin() == *graphIds.begin())) pCfg = &policy;

        if (policy.graphIds.size() != graphCount) continue;
        bool match = true;
        for (auto it = graphIds.cbegin(); it != graphIds.cend(); ++it) {
            if (policy.graphIds.find(*it) == policy.graphIds.end()) {
                match = false;
                break;
            }
        }
        if (match) {
            return &policy;
        }
    }

    LOGW("Couldn't find the executor policy in xml, need %lu graphs:", graphIds.size());
    for (auto it = graphIds.begin(); it != graphIds.end(); ++it) {
        LOGW("    graph id %d", *it);
    }
    if (pCfg) LOGW("%s: use cfg with graph id %d", __func__, *pCfg->graphIds.begin());
    return pCfg;
}

int PlatformData::numberOfCameras() {
    return getInstance()->mStaticCfg.mCameras.size();
}

int PlatformData::getXmlCameraNumber() {
    return getInstance()->mStaticCfg.mCommonConfig.cameraNumber;
}

MediaCtlConf* PlatformData::getMediaCtlConf(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf;
}

int PlatformData::getCameraInfo(int cameraId, camera_info_t& info) {
    info.device_version = 1;
    info.orientation = getInstance()->mStaticCfg.mCameras[cameraId].mOrientation;
    info.name = getSensorName(cameraId);
    info.description = getSensorDescription(cameraId);
    info.capability = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;

    const CameraMetadata& meta = icamera::ParameterHelper::getMetadata(*info.capability);
    auto entry = meta.find(CAMERA_LENS_FACING);
    info.facing = FACING_BACK;
    if (entry.count == 1) {
        info.facing = entry.data.u8[0] == CAMERA_LENS_FACING_BACK ?
                                          FACING_BACK : FACING_FRONT;
    }

    // VIRTUAL_CHANNEL_S
    info.vc.total_num = 0;
    if (getInstance()->mStaticCfg.mCameras[cameraId].mVirtualChannel) {
        info.vc.total_num = getInstance()->mStaticCfg.mCameras[cameraId].mVCNum;
        info.vc.sequence = getInstance()->mStaticCfg.mCameras[cameraId].mVCSeq;
        info.vc.group = getInstance()->mStaticCfg.mCameras[cameraId].mVCGroupId;
    }
    // VIRTUAL_CHANNEL_E
    return OK;
}

bool PlatformData::isFeatureSupported(int cameraId, camera_features feature) {
    camera_features_list_t features;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedFeatures(features);

    if (features.empty()) {
        return false;
    }
    for (auto& item : features) {
        if (item == feature) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isSupportedStream(int cameraId, const stream_t& conf) {
    int width = conf.width;
    int height = conf.height;
    int format = conf.format;
    int field = conf.field;

    stream_array_t availableConfigs;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedStreamConfig(
        availableConfigs);
    bool sameConfigFound = false;
    for (auto const& config : availableConfigs) {
        if (config.format == format && config.field == field && config.width == width &&
            config.height == height) {
            sameConfigFound = true;
            break;
        }
    }

    return sameConfigFound;
}

void PlatformData::getSupportedISysSizes(int cameraId, vector<camera_resolution_t>& resolutions) {
    resolutions = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysSizes;
}

bool PlatformData::getSupportedISysFormats(int cameraId, vector<int>& formats) {
    formats = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;

    return true;
}

int PlatformData::getISysFormat(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc;
}

/**
 * The ISYS format is determined by the steps below:
 * 1. Try to use the specified format in media control config if it exists.
 * 2. If the given format is supported by ISYS, then use it.
 * 3. Use the first supported format if still could not find an appropriate one.
 */
void PlatformData::selectISysFormat(int cameraId, int format) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    if (mc != nullptr && mc->format != -1) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = mc->format;
    } else if (isISysSupportedFormat(cameraId, format)) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = format;
    } else {
        // Set the first one in support list to default Isys output.
        vector<int> supportedFormat =
            getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = supportedFormat[0];
    }
}

/**
 * The media control config is determined by the steps below:
 * 1. Check if can get one from the given MC ID.
 * 2. And then, try to use ConfigMode to find matched one.
 * 3. Use stream config to get a corresponding mc id, and then get the config by id.
 * 4. Return nullptr if still could not find an appropriate one.
 */
void PlatformData::selectMcConf(int cameraId, stream_t stream, ConfigMode mode, int mcId) {
    if (!isIsysEnabled(cameraId)) return;

    const StaticCfg::CameraInfo& pCam = getInstance()->mStaticCfg.mCameras[cameraId];

    MediaCtlConf* mcConfig = getMcConfByMcId(pCam, mcId);
    if (!mcConfig) {
        mcConfig = getMcConfByConfigMode(pCam, stream, mode);
    }

    if (!mcConfig) {
        mcConfig = getMcConfByStream(pCam, stream);
    }

    getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf = mcConfig;

    if (!mcConfig) {
        LOGE("No matching McConf: cameraId %d, configMode %d, mcId %d", cameraId, mode, mcId);
    }
}

/*
 * Find the MediaCtlConf based on the given MC id.
 */
MediaCtlConf* PlatformData::getMcConfByMcId(const StaticCfg::CameraInfo& cameraInfo, int mcId) {
    if (mcId == -1) {
        return nullptr;
    }

    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        if (mcId == mc.mcId) {
            return (MediaCtlConf*)&mc;
        }
    }

    return nullptr;
}

/*
 * Find the MediaCtlConf based on MC id in mStreamToMcMap.
 */
MediaCtlConf* PlatformData::getMcConfByStream(const StaticCfg::CameraInfo& cameraInfo,
                                              const stream_t& stream) {
    int mcId = -1;
    for (auto& table : cameraInfo.mStreamToMcMap) {
        for (auto& config : table.second) {
            if (config.format == stream.format && config.field == stream.field &&
                config.width == stream.width && config.height == stream.height) {
                mcId = table.first;
                break;
            }
        }
        if (mcId != -1) {
            break;
        }
    }

    return getMcConfByMcId(cameraInfo, mcId);
}

/*
 * Find the MediaCtlConf based on operation mode and stream info.
 */
MediaCtlConf* PlatformData::getMcConfByConfigMode(const StaticCfg::CameraInfo& cameraInfo,
                                                  const stream_t& stream, ConfigMode mode) {
    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        for (auto& cfgMode : mc.configMode) {
            if (mode != cfgMode) continue;

            int outputWidth = mc.outputWidth;
            int outputHeight = mc.outputHeight;
            int stride = CameraUtils::getStride(mc.format, mc.outputWidth);
            bool sameStride = (stride == CameraUtils::getStride(mc.format, stream.width));
            /*
             * outputWidth and outputHeight is 0 means the ISYS output size
             * is dynamic, we don't need to check if it matches with stream config.
             */
            if ((outputWidth == 0 && outputHeight == 0) ||
                ((stream.width == outputWidth || sameStride) && stream.height == outputHeight)) {
                return (MediaCtlConf*)&mc;
            }
        }
    }

    return nullptr;
}

/*
 * Check if video node is enabled via camera Id and video node type.
 */
bool PlatformData::isVideoNodeEnabled(int cameraId, VideoNodeType type) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    if (!mc) return false;

    for (auto const& nd : mc->videoNodes) {
        if (type == nd.videoNodeType) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isISysSupportedFormat(int cameraId, int format) {
    vector<int> supportedFormat;
    getSupportedISysFormats(cameraId, supportedFormat);

    for (auto const fmt : supportedFormat) {
        if (format == fmt) return true;
    }
    return false;
}

bool PlatformData::isISysSupportedResolution(int cameraId, camera_resolution_t resolution) {
    vector<camera_resolution_t> res;
    getSupportedISysSizes(cameraId, res);

    for (auto const& size : res) {
        if (resolution.width == size.width && resolution.height == size.height) return true;
    }

    return false;
}

int PlatformData::getISysRawFormat(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysRawFormat;
}

stream_t PlatformData::getISysOutputByPort(int cameraId, Port port) {
    stream_t config;
    CLEAR(config);

    MediaCtlConf* mc = PlatformData::getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, config, "Invalid media control config.");

    for (const auto& output : mc->outputs) {
        if (output.port == port) {
            config.format = output.v4l2Format;
            config.width = output.width;
            config.height = output.height;
            break;
        }
    }

    return config;
}

// DOL_FEATURE_S
int PlatformData::getFixedVbp(int cameraId) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    if (!mc) {
        LOGW("%s: Failed to get MC for fixed VBP, disable fixed VBP.", __func__);
        return -1;
    }
    return mc->vbp;
}

bool PlatformData::needHandleVbpInMetaData(int cameraId, ConfigMode configMode) {
    int fixedVbp;

    if (configMode != CAMERA_STREAM_CONFIGURATION_MODE_HDR) {
        return false;
    }

    // Fixed VBP take higher priority when both fixed and dynamic VBP are configured
    fixedVbp = getFixedVbp(cameraId);
    if (fixedVbp >= 0) {
        LOG2("%s: fixed VBP configure detected, no need to handle VBP in meta", __func__);
        return false;
    }

    vector<int> vbpOffset;
    PlatformData::getDolVbpOffset(cameraId, vbpOffset);
    if (vbpOffset.size() > 0) {
        return true;
    }

    return false;
}

bool PlatformData::needSetVbp(int cameraId, ConfigMode configMode) {
    int fixedVbp;

    if (configMode != CAMERA_STREAM_CONFIGURATION_MODE_HDR) {
        return false;
    }

    fixedVbp = getFixedVbp(cameraId);
    if (fixedVbp >= 0) {
        LOG2("%s: Fixed VBP configure detected, value %d", __func__, fixedVbp);
        return true;
    }

    vector<int> vbpOffset;
    PlatformData::getDolVbpOffset(cameraId, vbpOffset);
    if (vbpOffset.size() > 0) {
        LOG2("%s: Dynamic VBP configure detected", __func__);
        return true;
    }

    return false;
}

void PlatformData::getDolVbpOffset(int cameraId, vector<int>& dolVbpOffset) {
    dolVbpOffset = getInstance()->mStaticCfg.mCameras[cameraId].mDolVbpOffset;
}

bool PlatformData::isDolShortEnabled(int cameraId) {
    return isVideoNodeEnabled(cameraId, VIDEO_GENERIC_SHORT_EXPO);
}

bool PlatformData::isDolMediumEnabled(int cameraId) {
    return isVideoNodeEnabled(cameraId, VIDEO_GENERIC_MEDIUM_EXPO);
}
// DOL_FEATURE_E

// CSI_META_S
bool PlatformData::isCsiMetaEnabled(int cameraId) {
    // FILE_SOURCE_S
    if (isFileSourceEnabled()) return false;
    // FILE_SOURCE_E
    return isVideoNodeEnabled(cameraId, VIDEO_CSI_META);
}
// CSI_META_E

bool PlatformData::isAiqdEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAiqd;
}

int PlatformData::getFormatByDevName(int cameraId, const string& devName, McFormat& format) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (auto& fmt : mc->formats) {
        if (fmt.formatType == FC_FORMAT && devName == fmt.entityName) {
            format = fmt;
            return OK;
        }
    }

    LOGE("Failed to find DevName for cameraId: %d, devname: %s", cameraId, devName.c_str());
    return BAD_VALUE;
}

int PlatformData::getVideoNodeNameByType(int cameraId, VideoNodeType videoNodeType,
                                         string& videoNodeName) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (auto const& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            videoNodeName = nd.name;
            return OK;
        }
    }

    LOGE("failed to find video note name for cameraId: %d", cameraId);
    return BAD_VALUE;
}

int PlatformData::getDevNameByType(int cameraId, VideoNodeType videoNodeType, string& devName) {
    if (!isIsysEnabled(cameraId)) return OK;

    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    bool isSubDev = false;

    switch (videoNodeType) {
        case VIDEO_PIXEL_ARRAY:
        case VIDEO_PIXEL_BINNER:
        case VIDEO_PIXEL_SCALER: {
            isSubDev = true;
            // For sensor subdevices are fixed and sensor HW may be initialized before configure,
            // the first MediaCtlConf is used to find sensor subdevice name.
            PlatformData::StaticCfg::CameraInfo* pCam =
                &getInstance()->mStaticCfg.mCameras[cameraId];
            mc = &pCam->mMediaCtlConfs[0];
            break;
        }
        case VIDEO_ISYS_RECEIVER_BACKEND:
        case VIDEO_ISYS_RECEIVER: {
            isSubDev = true;
            break;
        }
        default:
            break;
    }

    CheckAndLogError(!mc, NAME_NOT_FOUND, "failed to get MediaCtlConf, videoNodeType %d",
                     videoNodeType);

    for (auto& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            string tmpDevName;
            CameraUtils::getDeviceName(nd.name.c_str(), tmpDevName, isSubDev);
            if (!tmpDevName.empty()) {
                devName = tmpDevName;
                LOG2("@%s, Found DevName. cameraId: %d, get video node: %s, devname: %s", __func__,
                     cameraId, nd.name.c_str(), devName.c_str());
                return OK;
            } else {
                // Use default device name if cannot find it
                if (isSubDev)
                    devName = "/dev/v4l-subdev1";
                else
                    devName = "/dev/video5";
                LOGE("Failed to find DevName for cameraId: %d, get video node: %s, devname: %s",
                     cameraId, nd.name.c_str(), devName.c_str());
                return NAME_NOT_FOUND;
            }
        }
    }

    LOG1("Failed to find devname for cameraId: %d, use default setting instead", cameraId);
    return NAME_NOT_FOUND;
}

/**
 * The ISYS best resolution is determined by the steps below:
 * 1. If the resolution is specified in MediaCtlConf, then use it.
 * 2. Try to find the exact matched one in ISYS supported resolutions.
 * 3. Try to find the same ratio resolution.
 * 4. If still couldn't get one, then use the biggest one.
 */
camera_resolution_t PlatformData::getISysBestResolution(int cameraId, int width, int height,
                                                        int field) {
    LOG1("@%s, width:%d, height:%d", __func__, width, height);

    // Skip for interlace, we only support by-pass in interlaced mode
    if (field == V4L2_FIELD_ALTERNATE) {
        return {width, height};
    }

    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    // The isys output size is fixed if outputWidth/outputHeight != 0
    // So we use it to as the ISYS resolution.
    if (mc != nullptr && mc->outputWidth != 0 && mc->outputHeight != 0) {
        return {mc->outputWidth, mc->outputHeight};
    }

    const float RATIO_TOLERANCE = 0.05f;  // Supported aspect ratios that are within RATIO_TOLERANCE
    const float kTargetRatio = static_cast<float>(width) / height;

    vector<camera_resolution_t> res;
    // The supported resolutions are saved in res with ascending order(small -> bigger)
    getSupportedISysSizes(cameraId, res);

    // Try to find out the same resolution in the supported isys resolution list
    // if it couldn't find out the same one, then use the bigger one which is the same ratio
    for (auto const& size : res) {
        if (width <= size.width && height <= size.height &&
            fabs(static_cast<float>(size.width) / size.height - kTargetRatio) < RATIO_TOLERANCE) {
            LOG1("@%s: Found the best ISYS resoltoution (%d)x(%d)", __func__, size.width,
                 size.height);
            return {size.width, size.height};
        }
    }

    // If it still couldn't find one, then use the biggest one in the supported list.
    LOG1("@%s: ISYS resolution not found, used the biggest one: (%d)x(%d)", __func__,
         res.back().width, res.back().height);
    return {res.back().width, res.back().height};
}

bool PlatformData::isIsysEnabled(int cameraId) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mMediaCtlConfs.empty()) {
        return false;
    }
    return true;
}

int PlatformData::calculateFrameParams(int cameraId, SensorFrameParams& sensorFrameParams) {
    if (!isIsysEnabled(cameraId)) {
        LOG2("%s, no mc, just use default from xml", __func__);
        vector<camera_resolution_t> res;
        getSupportedISysSizes(cameraId, res);

        CheckAndLogError(res.empty(), BAD_VALUE, "Supported ISYS resolutions are not configured.");
        sensorFrameParams = {
            0, 0, static_cast<uint32_t>(res[0].width), static_cast<uint32_t>(res[0].height), 1, 1,
            1, 1};

        return OK;
    }

    CLEAR(sensorFrameParams);

    uint32_t width = 0;
    uint32_t horizontalOffset = 0;
    uint32_t horizontalBinNum = 1;
    uint32_t horizontalBinDenom = 1;
    uint32_t horizontalBin = 1;

    uint32_t height = 0;
    uint32_t verticalOffset = 0;
    uint32_t verticalBinNum = 1;
    uint32_t verticalBinDenom = 1;
    uint32_t verticalBin = 1;

    /**
     * For this function, it may be called without configuring stream
     * in some UT cases, the mc is nullptr at this moment. So we need to
     * get one default mc to calculate frame params.
     */
    MediaCtlConf* mc = PlatformData::getMediaCtlConf(cameraId);
    if (mc == nullptr) {
        PlatformData::StaticCfg::CameraInfo* pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
        mc = &pCam->mMediaCtlConfs[0];
    }

    bool pixArraySizeFound = false;
    for (auto const& current : mc->formats) {
        if (!pixArraySizeFound && current.width > 0 && current.height > 0) {
            width = current.width;
            height = current.height;
            pixArraySizeFound = true;
            LOG2("%s: active pixel array H=%d, W=%d", __func__, height, width);
            // Setup initial sensor frame params.
            sensorFrameParams.horizontal_crop_offset += horizontalOffset;
            sensorFrameParams.vertical_crop_offset += verticalOffset;
            sensorFrameParams.cropped_image_width = width;
            sensorFrameParams.cropped_image_height = height;
            sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
            sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
            sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
            sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
        }

        if (current.formatType != FC_SELECTION) {
            continue;
        }

        if (current.selCmd == V4L2_SEL_TGT_CROP) {
            width = current.width * horizontalBin;
            horizontalOffset = current.left * horizontalBin;
            height = current.height * verticalBin;
            verticalOffset = current.top * verticalBin;

            LOG2("%s: crop (binning factor: hor/vert:%d,%d)", __func__, horizontalBin, verticalBin);

            LOG2("%s: crop left = %d, top = %d, width = %d height = %d", __func__, horizontalOffset,
                 verticalOffset, width, height);

        } else if (current.selCmd == V4L2_SEL_TGT_COMPOSE) {
            if (width == 0 || height == 0) {
                LOGE(
                    "Invalid XML configuration, no pixel array width/height when handling compose, "
                    "skip.");
                return BAD_VALUE;
            }
            if (current.width == 0 || current.height == 0) {
                LOGW("%s: Invalid XML configuration for TGT_COMPOSE,"
                     "0 value detected in width or height",
                     __func__);
                return BAD_VALUE;
            } else {
                LOG2("%s: Compose width %d/%d, height %d/%d", __func__, width, current.width,
                     height, current.height);
                // the scale factor should be float, so multiple numerator and denominator
                // with coefficient to indicate float factor
                const int SCALE_FACTOR_COEF = 10;
                horizontalBin = width / current.width;
                horizontalBinNum = width * SCALE_FACTOR_COEF / current.width;
                horizontalBinDenom = SCALE_FACTOR_COEF;
                verticalBin = height / current.height;
                verticalBinNum = height * SCALE_FACTOR_COEF / current.height;
                verticalBinDenom = SCALE_FACTOR_COEF;
            }

            LOG2("%s: COMPOSE horizontal bin factor=%d, (%d/%d)", __func__, horizontalBin,
                 horizontalBinNum, horizontalBinDenom);
            LOG2("%s: COMPOSE vertical bin factor=%d, (%d/%d)", __func__, verticalBin,
                 verticalBinNum, verticalBinDenom);
        } else {
            LOGW("%s: Target for selection is not CROP neither COMPOSE!", __func__);
            continue;
        }

        sensorFrameParams.horizontal_crop_offset += horizontalOffset;
        sensorFrameParams.vertical_crop_offset += verticalOffset;
        sensorFrameParams.cropped_image_width = width;
        sensorFrameParams.cropped_image_height = height;
        sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
        sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
        sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
        sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
    }

    return OK;
}

void PlatformData::getSupportedTuningConfig(int cameraId, vector<TuningConfig>& configs) {
    configs = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig;
}

bool PlatformData::usePsys(int cameraId, int format) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty()) {
        LOG1("@%s, the tuning config in xml does not exist", __func__);
        return false;
    }

    if (getInstance()->mStaticCfg.mCameras[cameraId].mPSysFormat.empty()) {
        LOG1("@%s, the psys supported format does not exist", __func__);
        return false;
    }

    for (auto& psys_fmt : getInstance()->mStaticCfg.mCameras[cameraId].mPSysFormat) {
        if (format == psys_fmt) return true;
    }

    LOGW("%s, No matched format found, but expected format:%s", __func__,
         CameraUtils::pixelCode2String(format));

    return false;
}

int PlatformData::getConfigModesByOperationMode(int cameraId, uint32_t operationMode,
                                                vector<ConfigMode>& configModes) {
    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_END) {
        LOG2("%s: operationMode was invalid operation mode", __func__);
        return INVALID_OPERATION;
    }

    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist", __func__);

    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_AUTO) {
        if (getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto.empty()) {
            // Use the first config mode as default for auto
            configModes.push_back(
                getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig[0].configMode);
            LOG2("%s: add config mode %d for operation mode %d", __func__, configModes[0],
                 operationMode);
        } else {
            configModes = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto;
        }
    } else {
        for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
            if (operationMode == (uint32_t)cfg.configMode) {
                configModes.push_back(cfg.configMode);
                LOG2("%s: add config mode %d for operation mode %d", __func__, cfg.configMode,
                     operationMode);
            }
        }
    }

    if (configModes.size() > 0) return OK;
    LOGW("%s, configure number %zu, operationMode %x, cameraId %d", __func__, configModes.size(),
         operationMode, cameraId);
    return INVALID_OPERATION;
}

/*
 * This function adds for binning mode tuning support, and there are 2 tuning modes for configMode.
 * The first tuning mode will be used when selecting tuning mode.
 * When selecting small ISYS output, the binning mode should be moved to first, otherwise full size
 * mode should be the first tuning mode.
 */
void PlatformData::reorderSupportedTuningConfig(int cameraId, ConfigMode configMode) {
    auto pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
    int binningIdx = -1;
    int fullIdx = -1;
    int idx = -1;
    for (auto& cfg : pCam->mSupportedTuningConfig) {
        idx++;
        LOG1("%s, tuningMode %d, configMode %x, idx %d", __func__, cfg.tuningMode, cfg.configMode,
             idx);
        if (cfg.configMode == configMode) {
            if (cfg.tuningMode == TUNING_MODE_VIDEO_BINNING) {
                binningIdx = idx;
            } else {
                fullIdx = idx;
            }
        }
    }

    if (binningIdx < 0 || fullIdx < 0) return;

    bool smallSize = false;
    MediaCtlConf* mc = PlatformData::getMediaCtlConf(cameraId);
    vector<camera_resolution_t> res;
    // The supported resolutions are saved in res with ascending order(small -> bigger)
    getSupportedISysSizes(cameraId, res);
    if (!res.empty() && mc) {
        for (const auto& output : mc->outputs) {
            if (res.back().width > output.width || res.back().height > output.height) {
                smallSize = true;
                break;
            }
        }
    }

    if ((smallSize && binningIdx > fullIdx) || (!smallSize && binningIdx < fullIdx)) {
        // Switch binning mode and full mode
        auto config = pCam->mSupportedTuningConfig[fullIdx];
        pCam->mSupportedTuningConfig[fullIdx] = pCam->mSupportedTuningConfig[binningIdx];
        pCam->mSupportedTuningConfig[binningIdx] = config;
    }
}

int PlatformData::getTuningModeByConfigMode(int cameraId, ConfigMode configMode,
                                            TuningMode& tuningMode) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "the tuning config in xml does not exist");

    for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        LOG2("%s, tuningMode %d, configMode %x", __func__, cfg.tuningMode, cfg.configMode);
        if (cfg.configMode == configMode) {
            tuningMode = cfg.tuningMode;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no tuningModes", __func__, configMode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getTuningConfigByConfigMode(int cameraId, ConfigMode mode, TuningConfig& config) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist.", __func__);

    for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        if (cfg.configMode == mode) {
            config = cfg;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no TuningConfig", __func__, mode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getStreamIdByConfigMode(int cameraId, ConfigMode configMode) {
    std::map<int, int> modeMap = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModeToStreamId;
    return modeMap.find(configMode) == modeMap.end() ? -1 : modeMap[configMode];
}

int PlatformData::getMaxRequestsInflight(int cameraId) {
    int inflight = getInstance()->mStaticCfg.mCameras[cameraId].mMaxRequestsInflight;
    if (inflight <= 0) {
        inflight = isEnableAIQ(cameraId) ? 4 : MAX_BUFFER_COUNT;
    }

    return inflight;
}

bool PlatformData::getGraphConfigNodes(int cameraId) {
    return !(getInstance()->mStaticCfg.mCameras[cameraId].mGraphSettingsFile.empty());
}

GraphSettingType PlatformData::getGraphSettingsType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mGraphSettingsType;
}

camera_yuv_color_range_mode_t PlatformData::getYuvColorRangeMode(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mYuvColorRangeMode;
}

ia_binary_data* PlatformData::getAiqd(int cameraId, TuningMode mode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getAiqd(mode);
}

void PlatformData::saveAiqd(int cameraId, TuningMode tuningMode, const ia_binary_data& data) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    aiqInitData->saveAiqd(tuningMode, data);
}

int PlatformData::getCpf(int cameraId, TuningMode mode, ia_binary_data* aiqbData) {
    CheckAndLogError(cameraId >= MAX_CAMERA_NUMBER, BAD_VALUE, "@%s, bad cameraId:%d", __func__,
                     cameraId);
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist", __func__);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getCpf(mode, aiqbData);
}

bool PlatformData::isCSIBackEndCapture(int cameraId) {
    bool isCsiBECapture = false;
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (const auto& node : mc->videoNodes) {
        if (IPU6_UPSTREAM && node.videoNodeType == VIDEO_GENERIC &&
            node.name.find("ISYS capture") != string::npos) {
            isCsiBECapture = true;
            break;
        }
        if (node.videoNodeType == VIDEO_GENERIC &&
            (node.name.find("BE capture") != string::npos ||
             node.name.find("BE SOC capture") != string::npos)) {
            isCsiBECapture = true;
            break;
        }
    }

    return isCsiBECapture;
}

bool PlatformData::isCSIFrontEndCapture(int cameraId) {
    bool isCsiFeCapture = false;
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (const auto& node : mc->videoNodes) {
        if (IPU6_UPSTREAM && node.videoNodeType == VIDEO_GENERIC &&
            node.name.find("CSI2") != string::npos) {
            isCsiFeCapture = true;
            break;
        }
        if (node.videoNodeType == VIDEO_GENERIC &&
            (node.name.find("CSI-2") != string::npos || node.name.find("TPG") != string::npos)) {
            isCsiFeCapture = true;
            break;
        }
    }
    return isCsiFeCapture;
}

bool PlatformData::isTPGReceiver(int cameraId) {
    bool isTPGCapture = false;
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (const auto& node : mc->videoNodes) {
        if (node.videoNodeType == VIDEO_ISYS_RECEIVER && (node.name.find("TPG") != string::npos)) {
            isTPGCapture = true;
            break;
        }
    }
    return isTPGCapture;
}

int PlatformData::getSupportAeExposureTimeRange(int cameraId, camera_scene_mode_t sceneMode,
                                                camera_range_t& etRange) {
    Parameters* param = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;
    int ret = param->getSupportedSensorExposureTimeRange(etRange);
    if (ret == OK) return OK;

    vector<camera_ae_exposure_time_range_t> ranges;
    param->getSupportedAeExposureTimeRange(ranges);

    if (ranges.empty()) return NAME_NOT_FOUND;

    for (auto& item : ranges) {
        if (item.scene_mode == sceneMode) {
            etRange = item.et_range;
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

int PlatformData::getSupportAeGainRange(int cameraId, camera_scene_mode_t sceneMode,
                                        camera_range_t& gainRange) {
    vector<camera_ae_gain_range_t> ranges;
    getInstance()->mStaticCfg.mCameras[cameraId].mCapability.getSupportedAeGainRange(ranges);

    if (ranges.empty()) {
        return NAME_NOT_FOUND;
    }

    for (auto& item : ranges) {
        if (item.scene_mode == sceneMode) {
            gainRange = item.gain_range;
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

bool PlatformData::isUsingCrlModule(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseCrlModule;
}

vector<MultiExpRange> PlatformData::getMultiExpRanges(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMultiExpRanges;
}

// FILE_SOURCE_S
const char* PlatformData::getInjectedFile() {
    const char* PROP_CAMERA_FILE_INJECTION = "cameraInjectFile";
    return getenv(PROP_CAMERA_FILE_INJECTION);
}

bool PlatformData::isFileSourceEnabled() {
    return getInjectedFile() != nullptr;
}
// FILE_SOURCE_E

// VIRTUAL_CHANNEL_S
int PlatformData::getVirtualChannelSequence(int cameraId) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mVirtualChannel) {
        return getInstance()->mStaticCfg.mCameras[cameraId].mVCSeq;
    }

    return -1;
}

int PlatformData::getVcAggregator(int cameraId, struct VcAggregator& aggregator) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mVcAggregator.mIndex >= 0) {
        aggregator = getInstance()->mStaticCfg.mCameras[cameraId].mVcAggregator;
        return OK;
    }
    return NO_ENTRY;
}
// VIRTUAL_CHANNEL_E

camera_resolution_t* PlatformData::getPslOutputForRotation(int width, int height, int cameraId) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mOutputMap.empty(), nullptr,
                     "<id%d>@%s, there isn't pslOutputMapForRotation field in xml.", cameraId,
                     __func__);

    vector<UserToPslOutputMap>& outputMap = getInstance()->mStaticCfg.mCameras[cameraId].mOutputMap;
    for (auto& map : outputMap) {
        if (width == map.User.width && height == map.User.height) {
            LOG2("<id%d> find the psl output resoltion(%d, %d) for %dx%d", cameraId, map.Psl.width,
                 map.Psl.height, map.User.width, map.User.height);
            return &map.Psl;
        }
    }

    return nullptr;
}

const camera_resolution_t* PlatformData::getPreferOutput(int width, int height, int cameraId) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mPreferOutput.empty()) return nullptr;

    const std::vector<camera_resolution_t>& preferOutput =
        getInstance()->mStaticCfg.mCameras[cameraId].mPreferOutput;
    for (const auto& output : preferOutput) {
        // get preferred output for small size
        if ((width < output.width || height < output.height)
            && (width * output.height == height * output.width)) {
            LOG2("<id%d> the psl output: (%dx%d) for user: %dx%d", cameraId, output.width,
                 output.height, width, height);
            return &output;
        }
    }

    return nullptr;
}

bool PlatformData::isTestPatternSupported(int cameraId) {
    return !getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty();
}

int32_t PlatformData::getSensorTestPattern(int cameraId, int32_t mode) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty(), -1,
                     "<id%d>@%s, mTestPatternMap is empty!", cameraId, __func__);
    auto testPatternMap = getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap;

    if (testPatternMap.find(mode) == testPatternMap.end()) {
        LOGW("Test pattern %d wasn't found in configuration file, return -1", mode);
        return -1;
    }
    return testPatternMap[mode];
}

ia_binary_data* PlatformData::getNvm(int cameraId) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    // Allow overwritten nvm file if needed
    int size = getInstance()->mStaticCfg.mCameras[cameraId].mNvmOverwrittenFileSize;
    const char* nvmFile = getInstance()->mStaticCfg.mCameras[cameraId].mNvmOverwrittenFile.c_str();
    return getInstance()->mAiqInitData[cameraId]->getNvm(cameraId, nvmFile, size);
}

camera_coordinate_system_t PlatformData::getActivePixelArray(int cameraId) {
    camera_coordinate_system_t arraySize;
    CLEAR(arraySize);

    Parameters* param = &getInstance()->mStaticCfg.mCameras[cameraId].mCapability;
    if (param->getSensorActiveArraySize(arraySize) != OK) {
        return {0, 0, 0, 0};
    }

    return {arraySize.left, arraySize.top, arraySize.right, arraySize.bottom};
}

string PlatformData::getCameraCfgPath() {
    string cfgPath = string(CAMERA_DEFAULT_CFG_PATH);
#ifdef SUB_CONFIG_PATH
    cfgPath += string(SUB_CONFIG_PATH);
    cfgPath.append("/");
#endif

    char* p = getenv("CAMERA_CFG_PATH");
    return p ? string(p) : cfgPath;
}

string PlatformData::getGraphDescFilePath() {
    return PlatformData::getCameraCfgPath() + string(CAMERA_GRAPH_DESCRIPTOR_FILE);
}

string PlatformData::getGraphSettingFilePath() {
    return PlatformData::getCameraCfgPath() + string(CAMERA_GRAPH_SETTINGS_DIR);
}

int PlatformData::getSensorDigitalGain(int cameraId, float realDigitalGain) {
    int sensorDg = 0;
    int maxSensorDg = PlatformData::getMaxSensorDigitalGain(cameraId);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        int index = 0;
        while (pow(2, index) <= realDigitalGain) {
            sensorDg = index;
            index++;
        }
        sensorDg = CLIP(sensorDg, maxSensorDg, 0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d", __func__,
             PlatformData::sensorDigitalGainType(cameraId));
    }

    return sensorDg;
}

float PlatformData::getIspDigitalGain(int cameraId, float realDigitalGain) {
    float ispDg = 1.0f;
    int sensorDg = getSensorDigitalGain(cameraId, realDigitalGain);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        ispDg = realDigitalGain / pow(2, sensorDg);
        ispDg = CLIP(ispDg, ispDg, 1.0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d", __func__,
             PlatformData::sensorDigitalGainType(cameraId));
    }

    return ispDg;
}

int PlatformData::initMakernote(int cameraId, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);
    return getInstance()->mAiqInitData[cameraId]->initMakernote(cameraId, tuningMode);
}

int PlatformData::deinitMakernote(int cameraId, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);
    return getInstance()->mAiqInitData[cameraId]->deinitMakernote(cameraId, tuningMode);
}

int PlatformData::saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                    int64_t sequence, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->saveMakernoteData(cameraId, makernoteMode,
                                                                    sequence, tuningMode);
}

void* PlatformData::getMakernoteBuf(int cameraId, camera_makernote_mode_t makernoteMode,
                                    bool& dump) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->getMakernoteBuf(makernoteMode, dump);
}

void PlatformData::updateMakernoteTimeStamp(int cameraId, int64_t sequence, uint64_t timestamp) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    getInstance()->mAiqInitData[cameraId]->updateMakernoteTimeStamp(sequence, timestamp);
}

void PlatformData::acquireMakernoteData(int cameraId, uint64_t timestamp, Parameters* param) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    getInstance()->mAiqInitData[cameraId]->acquireMakernoteData(timestamp, param);
}

int PlatformData::getScalerInfo(int cameraId, int32_t streamId, float* scalerWidth,
                                float* scalerHeight) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.empty()) {
        *scalerWidth = 1.0;
        *scalerHeight = 1.0;
        return OK;
    }

    for (auto& scalerInfo : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
        LOG2("%s, streamId %d, scalerWidth %f, scalerHeight %f", __func__, scalerInfo.streamId,
             scalerInfo.scalerWidth, scalerInfo.scalerHeight);
        if (scalerInfo.streamId == streamId) {
            *scalerWidth = scalerInfo.scalerWidth;
            *scalerHeight = scalerInfo.scalerHeight;
            break;
        }
    }

    return OK;
}

void PlatformData::setScalerInfo(int cameraId, std::vector<IGraphType::ScalerInfo> scalerInfo) {
    for (auto& scalerInfoInput : scalerInfo) {
        bool flag = false;
        for (auto& scalerInfoTmp : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
            if (scalerInfoInput.streamId == scalerInfoTmp.streamId) {
                scalerInfoTmp.scalerWidth = scalerInfoInput.scalerWidth;
                scalerInfoTmp.scalerHeight = scalerInfoInput.scalerHeight;
                flag = true;
                break;
            }
        }
        if (!flag) {
            getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.push_back(scalerInfoInput);
        }
    }
}

int PlatformData::getVideoStreamNum() {
    return getInstance()->mStaticCfg.mCommonConfig.videoStreamNum;
}

bool PlatformData::supportUpdateTuning(int cameraId) {
    // Check if support UpdateTuning per platform config
    if (getInstance()->mStaticCfg.mCommonConfig.supportIspTuningUpdate) return true;

    // check if support UpdateTuning per sensor config
    return getInstance()->mStaticCfg.mCameras[cameraId].mIspTuningUpdate;
}

bool PlatformData::supportHwJpegEncode() {
    return getInstance()->mStaticCfg.mCommonConfig.supportHwJpegEncode;
}

int PlatformData::getMaxIsysTimeout() {
    return getInstance()->mStaticCfg.mCommonConfig.maxIsysTimeoutValue;
}

bool PlatformData::isUsingGpuAlgo() {
    bool enabled = false;
    for (int cameraId = static_cast<int>(getInstance()->mStaticCfg.mCameras.size()) - 1;
         cameraId >= 0; cameraId--)
        enabled |= isGpuTnrEnabled(cameraId);
    // LEVEL0_ICBM_S
    enabled |= isGPUICBMEnabled();
    // LEVEL0_ICBM_E
    return enabled;
}

bool PlatformData::isStillTnrPrior() {
    return getInstance()->mStaticCfg.mCommonConfig.isStillTnrPrior;
}

bool PlatformData::isTnrParamForceUpdate() {
    return getInstance()->mStaticCfg.mCommonConfig.isTnrParamForceUpdate;
}

int PlatformData::getTnrExtraFrameCount(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mTnrExtraFrameNum;
}

bool PlatformData::useTnrGlobalProtection() {
    return getInstance()->mStaticCfg.mCommonConfig.useTnrGlobalProtection;
}

void PlatformData::setSensorOrientation(int cameraId, int orientation) {
    getInstance()->mStaticCfg.mCameras[cameraId].mSensorOrientation = orientation;
}

int PlatformData::getSensorOrientation(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorOrientation;
}

bool PlatformData::isDummyStillSink(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDummyStillSink;
}

void PlatformData::getTnrThresholdSizes(int cameraId,
                                        std::vector<camera_resolution_t>& resolutions) {
    resolutions = getInstance()->mStaticCfg.mCameras[cameraId].mTnrThresholdSizes;
}

bool PlatformData::isGpuTnrEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mGpuTnrEnabled;
}

bool PlatformData::removeCacheFlushOutputBuffer(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mRemoveCacheFlushOutputBuffer;
}

bool PlatformData::getPLCEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPLCEnable;
}

// PRIVACY_MODE_S
PrivacyModeType PlatformData::getSupportPrivacy(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSupportPrivacy;
}

uint32_t PlatformData::getPrivacyModeThreshold(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPrivacyModeThreshold;
}

uint32_t PlatformData::getPrivacyModeFrameDelay(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPrivacyModeFrameDelay;
}
// PRIVACY_MODE_E

bool PlatformData::isStillOnlyPipeEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mStillOnlyPipe;
}

bool PlatformData::getDisableBLCByAGain(int cameraId, int& low, int& high) {
    low = getInstance()->mStaticCfg.mCameras[cameraId].mDisableBLCAGainLow;
    high = getInstance()->mStaticCfg.mCameras[cameraId].mDisableBLCAGainHigh;
    return getInstance()->mStaticCfg.mCameras[cameraId].mDisableBLCByAGain;
}

bool PlatformData::isResetLinkRoute(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mResetLinkRoute;
}

int64_t PlatformData::getReqWaitTimeout(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mReqWaitTimeout;
}

// LEVEL0_ICBM_S
bool PlatformData::isGPUICBMEnabled() {
    return getInstance()->mStaticCfg.mCommonConfig.isGPUICBMEnabled;
}
// LEVEL0_ICBM_E
}  // namespace icamera
