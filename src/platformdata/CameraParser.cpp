/*
 * Copyright (C) 2015-2024 Intel Corporation
 * Copyright 2008-2017, The Android Open Source Project
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
#define LOG_TAG CameraParser

#include "CameraParser.h"

#include <dirent.h>
#include <expat.h>
#include <string.h>
#include <sys/stat.h>

// CRL_MODULE_S
#include <linux/crlmodule.h>
// CRL_MODULE_E
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "metadata/ParameterHelper.h"

using std::string;
using std::vector;

#include "v4l2/NodeInfo.h"

namespace icamera {
#define LIBCAMHAL_PROFILE_NAME "libcamhal_profile.xml"
CameraParser::CameraParser(MediaControl* mc, PlatformData::StaticCfg* cfg)
        : mStaticCfg(cfg),
          mCurrentDataField(FIELD_INVALID),
          mSensorNum(0),
          mCurrentSensor(0),
          pCurrentCam(nullptr),
          mInMediaCtlCfg(false),
          mInStaticMetadata(false),
          mMC(mc),
          mMetadataCache(nullptr),
          mIsAvailableSensor(false) {
    LOG1("@%s", __func__);
    CheckAndLogError(cfg == nullptr, VOID_VALUE, "@%s, cfg is nullptr", __func__);

    // Get common data from libcamhal_profile.xml
    int ret = getDataFromXmlFile(LIBCAMHAL_PROFILE_NAME);
    CheckAndLogError(ret != OK, VOID_VALUE, "Failed to get libcamhal profile data from %s",
                     LIBCAMHAL_PROFILE_NAME);

    mGenericStaticMetadataToTag = {
        {"ae.lockAvailable", CAMERA_AE_LOCK_AVAILABLE},
        {"awb.lockAvailable", CAMERA_AWB_LOCK_AVAILABLE},
        {"control.availableModes", CAMERA_CONTROL_AVAILABLE_MODES},
        {"control.availableSceneModes", CAMERA_CONTROL_AVAILABLE_SCENE_MODES},
        {"control.maxRegions", CAMERA_CONTROL_MAX_REGIONS},
        {"statistics.info.availableFaceDetectModes",
         CAMERA_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES},
        {"statistics.info.maxFaceCount", CAMERA_STATISTICS_INFO_MAX_FACE_COUNT},
        {"sensor.maxAnalogSensitivity", CAMERA_SENSOR_MAX_ANALOG_SENSITIVITY},
        {"sensor.info.activeArraySize", CAMERA_SENSOR_INFO_ACTIVE_ARRAY_SIZE},
        {"sensor.info.pixelArraySize", CAMERA_SENSOR_INFO_PIXEL_ARRAY_SIZE},
        {"sensor.info.physicalSize", CAMERA_SENSOR_INFO_PHYSICAL_SIZE},
        {"sensor.info.sensitivityRange", CAMERA_SENSOR_INFO_SENSITIVITY_RANGE},
        {"sensor.info.exposureTimeRange", CAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE},
        {"sensor.info.colorFilterArrangement", CAMERA_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT},
        {"sensor.info.whitelevel", CAMERA_SENSOR_INFO_WHITE_LEVEL},
        {"sensor.availableTestPatternModes", CAMERA_SENSOR_AVAILABLE_TEST_PATTERN_MODES},
        {"sensor.orientation", CAMERA_SENSOR_ORIENTATION},
        {"sensor.opaqueRawSize", CAMERA_SENSOR_OPAQUE_RAW_SIZE},
        {"shading.availableModes", CAMERA_SHADING_AVAILABLE_MODES},
        {"lens.facing", CAMERA_LENS_FACING},
        {"lens.info.availableApertures", CAMERA_LENS_INFO_AVAILABLE_APERTURES},
        {"lens.info.availableFilterDensities", CAMERA_LENS_INFO_AVAILABLE_FILTER_DENSITIES},
        {"lens.info.availableFocalLengths", CAMERA_LENS_INFO_AVAILABLE_FOCAL_LENGTHS},
        {"lens.info.availableOpticalStabilization",
         CAMERA_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION},
        {"lens.info.hyperfocalDistance", CAMERA_LENS_INFO_HYPERFOCAL_DISTANCE},
        {"lens.info.minimumFocusDistance", CAMERA_LENS_INFO_MINIMUM_FOCUS_DISTANCE},
        {"lens.info.shadingMapSize", CAMERA_LENS_INFO_SHADING_MAP_SIZE},
        {"lens.info.focusDistanceCalibration", CAMERA_LENS_INFO_FOCUS_DISTANCE_CALIBRATION},
        {"request.maxNumOutputStreams", CAMERA_REQUEST_MAX_NUM_OUTPUT_STREAMS},
        {"request.maxNumInputStreams", CAMERA_REQUEST_MAX_NUM_INPUT_STREAMS},
        {"request.pipelineMaxDepth", CAMERA_REQUEST_PIPELINE_MAX_DEPTH},
        {"request.availableCapabilities", CAMERA_REQUEST_AVAILABLE_CAPABILITIES},
        {"scaler.availableInputOutputFormatsMap", CAMERA_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP},
        {"scaler.availableStreamConfigurations", CAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS},
        {"scaler.availableMinFrameDurations", CAMERA_SCALER_AVAILABLE_MIN_FRAME_DURATIONS},
        {"scaler.availableStallDurations", CAMERA_SCALER_AVAILABLE_STALL_DURATIONS},
        {"scaler.availableMaxDigitalZoom", CAMERA_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM},
        {"reprocess.maxCaptureStall", CAMERA_REPROCESS_MAX_CAPTURE_STALL},
        {"jpeg.maxSize", CAMERA_JPEG_MAX_SIZE},
        {"jpeg.availableThumbnailSizes", CAMERA_JPEG_AVAILABLE_THUMBNAIL_SIZES},
        {"edge.availableEdgeModes", CAMERA_EDGE_AVAILABLE_EDGE_MODES},
        {"hotPixel.availableHotPixelModes", CAMERA_HOT_PIXEL_AVAILABLE_HOT_PIXEL_MODES},
        {"noiseReduction.availableNoiseReductionModes",
         CAMERA_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES},
        {"tonemap.maxCurvePoints", CAMERA_TONEMAP_MAX_CURVE_POINTS},
        {"tonemap.availableToneMapModes", CAMERA_TONEMAP_AVAILABLE_TONE_MAP_MODES},
        {"info.supportedHardwareLevel", CAMERA_INFO_SUPPORTED_HARDWARE_LEVEL},
        {"sync.maxLatency", CAMERA_SYNC_MAX_LATENCY},
    };

    // Get sensor data from sensor xml.
    mMetadataCache = new int64_t[mMetadataCacheSize];
    getSensorDataFromXmlFile();

    dumpSensorInfo();
}

CameraParser::~CameraParser() {
    delete[] mMetadataCache;
}

/**
 * Replacing $I2CBUS with the real mI2CBus if the value contains the string "$I2CBUS"
 * one example: "imx319 $I2CBUS"
 * Replacing $CSI_PORT with the real mCsiPort if the value contains the string "$CSI_PORT"
 * one example: "Intel IPU6 CSI-2 $CSI_PORT" or "Intel IPU6 CSI2 $CSI_PORT"
 *
 * \param profiles: the pointer of the CameraParser.
 * \param value: camera information.
 * \return: if the value contains the string, it will be replaced.
 */
string CameraParser::replaceStringInXml(CameraParser* profiles, const char* value) {
    string valueTmp;
    CheckAndLogError(value == nullptr, valueTmp, "value is nullptr");

    valueTmp = value;
    string::size_type found = string::npos;
    if ((found = valueTmp.find("$I2CBUS")) != string::npos) {
        valueTmp.replace(found, sizeof("$I2CBUS"), profiles->mI2CBus);
    } else if ((found = valueTmp.find("$CSI_PORT")) != string::npos) {
        valueTmp.replace(found, sizeof("$CSI_PORT"), profiles->mCsiPort);
    }

    return valueTmp;
}

/**
 * Get CSI port and I2C bus address according to current sensor name
 *
 * \param profiles: the pointer of the CameraParser.
 */
void CameraParser::getCsiPortAndI2CBus(CameraParser* profiles) {
    std::string fullSensorName = profiles->pCurrentCam->sensorName;
    if (fullSensorName.empty()) {
        LOG1("@%s, Faild to find sensorName", __func__);
        return;
    }

    for (auto availableSensorTmp : profiles->mAvailableSensor) {
        AvailableSensorInfo* sensorInfo = &availableSensorTmp.second;
        if ((availableSensorTmp.first.find(fullSensorName) != string::npos) &&
            (sensorInfo->sensorFlag != true)) {
            /* parameters information format example:
               sinkEntityName is "Intel IPU6 CSI-2 1" or "Intel IPU6 CSI2 1"
               profiles->pCurrentCam->sensorName is "ov8856-wf" or "ov8856"
               sensorName is "ov8856"
            */
            string sinkEntityName = sensorInfo->sinkEntityName;
            sensorInfo->sensorFlag = true;
            profiles->mCsiPort = sinkEntityName.substr(sinkEntityName.find_last_of(' ') + 1);

            string sensorName = fullSensorName;
            if (sensorName.find_first_of('-') != string::npos)
                sensorName = fullSensorName.substr(0, (sensorName.find_first_of('-')));

            if (profiles->mMC) {
                profiles->mMC->getI2CBusAddress(sensorName, sinkEntityName, &profiles->mI2CBus);
            }
            LOG1("@%s, mI2CBus:%s, cisPort:%s", __func__, profiles->mI2CBus.c_str(),
                 profiles->mCsiPort.c_str());
            break;
        }
    }

    return;
}

/**
 * This function will check which field that the parser parses to.
 *
 * The field is set to 3 types.
 * FIELD_INVALID FIELD_SENSOR and FIELD_COMMON
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void CameraParser::checkField(CameraParser* profiles, const char* name, const char** atts) {
    if (strcmp(name, "CameraSettings") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        return;
    } else if (strcmp(name, "Sensor") == 0) {
        // If it already has a available sensor, it doesn't need to parser others
        if (profiles->mIsAvailableSensor) {
            profiles->mCurrentDataField = FIELD_INVALID;
            return;
        }

        profiles->mSensorNum++;
        profiles->mCurrentSensor = profiles->mSensorNum - 1;
        LOG1("@%s, mCurrentSensor %d", __func__, profiles->mCurrentSensor);
        if (profiles->mCurrentSensor >= 0 && profiles->mCurrentSensor < MAX_CAMERA_NUMBER) {
            profiles->pCurrentCam = new PlatformData::StaticCfg::CameraInfo;

            int idx = 0;
            string sensorEntityName;
            string sinkEntityName;
            while (atts[idx]) {
                const char* key = atts[idx];
                const char* val = atts[idx + 1];
                LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1,
                     val);
                if (strcmp(key, "name") == 0) {
                    profiles->pCurrentCam->sensorName = val;
                } else if (strcmp(key, "description") == 0) {
                    profiles->pCurrentCam->sensorDescription = val;
                    // VIRTUAL_CHANNEL_S
                } else if (strcmp(key, "virtualChannel") == 0) {
                    profiles->pCurrentCam->mVirtualChannel =
                        strcmp(val, "true") == 0 ? true : false;
                } else if (strcmp(key, "vcNum") == 0) {
                    profiles->pCurrentCam->mVCNum = strtoul(val, nullptr, 10);
                } else if (strcmp(key, "vcSeq") == 0) {
                    profiles->pCurrentCam->mVCSeq = strtoul(val, nullptr, 10);
                } else if (strcmp(key, "vcGroupId") == 0) {
                    profiles->pCurrentCam->mVCGroupId = strtoul(val, nullptr, 10);
                    // VIRTUAL_CHANNEL_E
                }
                idx += 2;
            }

            getCsiPortAndI2CBus(profiles);
            profiles->mMetadata.clear();
            profiles->mCurrentDataField = FIELD_SENSOR;

            return;
        }
    } else if (strcmp(name, "Common") == 0) {
        profiles->mCurrentDataField = FIELD_COMMON;
        return;
    }

    LOGE("@%s, name:%s, atts[0]:%s, xml format wrong", __func__, name, atts[0]);
    return;
}

/**
 * This function will handle all the common related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void CameraParser::handleCommon(CameraParser* profiles, const char* name, const char** atts) {
    CheckAndLogError(strcmp(atts[0], "value") != 0 || (atts[1] == nullptr), VOID_VALUE,
                     "@%s, name:%s, atts[0]:%s or atts[1] is nullptr, xml format wrong", __func__,
                     name, atts[0]);

    LOG2("@%s, name:%s, atts[0]:%s, atts[1]: %s", __func__, name, atts[0], atts[1]);
    CommonConfig* cfg = &profiles->mStaticCfg->mCommonConfig;
    if (strcmp(name, "version") == 0) {
        cfg->xmlVersion = atof(atts[1]);
    } else if (strcmp(name, "platform") == 0) {
        cfg->ipuName = atts[1];
    } else if (strcmp(name, "availableSensors") == 0) {
        parseXmlConvertStrings(atts[1], cfg->availableSensors, convertCharToString);
    } else if (strcmp(name, "cameraNumber") == 0) {
        cfg->cameraNumber = atoi(atts[1]);
    } else if (strcmp(name, "stillTnrPrior") == 0) {
        cfg->isStillTnrPrior = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "tnrParamForceUpdate") == 0) {
        cfg->isTnrParamForceUpdate = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "tnrGlobalProtection") == 0) {
        cfg->useTnrGlobalProtection = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "videoStreamNum") == 0) {
        int val = atoi(atts[1]);
        cfg->videoStreamNum = val > 0 ? val : DEFAULT_VIDEO_STREAM_NUM;
    } else if (strcmp(name, "supportIspTuningUpdate") == 0) {
        cfg->supportIspTuningUpdate = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "supportHwJpegEncode") == 0) {
        cfg->supportHwJpegEncode = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "maxIsysTimeoutValue") == 0) {
        cfg->maxIsysTimeoutValue = atoi(atts[1]);
    // LEVEL0_ICBM_S
    } else if (strcmp(name, "useGPUICBM") == 0) {
        cfg->isGPUICBMEnabled = strcmp(atts[1], "true") == 0;
    // LEVEL0_ICBM_E
    }
}

/**
 * This function will handle all the sensor related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void CameraParser::handleSensor(CameraParser* profiles, const char* name, const char** atts) {
    CheckAndLogError(strcmp(atts[0], "value") != 0 || (atts[1] == nullptr), VOID_VALUE,
                     "@%s, name:%s, atts[0]:%s or atts[1] is nullptr, xml format wrong", __func__,
                     name, atts[0]);

    LOG2("@%s, name:%s, atts[0]:%s, atts[1]:%s", __func__, name, atts[0], atts[1]);
    if (strcmp(name, "supportedISysSizes") == 0) {
        parseSizesList(atts[1], pCurrentCam->mSupportedISysSizes);
        for (const auto& s : pCurrentCam->mSupportedISysSizes)
            LOG2("@%s, mSupportedISysSizes: width:%d, height:%d", __func__, s.width, s.height);
    } else if (strcmp(name, "supportedISysFormat") == 0) {
        getSupportedFormat(atts[1], pCurrentCam->mSupportedISysFormat);
    } else if (strcmp(name, "iSysRawFormat") == 0) {
        pCurrentCam->mISysRawFormat = CameraUtils::string2PixelCode(atts[1]);
    } else if (strcmp(name, "preferredOutput") == 0) {
        parseSizesList(atts[1], pCurrentCam->mPreferOutput);
    } else if (strcmp(name, "configModeToStreamId") == 0) {
        char* srcDup = strdup(atts[1]);
        CheckAndLogError(!srcDup, VOID_VALUE, "Create a copy of source string failed.");

        char* endPtr = strchr(srcDup, ',');
        if (endPtr) {
            *endPtr = 0;
            ConfigMode configMode = CameraUtils::getConfigModeByName(srcDup);
            int streamId = atoi(endPtr + 1);
            pCurrentCam->mConfigModeToStreamId[configMode] = streamId;
        }
        free(srcDup);
    } else if (strcmp(name, "pSysFormat") == 0) {
        getSupportedFormat(atts[1], pCurrentCam->mPSysFormat);
    } else if (strcmp(name, "enableAIQ") == 0) {
        pCurrentCam->mEnableAIQ = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "enableMkn") == 0) {
        pCurrentCam->mEnableMkn = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "ispTuningUpdate") == 0) {
        pCurrentCam->mIspTuningUpdate = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "AiqRunningInterval") == 0) {
        pCurrentCam->mAiqRunningInterval = atoi(atts[1]);
    } else if (strcmp(name, "AlgoRunningRate") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        int algo = IMAGING_ALGO_NONE;
        char* savePtr = nullptr;

        char* tablePtr = strtok_r(src, ",", &savePtr);
        float awbRunningRate = 0.0;
        float aeRunningRate = 0.0;
        while (tablePtr) {
            if (strcmp(tablePtr, "AE") == 0) {
                algo = IMAGING_ALGO_AE;
            } else if (strcmp(tablePtr, "AWB") == 0) {
                algo = IMAGING_ALGO_AWB;
            } else if (strcmp(tablePtr, "AF") == 0) {
                algo = IMAGING_ALGO_AF;
            } else {
                LOGE("The wrong algo type %s", tablePtr);
                return;
            }

            tablePtr = strtok_r(nullptr, ",", &savePtr);
            CheckAndLogError(!tablePtr, VOID_VALUE, "the run rate of algo %d is nullptr", algo);

            if (algo == IMAGING_ALGO_AE) aeRunningRate = atof(tablePtr);
            if (algo == IMAGING_ALGO_AWB) awbRunningRate = atof(tablePtr);

            pCurrentCam->mAlgoRunningRateMap[algo] = atof(tablePtr);
            tablePtr = strtok_r(nullptr, ",", &savePtr);
        }

        if (awbRunningRate > EPSILON && aeRunningRate > EPSILON &&
            fabs(awbRunningRate - aeRunningRate) < EPSILON) {
            // if same runnning rate of AE and AWB, stats running rate is supported
            pCurrentCam->mStatsRunningRate = true;
        }
    } else if (strcmp(name, "disableHDRnetBoards") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr = nullptr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        while (tablePtr) {
            pCurrentCam->mDisableHDRnetBoards.push_back(tablePtr);
            tablePtr = strtok_r(nullptr, ",", &savePtr);
        }
    } else if (strcmp(name, "useCrlModule") == 0) {
        pCurrentCam->mUseCrlModule = strcmp(atts[1], "true") == 0;
        // DOL_FEATURE_S
    } else if (strcmp(name, "dolVbpOffset") == 0) {
        parseXmlConvertStrings(atts[1], pCurrentCam->mDolVbpOffset, atoi);
        // DOL_FEATURE_E
    } else if (strcmp(name, "skipFrameV4L2Error") == 0) {
        pCurrentCam->mSkipFrameV4L2Error = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "useSensorDigitalGain") == 0) {
        pCurrentCam->mUseSensorDigitalGain = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "useIspDigitalGain") == 0) {
        pCurrentCam->mUseIspDigitalGain = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "preRegisterBuffer") == 0) {
        pCurrentCam->mNeedPreRegisterBuffers = strcmp(atts[1], "true") == 0;
        // FRAME_SYNC_S
    } else if (strcmp(name, "enableFrameSyncCheck") == 0) {
        pCurrentCam->mFrameSyncCheckEnabled = strcmp(atts[1], "true") == 0;
        // FRAME_SYNC_E
    } else if (strcmp(name, "lensName") == 0) {
        string vcmName = atts[1];
        if (!profiles->mI2CBus.empty()) {
            int i2cBusId = atoi(profiles->mI2CBus.c_str());
            vcmName.append(" ");
            vcmName.append(std::to_string(i2cBusId));
        }
        if (profiles->mMC) {
            profiles->mMC->getVCMI2CAddr(vcmName.c_str(), &pCurrentCam->mLensName);
        }
    } else if (strcmp(name, "lensHwType") == 0) {
        if (strcmp(atts[1], "LENS_VCM_HW") == 0) {
            pCurrentCam->mLensHwType = LENS_VCM_HW;
        } else {
            LOGE("unknown Lens HW type %s, set to LENS_NONE_HW", atts[1]);
            pCurrentCam->mLensHwType = LENS_NONE_HW;
        }
    } else if (strcmp(name, "tuningModeToSensitivityMap") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';

        char* savePtr = nullptr;
        char* tuningMode = strtok_r(src, ",", &savePtr);
        while (tuningMode) {
            SensitivityRange range;
            char* min = strtok_r(nullptr, ",", &savePtr);
            char* max = strtok_r(nullptr, ",", &savePtr);
            CheckAndLogError(!tuningMode || !min || !max, VOID_VALUE, "Wrong sensitivity map");

            TuningMode mode = CameraUtils::string2TuningMode(tuningMode);
            range.min = atoi(min);
            range.max = atoi(max);
            pCurrentCam->mTuningModeToSensitivityMap[mode] = range;

            tuningMode = strtok_r(nullptr, ",", &savePtr);
        }
    } else if (strcmp(name, "sensorMode") == 0) {
        if (strcmp(atts[1], "binning") == 0) {
            pCurrentCam->mSensorMode = SENSOR_MODE_BINNING;
        } else if (strcmp(atts[1], "full") == 0) {
            pCurrentCam->mSensorMode = SENSOR_MODE_FULL;
        }
    } else if (strcmp(name, "enablePdaf") == 0) {
        pCurrentCam->mEnablePdaf = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "sensorAwb") == 0) {
        pCurrentCam->mSensorAwb = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "sensorAe") == 0) {
        pCurrentCam->mSensorAe = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "runIspAlways") == 0) {
        pCurrentCam->mRunIspAlways = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "lensCloseCode") == 0) {
        pCurrentCam->mLensCloseCode = atoi(atts[1]);
    } else if (strcmp(name, "cITMaxMargin") == 0) {
        pCurrentCam->mCITMaxMargin = atoi(atts[1]);
    } else if (strcmp(name, "ltmGainLag") == 0) {
        pCurrentCam->mLtmGainLag = atoi(atts[1]);
    } else if (strcmp(name, "enableLtmThread") == 0) {
        pCurrentCam->mEnableLtmThread = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "enableLtmDefog") == 0) {
        pCurrentCam->mEnableLtmDefog = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "enableLtm") == 0) {
        pCurrentCam->mLtmEnabled = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "maxSensorDg") == 0) {
        pCurrentCam->mMaxSensorDigitalGain = atoi(atts[1]);
    } else if (strcmp(name, "sensorDgType") == 0) {
        if (strcmp(atts[1], "type_2_x") == 0) {
            pCurrentCam->mSensorDgType = SENSOR_DG_TYPE_2_X;
        } else if (strcmp(atts[1], "type_x") == 0) {
            pCurrentCam->mSensorDgType = SENSOR_DG_TYPE_X;
        } else {
            LOGE("unknown sensor digital gain type:%s, set to SENSOR_DG_TYPE_NONE", atts[1]);
            pCurrentCam->mSensorDgType = SENSOR_DG_TYPE_NONE;
        }
    } else if (strcmp(name, "digitalGainLag") == 0) {
        pCurrentCam->mDigitalGainLag = atoi(atts[1]);
    } else if (strcmp(name, "exposureLag") == 0) {
        pCurrentCam->mExposureLag = atoi(atts[1]);
        // HDR_FEATURE_S
    } else if (strcmp(name, "hdrExposureType") == 0) {
        if (strcmp(atts[1], "fix-exposure-ratio") == 0) {
            pCurrentCam->mSensorExposureType = SENSOR_FIX_EXPOSURE_RATIO;
        } else if (strcmp(atts[1], "relative-multi-exposures") == 0) {
            pCurrentCam->mSensorExposureType = SENSOR_RELATIVE_MULTI_EXPOSURES;
        } else if (strcmp(atts[1], "multi-exposures") == 0) {
            pCurrentCam->mSensorExposureType = SENSOR_MULTI_EXPOSURES;
        } else if (strcmp(atts[1], "dual-exposures-dcg-and-vs") == 0) {
            pCurrentCam->mSensorExposureType = SENSOR_DUAL_EXPOSURES_DCG_AND_VS;
        } else {
            LOGE("unknown SENSOR exposure type %s, set to SENSOR_EXPOSURE_SINGLE", atts[1]);
            pCurrentCam->mSensorExposureType = SENSOR_EXPOSURE_SINGLE;
        }
    } else if (strcmp(name, "hdrExposureNum") == 0) {
        pCurrentCam->mSensorExposureNum = atoi(atts[1]);
    } else if (strcmp(name, "hdrStatsInputBitDepth") == 0) {
        pCurrentCam->mHdrStatsInputBitDepth = atoi(atts[1]);
    } else if (strcmp(name, "hdrStatsOutputBitDepth") == 0) {
        pCurrentCam->mHdrStatsOutputBitDepth = atoi(atts[1]);
    } else if (strcmp(name, "useFixedHdrExposureInfo") == 0) {
        pCurrentCam->mUseFixedHdrExposureInfo = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "hdrGainType") == 0) {
        if (strcmp(atts[1], "multi-dg-and-convertion-ag") == 0) {
            pCurrentCam->mSensorGainType = SENSOR_MULTI_DG_AND_CONVERTION_AG;
        } else if (strcmp(atts[1], "isp-dg-and-sensor-direct-ag") == 0) {
            pCurrentCam->mSensorGainType = ISP_DG_AND_SENSOR_DIRECT_AG;
        } else if (strcmp(atts[1], "multi-dg-and-direct-ag") == 0) {
            pCurrentCam->mSensorGainType = SENSOR_MULTI_DG_AND_DIRECT_AG;
        } else {
            LOGE("unknown sensor gain type %s, set to SENSOR_GAIN_NONE", atts[1]);
            pCurrentCam->mSensorGainType = SENSOR_GAIN_NONE;
        }
        // HDR_FEATURE_E
    } else if (strcmp(name, "graphSettingsFile") == 0) {
        pCurrentCam->mGraphSettingsFile = atts[1];
    } else if (strcmp(name, "graphSettingsType") == 0) {
        if (strcmp(atts[1], "coupled") == 0) {
            pCurrentCam->mGraphSettingsType = COUPLED;
        } else if (strcmp(atts[1], "dispersed") == 0) {
            pCurrentCam->mGraphSettingsType = DISPERSED;
        } else {
            LOGW("unknown graph settings type %s, set to COUPLED", atts[1]);
            pCurrentCam->mGraphSettingsType = COUPLED;
        }
    } else if (strcmp(name, "gainLag") == 0) {
        pCurrentCam->mAnalogGainLag = atoi(atts[1]);
    } else if (strcmp(name, "customAicLibraryName") == 0) {
        pCurrentCam->mCustomAicLibraryName = atts[1];
    } else if (strcmp(name, "custom3ALibraryName") == 0) {
        pCurrentCam->mCustom3ALibraryName = atts[1];
    } else if (strcmp(name, "yuvColorRangeMode") == 0) {
        if (strcmp(atts[1], "full") == 0) {
            pCurrentCam->mYuvColorRangeMode = CAMERA_FULL_MODE_YUV_COLOR_RANGE;
        } else if (strcmp(atts[1], "reduced") == 0) {
            pCurrentCam->mYuvColorRangeMode = CAMERA_REDUCED_MODE_YUV_COLOR_RANGE;
        }
    } else if (strcmp(name, "initialSkipFrame") == 0) {
        pCurrentCam->mInitialSkipFrame = atoi(atts[1]);
    } else if (strcmp(name, "initialPendingFrame") == 0) {
        pCurrentCam->mInitialPendingFrame = atoi(atts[1]);
    } else if (strcmp(name, "maxRawDataNum") == 0) {
        pCurrentCam->mMaxRawDataNum = atoi(atts[1]);
    } else if (strcmp(name, "topBottomReverse") == 0) {
        pCurrentCam->mTopBottomReverse = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "maxRequestsInflight") == 0) {
        pCurrentCam->mMaxRequestsInflight = atoi(atts[1]);
    } else if (strcmp(name, "psysContinueStats") == 0) {
        pCurrentCam->mPsysContinueStats = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "preferredBufQSize") == 0) {
        pCurrentCam->mPreferredBufQSize = atoi(atts[1]);
    } else if (strcmp(name, "supportedTuningConfig") == 0) {
        parseSupportedTuningConfig(atts[1], pCurrentCam->mSupportedTuningConfig);
    } else if (strcmp(name, "enableAiqd") == 0) {
        pCurrentCam->mEnableAiqd = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "testPatternMap") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        int32_t mode = TEST_PATTERN_OFF;
        char* savePtr = nullptr;

        char* tablePtr = strtok_r(src, ",", &savePtr);
        while (tablePtr) {
            if (strcmp(tablePtr, "Off") == 0) {
                mode = TEST_PATTERN_OFF;
            } else if (strcmp(tablePtr, "ColorBars") == 0) {
                mode = COLOR_BARS;
            } else if (strcmp(tablePtr, "SolidColor") == 0) {
                mode = SOLID_COLOR;
            } else if (strcmp(tablePtr, "ColorBarsFadeToGray") == 0) {
                mode = COLOR_BARS_FADE_TO_GRAY;
            } else if (strcmp(tablePtr, "PN9") == 0) {
                mode = PN9;
            } else if (strcmp(tablePtr, "CUSTOM1") == 0) {
                mode = TEST_PATTERN_CUSTOM1;
            } else {
                LOGE("Test pattern string %s is unknown, please check", tablePtr);
                return;
            }

            tablePtr = strtok_r(nullptr, ",", &savePtr);
            CheckAndLogError(tablePtr == nullptr, VOID_VALUE, "Driver test pattern is nullptr");

            pCurrentCam->mTestPatternMap[mode] = atoi(tablePtr);

            tablePtr = strtok_r(nullptr, ",", &savePtr);
        }
    } else if (strcmp(name, "lardTags") == 0) {
        parseLardTags(atts[1], pCurrentCam->mLardTagsConfig);
    } else if (strcmp(name, "availableConfigModeForAuto") == 0) {
        parseXmlConvertStrings(atts[1], pCurrentCam->mConfigModesForAuto,
                               CameraUtils::getConfigModeByName);
    } else if (strcmp(name, "supportedAeMultiExpRange") == 0) {
        parseMultiExpRange(atts[1]);
    } else if (strcmp(name, "dvsType") == 0) {
        if (strcmp(atts[1], "MORPH_TABLE") == 0) {
            pCurrentCam->mDVSType = MORPH_TABLE;
        } else if (strcmp(atts[1], "IMG_TRANS") == 0) {
            pCurrentCam->mDVSType = IMG_TRANS;
        }
    } else if (strcmp(name, "pslOutputMapForRotation") == 0) {
        parseOutputMap(atts[1], pCurrentCam->mOutputMap);
    } else if (strcmp(name, "nvmOverwrittenFile") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr = nullptr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        if (tablePtr) profiles->pCurrentCam->mNvmOverwrittenFile = tablePtr;
        tablePtr = strtok_r(nullptr, ",", &savePtr);
        if (tablePtr) profiles->pCurrentCam->mNvmOverwrittenFileSize = atoi(tablePtr);
    } else if (strcmp(name, "nvmDeviceInfo") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr = nullptr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        struct NvmDeviceInfo info;
        while (tablePtr) {
            info.nodeName = tablePtr;

            tablePtr = strtok_r(nullptr, ",", &savePtr);
            CheckAndLogError(tablePtr == nullptr, VOID_VALUE, "failed to parse nvmDeviceInfo");
            info.dataSize = atoi(tablePtr);

            mNvmDeviceInfo.push_back(info);

            tablePtr = strtok_r(nullptr, ",", &savePtr);
        }
    } else if (strcmp(name, "supportModuleNames") == 0) {
        int sz = strlen(atts[1]);
        char src[sz + 1];
        MEMCPY_S(src, sz, atts[1], sz);
        src[sz] = '\0';
        char* savePtr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        while (tablePtr) {
            pCurrentCam->mSupportModuleNames.push_back(tablePtr);
            tablePtr = strtok_r(nullptr, ",", &savePtr);
        }
    } else if (strcmp(name, "isISYSCompression") == 0) {
        pCurrentCam->mISYSCompression = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "isPSACompression") == 0) {
        pCurrentCam->mPSACompression = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "isOFSCompression") == 0) {
        pCurrentCam->mOFSCompression = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "schedulerEnabled") == 0) {
        pCurrentCam->mSchedulerEnabled = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "faceAeEnabled") == 0) {
        pCurrentCam->mFaceAeEnabled = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "psysAlignWithSof") == 0) {
        pCurrentCam->mPsysAlignWithSof = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "psysBundleWithAic") == 0) {
        pCurrentCam->mPsysBundleWithAic = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "swProcessingAlignWithIsp") == 0) {
        pCurrentCam->mSwProcessingAlignWithIsp = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "faceEngineVendor") == 0) {
        int val = atoi(atts[1]);
        pCurrentCam->mFaceEngineVendor = val >= 0 ? val : FACE_ENGINE_INTEL_PVL;
    } else if (strcmp(name, "faceEngineRunningInterval") == 0) {
        int val = atoi(atts[1]);
        pCurrentCam->mFaceEngineRunningInterval =
            val > 0 ? val : FACE_ENGINE_DEFAULT_RUNNING_INTERVAL;
    } else if (strcmp(name, "faceEngineRunningIntervalNoFace") == 0) {
        int val = atoi(atts[1]);
        pCurrentCam->mFaceEngineRunningIntervalNoFace =
            val > 0 ? val : FACE_ENGINE_DEFAULT_RUNNING_INTERVAL;
    } else if (strcmp(name, "faceEngineRunningSync") == 0) {
        pCurrentCam->mFaceEngineRunningSync = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "faceEngineByIPU") == 0) {
        pCurrentCam->mFaceEngineByIPU = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "maxFaceDetectionNumber") == 0) {
        int val = atoi(atts[1]);
        pCurrentCam->mMaxFaceDetectionNumber =
            val > 0 ? std::min(val, MAX_FACES_DETECTABLE) : MAX_FACES_DETECTABLE;
    } else if (strcmp(name, "tnrExtraFrameNum") == 0) {
        int val = atoi(atts[1]);
        pCurrentCam->mTnrExtraFrameNum = val > 0 ? val : 0;
    } else if (strcmp(name, "dummyStillSink") == 0) {
        pCurrentCam->mDummyStillSink = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "tnrThresholdSizes") == 0) {
        parseSizesList(atts[1], pCurrentCam->mTnrThresholdSizes);
        for (const auto& s : pCurrentCam->mTnrThresholdSizes)
            LOG2("@%s, mTnrThresholdSizes: width:%d, height:%d", __func__, s.width, s.height);
    } else if (strcmp(name, "useGpuTnr") == 0) {
        pCurrentCam->mGpuTnrEnabled = strcmp(atts[1], "true") == 0;
    } else if (!strcmp(name, "removeCacheFlushOutputBuffer")) {
        pCurrentCam->mRemoveCacheFlushOutputBuffer = strcmp(atts[1], "true") == 0;
    } else if (!strcmp(name, "isPLCEnable")) {
        pCurrentCam->mPLCEnable = strcmp(atts[1], "true") == 0;
    // PRIVACY_MODE_S
    } else if (strcmp(name, "supportPrivacy") == 0) {
        int val = atoi(atts[1]);
        if (val > 0 && val <= 2) {
            pCurrentCam->mSupportPrivacy = static_cast<PrivacyModeType>(val);
        }
    } else if (strcmp(name, "privacyModeThreshold") == 0) {
        int val = atoi(atts[1]);
        if (val > 0 && val < 255) {
            pCurrentCam->mPrivacyModeThreshold = val;
        }
    } else if (strcmp(name, "privacyModeFrameDelay") == 0) {
        int val = atoi(atts[1]);
        if (val >= 0) {
            pCurrentCam->mPrivacyModeFrameDelay = val;
        }
    // PRIVACY_MODE_E
    } else if (strcmp(name, "stillOnlyPipe") == 0) {
        pCurrentCam->mStillOnlyPipe = strcmp(atts[1], "true") == 0;
    // VIRTUAL_CHANNEL_S
    } else if (strcmp(name, "vcAggregator") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr = nullptr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        if (tablePtr) pCurrentCam->mVcAggregator.mName = tablePtr;
        tablePtr = strtok_r(nullptr, ",", &savePtr);
        if (tablePtr) pCurrentCam->mVcAggregator.mIndex = atoi(tablePtr);
    // VIRTUAL_CHANNEL_E
    } else if (strcmp(name, "disableBLCByAGain") == 0) {
        int size = strlen(atts[1]);
        char src[size + 1];
        MEMCPY_S(src, size, atts[1], size);
        src[size] = '\0';
        char* savePtr = nullptr;
        char* tablePtr = strtok_r(src, ",", &savePtr);
        if (tablePtr) profiles->pCurrentCam->mDisableBLCAGainLow = atoi(tablePtr);
        tablePtr = strtok_r(nullptr, ",", &savePtr);
        if (tablePtr) profiles->pCurrentCam->mDisableBLCAGainHigh = atoi(tablePtr);
        profiles->pCurrentCam->mDisableBLCByAGain = true;
    } else if (strcmp(name, "resetLinkRoute") == 0) {
        pCurrentCam->mResetLinkRoute = strcmp(atts[1], "true") == 0;
    } else if (strcmp(name, "reqWaitTimeoutNs") == 0) {
        int64_t val = atoi(atts[1]);
        pCurrentCam->mReqWaitTimeout = (val >= 0) ? val : 0;
    }
}

int CameraParser::parseSupportedTuningConfig(const char* str, vector<TuningConfig>& config) {
    CheckAndLogError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    char* savePtr;
    char* configMode = strtok_r(src, ",", &savePtr);
    TuningConfig cfg;
    while (configMode) {
        char* tuningMode = strtok_r(nullptr, ",", &savePtr);
        char* aiqb = strtok_r(nullptr, ",", &savePtr);
        CheckAndLogError(configMode == nullptr || tuningMode == nullptr || aiqb == nullptr, -1,
                         "@%s, wrong str %s", __func__, str);

        LOG2("@%s, configMode %s, tuningMode %s, aiqb name %s", __func__, configMode, tuningMode,
             aiqb);
        cfg.configMode = CameraUtils::getConfigModeByName(configMode);
        cfg.tuningMode = CameraUtils::string2TuningMode(tuningMode);
        cfg.aiqbName = aiqb;
        config.push_back(cfg);
        if (savePtr != nullptr) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        configMode = strtok_r(nullptr, ",", &savePtr);
    }
    return 0;
}

int CameraParser::parseLardTags(const char* str, vector<LardTagConfig>& lardTags) {
    CheckAndLogError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';

    char* savePtr;
    char* tuningMode = strtok_r(src, ",", &savePtr);
    LardTagConfig cfg;
    while (tuningMode) {
        char* cmcTag = strtok_r(nullptr, ",", &savePtr);
        char* aiqTag = strtok_r(nullptr, ",", &savePtr);
        char* ispTag = strtok_r(nullptr, ",", &savePtr);
        char* othersTag = strtok_r(nullptr, ",", &savePtr);

        cfg.tuningMode = CameraUtils::string2TuningMode(tuningMode);
        cfg.cmcTag = CameraUtils::fourcc2UL(cmcTag);
        cfg.aiqTag = CameraUtils::fourcc2UL(aiqTag);
        cfg.ispTag = CameraUtils::fourcc2UL(ispTag);
        cfg.othersTag = CameraUtils::fourcc2UL(othersTag);
        CheckAndLogError(
            cfg.cmcTag == 0 || cfg.aiqTag == 0 || cfg.ispTag == 0 || cfg.othersTag == 0, -1,
            "@%s, wrong str %s", __func__, str);

        lardTags.push_back(cfg);
        LOG2("@%s, tuningMode %s, cmc %s, aiq %s, isp %s, others %s", __func__, tuningMode, cmcTag,
             aiqTag, ispTag, othersTag);

        if (savePtr != nullptr) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        tuningMode = strtok_r(nullptr, ",", &savePtr);
    }

    return 0;
}

void CameraParser::parseMediaCtlConfigElement(CameraParser* profiles, const char* name,
                                              const char** atts) {
    MediaCtlConf mc;
    int idx = 0;

    while (atts[idx]) {
        const char* key = atts[idx];
        LOG2("%s: name: %s, value: %s", __func__, atts[idx], atts[idx + 1]);
        if (strcmp(key, "id") == 0) {
            mc.mcId = strtol(atts[idx + 1], nullptr, 10);
        } else if (strcmp(key, "ConfigMode") == 0) {
            parseXmlConvertStrings(atts[idx + 1], mc.configMode, CameraUtils::getConfigModeByName);
        } else if (strcmp(key, "outputWidth") == 0) {
            mc.outputWidth = strtoul(atts[idx + 1], nullptr, 10);
        } else if (strcmp(key, "outputHeight") == 0) {
            mc.outputHeight = strtoul(atts[idx + 1], nullptr, 10);
        } else if (strcmp(key, "format") == 0) {
            mc.format = CameraUtils::string2PixelCode(atts[idx + 1]);
            // DOL_FEATURE_S
        } else if (strcmp(key, "vbp") == 0) {
            mc.vbp = strtoul(atts[idx + 1], nullptr, 10);
            // DOL_FEATURE_E
        }
        idx += 2;
    }

    LOG2("@%s, name:%s, atts[0]:%s, id: %d", __func__, name, atts[0], mc.mcId);
    // Add a new empty MediaControl Configuration
    profiles->pCurrentCam->mMediaCtlConfs.push_back(mc);
}

#define V4L2_CID_WATERMARK 0x00982901
#define V4L2_CID_WATERMARK2 0x00982902
void CameraParser::parseControlElement(CameraParser* profiles, const char* name,
                                       const char** atts) {
    McCtl ctl;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "name") == 0) {
            ctl.entityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                ctl.entity = profiles->mMC->getEntityIdByName(ctl.entityName.c_str());
            }
        } else if (strcmp(key, "ctrlId") == 0) {
            if (!strcmp(val, "V4L2_CID_LINK_FREQ")) {
                ctl.ctlCmd = V4L2_CID_LINK_FREQ;
            } else if (!strcmp(val, "V4L2_CID_VBLANK")) {
                ctl.ctlCmd = V4L2_CID_VBLANK;
            } else if (!strcmp(val, "V4L2_CID_HBLANK")) {
                ctl.ctlCmd = V4L2_CID_HBLANK;
            } else if (!strcmp(val, "V4L2_CID_EXPOSURE")) {
                ctl.ctlCmd = V4L2_CID_EXPOSURE;
            } else if (!strcmp(val, "V4L2_CID_ANALOGUE_GAIN")) {
                ctl.ctlCmd = V4L2_CID_ANALOGUE_GAIN;
            } else if (!strcmp(val, "V4L2_CID_HFLIP")) {
                ctl.ctlCmd = V4L2_CID_HFLIP;
            } else if (!strcmp(val, "V4L2_CID_VFLIP")) {
                ctl.ctlCmd = V4L2_CID_VFLIP;
            } else if (!strcmp(val, "V4L2_CID_WATERMARK")) {
                ctl.ctlCmd = V4L2_CID_WATERMARK;
            } else if (!strcmp(val, "V4L2_CID_WATERMARK2")) {
                ctl.ctlCmd = V4L2_CID_WATERMARK2;
            } else if (!strcmp(val, "V4L2_CID_TEST_PATTERN")) {
                ctl.ctlCmd = V4L2_CID_TEST_PATTERN;
#ifdef LINUX_BUILD
            } else if (!strcmp(val, "V4L2_CID_MIPI_LANES")) {
                ctl.ctlCmd = V4L2_CID_MIPI_LANES;
#endif
                // HDR_FEATURE_S
            } else if (!strcmp(val, "V4L2_CID_WDR_MODE")) {
                ctl.ctlCmd = V4L2_CID_WDR_MODE;
                // HDR_FEATURE_E
                // CRL_MODULE_S
            } else if (!strcmp(val, "V4L2_CID_LINE_LENGTH_PIXELS")) {
                ctl.ctlCmd = V4L2_CID_LINE_LENGTH_PIXELS;
            } else if (!strcmp(val, "V4L2_CID_FRAME_LENGTH_LINES")) {
                ctl.ctlCmd = V4L2_CID_FRAME_LENGTH_LINES;
            } else if (!strcmp(val, "CRL_CID_SENSOR_MODE")) {
                ctl.ctlCmd = CRL_CID_SENSOR_MODE;
            } else if (!strcmp(val, "CRL_CID_EXPOSURE_MODE")) {
                ctl.ctlCmd = CRL_CID_EXPOSURE_MODE;
            } else if (!strcmp(val, "CRL_CID_EXPOSURE_HDR_RATIO")) {
                ctl.ctlCmd = CRL_CID_EXPOSURE_HDR_RATIO;
                // CRL_MODULE_E
            } else {
                LOGE("Unknow ioctl command %s", val);
                ctl.ctlCmd = -1;
            }
        } else if (strcmp(key, "value") == 0) {
            ctl.ctlValue = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "ctrlName") == 0) {
            ctl.ctlName = val;
        }
        idx += 2;
    }

    mc.ctls.push_back(ctl);
}

void CameraParser::parseSelectionElement(CameraParser* profiles, const char* name,
                                         const char** atts) {
    McFormat sel;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();

    sel.top = -1;    // top is not specified, need to be calc later.
    sel.left = -1;   // left is not specified, need to be calc later.
    sel.width = 0;   // width is not specified, need to be calc later.
    sel.height = 0;  // height is not specified, need to be calc later.
    sel.formatType = FC_SELECTION;

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "name") == 0) {
            sel.entityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                sel.entity = profiles->mMC->getEntityIdByName(sel.entityName.c_str());
            }
        } else if (strcmp(key, "pad") == 0) {
            sel.pad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "target") == 0) {
            if (!strcmp(val, "V4L2_SEL_TGT_COMPOSE")) {
                sel.selCmd = V4L2_SEL_TGT_COMPOSE;
            } else if (!strcmp(val, "V4L2_SEL_TGT_CROP")) {
                sel.selCmd = V4L2_SEL_TGT_CROP;
            }
        } else if (strcmp(key, "top") == 0) {
            sel.top = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "left") == 0) {
            sel.left = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "width") == 0) {
            sel.width = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "height") == 0) {
            sel.height = strtoul(val, nullptr, 10);
        }
        idx += 2;
    }

    mc.formats.push_back(sel);
}

/**
 * Store the MediaCtlConf mapping table for supportedStreamConfig by id.
 * Then we can select the MediaCtlConf through this table and configured stream.
 */
void CameraParser::storeMcMappForConfig(int mcId, stream_t streamCfg) {
    // We need to insert new one if mcId isn't in mStreamToMcMap.
    if (pCurrentCam->mStreamToMcMap.find(mcId) == pCurrentCam->mStreamToMcMap.end()) {
        pCurrentCam->mStreamToMcMap.insert(std::pair<int, stream_array_t>(mcId, stream_array_t()));
    }

    stream_array_t& streamVector = pCurrentCam->mStreamToMcMap[mcId];
    streamVector.push_back(streamCfg);
}

/**
 * \brief Parses the string with the supported stream configurations
 * a stream configuration is made of 4 necessary elements
 * - Format
 * - Resolution
 * - Field (Interlaced field)
 * - Media config ID
 * we parse the string in 4 steps
 * example of valid stream configuration is: V4L2_PIX_FMT_NV12,1920x1080,0,0

 * the following elements are optional:
 * - Max fps, for continuous streaming and high quality capture. (optional)
 * example: V4L2_PIX_FMT_NV12,1920x1080,0,0,(30/15)
 *
 * \param src: string to be parsed
 * \param configs: Stream config array needs to be filled in
 *
 */
void CameraParser::parseStreamConfig(const char* src, stream_array_t& configs) {
    HAL_TRACE_CALL(1);

    int mcId = -1;
    char* endPtr = nullptr;
    char* separatorPtr = nullptr;
    int parseStep = 0;
    stream_t config;
    CLEAR(config);

#define NUM_ELEMENTS_NECESSARY 4
// Has optional element
#define NUM_ELEMENTS (NUM_ELEMENTS_NECESSARY + 1)

    bool lastElement = false;  // the last one?
    do {
        parseStep++;

        // Get the next segement for necessary element
        // Get the next segement for optional element if it exist
        if (parseStep <= NUM_ELEMENTS_NECESSARY || (!lastElement && (*src == '('))) {
            separatorPtr = const_cast<char*>(strchr(src, ','));
            if (separatorPtr) {
                *separatorPtr = 0;
            } else {
                lastElement = true;
            }
        }

        switch (parseStep) {
            case 1:  // Step 1: Parse format
                LOG2("stream format is %s", src);
                config.format = CameraUtils::string2PixelCode(src);
                CheckAndLogError(config.format == -1, VOID_VALUE, "@%s, format fails", __func__);
                break;
            case 2:  // Step 2: Parse the resolution
                config.width = strtol(src, &endPtr, 10);
                CheckAndLogError(!endPtr || *endPtr != 'x', VOID_VALUE, "@%s, width fails",
                                 __func__);
                src = endPtr + 1;
                config.height = strtol(src, &endPtr, 10);
                LOG2("(%dx%d)", config.width, config.height);
                break;
            case 3:  // Step 3: Parse field
                config.field = strtol(src, &endPtr, 10);
                LOG2("stream field is %d", config.field);
                break;
            case 4:  // Step 4: Parse MediaCtlConf id.
                mcId = strtol(src, &endPtr, 10);
                CheckAndLogError(mcId < 0, VOID_VALUE, "@%s, mcId fails", __func__);
                LOG2("the mcId for supported stream config is %d", mcId);
                break;
        }

        if (!lastElement) {
            // Move to the next element
            src = separatorPtr + 1;
            src = skipWhiteSpace(src);
        } else if (parseStep < NUM_ELEMENTS_NECESSARY) {
            LOGE("Malformed stream configuration, only finish step %d", parseStep);
            return;
        }

        // Finish all elements for one config
        if (parseStep >= NUM_ELEMENTS) {
            configs.push_back(config);
            storeMcMappForConfig(mcId, config);
            CLEAR(config);
            mcId = -1;
            parseStep = 0;
            LOG2("Stream Configuration found");
            if (lastElement) {
                break;
            }
        }
    } while (true);
}

void CameraParser::parseSupportedFeatures(const char* src, camera_features_list_t& features) {
    HAL_TRACE_CALL(1);

    char* endPtr = nullptr;
    camera_features feature = INVALID_FEATURE;
    do {
        endPtr = const_cast<char*>(strchr(src, ','));
        if (endPtr) {
            *endPtr = 0;
        }
        if (strcmp(src, "MANUAL_EXPOSURE") == 0) {
            feature = MANUAL_EXPOSURE;
        } else if (strcmp(src, "MANUAL_WHITE_BALANCE") == 0) {
            feature = MANUAL_WHITE_BALANCE;
        } else if (strcmp(src, "IMAGE_ENHANCEMENT") == 0) {
            feature = IMAGE_ENHANCEMENT;
        } else if (strcmp(src, "NOISE_REDUCTION") == 0) {
            feature = NOISE_REDUCTION;
        } else if (strcmp(src, "SCENE_MODE") == 0) {
            feature = SCENE_MODE;
        } else if (strcmp(src, "WEIGHT_GRID_MODE") == 0) {
            feature = WEIGHT_GRID_MODE;
        } else if (strcmp(src, "PER_FRAME_CONTROL") == 0) {
            feature = PER_FRAME_CONTROL;
        } else if (strcmp(src, "ISP_CONTROL") == 0) {
            feature = ISP_CONTROL;
        } else {
            feature = INVALID_FEATURE;
        }

        if (feature != INVALID_FEATURE) {
            features.push_back(feature);
        }

        if (endPtr) {
            src = endPtr + 1;
            src = skipWhiteSpace(src);
        }
    } while (endPtr);
}

int CameraParser::parseSupportedVideoStabilizationMode(
    const char* str, camera_video_stabilization_list_t& supportedModes) {
    HAL_TRACE_CALL(1);
    CheckAndLogError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    char *savePtr, *tablePtr;
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    camera_video_stabilization_mode_t mode = VIDEO_STABILIZATION_MODE_OFF;

    tablePtr = strtok_r(src, ",", &savePtr);
    while (tablePtr) {
        if (strcmp(tablePtr, "ON") == 0) {
            mode = VIDEO_STABILIZATION_MODE_ON;
        } else if (strcmp(tablePtr, "OFF") == 0) {
            mode = VIDEO_STABILIZATION_MODE_OFF;
        }
        supportedModes.push_back(mode);

        if (savePtr != nullptr) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        tablePtr = strtok_r(nullptr, ",", &savePtr);
    }

    return OK;
}

int CameraParser::parseSupportedAeMode(const char* str, vector<camera_ae_mode_t>& supportedModes) {
    HAL_TRACE_CALL(1);
    CheckAndLogError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    char *savePtr, *tablePtr;
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    camera_ae_mode_t aeMode = AE_MODE_AUTO;

    tablePtr = strtok_r(src, ",", &savePtr);
    while (tablePtr) {
        if (strcmp(tablePtr, "AUTO") == 0) {
            aeMode = AE_MODE_AUTO;
        } else if (strcmp(tablePtr, "MANUAL") == 0) {
            aeMode = AE_MODE_MANUAL;
        }
        supportedModes.push_back(aeMode);
        if (savePtr != nullptr) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        tablePtr = strtok_r(nullptr, ",", &savePtr);
    }

    return OK;
}

int CameraParser::parseSupportedAfMode(const char* str, vector<camera_af_mode_t>& supportedModes) {
    HAL_TRACE_CALL(1);
    CheckAndLogError(str == NULL, -1, "@%s, str is NULL", __func__);

    char *savePtr, *tablePtr;
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    camera_af_mode_t afMode = AF_MODE_AUTO;

    tablePtr = strtok_r(src, ",", &savePtr);
    while (tablePtr) {
        if (strcmp(tablePtr, "AUTO") == 0) {
            afMode = AF_MODE_AUTO;
        } else if (strcmp(tablePtr, "MACRO") == 0) {
            afMode = AF_MODE_MACRO;
        } else if (strcmp(tablePtr, "CONTINUOUS_VIDEO") == 0) {
            afMode = AF_MODE_CONTINUOUS_VIDEO;
        } else if (strcmp(tablePtr, "CONTINUOUS_PICTURE") == 0) {
            afMode = AF_MODE_CONTINUOUS_PICTURE;
        } else if (strcmp(tablePtr, "OFF") == 0) {
            afMode = AF_MODE_OFF;
        }
        supportedModes.push_back(afMode);
        if (savePtr != NULL) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        tablePtr = strtok_r(NULL, ",", &savePtr);
    }

    return OK;
}

int CameraParser::parseSupportedAntibandingMode(const char* str,
                                                vector<camera_antibanding_mode_t>& supportedModes) {
    HAL_TRACE_CALL(1);
    CheckAndLogError(str == nullptr, -1, "@%s, str is nullptr", __func__);

    char *savePtr, *tablePtr;
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    camera_antibanding_mode_t antibandingMode = ANTIBANDING_MODE_OFF;

    tablePtr = strtok_r(src, ",", &savePtr);
    while (tablePtr) {
        if (strcmp(tablePtr, "AUTO") == 0) {
            antibandingMode = ANTIBANDING_MODE_AUTO;
        } else if (strcmp(tablePtr, "50Hz") == 0) {
            antibandingMode = ANTIBANDING_MODE_50HZ;
        } else if (strcmp(tablePtr, "60Hz") == 0) {
            antibandingMode = ANTIBANDING_MODE_60HZ;
        } else if (strcmp(tablePtr, "OFF") == 0) {
            antibandingMode = ANTIBANDING_MODE_OFF;
        }
        supportedModes.push_back(antibandingMode);
        if (savePtr != nullptr) savePtr = const_cast<char*>(skipWhiteSpace(savePtr));
        tablePtr = strtok_r(nullptr, ",", &savePtr);
    }

    return OK;
}

int CameraParser::parseSupportedAeParamRange(const char* src, vector<int>& scenes,
                                             vector<float>& minValues, vector<float>& maxValues) {
    HAL_TRACE_CALL(1);
    char* srcDup = strdup(src);
    CheckAndLogError((srcDup == nullptr), NO_MEMORY, "Create a copy of source string failed.");

    char* srcTmp = srcDup;
    char* endPtr = nullptr;
    while ((endPtr = const_cast<char*>(strchr(srcTmp, ',')))) {
        if (endPtr) *endPtr = 0;

        camera_scene_mode_t scene = CameraUtils::getSceneModeByName(srcTmp);
        scenes.push_back(scene);
        if (endPtr) {
            srcTmp = endPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }

        float min = strtof(srcTmp, &endPtr);
        minValues.push_back(min);
        if (endPtr == nullptr || *endPtr != ',') {
            LOGE("Malformed ET range in exposure time range configuration");
            free(srcDup);
            return UNKNOWN_ERROR;
        }
        srcTmp = endPtr + 1;
        float max = strtof(srcTmp, &endPtr);
        maxValues.push_back(max);

        if (endPtr) {
            srcTmp = endPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }
    }
    free(srcDup);
    return OK;
}

void CameraParser::parseFormatElement(CameraParser* profiles, const char* name, const char** atts) {
    McFormat fmt;
    fmt.type = RESOLUTION_TARGET;

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "name") == 0) {
            fmt.entityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                fmt.entity = profiles->mMC->getEntityIdByName(fmt.entityName.c_str());
            }
        } else if (strcmp(key, "pad") == 0) {
            fmt.pad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "stream") == 0) {
            fmt.stream = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "type") == 0) {
            if (strcmp(val, "RESOLUTION_MAX") == 0) {
                fmt.type = RESOLUTION_MAX;
            } else if (strcmp(val, "RESOLUTION_COMPOSE") == 0) {
                fmt.type = RESOLUTION_COMPOSE;
            } else if (strcmp(val, "RESOLUTION_CROP") == 0) {
                fmt.type = RESOLUTION_CROP;
            } else if (strcmp(val, "RESOLUTION_TARGET") == 0) {
                fmt.type = RESOLUTION_TARGET;
            } else {
                LOGE("Parse format type failed. type = %s", val);
                return;
            }
        } else if (strcmp(key, "width") == 0) {
            fmt.width = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "height") == 0) {
            fmt.height = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "format") == 0) {
            fmt.pixelCode = CameraUtils::string2PixelCode(val);
        }
        idx += 2;
    }

    fmt.formatType = FC_FORMAT;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();
    mc.formats.push_back(fmt);
}

void CameraParser::parseLinkElement(CameraParser* profiles, const char* name, const char** atts) {
    McLink link;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "srcName") == 0) {
            link.srcEntityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                link.srcEntity = profiles->mMC->getEntityIdByName(link.srcEntityName.c_str());
            }
        } else if (strcmp(key, "srcPad") == 0) {
            link.srcPad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "sinkName") == 0) {
            link.sinkEntityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                link.sinkEntity = profiles->mMC->getEntityIdByName(link.sinkEntityName.c_str());
            }
        } else if (strcmp(key, "sinkPad") == 0) {
            link.sinkPad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "enable") == 0) {
            link.enable = strcmp(val, "true") == 0;
        }

        idx += 2;
    }

    mc.links.push_back(link);
}

void CameraParser::parseRouteElement(CameraParser* profiles, const char* name, const char** atts) {
    McRoute route;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();
    route.flag = MEDIA_LNK_FL_ENABLED;

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "name") == 0) {
            route.entityName = replaceStringInXml(profiles, val);
            if (profiles->mMC) {
                route.entity = profiles->mMC->getEntityIdByName(route.entityName.c_str());
            }
        } else if (strcmp(key, "srcPad") == 0) {
            route.srcPad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "sinkPad") == 0) {
            route.sinkPad = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "srcStream") == 0) {
            route.srcStream = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "sinkStream") == 0) {
            route.sinkStream = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "flag") == 0) {
            route.flag = strtoul(val, nullptr, 10);
        }
        idx += 2;
    }

    mc.routes.push_back(route);
}

void CameraParser::parseVideoElement(CameraParser* profiles, const char* /*name*/,
                                     const char** atts) {
    McVideoNode videoNode;
    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();

    videoNode.name = replaceStringInXml(profiles, atts[1]);
    videoNode.videoNodeType = GetNodeType(atts[3]);
    LOG2("@%s, name:%s, videoNodeType:%d", __func__, videoNode.name.c_str(),
         videoNode.videoNodeType);

    mc.videoNodes.push_back(videoNode);
}

// MediaCtl output tag xml parsing code for the field like:
// <output port="main" width="1920" height="1088" format="V4L2_PIX_FMT_YUYV420_V32"/>
// <output port="second" width="3264" height="2448" format="V4L2_PIX_FMT_SGRBG12V32"/>
void CameraParser::parseOutputElement(CameraParser* profiles, const char* name, const char** atts) {
    McOutput output;

    int idx = 0;
    while (atts[idx]) {
        const char* key = atts[idx];
        const char* val = atts[idx + 1];
        LOG2("@%s, name:%s, atts[%d]:%s, atts[%d]:%s", __func__, name, idx, key, idx + 1, val);
        if (strcmp(key, "port") == 0) {
            if (strcmp(val, "main") == 0)
                output.port = MAIN_PORT;
            else if (strcmp(val, "second") == 0)
                output.port = SECOND_PORT;
            else if (strcmp(val, "third") == 0)
                output.port = THIRD_PORT;
            else if (strcmp(val, "forth") == 0)
                output.port = FORTH_PORT;
            else
                output.port = INVALID_PORT;
        } else if (strcmp(key, "width") == 0) {
            output.width = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "height") == 0) {
            output.height = strtoul(val, nullptr, 10);
        } else if (strcmp(key, "format") == 0) {
            output.v4l2Format = CameraUtils::string2PixelCode(val);
        }
        idx += 2;
    }

    LOG2("@%s, port:%d, output size:%dx%d, v4l2Format:%x", __func__, output.port, output.width,
         output.height, output.v4l2Format);

    MediaCtlConf& mc = profiles->pCurrentCam->mMediaCtlConfs.back();
    mc.outputs.push_back(output);
}

void CameraParser::parseMultiExpRange(const char* src) {
    ExpRange* range = nullptr;
    MultiExpRange multiRange;
    MultiExpRange* pCurrRange = nullptr;
    pCurrentCam->mMultiExpRanges.clear();
    static const int MULTI_EXPOSURE_TAG_SHS1 = 0;
    static const int MULTI_EXPOSURE_TAG_RHS1 = 1;
    static const int MULTI_EXPOSURE_TAG_SHS2 = 2;
    static const int MULTI_EXPOSURE_TAG_RHS2 = 3;
    static const int MULTI_EXPOSURE_TAG_SHS3 = 4;

    string srcDup = src;
    CheckAndLogError((srcDup.c_str() == nullptr), VOID_VALUE,
                     "Create a copy of source string failed.");

    const char* srcTmp = srcDup.c_str();
    char* endPtr = nullptr;
    int tag = -1;
    while ((endPtr = const_cast<char*>(strchr(srcTmp, ',')))) {
        *endPtr = 0;
        if (strcmp(srcTmp, "SHS1") == 0) {
            tag = MULTI_EXPOSURE_TAG_SHS1;
        } else if (strcmp(srcTmp, "RHS1") == 0) {
            tag = MULTI_EXPOSURE_TAG_RHS1;
        } else if (strcmp(srcTmp, "SHS2") == 0) {
            tag = MULTI_EXPOSURE_TAG_SHS2;
        } else if (strcmp(srcTmp, "RHS2") == 0) {
            tag = MULTI_EXPOSURE_TAG_RHS2;
        } else if (strcmp(srcTmp, "SHS3") == 0) {
            tag = MULTI_EXPOSURE_TAG_SHS3;
        } else {
            LOGE("Malformed tag for multi-exposure range configuration");
            return;
        }

        if (endPtr) {
            srcTmp = endPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }

        CLEAR(multiRange);
        multiRange.Resolution.width = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed resolution for multi-exposure range configuration");

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        multiRange.Resolution.height = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed resolution for multi-exposure range configuration");

        pCurrRange = nullptr;
        for (unsigned int i = 0; i < pCurrentCam->mMultiExpRanges.size(); i++) {
            if (pCurrentCam->mMultiExpRanges[i].Resolution.width == multiRange.Resolution.width &&
                pCurrentCam->mMultiExpRanges[i].Resolution.height == multiRange.Resolution.height) {
                pCurrRange = &(pCurrentCam->mMultiExpRanges[i]);
                break;
            }
        }
        if (pCurrRange) {
            switch (tag) {
                case MULTI_EXPOSURE_TAG_SHS1:
                    range = &pCurrRange->SHS1;
                    break;
                case MULTI_EXPOSURE_TAG_RHS1:
                    range = &pCurrRange->RHS1;
                    break;
                case MULTI_EXPOSURE_TAG_SHS2:
                    range = &pCurrRange->SHS2;
                    break;
                case MULTI_EXPOSURE_TAG_RHS2:
                    range = &pCurrRange->RHS2;
                    break;
                case MULTI_EXPOSURE_TAG_SHS3:
                    range = &pCurrRange->SHS3;
                    break;
                default:
                    LOGE("Wrong tag for multi-exposure range configuration");
                    return;
            }
        } else {
            switch (tag) {
                case MULTI_EXPOSURE_TAG_SHS1:
                    range = &multiRange.SHS1;
                    break;
                case MULTI_EXPOSURE_TAG_RHS1:
                    range = &multiRange.RHS1;
                    break;
                case MULTI_EXPOSURE_TAG_SHS2:
                    range = &multiRange.SHS2;
                    break;
                case MULTI_EXPOSURE_TAG_RHS2:
                    range = &multiRange.RHS2;
                    break;
                case MULTI_EXPOSURE_TAG_SHS3:
                    range = &multiRange.SHS3;
                    break;
                default:
                    LOGE("Wrong tag for multi-exposure range configuration");
                    return;
            }
        }

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        range->min = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed range for multi-exposure range configuration");

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        range->max = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed range for multi-exposure range configuration");

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        range->step = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed range for multi-exposure range configuration");

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        range->lowerBound = strtol(srcTmp, &endPtr, 10);
        CheckAndLogError((endPtr == nullptr || *endPtr != ','), VOID_VALUE,
                         "Malformed range for multi-exposure range configuration");

        srcTmp = endPtr + 1;
        srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        range->upperBound = strtol(srcTmp, &endPtr, 10);

        if (endPtr) {
            srcTmp = endPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }

        if (!pCurrRange) {
            pCurrentCam->mMultiExpRanges.push_back(multiRange);
        }
    }
}

int CameraParser::parsePair(const char* str, int* first, int* second, char delim, char** endptr) {
    // Find the first integer.
    char* end;
    int w = static_cast<int>(strtol(str, &end, 10));
    // If a delimeter does not immediately follow, give up.
    if (*end != delim) {
        LOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = static_cast<int>(strtol(end + 1, &end, 10));

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}

void CameraParser::parseSizesList(const char* sizesStr, vector<camera_resolution_t>& sizes) {
    if (sizesStr == 0) {
        return;
    }

    char* sizeStartPtr = const_cast<char*>(sizesStr);
    while (true) {
        camera_resolution_t r;
        int success = parsePair(sizeStartPtr, &r.width, &r.height, 'x', &sizeStartPtr);
        if (success == -1 || (*sizeStartPtr != ',' && *sizeStartPtr != '\0')) {
            LOGE("Picture sizes string \"%s\" contains invalid character.", sizesStr);
            return;
        }
        if (r.width > 0 && r.height > 0) sizes.push_back(r);

        if (*sizeStartPtr == '\0') {
            return;
        }
        sizeStartPtr++;
    }
}

/*
 * The pls output to user requirement mapping table
 *
 * first: user requirement, second: psl output
 * eg: <pslOutputForRotation value="3264x2448@1200x1600"/>
 */
void CameraParser::parseOutputMap(const char* str, vector<UserToPslOutputMap>& outputMap) {
    char* srcDup = strdup(str);
    CheckAndLogError((srcDup == nullptr), VOID_VALUE, "Create a copy of source string failed.");

    char* srcTmp = srcDup;
    char* endPtr = nullptr;
    do {
        endPtr = strchr(srcTmp, ',');
        if (endPtr) {
            *endPtr = 0;
        }
        char* tmpPtr = strchr(srcTmp, '@');
        if (tmpPtr) {
            *tmpPtr = 0;
        }

        UserToPslOutputMap map;
        parsePair(srcTmp, &(map.User).width, &(map.User).height, 'x');
        if (tmpPtr) {
            srcTmp = tmpPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }
        parsePair(srcTmp, &(map.Psl).width, &(map.Psl).height, 'x');
        outputMap.push_back(map);

        if (endPtr) {
            srcTmp = endPtr + 1;
            srcTmp = const_cast<char*>(skipWhiteSpace(srcTmp));
        }
    } while (endPtr);

    free(srcDup);
}

int CameraParser::getSupportedFormat(const char* str, vector<int>& supportedFormat) {
    if (str == nullptr) {
        LOGE("the str is nullptr");
        return -1;
    }

    LOG2("@%s, str:%s", __func__, str);
    int sz = strlen(str);
    char src[sz + 1];
    MEMCPY_S(src, sz, str, sz);
    src[sz] = '\0';
    char* savePtr;
    char* fmt = strtok_r(src, ",", &savePtr);
    while (fmt) {
        int actual = CameraUtils::string2PixelCode(fmt);
        if (actual != -1) {
            supportedFormat.push_back(actual);
            LOG2("@%s, add format:%d", __func__, actual);
        }
        fmt = strtok_r(nullptr, ",", &savePtr);
    }

    return 0;
}

/**
 * This function will handle all the MediaCtlCfg related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void CameraParser::handleMediaCtlCfg(CameraParser* profiles, const char* name, const char** atts) {
    LOG2("@%s, name:%s, atts[0]:%s, profiles->mCurrentSensor:%d", __func__, name, atts[0],
         profiles->mCurrentSensor);
    if (strcmp(name, "MediaCtlConfig") == 0) {
        parseMediaCtlConfigElement(profiles, name, atts);
    } else if (strcmp(name, "link") == 0) {
        parseLinkElement(profiles, name, atts);
    } else if (strcmp(name, "route") == 0) {
        parseRouteElement(profiles, name, atts);
    } else if (strcmp(name, "control") == 0) {
        parseControlElement(profiles, name, atts);
    } else if (strcmp(name, "selection") == 0) {
        parseSelectionElement(profiles, name, atts);
    } else if (strcmp(name, "format") == 0) {
        parseFormatElement(profiles, name, atts);
    } else if (strcmp(name, "videonode") == 0) {
        parseVideoElement(profiles, name, atts);
    } else if (strcmp(name, "output") == 0) {
        parseOutputElement(profiles, name, atts);
    }
}

/**
 * This function will handle all the StaticMetadata related elements.
 *
 * It will be called in the function startElement
 *
 * \param profiles: the pointer of the CameraParser.
 * \param name: the element's name.
 * \param atts: the element's attribute.
 */
void CameraParser::handleStaticMetaData(CameraParser* profiles, const char* name,
                                        const char** atts) {
    LOG2("@%s, name:%s, atts[0]:%s, profiles->mCurrentSensor:%d", __func__, name, atts[0],
         profiles->mCurrentSensor);
    if (strcmp(name, "supportedStreamConfig") == 0) {
        stream_array_t configsArray;
        parseStreamConfig(atts[1], configsArray);
        const int STREAM_MEMBER_NUM = sizeof(stream_t) / sizeof(int);
        int dataSize = configsArray.size() * STREAM_MEMBER_NUM;
        int configs[dataSize];
        CLEAR(configs);
        for (size_t i = 0; i < configsArray.size(); i++) {
            LOG2("@%s, stream config info: format=%s (%dx%d) field=%d type=%d", __func__,
                 CameraUtils::format2string(configsArray[i].format).c_str(), configsArray[i].width,
                 configsArray[i].height, configsArray[i].field, configsArray[i].streamType);
            MEMCPY_S(&configs[i * STREAM_MEMBER_NUM], sizeof(stream_t), &configsArray[i],
                     sizeof(stream_t));
        }
        mMetadata.update(INTEL_INFO_AVAILABLE_CONFIGURATIONS, configs, dataSize);
    } else if (strcmp(name, "fpsRange") == 0) {
        vector<double> rangeArray;
        parseXmlConvertStrings(atts[1], rangeArray, atof);
        float fpsRange[rangeArray.size()];
        CLEAR(fpsRange);
        for (size_t i = 0; i < rangeArray.size(); i++) {
            fpsRange[i] = static_cast<float>(rangeArray[i]);
        }
        LOG2("@%s, supported fps range size: %zu", __func__, rangeArray.size());
        mMetadata.update(CAMERA_AE_AVAILABLE_TARGET_FPS_RANGES, fpsRange, ARRAY_SIZE(fpsRange));
    } else if (strcmp(name, "evRange") == 0) {
        vector<int> rangeArray;
        parseXmlConvertStrings(atts[1], rangeArray, atoi);

        int evRange[rangeArray.size()];
        CLEAR(evRange);
        for (size_t i = 0; i < rangeArray.size(); i++) {
            evRange[i] = rangeArray[i];
        }
        LOG2("@%s, supported ev range size: %zu", __func__, rangeArray.size());
        mMetadata.update(CAMERA_AE_COMPENSATION_RANGE, evRange, ARRAY_SIZE(evRange));
    } else if (strcmp(name, "evStep") == 0) {
        vector<int> rationalType;
        int ret = parseXmlConvertStrings(atts[1], rationalType, atoi);
        CheckAndLogError((ret != OK), VOID_VALUE, "Parse evStep failed");

        icamera_metadata_rational_t evStep = {rationalType[0], rationalType[1]};
        LOG2("@%s, the numerator: %d, denominator: %d", __func__, evStep.numerator,
             evStep.denominator);
        mMetadata.update(CAMERA_AE_COMPENSATION_STEP, &evStep, 1);
    } else if (strcmp(name, "supportedFeatures") == 0) {
        camera_features_list_t supportedFeatures;
        parseSupportedFeatures(atts[1], supportedFeatures);
        int numberOfFeatures = supportedFeatures.size();
        uint8_t features[numberOfFeatures];
        CLEAR(features);
        for (int i = 0; i < numberOfFeatures; i++) {
            features[i] = supportedFeatures[i];
        }
        mMetadata.update(INTEL_INFO_AVAILABLE_FEATURES, features, numberOfFeatures);
    } else if (strcmp(name, "supportedAeExposureTimeRange") == 0) {
        vector<int> scenes;
        vector<float> minValues, maxValues;
        int ret = parseSupportedAeParamRange(atts[1], scenes, minValues, maxValues);
        CheckAndLogError((ret != OK), VOID_VALUE, "Parse AE eExposure time range failed");

        const int MEMBER_COUNT = 3;
        const int dataSize = scenes.size() * MEMBER_COUNT;
        int rangeData[dataSize];
        CLEAR(rangeData);

        for (size_t i = 0; i < scenes.size(); i++) {
            LOG2("@%s, scene mode:%d supported exposure time range (%f-%f)", __func__, scenes[i],
                 minValues[i], maxValues[i]);
            rangeData[i * MEMBER_COUNT] = scenes[i];
            rangeData[i * MEMBER_COUNT + 1] = static_cast<int>(minValues[i]);
            rangeData[i * MEMBER_COUNT + 2] = static_cast<int>(maxValues[i]);
        }
        mMetadata.update(INTEL_INFO_AE_EXPOSURE_TIME_RANGE, rangeData, dataSize);
    } else if (strcmp(name, "supportedAeGainRange") == 0) {
        vector<int> scenes;
        vector<float> minValues, maxValues;
        int ret = parseSupportedAeParamRange(atts[1], scenes, minValues, maxValues);
        CheckAndLogError((ret != OK), VOID_VALUE, "Parse AE gain range failed");

        const int MEMBER_COUNT = 3;
        const int dataSize = scenes.size() * MEMBER_COUNT;
        int rangeData[dataSize];
        CLEAR(rangeData);

        for (size_t i = 0; i < scenes.size(); i++) {
            LOG2("@%s, scene mode:%d supported gain range (%f-%f)", __func__, scenes[i],
                 minValues[i], maxValues[i]);
            rangeData[i * MEMBER_COUNT] = scenes[i];
            // Since we use int to store float, before storing it we multiply min and max by 100.
            rangeData[i * MEMBER_COUNT + 1] = static_cast<int>(minValues[i] * 100);
            rangeData[i * MEMBER_COUNT + 2] = static_cast<int>(maxValues[i] * 100);
        }
        mMetadata.update(INTEL_INFO_AE_GAIN_RANGE, rangeData, dataSize);
    } else if (strcmp(name, "supportedVideoStabilizationModes") == 0) {
        camera_video_stabilization_list_t supportedMode;
        parseSupportedVideoStabilizationMode(atts[1], supportedMode);
        uint8_t modes[supportedMode.size()];
        CLEAR(modes);
        for (size_t i = 0; i < supportedMode.size(); i++) {
            modes[i] = supportedMode[i];
        }
        mMetadata.update(CAMERA_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES, modes,
                         supportedMode.size());
    } else if (strcmp(name, "supportedAeMode") == 0) {
        vector<camera_ae_mode_t> supportedAeMode;
        parseSupportedAeMode(atts[1], supportedAeMode);
        uint8_t aeModes[supportedAeMode.size()];
        CLEAR(aeModes);
        for (size_t i = 0; i < supportedAeMode.size(); i++) {
            aeModes[i] = supportedAeMode[i];
        }
        mMetadata.update(CAMERA_AE_AVAILABLE_MODES, aeModes, supportedAeMode.size());
    } else if (strcmp(name, "supportedAwbMode") == 0) {
        vector<camera_awb_mode_t> supportedAwbMode;
        parseXmlConvertStrings(atts[1], supportedAwbMode, CameraUtils::getAwbModeByName);
        uint8_t awbModes[supportedAwbMode.size()];
        CLEAR(awbModes);
        for (size_t i = 0; i < supportedAwbMode.size(); i++) {
            awbModes[i] = supportedAwbMode[i];
        }
        mMetadata.update(CAMERA_AWB_AVAILABLE_MODES, awbModes, supportedAwbMode.size());
    } else if (strcmp(name, "supportedSceneMode") == 0) {
        vector<camera_scene_mode_t> supportedSceneMode;
        parseXmlConvertStrings(atts[1], supportedSceneMode, CameraUtils::getSceneModeByName);
        uint8_t sceneModes[supportedSceneMode.size()];
        CLEAR(sceneModes);
        for (size_t i = 0; i < supportedSceneMode.size(); i++) {
            sceneModes[i] = supportedSceneMode[i];
        }
        mMetadata.update(CAMERA_CONTROL_AVAILABLE_SCENE_MODES, sceneModes,
                         supportedSceneMode.size());
    } else if (strcmp(name, "supportedAfMode") == 0) {
        vector<camera_af_mode_t> supportedAfMode;
        parseSupportedAfMode(atts[1], supportedAfMode);
        uint8_t afModes[supportedAfMode.size()];
        CLEAR(afModes);
        for (size_t i = 0; i < supportedAfMode.size(); i++) {
            afModes[i] = supportedAfMode[i];
        }
        mMetadata.update(CAMERA_AF_AVAILABLE_MODES, afModes, supportedAfMode.size());
    } else if (strcmp(name, "supportedAntibandingMode") == 0) {
        vector<camera_antibanding_mode_t> supportedAntibandingMode;
        parseSupportedAntibandingMode(atts[1], supportedAntibandingMode);
        uint8_t antibandingModes[supportedAntibandingMode.size()];
        CLEAR(antibandingModes);
        for (size_t i = 0; i < supportedAntibandingMode.size(); i++) {
            antibandingModes[i] = supportedAntibandingMode[i];
        }
        mMetadata.update(CAMERA_AE_AVAILABLE_ANTIBANDING_MODES, antibandingModes,
                         supportedAntibandingMode.size());
    } else if (strcmp(name, "sensorMountType") == 0) {
        uint8_t mountType = WALL_MOUNTED;

        if (strcmp(atts[1], "CEILING_MOUNTED") == 0) mountType = CEILING_MOUNTED;

        mMetadata.update(INTEL_INFO_SENSOR_MOUNT_TYPE, &mountType, 1);
        LOG2("@%s, sensor mount type: %d", __func__, mountType);
    } else if (strcmp(name, "StaticMetadata") != 0) {
        // Make sure it doesn't reach the end of StaticMetadata.
        handleGenericStaticMetaData(name, atts[1], &mMetadata);
    }
}

/**
 * \brief Parses string for generic static metadata and save them.
 *
 * \param name: the element's name.
 * \param src: the element's value, only include data and separator 'x' or ','.
 * \param metadata: used to save metadata
 */
void CameraParser::handleGenericStaticMetaData(const char* name, const char* src,
                                               CameraMetadata* metadata) {
    CheckAndLogError(!metadata, VOID_VALUE, "metadata is nullptr");
    uint32_t tag = -1;
    if (mGenericStaticMetadataToTag.find(name) != mGenericStaticMetadataToTag.end()) {
        tag = mGenericStaticMetadataToTag[name];
    }

    int tagType = get_icamera_metadata_tag_type(tag);
    if (tagType == -1) {
        LOGW("Unsupported metadata %s", name);
        return;
    }

    union {
        uint8_t* u8;
        int32_t* i32;
        int64_t* i64;
        float* f;
        double* d;
        icamera_metadata_rational_t* r;
    } data;
    data.u8 = (unsigned char*)mMetadataCache;

    int index = 0;
    int maxIndex = mMetadataCacheSize / sizeof(double);  // worst case
    char* endPtr = nullptr;
    do {
        switch (tagType) {
            case ICAMERA_TYPE_BYTE:
                data.u8[index] = static_cast<char>(strtol(src, &endPtr, 10));
                LOG2(" - %d -", data.u8[index]);
                break;
            case ICAMERA_TYPE_INT32:
            case ICAMERA_TYPE_RATIONAL:
                data.i32[index] = strtol(src, &endPtr, 10);
                LOG2(" - %d -", data.i32[index]);
                break;
            case ICAMERA_TYPE_INT64:
                data.i64[index] = strtol(src, &endPtr, 10);
                LOG2(" - %ld -", data.i64[index]);
                break;
            case ICAMERA_TYPE_FLOAT:
                data.f[index] = strtof(src, &endPtr);
                LOG2(" - %8.3f -", data.f[index]);
                break;
            case ICAMERA_TYPE_DOUBLE:
                data.d[index] = strtof(src, &endPtr);
                LOG2(" - %8.3f -", data.d[index]);
                break;
        }
        index++;

        if (endPtr != nullptr && (*endPtr == 'x' || *endPtr == ',')) {
            src = endPtr + 1;
        } else {
            break;
        }
    } while (index < maxIndex);

    switch (tagType) {
        case ICAMERA_TYPE_BYTE:
            metadata->update(tag, data.u8, index);
            break;
        case ICAMERA_TYPE_INT32:
            metadata->update(tag, data.i32, index);
            break;
        case ICAMERA_TYPE_INT64:
            metadata->update(tag, data.i64, index);
            break;
        case ICAMERA_TYPE_FLOAT:
            metadata->update(tag, data.f, index);
            break;
        case ICAMERA_TYPE_DOUBLE:
            metadata->update(tag, data.d, index);
            break;
        case ICAMERA_TYPE_RATIONAL:
            metadata->update(tag, data.r, index / 2);
            break;
    }
}

/**
 * the callback function of the libexpat for handling of one element start
 *
 * When it comes to the start of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void CameraParser::startParseElement(void* userData, const char* name, const char** atts) {
    CameraParser* profiles = reinterpret_cast<CameraParser*>(userData);

    if (profiles->mCurrentDataField == FIELD_INVALID) {
        profiles->checkField(profiles, name, atts);
        return;
    }

    switch (profiles->mCurrentDataField) {
        case FIELD_SENSOR:
            if (strcmp(name, "MediaCtlConfig") == 0) {
                profiles->mInMediaCtlCfg = true;
                LOG2("@%s %s, mInMediaCtlCfg is set to true", __func__, name);
            } else if (strcmp(name, "StaticMetadata") == 0) {
                profiles->mInStaticMetadata = true;
                LOG2("@%s %s, mInStaticMetadata is set to true", __func__, name);
            } else if (strncmp(name, "CameraModuleInfo_", strlen("CameraModuleInfo_")) == 0) {
                // tag name like this: CameraModuleInfo_XXX
                std::string tagName(name);
                profiles->mCameraModuleName = tagName.substr(strlen("CameraModuleInfo_"));
                LOG2("@%s, mCameraModuleInfo %s is set", __func__, name);
                break;
            }

            if (profiles->mInMediaCtlCfg) {
                // The MediaCtlCfg belongs to the sensor segments
                profiles->handleMediaCtlCfg(profiles, name, atts);
            } else if (profiles->mInStaticMetadata) {
                // The StaticMetadata belongs to the sensor segments
                profiles->handleStaticMetaData(profiles, name, atts);
            } else if (!profiles->mCameraModuleName.empty()) {
                // The CameraModuleInfo belongs to the sensor segments
                LOG2("@%s, name:%s, atts[1]:%s, profiles->mCurrentSensor:%d", __func__, name,
                     atts[1], profiles->mCurrentSensor);
                profiles->handleGenericStaticMetaData(name, atts[1],
                                                      &profiles->mCameraModuleMetadata);
            } else {
                profiles->handleSensor(profiles, name, atts);
            }
            break;
        case FIELD_COMMON:
            profiles->handleCommon(profiles, name, atts);
            break;
        default:
            LOGE("@%s, line:%d, go to default handling", __func__, __LINE__);
            break;
    }
}

/**
 * the callback function of the libexpat for handling of one element end
 *
 * When it comes to the end of one element. This function will be called.
 *
 * \param userData: the pointer we set by the function XML_SetUserData.
 * \param name: the element's name.
 */
void CameraParser::endParseElement(void* userData, const char* name) {
    LOG2("@%s %s", __func__, name);

    CameraParser* profiles = reinterpret_cast<CameraParser*>(userData);

    if (strcmp(name, "Sensor") == 0) {
        profiles->mCurrentDataField = FIELD_INVALID;
        if (profiles->pCurrentCam) {
            LOG2("@%s: Add camera id %d (%s)", __func__, profiles->mCurrentSensor,
                 profiles->pCurrentCam->sensorName.c_str());
            if (profiles->pCurrentCam->mLensName.empty() &&
                profiles->pCurrentCam->sensorName.find("-wf") != string::npos) {
                if (profiles->mMC) {
                    int ret = profiles->mMC->getLensName(&profiles->pCurrentCam->mLensName);
                    if (ret != OK) {
                        LOG2("@%s, Failed to getLensName", __func__);
                    }
                }
            }

            // I2CBus is adaptor-bus, like 18-0010, and use adaptor id to select NVM path.
            if ((profiles->mI2CBus.size() >= 2) && !profiles->mNvmDeviceInfo.empty()) {
                getNVMDirectory(profiles);
            }

            profiles->mNvmDeviceInfo.clear();

            bool isCameraAvailable = true;
            // Check if the camera is available
            if (!profiles->pCurrentCam->mSupportModuleNames.empty()) {
                isCameraAvailable = false;
                for (size_t i = 0; i < profiles->pCurrentCam->mSupportModuleNames.size(); i++) {
                    if ((strcmp(pCurrentCam->mSupportModuleNames[i].c_str(),
                                profiles->pCurrentCam->mCamModuleName.c_str()) == 0) ||
                         (strcmp(pCurrentCam->mSupportModuleNames[i].c_str(),
                                 DEFAULT_MODULE_NAME) == 0)) {
                        isCameraAvailable = true;
                        // If find an available sensor, it will not search other sensors
                        profiles->mIsAvailableSensor = true;
                        break;
                    }
                }
            }

            if (isCameraAvailable) {
                // Merge the content of mMetadata into mCapability.
                ParameterHelper::merge(profiles->mMetadata, &profiles->pCurrentCam->mCapability);

                // For non-extended camera, it should be in order by mCurrentSensor
                profiles->mStaticCfg->mCameras.insert(
                    profiles->mStaticCfg->mCameras.begin() + profiles->mCurrentSensor,
                    *(profiles->pCurrentCam));
            } else {
                profiles->mSensorNum--;
                if (profiles->mCurrentSensor > 0) profiles->mCurrentSensor--;
            }

            profiles->mMetadata.clear();
            delete profiles->pCurrentCam;
            profiles->pCurrentCam = nullptr;
        }
    }

    if (strcmp(name, "MediaCtlConfig") == 0) {
        LOG2("@%s %s, mInMediaCtlCfg is set to false", __func__, name);
        profiles->mInMediaCtlCfg = false;
    }

    if (strcmp(name, "StaticMetadata") == 0) {
        LOG2("@%s %s, mInStaticMetadata is set to false", __func__, name);
        profiles->mInStaticMetadata = false;
    }

    if (strncmp(name, "CameraModuleInfo_", strlen("CameraModuleInfo_")) == 0) {
        LOG2("@%s Camera Module Name is %s", __func__, name);
        if (!profiles->mCameraModuleName.empty()) {
            profiles->pCurrentCam->mCameraModuleInfoMap[profiles->mCameraModuleName] =
                mCameraModuleMetadata;
            profiles->mCameraModuleName.clear();
        }
    }

    if (strcmp(name, "Common") == 0) profiles->mCurrentDataField = FIELD_INVALID;

    if (strcmp(name, "CameraSettings") == 0) {
        profiles->mIsAvailableSensor = false;
        LOG2("@%s Camera mSensorNum:%d, mCurrentSensor:%d", __func__, profiles->mSensorNum,
             profiles->mCurrentSensor);
    }
}

int CameraParser::getCameraModuleNameFromEEPROM(PlatformData::StaticCfg::CameraInfo* cam) {
    const int moduleInfoOffset = CAMERA_MODULE_INFO_OFFSET;
    FILE* eepromFile = fopen(cam->mNvmDirectory.c_str(), "rb");
    CheckAndLogError(!eepromFile, UNKNOWN_ERROR, "Failed to open EEPROM file in %s",
                     cam->mNvmDirectory.c_str());

    // file size should be larger than CAMERA_MODULE_INFO_OFFSET
    fseek(eepromFile, 0, SEEK_END);
    int nvmDataSize = static_cast<int>(ftell(eepromFile));
    if (nvmDataSize < moduleInfoOffset) {
        LOGE("EEPROM data is too small");
        fclose(eepromFile);
        return NOT_ENOUGH_DATA;
    }

    fseek(eepromFile, -1 * moduleInfoOffset, SEEK_END);

    const int moduleInfoSize = CAMERA_MODULE_INFO_SIZE;
    struct CameraModuleInfo cameraModuleInfo;
    CLEAR(cameraModuleInfo);
    int ret = fread(&cameraModuleInfo, moduleInfoSize, 1, eepromFile);
    fclose(eepromFile);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "Failed to read module info %d", ret);

    if (strncmp(cameraModuleInfo.mOsInfo, NVM_OS, strlen(NVM_OS)) != 0) {
        LOG1("NVM OS string doesn't match with module info");
        return NO_ENTRY;
    }

    char tmpName[CAMERA_MODULE_INFO_SIZE];
    snprintf(tmpName, CAMERA_MODULE_INFO_SIZE, "%c%c_%04x", cameraModuleInfo.mModuleVendor[0],
             cameraModuleInfo.mModuleVendor[1], cameraModuleInfo.mModuleProduct);

    cam->mCamModuleName.assign(tmpName);
    LOG1("%s, aiqb name %s", __func__, cam->mCamModuleName.c_str());

    char moduleId[CAMERA_MODULE_INFO_SIZE];
    snprintf(moduleId, CAMERA_MODULE_INFO_SIZE, "%c%c%04x", cameraModuleInfo.mModuleVendor[0],
             cameraModuleInfo.mModuleVendor[1], cameraModuleInfo.mModuleProduct);
    cam->mModuleId.assign(moduleId);

    char sensorId[CAMERA_MODULE_INFO_SIZE];
    snprintf(sensorId, CAMERA_MODULE_INFO_SIZE, "%c%c%04x", cameraModuleInfo.mSensorVendor[0],
             cameraModuleInfo.mSensorVendor[1], cameraModuleInfo.mSensorModel);
    cam->mSensorId.assign(sensorId);

    LOG1("module id %s, sensor id %s", cam->mModuleId.c_str(), cam->mSensorId.c_str());

    return OK;
}

/* the path of NVM device is in /sys/bus/i2c/devices/i2c-'adaptorId'/firmware_node/XXXX/path. */
void CameraParser::getNVMDirectory(CameraParser* profiles) {
    LOG2("@%s", __func__);

    std::string nvmPath("/sys/bus/i2c/devices/i2c-");
    // attach i2c adaptor id, like 18-0010
    std::size_t found = profiles->mI2CBus.find("-");
    CheckAndLogError(found == std::string::npos, VOID_VALUE, "Failed to get adaptor id");
    nvmPath += profiles->mI2CBus.substr(0, found);
    nvmPath += "/firmware_node/";
    DIR* dir = opendir(nvmPath.c_str());
    if (dir) {
        struct dirent* direntPtr = nullptr;
        while ((direntPtr = readdir(dir)) != nullptr) {
            if (direntPtr->d_type != DT_DIR) continue;

            std::string fwNodePath(nvmPath.c_str());
            fwNodePath += direntPtr->d_name;
            fwNodePath += "/path";

            bool found = false;
            FILE* fp = fopen(fwNodePath.c_str(), "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                int size = static_cast<int>(ftell(fp));
                fseek(fp, 0, SEEK_SET);
                std::unique_ptr<char[]> ptr(new char[size + 1]);
                size_t readSize = fread(ptr.get(), sizeof(char), size, fp);
                ptr[readSize] = 0;
                fclose(fp);

                if (readSize > 0) {
                    for (auto& nvm : profiles->mNvmDeviceInfo) {
                        if (strstr(ptr.get(), nvm.nodeName.c_str()) != nullptr) {
                            std::string nvmPath(NVM_DATA_PATH);
                            nvmPath.append("i2c-");
                            nvmPath.append(direntPtr->d_name);
                            nvmPath.append("/eeprom");
                            // Check if eeprom file exists
                            struct stat buf;
                            int ret = stat(nvmPath.c_str(), &buf);
                            LOG1("%s, nvmPath %s, ret %d", __func__, nvmPath.c_str(), ret);
                            if (ret == 0) {
                                nvm.directory = "i2c-";
                                nvm.directory += direntPtr->d_name;
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (found) break;
        }
        closedir(dir);
    } else {
        LOGE("Failed to open dir %s", nvmPath.c_str());
    }

    for (auto nvm : profiles->mNvmDeviceInfo) {
        if (!nvm.directory.empty()) {
            // The first one in list is prioritized and should be selected.
            std::string nvmPath;
            nvmPath.append(NVM_DATA_PATH);
            nvmPath.append(nvm.directory);
            if (nvmPath.back() != '/') nvmPath.append("/");

            nvmPath.append("eeprom");
            LOG2("NVM data is located in %s", nvmPath.c_str());
            profiles->pCurrentCam->mNvmDirectory = nvmPath;
            profiles->pCurrentCam->mMaxNvmDataSize = nvm.dataSize;
            int ret = getCameraModuleNameFromEEPROM(profiles->pCurrentCam);
            LOG2("NVM dir %s, ret %d", profiles->pCurrentCam->mNvmDirectory.c_str(), ret);
            break;
        } else {
            LOGE("Failed to find NVM directory");
        }
    }
}

/**
 * Get available sensors.
 *
 * The function will read libcamhal_profile.xml, and parse out all of sensors.
 * Then those sensors will be checked if it exists in mediaEntity, if it exists,
 * we put it in availableSensors.
 * In libcamhal_profile.xml it should have the following requirements:
 * 1. <availableSensors value="ov8856-wf-2,ov2740-uf-0,ov2740-wf-2"/>
 *     The value is "'camera name'-wf/uf-'CSI port number'".
 *     For example: camera name is "ov8856". Sensor's sink entity name is
 *      "Intel IPU6 CSI-2 2" or "Intel IPU6 CSI2 2" and it is word facing.
 *      The value is ov8856-wf-2.
 * 2. <platform value="IPU6"/> the platform value must be uppercase letter.
 *
 */
std::vector<std::string> CameraParser::getAvailableSensors(
    const std::string& ipuName, const std::vector<std::string>& sensorsList) {
    LOG2("@%s, ipuName:%s", __func__, ipuName.c_str());

    /* if the string doesn't contain -wf- or -uf-, it needn't be parsed */
    if ((sensorsList[0].find("-wf-") == string::npos) &&
        (sensorsList[0].find("-uf-") == string::npos)) {
        return sensorsList;
    }

    // sensor's sink entity name prefix:"Intel IPU6 CSI-2 2" or "Intel IPU6 CSI2 2"
    std::string sensorSinkName = "Intel ";
    sensorSinkName.append(ipuName);
    if (IPU6_UPSTREAM)
        sensorSinkName.append(" CSI2 ");
    else
        sensorSinkName.append(" CSI-2 ");

    std::vector<string> availableSensors;
    for (auto& sensor : sensorsList) {
        std::string srcSensor = sensor;
        std::string portNum = srcSensor.substr(srcSensor.find_last_of('-') + 1);
        std::string sensorSinkNameTmp = sensorSinkName;
        sensorSinkNameTmp.append(portNum);
        std::string sensorName = srcSensor.substr(0, srcSensor.find_first_of('-'));

        if (mMC && mMC->checkAvailableSensor(sensorName, sensorSinkNameTmp)) {
            AvailableSensorInfo sensorInfo = {sensorSinkNameTmp, false};
            availableSensors.push_back(srcSensor);
            mAvailableSensor[srcSensor] = sensorInfo;
            LOG2("@%s, The availabel sensor name:%s, sensorSinkNameTmp:%s", __func__,
                 srcSensor.c_str(), sensorSinkNameTmp.c_str());
        }
    }

#ifdef LINUX_BUILD
    // Return all the supported sensors list if detecting them isn't ready or
    // don't have permissions
    if (availableSensors.empty()) {
        LOG2("@%s, Detect sensor dynamically isn't supported, return all the list", __func__);
        return sensorsList;
    }
#endif

    return availableSensors;
}

/**
 * Get camera configuration from xml file
 *
 * The function will read the xml configuration file firstly.
 * Then it will parse out the camera settings.
 * The camera setting is stored inside this CameraParser class.
 *
 */
void CameraParser::getSensorDataFromXmlFile(void) {
    // According to sensor name to get sensor data
    LOG1("%s, available sensors: %zu", __func__, mStaticCfg->mCommonConfig.availableSensors.size());
    vector<string> allSensors = getAvailableSensors(mStaticCfg->mCommonConfig.ipuName,
                                                    mStaticCfg->mCommonConfig.availableSensors);

    if (allSensors.size() == 0) {
        LOGW("The style of libcamhal_profile is too old, please switch it as soon as possible !!!");
        return;
    }

    for (auto sensor : allSensors) {
        string sensorName = "sensors/";
        if ((sensor.find("-wf-") != string::npos) || (sensor.find("-uf-") != string::npos)) {
            sensorName.append(sensor.substr(0, sensor.find_last_of('-')));
        } else {
            sensorName.append(sensor);
        }
        sensorName.append(".xml");
        LOG1("%s: parse sensor name %s", __func__, sensorName.c_str());
        int ret = getDataFromXmlFile(sensorName);
        CheckAndLogError(ret != OK, VOID_VALUE, "Failed to get sensor profile data from %s",
                         sensorName.c_str());
    }
}

void CameraParser::dumpSensorInfo(void) {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(CameraParser), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("@%s, sensor number: %d ==================", __func__, getSensorNum());
    for (unsigned i = 0; i < getSensorNum(); i++) {
        LOG3("Dump for mCameras[%d].sensorName:%s, mISysFourcc:%d", i,
             mStaticCfg->mCameras[i].sensorName.c_str(), mStaticCfg->mCameras[i].mISysFourcc);

        stream_array_t supportedConfigs;
        mStaticCfg->mCameras[i].mCapability.getSupportedStreamConfig(supportedConfigs);
        for (size_t j = 0; j < supportedConfigs.size(); j++) {
            LOG3("    format:%d size(%dx%d) field:%d", supportedConfigs[j].format,
                 supportedConfigs[j].width, supportedConfigs[j].height, supportedConfigs[j].field);
        }

        for (unsigned j = 0; j < mStaticCfg->mCameras[i].mSupportedISysFormat.size(); j++) {
            LOG3("    mSupportedISysFormat:%d", mStaticCfg->mCameras[i].mSupportedISysFormat[j]);
        }

        // dump the media controller mapping table for supportedStreamConfig
        LOG3("    The media controller mapping table size: %zu",
             mStaticCfg->mCameras[i].mStreamToMcMap.size());
        for (auto& pool : mStaticCfg->mCameras[i].mStreamToMcMap) {
            int mcId = pool.first;
            stream_array_t& mcMapVector = pool.second;
            LOG3("    mcId: %d, the supportedStreamConfig size: %zu", mcId, mcMapVector.size());
        }

        // dump the media controller information
        LOG3("    Format Configuration:");
        for (unsigned j = 0; j < mStaticCfg->mCameras[i].mMediaCtlConfs.size(); j++) {
            const MediaCtlConf* mc = &mStaticCfg->mCameras[i].mMediaCtlConfs[j];
            for (unsigned k = 0; k < mc->links.size(); k++) {
                const McLink* link = &mc->links[k];
                LOG3("        link src %s [%d:%d] ==> %s [%d:%d] enable %d",
                     link->srcEntityName.c_str(), link->srcEntity, link->srcPad,
                     link->sinkEntityName.c_str(), link->sinkEntity, link->sinkPad, link->enable);
            }
            for (unsigned k = 0; k < mc->ctls.size(); k++) {
                const McCtl* ctl = &mc->ctls[k];
                LOG3("        Ctl %s [%d] cmd %s [0x%08x] value %d", ctl->entityName.c_str(),
                     ctl->entity, ctl->ctlName.c_str(), ctl->ctlCmd, ctl->ctlValue);
            }
            for (unsigned k = 0; k < mc->formats.size(); k++) {
                const McFormat* format = &mc->formats[k];
                if (format->formatType == FC_FORMAT)
                    LOG3("        format %s [%d:%d] [%dx%d] %s", format->entityName.c_str(),
                         format->entity, format->pad, format->width, format->height,
                         CameraUtils::pixelCode2String(format->pixelCode));
                else if (format->formatType == FC_SELECTION)
                    LOG3("        select %s [%d:%d] selCmd: %d [%d, %d] [%dx%d]",
                         format->entityName.c_str(), format->entity, format->pad, format->selCmd,
                         format->top, format->left, format->width, format->height);
            }
        }
    }

    LOG3("@%s, done ==================", __func__);
}

}  // namespace icamera
