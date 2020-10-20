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

#define LOG_TAG "AiqSetting"

#include <algorithm>

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqSetting.h"
#include "PlatformData.h"
#include "ParameterHelper.h"

namespace icamera {

AiqSetting::AiqSetting(int cameraId) :
    mCameraId(cameraId),
    mPipeSwitchFrameCount(0)
{
    LOG3A("@%s", __func__);
}

AiqSetting::~AiqSetting()
{
    LOG3A("@%s", __func__);
}

int AiqSetting::init(void)
{
    LOG3A("@%s", __func__);
    AutoWMutex wlock(mParamLock);

    mPipeSwitchFrameCount = 0;

    mAiqParam.reset();

    camera_info_t info = {};
    PlatformData::getCameraInfo(mCameraId, info);
    info.capability->getAeCompensationRange(mAiqParam.evRange);
    info.capability->getAeCompensationStep(mAiqParam.evStep);
    info.capability->getLensInfoShadingMapSize(mAiqParam.lensShadingMapSize);
    info.capability->getLensMinFocusDistance(mAiqParam.minFocusDistance);

    return OK;
}

int AiqSetting::deinit(void)
{
    LOG3A("@%s", __func__);
    AutoWMutex wlock(mParamLock);

    return OK;
}

int AiqSetting::configure(const stream_config_t *streamList)
{
    LOG3A("@%s", __func__);
    AutoWMutex wlock(mParamLock);

    camera_resolution_t resolution = {streamList->streams[0].width, streamList->streams[0].height};
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            resolution = {streamList->streams[i].width, streamList->streams[i].height};
            break;
        }
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
    LOG3A("%s, tuningMode %d, configMode %x", __func__, mAiqParam.tuningMode, configModes[0]);

    return OK;
}

void AiqSetting::updateFrameUsage(const stream_config_t *streamList)
{
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

int AiqSetting::setParameters(const Parameters& params)
{
    LOG3A("@%s", __func__);
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

    int ret = params.getMakernoteMode(mAiqParam.makernoteMode);
    if (ret == NAME_NOT_FOUND) mAiqParam.makernoteMode = MAKERNOTE_MODE_OFF;

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

    mAiqParam.dump();

    return OK;
}

int AiqSetting::getAiqParameter(aiq_parameter_t &param)
{
    LOG3A("@%s", __func__);
    AutoRMutex rlock(mParamLock);

    param = mAiqParam;
    return OK;
}

void aiq_parameter_t::reset()
{
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

    tonemapMode = TONEMAP_MODE_FAST;
    tonemapPresetCurve = TONEMAP_PRESET_CURVE_SRGB;
    tonemapGamma = 0.0f;
    tonemapCurves.rSize = 0;
    tonemapCurves.gSize = 0;
    tonemapCurves.bSize = 0;
    tonemapCurves.rCurve = &tonemapCurveMem[0];
    tonemapCurves.gCurve = &tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM];
    tonemapCurves.bCurve = &tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM * 2];

    CLEAR(resolution);
}

void aiq_parameter_t::dump()
{
    // Log only printed when 3a log enabled.
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return;

    LOG3A("Application parameters:");
    LOG3A("frame usage mode %d", frameUsage);
    LOG3A("ae mode:%d, awb mode:%d, af mode:%d, scene mode:%d", aeMode, awbMode, afMode, sceneMode);
    LOG3A("ae lock:%d, awb lock:%d, af trigger:%d", aeForceLock, awbForceLock, afTrigger);
    LOG3A("EV:%f, manualExpTimeUs:%ld, manualGain:%f, manualIso %d",
          evShift, manualExpTimeUs, manualGain, manualIso);
    LOG3A("FPS:%f", fps);
    LOG3A("FPS range:(%f-%f)", aeFpsRange.min, aeFpsRange.max);
    LOG3A("Antibanding mode:%d", antibandingMode);
    LOG3A("cctRange:(%f-%f)", cctRange.min, cctRange.max);
    LOG3A("manual white point:(%d,%d)", whitePoint.x, whitePoint.y);
    LOG3A("manual awb gain:(%d,%d,%d)", awbManualGain.r_gain, awbManualGain.g_gain, awbManualGain.b_gain);
    LOG3A("manual awb gain shift:(%d,%d,%d)", awbGainShift.r_gain, awbGainShift.g_gain, awbGainShift.b_gain);
    for (int i = 0; i < 3; i++) {
        LOG3A("manual color matrix:  [%.3f %.3f %.3f]",
            manualColorMatrix.color_transform[i][0],
            manualColorMatrix.color_transform[i][1],
            manualColorMatrix.color_transform[i][2]);
    }
    LOG3A("manual color gains in rggb:(%.3f,%.3f,%.3f,%.3f)",
        manualColorGains.color_gains_rggb[0], manualColorGains.color_gains_rggb[1],
        manualColorGains.color_gains_rggb[2], manualColorGains.color_gains_rggb[3]);
    LOG3A("ae region size:%zu, blc area mode:%d", aeRegions.size(), blcAreaMode);
    for (auto &region : aeRegions) {
        LOG3A("ae region (%d, %d, %d, %d, %d)",
            region.left, region.top, region.right, region.bottom, region.weight);
    }
    LOG3A("af region size:%zu", aeRegions.size());
    for (auto &region : afRegions) {
        LOG3A("af region (%d, %d, %d, %d, %d)",
            region.left, region.top, region.right, region.bottom, region.weight);
    }
    LOG3A("manual focus distance: %f, min focus distance: %f", focusDistance, minFocusDistance);

    LOG3A("ae converge speed mode:(%d) awb converge speed mode:(%d)", aeConvergeSpeedMode, awbConvergeSpeedMode);
    LOG3A("ae converge speed:(%d) awb converge speed:(%d)", aeConvergeSpeed, awbConvergeSpeed);
    LOG3A("custom AIC parameter length:%d", customAicParam.length);
    if (customAicParam.length > 0) {
        LOG3A("custom AIC parameter data:%s", customAicParam.data);
    }
    if (tuningMode != TUNING_MODE_MAX) {
        LOG3A("camera mode:%d", tuningMode);
    }
    LOG3A("ltm strength:(%d)", ltmStrength);
    LOG3A("weight grid mode:%d", weightGridMode);
    LOG3A("AE Distribution Priority:%d", aeDistributionPriority);
    LOG3A("Yuv Color Range Mode:%d", yuvColorRangeMode);
    LOG3A("AE exposure time range, min %f, max %f", exposureTimeRange.min, exposureTimeRange.max);
    LOG3A("AE sensitivity gain range, min %.2f, max %.2f", sensitivityGainRange.min, sensitivityGainRange.max);
    LOG3A("DVS mode %d", videoStabilizationMode);

    LOG3A("Focus position %d, start timestamp %llu", lensPosition, lensMovementStartTimestamp);
    LOG3A("makernoteMode %d", makernoteMode);
    LOG3A("shadingMode %d", shadingMode);
    LOG3A("lensShadingMapMode %d", lensShadingMapMode);
    LOG3A("lensShadingMapSize x:%d, y:%d", lensShadingMapSize.x, lensShadingMapSize.y);

    LOG3A("tonemap mode %d, preset curve %d, gamma %f, curve points %d",
          tonemapMode, tonemapPresetCurve, tonemapGamma, tonemapCurves.gSize);
}

} /* namespace icamera */
