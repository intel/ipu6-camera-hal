/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#define LOG_TAG AiqSetting

#include <algorithm>

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqSetting.h"
#include "PlatformData.h"
#include "ParameterHelper.h"

namespace icamera {

AiqSetting::AiqSetting(int cameraId) :
    mCameraId(cameraId) {
}

AiqSetting::~AiqSetting() {
}

int AiqSetting::init(void) {
    AutoWMutex wlock(mParamLock);

    mAiqParam.reset();

    camera_info_t info = {};
    PlatformData::getCameraInfo(mCameraId, info);
    info.capability->getAeCompensationRange(mAiqParam.evRange);
    info.capability->getAeCompensationStep(mAiqParam.evStep);
    info.capability->getLensInfoShadingMapSize(mAiqParam.lensShadingMapSize);
    info.capability->getLensMinFocusDistance(mAiqParam.minFocusDistance);

    return OK;
}

int AiqSetting::deinit(void) {
    return OK;
}

int AiqSetting::configure(const stream_config_t *streamList) {
    AutoWMutex wlock(mParamLock);

    camera_resolution_t resolution = {streamList->streams[0].width, streamList->streams[0].height};
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            resolution = {streamList->streams[i].width, streamList->streams[i].height};
            break;
        }
    }

    // Use pixel array as resolution to calculate region if it is set.
    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(mCameraId);
    if ((activePixelArray.right > activePixelArray.left) &&
        (activePixelArray.bottom > activePixelArray.top)) {
        resolution.width = activePixelArray.right - activePixelArray.left;
        resolution.height = activePixelArray.bottom - activePixelArray.top;
    }

    updateFrameUsage(streamList);

    mAiqParam.tuningMode = TUNING_MODE_MAX;
    mAiqParam.resolution = resolution;

    mTuningModes.clear();
    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);
    for (auto cfg : configModes) {
        TuningMode tuningMode;
        if (PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tuningMode) == OK) {
            mTuningModes.push_back(tuningMode);
        }
    }
    if (!mTuningModes.empty()) {
        mAiqParam.tuningMode = mTuningModes[0];
    }
    LOG1("%s, tuningMode %d, configMode %x, fame usage %d, res %dx%d", __func__,
         mAiqParam.tuningMode, configModes[0], mAiqParam.frameUsage,
         mAiqParam.resolution.width, mAiqParam.resolution.height);

    return OK;
}

void AiqSetting::updateFrameUsage(const stream_config_t *streamList) {
    bool preview = false, still = false, video = false;
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            video = true;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_STILL_CAPTURE) {
            still = true;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW
                   || streamList->streams[i].usage == CAMERA_STREAM_APP) {
            preview = true;
        }
    }

    mAiqParam.frameUsage = FRAME_USAGE_PREVIEW;
    if (video) {
        mAiqParam.frameUsage = FRAME_USAGE_VIDEO;
    } else if (preview && still) {
        mAiqParam.frameUsage = FRAME_USAGE_CONTINUOUS;
    } else if (still) {
        mAiqParam.frameUsage = FRAME_USAGE_STILL;
    }
}

int AiqSetting::setParameters(const Parameters& params) {
    AutoWMutex wlock(mParamLock);

    // Update AE related parameters
    params.getAeMode(mAiqParam.aeMode);
    params.getAeLock(mAiqParam.aeForceLock);
    params.getExposureTime(mAiqParam.manualExpTimeUs);
    params.getSensitivityGain(mAiqParam.manualGain);
    params.getSensitivityIso(mAiqParam.manualIso);
    params.getBlcAreaMode(mAiqParam.blcAreaMode);
    params.getAeRegions(mAiqParam.aeRegions);
    params.getAeConvergeSpeedMode(mAiqParam.aeConvergeSpeedMode);
    params.getAeConvergeSpeed(mAiqParam.aeConvergeSpeed);
    params.getRun3ACadence(mAiqParam.run3ACadence);
    if (mAiqParam.run3ACadence < 1) {
        LOGW("Invalid 3A cadence %d, use default 1.", mAiqParam.run3ACadence);
        mAiqParam.run3ACadence = 1;
    }
    params.getCallbackRgbs(&mAiqParam.callbackRgbs);
    params.getCallbackTmCurve(&mAiqParam.callbackTmCurve);

    int ev = 0;
    params.getAeCompensation(ev);
    if (mAiqParam.evStep.denominator == 0) {
        mAiqParam.evShift = 0.0;
    } else {
        ev = CLIP(ev, mAiqParam.evRange.max, mAiqParam.evRange.min);
        mAiqParam.evShift = static_cast<float>(ev) *
            mAiqParam.evStep.numerator / mAiqParam.evStep.denominator;
    }

    params.getFrameRate(mAiqParam.fps);
    params.getFpsRange(mAiqParam.aeFpsRange);
    params.getAntiBandingMode(mAiqParam.antibandingMode);
    // Update AWB related parameters
    params.getAwbMode(mAiqParam.awbMode);
    params.getAwbLock(mAiqParam.awbForceLock);
    params.getAwbCctRange(mAiqParam.cctRange);
    params.getAwbGains(mAiqParam.awbManualGain);
    params.getAwbWhitePoint(mAiqParam.whitePoint);
    params.getAwbGainShift(mAiqParam.awbGainShift);
    params.getColorTransform(mAiqParam.manualColorMatrix);
    params.getColorGains(mAiqParam.manualColorGains);
    params.getAwbConvergeSpeedMode(mAiqParam.awbConvergeSpeedMode);
    params.getAwbConvergeSpeed(mAiqParam.awbConvergeSpeed);

    // Update AF related parameters
    params.getAfMode(mAiqParam.afMode);
    params.getAfRegions(mAiqParam.afRegions);
    params.getAfTrigger(mAiqParam.afTrigger);

    params.getWeightGridMode(mAiqParam.weightGridMode);
    params.getSceneMode(mAiqParam.sceneMode);

    params.getAeDistributionPriority(mAiqParam.aeDistributionPriority);

    params.getWdrLevel(mAiqParam.ltmStrength);

    unsigned int length = sizeof(mAiqParam.customAicParam.data);
    if (params.getCustomAicParam(mAiqParam.customAicParam.data, &length) == OK) {
        mAiqParam.customAicParam.length = length;
    }

    params.getYuvColorRangeMode(mAiqParam.yuvColorRangeMode);

    params.getExposureTimeRange(mAiqParam.exposureTimeRange);
    params.getSensitivityGainRange(mAiqParam.sensitivityGainRange);

    params.getVideoStabilizationMode(mAiqParam.videoStabilizationMode);
    params.getLdcMode(mAiqParam.ldcMode);
    params.getRscMode(mAiqParam.rscMode);
    params.getFlipMode(mAiqParam.flipMode);
    params.getDigitalZoomRatio(mAiqParam.digitalZoomRatio);
    params.getTestPatternMode(mAiqParam.testPatternMode);

    int ret = params.getMakernoteMode(mAiqParam.makernoteMode);
    if (ret == NAME_NOT_FOUND) mAiqParam.makernoteMode = MAKERNOTE_MODE_OFF;

    const CameraMetadata& meta = ParameterHelper::getMetadata(params);
    uint32_t tag = CAMERA_LENS_INFO_MINIMUM_FOCUS_DISTANCE;
    icamera_metadata_ro_entry entry = meta.find(tag);
    if (entry.count == 1) {
        mAiqParam.minFocusDistance = entry.data.f[0];
    }

    params.getFocusDistance(mAiqParam.focusDistance);
    params.getShadingMode(mAiqParam.shadingMode);
    params.getLensShadingMapMode(mAiqParam.lensShadingMapMode);

    params.getTonemapMode(mAiqParam.tonemapMode);
    params.getTonemapPresetCurve(mAiqParam.tonemapPresetCurve);
    params.getTonemapGamma(mAiqParam.tonemapGamma);
    if (mAiqParam.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        camera_tonemap_curves_t curves;
        if (params.getTonemapCurves(curves) == OK) {
            CheckWarningNoReturn(curves.rSize > DEFAULT_TONEMAP_CURVE_POINT_NUM,
                                 "user r curve size is too big %d", curves.rSize);
            CheckWarningNoReturn(curves.gSize > DEFAULT_TONEMAP_CURVE_POINT_NUM,
                                 "user g curve size is too big %d", curves.gSize);
            CheckWarningNoReturn(curves.bSize > DEFAULT_TONEMAP_CURVE_POINT_NUM,
                                 "user v curve size is too big %d", curves.bSize);
            int curveSize = sizeof(float) * DEFAULT_TONEMAP_CURVE_POINT_NUM;
            MEMCPY_S(&mAiqParam.tonemapCurveMem[0], curveSize,
                     curves.rCurve, sizeof(float) * curves.rSize);
            MEMCPY_S(&mAiqParam.tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM], curveSize,
                     curves.gCurve, sizeof(float) * curves.gSize);
            MEMCPY_S(&mAiqParam.tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM * 2], curveSize,
                     curves.bCurve, sizeof(float) * curves.bSize);
            mAiqParam.tonemapCurves.rSize = std::min(DEFAULT_TONEMAP_CURVE_POINT_NUM, curves.rSize);
            mAiqParam.tonemapCurves.gSize = std::min(DEFAULT_TONEMAP_CURVE_POINT_NUM, curves.gSize);
            mAiqParam.tonemapCurves.bSize = std::min(DEFAULT_TONEMAP_CURVE_POINT_NUM, curves.bSize);
        }
    } else {
        mAiqParam.tonemapCurves.rSize = 0;
        mAiqParam.tonemapCurves.gSize = 0;
        mAiqParam.tonemapCurves.bSize = 0;
    }

    uint8_t captureIntent = 0;
    if (params.getCaptureIntent(captureIntent) == OK) {
        switch (captureIntent) {
        case CAMERA_CONTROL_CAPTUREINTENT_STILL_CAPTURE:
            mAiqParam.frameUsage = FRAME_USAGE_STILL;
            break;
        case CAMERA_CONTROL_CAPTUREINTENT_VIDEO_RECORD:
        case CAMERA_CONTROL_CAPTUREINTENT_VIDEO_SNAPSHOT:
            mAiqParam.frameUsage = FRAME_USAGE_VIDEO;
            break;
        case CAMERA_CONTROL_CAPTUREINTENT_PREVIEW:
            mAiqParam.frameUsage = FRAME_USAGE_PREVIEW;
            break;
        default:
            mAiqParam.frameUsage = FRAME_USAGE_CONTINUOUS;
            break;
        }
    }

    params.getPowerMode(mAiqParam.powerMode);
    params.getTotalExposureTarget(mAiqParam.totalExposureTarget);

    mAiqParam.dump();

    return OK;
}

int AiqSetting::getAiqParameter(aiq_parameter_t &param) {
    AutoRMutex rlock(mParamLock);

    param = mAiqParam;
    return OK;
}

// HDR_FEATURE_S
/* When multi-TuningModes supported in AUTO ConfigMode, TuningMode may be changed
   based on AE result. Current it only has HDR and ULL mode switching case,
   this maybe changed if more cases are supported. */
void AiqSetting::updateTuningMode(aec_scene_t aecScene) {
    if (!PlatformData::isEnableHDR(mCameraId)
        || mTuningModes.size() <= 1
        || mAiqParam.aeMode != AE_MODE_AUTO) {
        return;
    }

    TuningMode tuningMode = mAiqParam.tuningMode;
    if (aecScene == AEC_SCENE_HDR) {
        tuningMode = TUNING_MODE_VIDEO_HDR;
    } else if (aecScene == AEC_SCENE_ULL) {
        tuningMode = TUNING_MODE_VIDEO_ULL;
    }

    bool found = false;
    for (auto &tMode : mTuningModes) {
        // Check tuningMode if support or not
        if (tMode == tuningMode) {
            found = true;
            break;
        }
    }
    if (!found) {
        LOG1("%s, new tuningMode %d isn't supported", __func__, tuningMode);
        return;
    }
}
// HDR_FEATURE_E

void aiq_parameter_t::reset() {
    frameUsage = FRAME_USAGE_VIDEO;
    aeMode = AE_MODE_AUTO;
    aeForceLock = false;
    awbMode = AWB_MODE_AUTO;
    awbForceLock = false;
    afMode = AF_MODE_AUTO;
    afTrigger = AF_TRIGGER_IDLE;
    sceneMode = SCENE_MODE_AUTO;
    manualExpTimeUs = -1;
    manualGain = -1;
    manualIso = 0;
    evShift = 0;
    evStep = {1, 3};
    evRange = {-6, 6};
    fps = 0;
    aeFpsRange = { 0.0, 0.0 };
    antibandingMode = ANTIBANDING_MODE_AUTO;
    cctRange = { 0, 0 };
    whitePoint = { 0, 0 };
    awbManualGain = { 0, 0, 0 };
    awbGainShift = { 0, 0, 0 };
    CLEAR(manualColorMatrix);
    CLEAR(manualColorGains);
    aeRegions.clear();
    blcAreaMode = BLC_AREA_MODE_OFF;
    aeConvergeSpeedMode = CONVERGE_SPEED_MODE_AIQ;
    awbConvergeSpeedMode = CONVERGE_SPEED_MODE_AIQ;
    aeConvergeSpeed = CONVERGE_NORMAL;
    awbConvergeSpeed = CONVERGE_NORMAL;
    run3ACadence = 1;
    ltmStrength = 0;
    weightGridMode = WEIGHT_GRID_AUTO;
    aeDistributionPriority = DISTRIBUTION_AUTO;
    CLEAR(customAicParam);
    yuvColorRangeMode = CAMERA_FULL_MODE_YUV_COLOR_RANGE;
    exposureTimeRange.min = -1;
    exposureTimeRange.max = -1;
    sensitivityGainRange.min = -1;
    sensitivityGainRange.max = -1;
    videoStabilizationMode = VIDEO_STABILIZATION_MODE_OFF;
    tuningMode = TUNING_MODE_MAX;
    ldcMode = LDC_MODE_OFF;
    rscMode = RSC_MODE_OFF;
    flipMode = FLIP_MODE_NONE;
    digitalZoomRatio = 1.0f;

    lensPosition = 0;
    lensMovementStartTimestamp = 0;
    makernoteMode = MAKERNOTE_MODE_OFF;
    minFocusDistance = 0.0f;
    focusDistance = 0.0f;
    shadingMode = SHADING_MODE_FAST;
    lensShadingMapMode = LENS_SHADING_MAP_MODE_OFF;
    lensShadingMapSize = {0, 0};
    testPatternMode = TEST_PATTERN_OFF;

    tonemapMode = TONEMAP_MODE_FAST;
    tonemapPresetCurve = TONEMAP_PRESET_CURVE_SRGB;
    tonemapGamma = 0.0f;
    tonemapCurves.rSize = 0;
    tonemapCurves.gSize = 0;
    tonemapCurves.bSize = 0;
    tonemapCurves.rCurve = &tonemapCurveMem[0];
    tonemapCurves.gCurve = &tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM];
    tonemapCurves.bCurve = &tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM * 2];
    callbackRgbs = false;
    callbackTmCurve = false;

    powerMode = CAMERA_HIGH_QUALITY;
    totalExposureTarget = 0;

    CLEAR(resolution);
}

void aiq_parameter_t::dump() {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(AiqSetting), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("Application parameters:");
    LOG3("3A mode: ae %d, awb %d, af %d, scene %d", aeMode, awbMode, afMode, sceneMode);
    LOG3("lock: ae %d, awb %d, af trigger:%d", aeForceLock, awbForceLock, afTrigger);
    LOG3("converge speed mode: ae %d, awb %d", aeConvergeSpeedMode, awbConvergeSpeedMode);
    LOG3("converge speed: ae %d, awb %d", aeConvergeSpeed, awbConvergeSpeed);

    LOG3("EV:%f, range (%f-%f), step %d/%d", evShift, evRange.min, evRange.max,
         evStep.numerator, evStep.denominator);
    LOG3("manualExpTimeUs:%ld, time range (%f-%f)", manualExpTimeUs,
         exposureTimeRange.min, exposureTimeRange.max);
    LOG3("manualGain %f, manualIso %d, gain range (%f-%f)", manualGain, manualIso,
         sensitivityGainRange.min, sensitivityGainRange.max);
    LOG3("FPS %f, range (%f-%f)", fps, aeFpsRange.min, aeFpsRange.max);
    for (auto &region : aeRegions) {
        LOG3("ae region (%d, %d, %d, %d, %d)",
             region.left, region.top, region.right, region.bottom, region.weight);
    }
    LOG3("Antibanding mode:%d", antibandingMode);
    LOG3("AE Distribution Priority:%d", aeDistributionPriority);

    LOG3("cctRange:(%f-%f)", cctRange.min, cctRange.max);
    LOG3("manual awb: white point:(%d,%d)", whitePoint.x, whitePoint.y);
    LOG3("manual awb gain:(%d,%d,%d), gain shift:(%d,%d,%d)",
         awbManualGain.r_gain, awbManualGain.g_gain, awbManualGain.b_gain,
         awbGainShift.r_gain, awbGainShift.g_gain, awbGainShift.b_gain);
    for (int i = 0; i < 3; i++) {
        LOG3("manual color matrix: [%.3f %.3f %.3f]",
             manualColorMatrix.color_transform[i][0],
             manualColorMatrix.color_transform[i][1],
             manualColorMatrix.color_transform[i][2]);
    }
    LOG3("manual color gains in rggb:(%.3f,%.3f,%.3f,%.3f)",
         manualColorGains.color_gains_rggb[0], manualColorGains.color_gains_rggb[1],
         manualColorGains.color_gains_rggb[2], manualColorGains.color_gains_rggb[3]);

    for (auto &region : afRegions) {
        LOG3("af region (%d, %d, %d, %d, %d)",
             region.left, region.top, region.right, region.bottom, region.weight);
    }
    LOG3("manual focus distance: %f, min focus distance: %f", focusDistance, minFocusDistance);
    LOG3("Focus position %d, start timestamp %llu", lensPosition, lensMovementStartTimestamp);

    LOG3("digitalZoomRatio %f", digitalZoomRatio);

    LOG3("custom AIC parameter length:%u", customAicParam.length);
    if (customAicParam.length > 0) {
        LOG3("custom AIC parameter data:%s", customAicParam.data);
    }
    if (tuningMode != TUNING_MODE_MAX) {
        LOG3("camera mode:%d", tuningMode);
    }
    LOG3("blc area mode:%d", blcAreaMode);
    LOG3("ltm strength:(%u)", ltmStrength);
    LOG3("weight grid mode:%d", weightGridMode);
    LOG3("Yuv Color Range Mode:%d", yuvColorRangeMode);
    LOG3("DVS mode %d", videoStabilizationMode);

    LOG3("makernoteMode %d", makernoteMode);
    LOG3("shadingMode %d, lensShadingMapMode %d, size %dx%d", shadingMode,
         lensShadingMapMode, lensShadingMapSize.x, lensShadingMapSize.y);

    LOG3("ldcMode %d, rscMode %d, flipMode %d", ldcMode, ldcMode, flipMode);

    LOG3("run3ACadence %d", run3ACadence);
    LOG3("tonemap mode %d, preset curve %d, gamma %f, curve points %d",
          tonemapMode, tonemapPresetCurve, tonemapGamma, tonemapCurves.gSize);
    LOG3("testPatternMode %d", testPatternMode);
    LOG3("power mode %d", powerMode);
    LOG3("totalExposureTarget %ld", totalExposureTarget);

    LOG3("callback RGBS stats %s", callbackRgbs ? "true" : "false");
    LOG3("callback Tonemap curve: %s", callbackTmCurve ? "true" : "false");
}

} /* namespace icamera */
