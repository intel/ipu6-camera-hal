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

#define LOG_TAG Intel3AParameter

#include "Intel3AParameter.h"

#include <algorithm>
#include <climits>
#include <cmath>

#include "AiqUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

namespace icamera {

#define VALID_COLOR_GAINS(colorGains) \
    (colorGains[0] > 0 && colorGains[1] > 0 && colorGains[2] > 0 && colorGains[3] > 0)

Intel3AParameter::Intel3AParameter(int cameraId)
        : mCameraId(cameraId),
          mTestPatternMode(TEST_PATTERN_OFF),
          mUseManualAwbGain(false),
          mUseManualColorMatrix(false),
          mWeightGridMode(WEIGHT_GRID_AUTO),
          mAePerTicks(1),
          mAwbPerTicks(1),
          mAfForceLock(false),
          mManualFocusDistance(0.0),
          mAeMode(AE_MODE_AUTO),
          mDuringAfTriggerScan(false) {
    CLEAR(mCMC);
    CLEAR(mAeParams);
    CLEAR(mAfParams);
    CLEAR(mAwbParams);

    CLEAR(mColorMatrix);
    CLEAR(mColorGains);
    CLEAR(mManualGains);
    CLEAR(mAwbGainShift);

    mAfTrigger = AF_TRIGGER_IDLE;
    mAfMode = AF_MODE_OFF;

    CLEAR(mSensitivityRange);
    camera_info_t info = {};
    PlatformData::getCameraInfo(mCameraId, info);
    info.capability->getSupportedSensorSensitivityRange(mSensitivityRange);
}

Intel3AParameter::~Intel3AParameter() {}

int Intel3AParameter::init() {
    // set default params
    initAeParameter();
    initAfParameter();
    initAwbParameter();
    mTestPatternMode = TEST_PATTERN_OFF;

    mWeightGridMode = WEIGHT_GRID_AUTO;
    mAePerTicks = 1;
    mAwbPerTicks = 1;

    mUseManualColorMatrix = false;
    CLEAR(mColorMatrix);
    CLEAR(mColorGains);

    mAfMode = AF_MODE_AUTO;
    mAfForceLock = false;
    mManualFocusDistance = 0.0;
    mAeMode = AE_MODE_AUTO;
    mAfTrigger = AF_TRIGGER_IDLE;
    mDuringAfTriggerScan = false;
    return OK;
}

int Intel3AParameter::getCMCInfo(TuningMode tuningMode) {
    IntelCca* intelCca = IntelCca::getInstance(mCameraId, tuningMode);
    CheckAndLogError(!intelCca, BAD_VALUE, "cca is nullptr, mode:%d", tuningMode);

    int ret = intelCca->getCMC(&mCMC);
    CheckAndLogError(ret != OK, ret, "failed to load CMC info");

    return OK;
}

void Intel3AParameter::initAeParameter() {
    mAeParams.is_bypass = false;
    mAeParams.num_exposures = 1;
    mAeParams.frame_use = ia_aiq_frame_use_video;
    mAeParams.flash_mode = ia_aiq_flash_mode_off;
    mAeParams.metering_mode = ia_aiq_ae_metering_mode_evaluative;
    mAeParams.priority_mode = ia_aiq_ae_priority_mode_normal;
    mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_auto;
    mAeParams.ev_shift = 0.0f;
    memset(mAeParams.manual_exposure_time_us, 0, sizeof(mAeParams.manual_exposure_time_us));
    memset(mAeParams.manual_analog_gain, 0, sizeof(mAeParams.manual_analog_gain));
    memset(mAeParams.manual_iso, 0, sizeof(mAeParams.manual_iso));
    mAeParams.num_sensor_descriptors = 1;

    CLEAR(mAeParams.exposure_window);
    CLEAR(mAeParams.exposure_coordinate);
    mAeParams.manual_convergence_time = -1.0;
    mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_auto;
}

void Intel3AParameter::initAfParameter() {
    mAfParams.frame_use = ia_aiq_frame_use_video;
    mAfParams.lens_position = 0;
    mAfParams.lens_movement_start_timestamp = 0;
    mAfParams.focus_mode = ia_aiq_af_operation_mode_infinity;
    mAfParams.flash_mode = ia_aiq_flash_mode_off;
    mAfParams.focus_rect.left = 0;
    mAfParams.focus_rect.top = 0;
    mAfParams.focus_rect.right = 0;
    mAfParams.focus_rect.bottom = 0;
    mAfParams.manual_focus_parameters.manual_focus_action = ia_aiq_manual_focus_action_none;
    mAfParams.manual_focus_parameters.manual_focus_distance = MAX_FOCUS_DISTANCE;
    mAfParams.manual_focus_parameters.manual_lens_position = 0;
    mAfParams.trigger_new_search = false;
}

void Intel3AParameter::initAwbParameter() {
    mAwbParams.is_bypass = false;
    mAwbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
    mAwbParams.manual_convergence_time = -1;

    mUseManualAwbGain = false;
    CLEAR(mManualGains);
    CLEAR(mAwbGainShift);
}

int Intel3AParameter::setSensorInfo(ia_aiq_exposure_sensor_descriptor descriptor) {
    ia_aiq_exposure_sensor_descriptor* desc = &mAeParams.sensor_descriptor[0];
    desc->pixel_clock_freq_mhz = descriptor.pixel_clock_freq_mhz;
    desc->pixel_periods_per_line = descriptor.pixel_periods_per_line;
    desc->line_periods_per_field = descriptor.line_periods_per_field;
    desc->line_periods_vertical_blanking = descriptor.line_periods_vertical_blanking;
    desc->fine_integration_time_min = descriptor.fine_integration_time_min;
    desc->fine_integration_time_max_margin = descriptor.fine_integration_time_max_margin;
    desc->coarse_integration_time_min = descriptor.coarse_integration_time_min;
    desc->coarse_integration_time_max_margin = descriptor.coarse_integration_time_max_margin;

    return OK;
}

int Intel3AParameter::updateParameter(aiq_parameter_t param) {
    updateAeParameter(param);
    updateAwbParameter(param);
    updateAfParameter(param);
    mTestPatternMode = param.testPatternMode;

    dumpParameter();

    return OK;
}

/**
 * Override ae result by those settings provided by application
 */
void Intel3AParameter::updateAeResult(cca::cca_ae_results* aeResult) {
    CheckAndLogError(!aeResult, VOID_VALUE, "Invalid aeResult");

}

float Intel3AParameter::convertdBGainToISO(float sensitivityGain, int baseIso) {
    // Convert db Gain to ISO
    float manualGain = pow(10, (sensitivityGain / 20));
    manualGain *= baseIso;
    return manualGain;
}

void Intel3AParameter::setAeManualLimits(const aiq_parameter_t& param) {
    // HDR_FEATURE_S
    if (PlatformData::getSensorAeEnable(mCameraId)) {
        LOG2("@%s, fix 2 auto exposure time to 11ms", __func__);
        mAeParams.manual_exposure_time_us[0] = 11000;
        mAeParams.manual_exposure_time_us[1] = 11000;
        return;
    }
    // HDR_FEATURE_E

    ia_aiq_ae_manual_limits* limit = &mAeParams.manual_limits[0];

    limit->manual_exposure_time_max = -1;
    limit->manual_exposure_time_min = -1;
    limit->manual_iso_min = -1;
    limit->manual_iso_max = -1;

    if (param.aeFpsRange.min > 0.01 && param.aeFpsRange.max >= param.aeFpsRange.min) {
        limit->manual_frame_time_us_max = 1000000 / param.aeFpsRange.min;
        limit->manual_frame_time_us_min = 1000000 / param.aeFpsRange.max;
    } else if (param.fps > 0.01) {
        limit->manual_frame_time_us_max = 1000000 / param.fps;
        limit->manual_frame_time_us_min = 1000000 / param.fps;
    }

    camera_range_t range = {-1, -1};
    int ret = PlatformData::getSupportAeExposureTimeRange(mCameraId, param.sceneMode, range);
    if (param.exposureTimeRange.min > 0 &&
        param.exposureTimeRange.max >= param.exposureTimeRange.min) {
        if (ret == OK) {
            range.min = CLIP(param.exposureTimeRange.min, range.max, range.min);
            range.max = CLIP(param.exposureTimeRange.max, range.max, range.min);
        } else {
            range.min = param.exposureTimeRange.min;
            range.max = param.exposureTimeRange.max;
        }
    }
    limit->manual_exposure_time_min = range.min;
    limit->manual_exposure_time_max = range.max;

    camera_range_t gainRange = {-1, -1};
    ret = PlatformData::getSupportAeGainRange(mCameraId, param.sceneMode, gainRange);
    if (param.sensitivityGainRange.min >= 0 &&
        param.sensitivityGainRange.max >= param.sensitivityGainRange.min) {
        if (ret == OK) {
            gainRange.min = CLIP(param.sensitivityGainRange.min, gainRange.max, gainRange.min);
            gainRange.max = CLIP(param.sensitivityGainRange.max, gainRange.max, gainRange.min);
        } else {
            gainRange.min = param.sensitivityGainRange.min;
            gainRange.max = param.sensitivityGainRange.max;
        }
    }

    if (gainRange.min >= 0 && gainRange.max >= gainRange.min) {
        float isoMin = convertdBGainToISO(gainRange.min, mCMC.base_iso);
        float isoMax = convertdBGainToISO(gainRange.max, mCMC.base_iso);
        // Cast these to doubles for this comparison, as float(INT_MAX) produces
        // the value 2147483648, which is INT_MAX+1. Clang warns about this.
        if (static_cast<double>(isoMin) <= INT_MAX &&
            static_cast<double>(isoMax) <= INT_MAX) {
            limit->manual_iso_min = static_cast<int>(isoMin);
            limit->manual_iso_max = static_cast<int>(isoMax);
        }
    }
}

void Intel3AParameter::setManualExposure(const aiq_parameter_t& param) {
    int64_t manualExpTimeUs = param.manualExpTimeUs;
    if (manualExpTimeUs <= 0 || param.aeDistributionPriority == DISTRIBUTION_ISO) {
        return;
    }

    camera_range_t range = {};
    if (PlatformData::getSupportAeExposureTimeRange(mCameraId, param.sceneMode, range) == OK) {
        manualExpTimeUs = CLIP(manualExpTimeUs, range.max, range.min);
    }

    for (unsigned int i = 0; i < mAeParams.num_exposures - 1; i++) {
        mAeParams.manual_exposure_time_us[i] = -1;
    }
    mAeParams.manual_exposure_time_us[mAeParams.num_exposures - 1] = manualExpTimeUs;
}

void Intel3AParameter::setManualGain(const aiq_parameter_t& param) {
    float manualGain = param.manualGain;
    if (manualGain < 0 || param.aeDistributionPriority == DISTRIBUTION_SHUTTER) {
        return;
    }

    camera_range_t gainRange = {};
    if (PlatformData::getSupportAeGainRange(mCameraId, param.sceneMode, gainRange) == OK) {
        manualGain = CLIP(manualGain, gainRange.max, gainRange.min);
    }

    // Convert db to sensor analog gain.
    for (unsigned int i = 0; i < mAeParams.num_exposures; i++) {
        mAeParams.manual_analog_gain[i] = pow(10, (manualGain / 20));
    }
}

void Intel3AParameter::setManualIso(const aiq_parameter_t& param) {
    int32_t manualIso = param.manualIso;
    if (manualIso <= 0 || param.aeDistributionPriority == DISTRIBUTION_SHUTTER) {
        return;
    }

    SensitivityRange range;
    if (PlatformData::getSensitivityRangeByTuningMode(mCameraId, param.tuningMode, range) == OK) {
        float ratio =
            (manualIso - mSensitivityRange.min) / (mSensitivityRange.max - mSensitivityRange.min);
        manualIso = range.min + ratio * (range.max - range.min);
        manualIso = CLIP(manualIso, range.max, range.min);

        LOG2("%s, param.manualIso %d, manualIso %d", __func__, param.manualIso, manualIso);
    }

    // Will overwrite manual_analog_gain
    for (unsigned int i = 0; i < mAeParams.num_exposures; i++) {
        mAeParams.manual_iso[i] = manualIso;
    }
}

void Intel3AParameter::updateAeParameter(const aiq_parameter_t& param) {
    mAeMode = param.aeMode;
    mAeParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);
    mAeParams.num_exposures = PlatformData::getExposureNum(
        mCameraId, PlatformData::isMultiExposureCase(mCameraId, param.tuningMode));

    setAeManualLimits(param);

    switch (param.antibandingMode) {
        case ANTIBANDING_MODE_AUTO:
            mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_auto;
            break;
        case ANTIBANDING_MODE_50HZ:
            mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_50hz;
            break;
        case ANTIBANDING_MODE_60HZ:
            mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_60hz;
            break;
        case ANTIBANDING_MODE_OFF:
            mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_off;
            break;
    }

    switch (param.aeDistributionPriority) {
        case DISTRIBUTION_AUTO:
            mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_auto;
            break;
        case DISTRIBUTION_SHUTTER:
            mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_shutter;
            break;
        case DISTRIBUTION_ISO:
            mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_iso;
            break;
        case DISTRIBUTION_APERTURE:
            mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_aperture;
            break;
        default:
            mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_auto;
            break;
    }

    memset(mAeParams.manual_exposure_time_us, 0, sizeof(mAeParams.manual_exposure_time_us));
    memset(mAeParams.manual_analog_gain, 0, sizeof(mAeParams.manual_analog_gain));
    memset(mAeParams.manual_iso, 0, sizeof(mAeParams.manual_iso));

    if (param.aeMode == AE_MODE_MANUAL) {
        setManualGain(param);
        setManualIso(param);
        setManualExposure(param);
    } else {
        mAeParams.ev_shift = param.evShift;
    }

    if (param.aeConvergeSpeedMode == CONVERGE_SPEED_MODE_AIQ) {
        mAePerTicks = 1;

        mAeParams.manual_convergence_time = AiqUtils::convertSpeedModeToTime(param.aeConvergeSpeed);
        // HDR_FEATURE_S
        if (PlatformData::isMultiExposureCase(mCameraId, param.tuningMode)) {
            mAeParams.manual_convergence_time =
                AiqUtils::convertSpeedModeToTimeForHDR(param.aeConvergeSpeed);
        }
        // HDR_FEATURE_E
    } else {
        mAeParams.manual_convergence_time = -1;

        /*
         * The unit of mAePerTicks is frame count, 3 means 3 frames.
         * The default value can be changed based on customer requirement.
         */
        switch (param.aeConvergeSpeed) {
            case CONVERGE_MID:
                mAePerTicks = 30;
                break;
            case CONVERGE_LOW:
                mAePerTicks = 60;
                break;
            case CONVERGE_NORMAL:
            default:
                mAePerTicks = 1;
                break;
        }
    }

    CLEAR(mAeParams.exposure_coordinate);
    if (param.blcAreaMode == BLC_AREA_MODE_ON && !param.aeRegions.empty()) {
        // Current only one AE metering window is supported, so use the latest one
        camera_window_t window = param.aeRegions.back();

        if (window.right > window.left && window.bottom > window.top) {
            if ((window.right - window.left) != param.resolution.width &&
                (window.bottom - window.top) != param.resolution.height) {
                camera_coordinate_t coordinate = {};
                coordinate.x = window.left + (window.right - window.left) / 2;
                coordinate.y = window.top + (window.bottom - window.top) / 2;
                camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width,
                                                         param.resolution.height};

                coordinate = AiqUtils::convertToIaCoordinate(frameCoord, coordinate);
                mAeParams.exposure_coordinate.x = coordinate.x;
                mAeParams.exposure_coordinate.y = coordinate.y;
            }
        }
    }

    CLEAR(mAeParams.manual_total_target_exposure);
    // Ignore TET in manual exposure case
    if (param.totalExposureTarget > 0 && param.manualExpTimeUs <= 0 && param.manualIso <= 0) {
        camera_range_t range = {-1, -1};
        int ret = PlatformData::getSupportAeExposureTimeRange(mCameraId, param.sceneMode, range);
        int64_t tet = param.totalExposureTarget;
        if (ret == OK && mCMC.base_iso > 0) {
            int64_t max = static_cast<int64_t>(range.max * mSensitivityRange.max / mCMC.base_iso);
            int64_t min = static_cast<int64_t>(range.min * mSensitivityRange.min / mCMC.base_iso);
            tet = CLIP(tet, max, min);
        }
        // Will overwrite total exposure
        for (unsigned int i = 0; i < mAeParams.num_exposures; i++) {
            mAeParams.manual_total_target_exposure[i] = tet;
        }
    }
}

void Intel3AParameter::updatePaResult(cca::cca_pa_params* paResult, bool awbLocked,
                                      const camera_color_gains_t& colorGains,
                                      const camera_color_transform_t& colorMatrix) {
    CheckAndLogError((paResult == nullptr), VOID_VALUE, "No Pa result provided.");

    paResult->enable_manual_settings = false;
    if (mUseManualColorMatrix) {
        if (VALID_COLOR_GAINS(mColorGains.color_gains_rggb)) {
            paResult->color_gains.r = mColorGains.color_gains_rggb[0];
            paResult->color_gains.gr = mColorGains.color_gains_rggb[1];
            paResult->color_gains.gb = mColorGains.color_gains_rggb[2];
            paResult->color_gains.b = mColorGains.color_gains_rggb[3];
        }

        // Override color_conversion_matrix and color_gains
        // when application requires manual color transform.
        MEMCPY_S(&paResult->color_conversion_matrix, sizeof(paResult->color_conversion_matrix),
                 &mColorMatrix.color_transform, sizeof(mColorMatrix.color_transform));

        paResult->enable_manual_settings = true;
    } else if (awbLocked) {
        paResult->color_gains.r = colorGains.color_gains_rggb[0];
        paResult->color_gains.gr = colorGains.color_gains_rggb[1];
        paResult->color_gains.gb = colorGains.color_gains_rggb[2];
        paResult->color_gains.b = colorGains.color_gains_rggb[3];

        MEMCPY_S(&paResult->color_conversion_matrix, sizeof(paResult->color_conversion_matrix),
                 &colorMatrix.color_transform, sizeof(colorMatrix.color_transform));

        paResult->enable_manual_settings = true;
    }
}

/**
 * Override awb result by AWB gains or gain shift provided by application
 */
void Intel3AParameter::updateAwbResult(cca::cca_awb_results* awbResult) {
    CheckAndLogError((awbResult == nullptr), VOID_VALUE, "No Awb result provided.");

    camera_awb_gains_t gains = {};
    if (mUseManualColorMatrix && VALID_COLOR_GAINS(mColorGains.color_gains_rggb)) {
        // According to color gain value to update the
        // awb result in manual color transform mode
        float maxChroma = std::max(static_cast<float>(1.0), mColorGains.color_gains_rggb[1]);
        awbResult->accurate_r_per_g =
            static_cast<float>(maxChroma / mColorGains.color_gains_rggb[0]);
        awbResult->accurate_b_per_g =
            static_cast<float>(maxChroma / mColorGains.color_gains_rggb[3]);
    } else {
        camera_awb_gains_t& gains = mUseManualAwbGain ? mManualGains : mAwbGainShift;
        float normalizedR = AiqUtils::normalizeAwbGain(gains.r_gain);
        float normalizedG = AiqUtils::normalizeAwbGain(gains.g_gain);
        float normalizedB = AiqUtils::normalizeAwbGain(gains.b_gain);

        const float MAX_PER_G = AWB_GAIN_NORMALIZED_START / AWB_GAIN_NORMALIZED_END;
        const float MIN_PER_G = 1.0 / MAX_PER_G;

        if (mUseManualAwbGain) {
            awbResult->accurate_b_per_g = CLIP((normalizedB / normalizedG), MAX_PER_G, MIN_PER_G);
            awbResult->accurate_r_per_g = CLIP((normalizedR / normalizedG), MAX_PER_G, MIN_PER_G);
        } else {
            awbResult->accurate_b_per_g *= CLIP((normalizedB / normalizedG), MAX_PER_G, MIN_PER_G);
            awbResult->accurate_r_per_g *= CLIP((normalizedR / normalizedG), MAX_PER_G, MIN_PER_G);
        }
    }

    // Only override final results when manual gain or gain shift applied.
    if (mUseManualColorMatrix || mUseManualAwbGain || gains.r_gain != 0 || gains.g_gain != 0 ||
        gains.b_gain != 0) {
        LOG1("%s: override final awb results", __func__);
    }

    LOG3("%s (r,g,b): (%d,%d,%d) -> (b/g, r/g): (%f,%f)",
         mUseManualColorMatrix ? "Color gain" : (mUseManualAwbGain ? "Manual gain" : "Gain shift"),
         gains.r_gain, gains.g_gain, gains.b_gain, awbResult->accurate_b_per_g,
         awbResult->accurate_r_per_g);
}

void Intel3AParameter::updateAwbParameter(const aiq_parameter_t& param) {
    mUseManualAwbGain = false;
    mUseManualColorMatrix = false;

    switch (param.awbMode) {
        case AWB_MODE_INCANDESCENT:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_incandescent;
            break;

        case AWB_MODE_FLUORESCENT:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_fluorescent;
            break;

        case AWB_MODE_DAYLIGHT:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_daylight;
            break;

        case AWB_MODE_FULL_OVERCAST:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_fully_overcast;
            break;

        case AWB_MODE_PARTLY_OVERCAST:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_partly_overcast;
            break;

        case AWB_MODE_SUNSET:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_sunset;
            break;

        case AWB_MODE_VIDEO_CONFERENCE:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_video_conference;
            break;

        case AWB_MODE_MANUAL_CCT_RANGE:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_manual_cct_range;
            mAwbParams.manual_cct_range.min_cct = std::min(param.cctRange.min, param.cctRange.max);
            mAwbParams.manual_cct_range.max_cct = std::max(param.cctRange.min, param.cctRange.max);
            break;

        case AWB_MODE_MANUAL_WHITE_POINT: {
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_manual_white;
            camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width,
                                                     param.resolution.height};
            camera_coordinate_t iaCoord =
                AiqUtils::convertToIaCoordinate(frameCoord, param.whitePoint);

            mAwbParams.manual_white_coordinate.x = iaCoord.x;
            mAwbParams.manual_white_coordinate.y = iaCoord.y;
            break;
        }
        case AWB_MODE_MANUAL_GAIN:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
            mManualGains = param.awbManualGain;
            mUseManualAwbGain = true;
            break;
        case AWB_MODE_MANUAL_COLOR_TRANSFORM:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
            mUseManualColorMatrix = true;
            mColorMatrix = param.manualColorMatrix;
            mColorGains = param.manualColorGains;
            break;
        default:
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
            break;
    }
    mAwbGainShift = param.awbGainShift;

    if (param.awbConvergeSpeedMode == CONVERGE_SPEED_MODE_AIQ) {
        mAwbPerTicks = 1;

        mAwbParams.manual_convergence_time =
            AiqUtils::convertSpeedModeToTime(param.awbConvergeSpeed);
    } else {
        mAwbParams.manual_convergence_time = -1;

        /*
         * The unit of mAePerTicks is frame count, 3 means 3 frames.
         * The default value can be changed based on customer requirement.
         */
        switch (param.aeConvergeSpeed) {
            case CONVERGE_MID:
                mAwbPerTicks = 30;
                break;
            case CONVERGE_LOW:
                mAwbPerTicks = 60;
                break;
            case CONVERGE_NORMAL:
            default:
                mAwbPerTicks = 1;
                break;
        }
    }
}

void Intel3AParameter::updateAfParameter(const aiq_parameter_t& param) {
    // Mode
    if (mAfMode != param.afMode) {
        // Reset af parameter
        initAfParameter();

        mAfMode = param.afMode;
        if (mAfMode == AF_MODE_CONTINUOUS_PICTURE || mAfMode == AF_MODE_CONTINUOUS_VIDEO) {
            mAfParams.focus_mode = ia_aiq_af_operation_mode_auto;
        }
        mAfTrigger = AF_TRIGGER_IDLE;
        mAfForceLock = false;
        mDuringAfTriggerScan = false;
    }

    mAfParams.lens_position = param.lensPosition;
    mAfParams.lens_movement_start_timestamp = param.lensMovementStartTimestamp;
    mAfParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);

    // Trigger
    mAfParams.trigger_new_search = false;
    if (mAfTrigger != AF_TRIGGER_START && param.afTrigger == AF_TRIGGER_START) {
        LOG3("%s: Trigger AF scan, mode %d", __func__, mAfMode);
        updateAfParameterForAfTriggerStart();
    } else if (mAfTrigger != AF_TRIGGER_CANCEL && param.afTrigger == AF_TRIGGER_CANCEL) {
        LOG3("%s: Cancel AF scan, mode %d", __func__, mAfMode);
        updateAfParameterForAfTriggerCancel();
    }
    mAfTrigger = param.afTrigger;

    // Region
    mAfParams.focus_rect = {};
    mAfParams.focus_metering_mode = ia_aiq_af_metering_mode_auto;
    if (!param.afRegions.empty()) {
        // Current only one AF metering window is supported, so use the latest one
        camera_window_t window = param.afRegions.back();
        if (window.right > window.left && window.bottom > window.top) {
            camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width,
                                                     param.resolution.height};
            window = AiqUtils::convertToIaWindow(frameCoord, window);
            mAfParams.focus_metering_mode = ia_aiq_af_metering_mode_touch;
            mAfParams.focus_rect = {window.left, window.top, window.right, window.bottom};
        }
    }

    // Manual lens position
    if (mAfMode == AF_MODE_OFF) {
        mAfParams.focus_mode = ia_aiq_af_operation_mode_manual;

        // Set AIQ manual action to 'none' by default
        mAfParams.manual_focus_parameters.manual_focus_action = ia_aiq_manual_focus_action_none;

        // The focusDistance value from app is diopters, so the focusInMm = 1 / focusDistance
        // Clamp focus distance between [0.0, minFocusDistance].
        float focusDistance = param.focusDistance;
        if (focusDistance > param.minFocusDistance) {
            focusDistance = param.minFocusDistance;
        } else if (focusDistance < 0.0f) {
            focusDistance = 0.0f;
        }

        unsigned focusInMm = 0;
        if (focusDistance != 0.0f) {
            focusInMm = 1000 * (1.0f / focusDistance);
            mAfParams.manual_focus_parameters.manual_focus_action =
                ia_aiq_manual_focus_action_set_distance;
            mManualFocusDistance = focusDistance;
        } else {
            // 0.0f focus distance means infinity
            mAfParams.focus_mode = ia_aiq_af_operation_mode_infinity;
        }

        mAfParams.manual_focus_parameters.manual_focus_distance = focusInMm;
    } else {
        mAfParams.manual_focus_parameters = {};
    }
}

void Intel3AParameter::updateAfParameterForAfTriggerStart() {
    mDuringAfTriggerScan = true;
    mAfForceLock = false;

    switch (mAfMode) {
        case AF_MODE_AUTO:
        case AF_MODE_MACRO:
            // Start user af scan in this frame.
            mAfParams.focus_mode = ia_aiq_af_operation_mode_auto;
            mAfParams.trigger_new_search = true;
            break;
        case AF_MODE_CONTINUOUS_VIDEO:
            // Lock AF immediately
            mAfForceLock = true;
            break;
        case AF_MODE_CONTINUOUS_PICTURE:
            // Continue the current scan and check the af result later
            break;
        default:
            break;
    }
}

void Intel3AParameter::updateAfParameterForAfTriggerCancel() {
    mDuringAfTriggerScan = false;
    mAfForceLock = false;

    switch (mAfMode) {
        case AF_MODE_AUTO:
        case AF_MODE_MACRO:
            // Stop AF scan triggered by user.
            mAfParams.focus_mode = ia_aiq_af_operation_mode_infinity;
            break;
        default:
            break;
    }
}

void Intel3AParameter::fillAfTriggerResult(cca::cca_af_results* afResults) {
    if (!afResults || !mAfForceLock) {
        return;
    }

    // Check the result of autofocus triggered by user
    switch (mAfMode) {
        case AF_MODE_CONTINUOUS_PICTURE:
        case AF_MODE_AUTO:
        case AF_MODE_MACRO:
            // Lock AF after current scan
            mAfForceLock = (afResults->status != ia_aiq_af_status_local_search &&
                            afResults->status != ia_aiq_af_status_extended_search);
            break;
        default:
            break;
    }
}

void Intel3AParameter::dumpParameter() {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(Intel3AParameter), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("AE parameters: mode %d, bypass %d, frame_use %d, PerTicks %d", mAeMode,
         mAeParams.is_bypass, mAeParams.frame_use, mAePerTicks);
    LOG3("  flash_mode %d, metering_mode %d, priority_mode %d, flicker_reduction_mode %d",
         mAeParams.flash_mode, mAeParams.metering_mode, mAeParams.priority_mode,
         mAeParams.flicker_reduction_mode);
    LOG3("  ev_shift %f, exposure window (%d %d %d %d), coordinate (%d, %d)", mAeParams.ev_shift,
         mAeParams.exposure_window.left, mAeParams.exposure_window.top,
         mAeParams.exposure_window.right, mAeParams.exposure_window.bottom,
         mAeParams.exposure_coordinate.x, mAeParams.exposure_coordinate.y);
    LOG3("  manual et %u, ag %f, iso %d",
         mAeParams.manual_exposure_time_us[mAeParams.num_exposures - 1],
         mAeParams.manual_analog_gain[0], mAeParams.manual_iso[0]);
    LOG3("  manual total et %u", mAeParams.manual_total_target_exposure[0]);
    ia_aiq_ae_manual_limits* limit = &mAeParams.manual_limits[0];
    LOG3("  manual limited ISO-[%d--%d], expo-[%d--%d], frame time-[%d--%d]", limit->manual_iso_min,
         limit->manual_iso_max, limit->manual_exposure_time_min, limit->manual_exposure_time_max,
         limit->manual_frame_time_us_min, limit->manual_frame_time_us_max);
    LOG3("  exposure_distribution_priority %d", mAeParams.exposure_distribution_priority);
    LOG3("  manual_convergence_time %f", mAeParams.manual_convergence_time);

    LOG3("AWB parameters: bypass %d, scene_mode %d, PerTick %d", mAwbParams.is_bypass,
         mAwbParams.scene_mode, mAwbPerTicks);
    LOG3("  manual cct_range (%u-%u), white_coordinate (%d, %d), convergence_time %f",
         mAwbParams.manual_cct_range.min_cct, mAwbParams.manual_cct_range.max_cct,
         mAwbParams.manual_white_coordinate.x, mAwbParams.manual_white_coordinate.y,
         mAwbParams.manual_convergence_time);

    LOG3("AF parameters: mode %d, frame_use %d, ForceLock %d, trigger %d", mAfMode,
         mAfParams.frame_use, mAfForceLock, mAfTrigger);
    LOG3("  focus_mode %d, flash_mode %d, trigger_new_search %d", mAfParams.focus_mode,
         mAfParams.flash_mode, mAfParams.trigger_new_search);
    LOG3("  lens pos %u, move start time %lu", mAfParams.lens_position,
         mAfParams.lens_movement_start_timestamp);
    LOG3("  focus_rect (%d %d %d %d)", mAfParams.focus_rect.left, mAfParams.focus_rect.top,
         mAfParams.focus_rect.right, mAfParams.focus_rect.bottom);
    LOG3("  manual focus_distance %u, lens_position %d, focal_distance %u, action %d",
         mAfParams.manual_focus_parameters.manual_focus_distance,
         mAfParams.manual_focus_parameters.manual_lens_position,
         mAfParams.manual_focus_parameters.manual_focal_distance,
         mAfParams.manual_focus_parameters.manual_focus_action);

    LOG3("test pattern %d", mTestPatternMode);
}

} /* namespace icamera */
