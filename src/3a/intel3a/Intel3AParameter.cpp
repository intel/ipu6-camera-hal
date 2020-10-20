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

#define LOG_TAG "Intel3AParameter"

#include <cmath>
#include <climits>

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "Intel3AParameter.h"
#include "AiqUtils.h"

namespace icamera {

#define VALID_COLOR_GAINS(colorGains) (colorGains[0] > 0 && colorGains[1] > 0 && \
                                       colorGains[2] > 0 && colorGains[3] > 0)

Intel3AParameter::Intel3AParameter(int cameraId) :
    mCameraId(cameraId),
    mUseManualAwbGain(false),
    mUseManualColorMatrix(false),
    mWeightGridMode(WEIGHT_GRID_AUTO),
    mAePerTicks(1),
    mAwbPerTicks(1),
    mAfForceLock(false),
    mDuringAfTriggerScan(false)
{
    LOG3A("%s", __func__);

    CLEAR(mAeParams);
    CLEAR(mAfParams);
    CLEAR(mAwbParams);

    CLEAR(mColorMatrix);
    CLEAR(mColorGains);
    CLEAR(mManualGains);
    CLEAR(mAwbGainShift);

    CLEAR(mSensorDescriptor);
    CLEAR(mExposureWindow);
    CLEAR(mExposureCoordinate);
    CLEAR(mAeFeatures);
    CLEAR(mAeManualLimits);
    CLEAR(mManualFocusParams);
    CLEAR(mFocusRect);
    CLEAR(mManualCctRange);
    CLEAR(mManualWhiteCoordinate);

    CLEAR(mManualExposureTimeUs);
    CLEAR(mManualAnalogGain);
    CLEAR(mManualIso);
    mAfTrigger = AF_TRIGGER_IDLE;
    mAfMode = AF_MODE_OFF;
}

Intel3AParameter::~Intel3AParameter()
{
    LOG3A("%s", __func__);
}

int Intel3AParameter::init()
{
    LOG3A("%s", __func__);

    mAeParams.sensor_descriptor = &mSensorDescriptor;
    mAeParams.exposure_window = &mExposureWindow;
    mAeParams.exposure_coordinate = &mExposureCoordinate;
    mAeParams.aec_features = &mAeFeatures;
    mAeParams.manual_limits = &mAeManualLimits;

    mAfParams.focus_rect = &mFocusRect;
    mAfParams.manual_focus_parameters = &mManualFocusParams;

    mAwbParams.manual_cct_range = &mManualCctRange;
    mAwbParams.manual_white_coordinate = &mManualWhiteCoordinate;

    // set default params
    initAeParameter();
    initAfParameter();
    initAwbParameter();

    mWeightGridMode = WEIGHT_GRID_AUTO;
    mAePerTicks = 1;
    mAwbPerTicks = 1;

    mUseManualColorMatrix = false;
    CLEAR(mColorMatrix);
    CLEAR(mColorGains);

    mAfMode = AF_MODE_AUTO;
    mAfForceLock = false;
    mAfTrigger = AF_TRIGGER_IDLE;
    mDuringAfTriggerScan = false;
    return OK;
}

void Intel3AParameter::initAeParameter()
{
    mAeParams.num_exposures = 1;
    mAeParams.frame_use = ia_aiq_frame_use_video;
    mAeParams.flash_mode = ia_aiq_flash_mode_off;
    mAeParams.operation_mode = ia_aiq_ae_operation_mode_automatic;
    mAeParams.metering_mode = ia_aiq_ae_metering_mode_evaluative;
    mAeParams.priority_mode = ia_aiq_ae_priority_mode_normal;
    mAeParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_auto;
    mAeParams.ev_shift = 0.0f;
    mAeParams.manual_exposure_time_us = nullptr;
    mAeParams.manual_analog_gain = nullptr;
    mAeParams.manual_iso = nullptr;

    mAeParams.exposure_window = nullptr;
    mAeParams.exposure_coordinate = nullptr;
    mAeParams.aec_features = nullptr;
    mAeParams.manual_convergence_time = -1;
    mAeParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_auto;
}

void Intel3AParameter::initAfParameter()
{
    mAfParams.frame_use = ia_aiq_frame_use_video;
    mAfParams.lens_position = 0;
    mAfParams.lens_movement_start_timestamp = 0;
    mAfParams.focus_mode = ia_aiq_af_operation_mode_infinity;
    mAfParams.focus_range = ia_aiq_af_range_extended;
    mAfParams.focus_metering_mode = ia_aiq_af_metering_mode_auto;
    mAfParams.flash_mode = ia_aiq_flash_mode_off;
    mAfParams.focus_rect = &mFocusRect;
    mAfParams.focus_rect->left = 0;
    mAfParams.focus_rect->top = 0;
    mAfParams.focus_rect->right = 0;
    mAfParams.focus_rect->bottom = 0;
    mAfParams.manual_focus_parameters = &mManualFocusParams;
    mAfParams.manual_focus_parameters->manual_focus_action = ia_aiq_manual_focus_action_none;
    mAfParams.manual_focus_parameters->manual_focus_distance = MAX_FOCUS_DISTANCE;
    mAfParams.manual_focus_parameters->manual_lens_position = 0;
    mAfParams.trigger_new_search = false;
}

void Intel3AParameter::initAwbParameter()
{
    mAwbParams.frame_use = ia_aiq_frame_use_video;
    mAwbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
    mAwbParams.manual_convergence_time = -1;

    mUseManualAwbGain = false;
    CLEAR(mManualGains);
    CLEAR(mAwbGainShift);
}

int Intel3AParameter::setSensorInfo(ia_aiq_exposure_sensor_descriptor descriptor)
{
    LOG3A("%s", __func__);

    mSensorDescriptor.pixel_clock_freq_mhz = descriptor.pixel_clock_freq_mhz;
    mSensorDescriptor.pixel_periods_per_line = descriptor.pixel_periods_per_line;
    mSensorDescriptor.line_periods_per_field = descriptor.line_periods_per_field;
    mSensorDescriptor.line_periods_vertical_blanking = descriptor.line_periods_vertical_blanking;
    mSensorDescriptor.fine_integration_time_min = descriptor.fine_integration_time_min;
    mSensorDescriptor.fine_integration_time_max_margin = descriptor.fine_integration_time_max_margin;
    mSensorDescriptor.coarse_integration_time_min = descriptor.coarse_integration_time_min;
    mSensorDescriptor.coarse_integration_time_max_margin = descriptor.coarse_integration_time_max_margin;

    return OK;
}

int Intel3AParameter::updateParameter(aiq_parameter_t param)
{
    LOG3A("%s", __func__);

    updateAeParameter(param);
    updateAwbParameter(param);
    updateAfParameter(param);

    return OK;
}

/**
 * Override ae result by those settings provided by application
 */
void Intel3AParameter::updateAeResult(ia_aiq_ae_results* aeResult)
{
    CheckError(!aeResult || !aeResult->weight_grid, VOID_VALUE, "Invalid aeResult");

}

float Intel3AParameter::convertdBGainToISO(float sensitivityGain, int baseIso)
{
    // Convert db Gain to ISO
    float manualGain = pow(10, (sensitivityGain / 20));
    manualGain *= baseIso;
    return manualGain;
}

void Intel3AParameter::setAeManualLimits(const aiq_parameter_t& param)
{
    mAeParams.manual_limits = &mAeManualLimits;

    if (param.aeFpsRange.min > 0.01 && param.aeFpsRange.max >= param.aeFpsRange.min) {
        mAeManualLimits.manual_frame_time_us_max = 1000000 / param.aeFpsRange.min;
        mAeManualLimits.manual_frame_time_us_min = 1000000 / param.aeFpsRange.max;
    } else if (param.fps > 0.01) {
        mAeManualLimits.manual_frame_time_us_max = 1000000 / param.fps;
        mAeManualLimits.manual_frame_time_us_min = 1000000 / param.fps;
    }

    camera_range_t etRange = {-1, -1};
    int ret = PlatformData::getSupportAeExposureTimeRange(mCameraId, param.sceneMode, etRange);
    if (param.exposureTimeRange.min > 0 &&
        param.exposureTimeRange.max >= param.exposureTimeRange.min) {
        if (ret == OK) {
            etRange.min = CLIP(param.exposureTimeRange.min, etRange.max, etRange.min);
            etRange.max = CLIP(param.exposureTimeRange.max, etRange.max, etRange.min);
        } else {
            etRange.min = param.exposureTimeRange.min;
            etRange.max = param.exposureTimeRange.max;
        }
    }
    mAeManualLimits.manual_exposure_time_min = etRange.min;
    mAeManualLimits.manual_exposure_time_max = etRange.max;

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

    mAeManualLimits.manual_iso_min = -1;
    mAeManualLimits.manual_iso_max = -1;
    if (gainRange.min >= 0 && gainRange.max >= gainRange.min) {
        ia_binary_data aiqData;
        ia_cmc_t *cmc = nullptr;

        int ret = PlatformData::getCpfAndCmc(mCameraId, nullptr, &aiqData, nullptr,
                                             nullptr, param.tuningMode, &cmc);
        if (ret == OK) {
            if (cmc != nullptr && cmc->cmc_sensitivity != nullptr) {
                float isoMin = convertdBGainToISO(gainRange.min, cmc->cmc_sensitivity->base_iso);
                float isoMax = convertdBGainToISO(gainRange.max, cmc->cmc_sensitivity->base_iso);
                if (isoMin <= INT_MAX && isoMax <= INT_MAX) {
                    mAeManualLimits.manual_iso_min = static_cast<int>(isoMin);
                    mAeManualLimits.manual_iso_max = static_cast<int>(isoMax);
                }
            }
        }
    }

    LOG3A("%s, manual limited ISO-[%d--%d], expo-[%d--%d], frame time-[%d--%d]", __func__,
          mAeManualLimits.manual_iso_min, mAeManualLimits.manual_iso_max,
          mAeManualLimits.manual_exposure_time_min, mAeManualLimits.manual_exposure_time_max,
          mAeManualLimits.manual_frame_time_us_min, mAeManualLimits.manual_frame_time_us_max);
}

void Intel3AParameter::setManualExposure(const aiq_parameter_t& param)
{
    int64_t manualExpTimeUs = param.manualExpTimeUs;
    if (manualExpTimeUs <= 0 || param.aeDistributionPriority == DISTRIBUTION_ISO) {
        return;
    }

    camera_range_t etRange;
    CLEAR(etRange);
    if (PlatformData::getSupportAeExposureTimeRange(mCameraId, param.sceneMode, etRange) == OK) {
         manualExpTimeUs = CLIP(manualExpTimeUs, etRange.max, etRange.min);
    }

    mAeParams.manual_exposure_time_us = mManualExposureTimeUs;
    for (unsigned int i = 0; i < mAeParams.num_exposures - 1; i++) {
        mAeParams.manual_exposure_time_us[i] = -1;
    }
    mAeParams.manual_exposure_time_us[mAeParams.num_exposures - 1] = manualExpTimeUs;
    LOG3A("%s, manual exposure %ld", __func__, manualExpTimeUs);
}

void Intel3AParameter::setManualGain(const aiq_parameter_t& param)
{
    float manualGain = param.manualGain;
    if (manualGain < 0 || param.aeDistributionPriority == DISTRIBUTION_SHUTTER) {
        return;
    }

    camera_range_t gainRange;
    CLEAR(gainRange);
    if (PlatformData::getSupportAeGainRange(mCameraId, param.sceneMode, gainRange) == OK) {
        manualGain = CLIP(manualGain, gainRange.max, gainRange.min);
    }

    mAeParams.manual_analog_gain = mManualAnalogGain;
    // Convert db to sensor analog gain.
    for (unsigned int i = 0; i < mAeParams.num_exposures; i++) {
        mAeParams.manual_analog_gain[i] = pow(10, (manualGain / 20));
    }

    LOG3A("%s, manual gain %f, AG %f", __func__, manualGain, mManualAnalogGain[0]);
}

void Intel3AParameter::setManualIso(const aiq_parameter_t& param)
{
    int32_t manualIso = param.manualIso;
    if (manualIso <= 0 || param.aeDistributionPriority == DISTRIBUTION_SHUTTER) {
        return;
    }

    if (mAeManualLimits.manual_iso_min >= 0
        && mAeManualLimits.manual_iso_max >= mAeManualLimits.manual_iso_min) {
        manualIso = CLIP(manualIso, mAeManualLimits.manual_iso_max, mAeManualLimits.manual_iso_min);
    }

    // Will overwrite manual_analog_gain
    mAeParams.manual_iso = mManualIso;
    for (unsigned int i = 0; i < mAeParams.num_exposures; i++) {
        mAeParams.manual_iso[i] = manualIso;
    }

    LOG3A("%s, manual iso  %d", __func__, manualIso);
}

void Intel3AParameter::updateAeParameter(const aiq_parameter_t& param)
{
    mAeParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);
    mAeParams.num_exposures = PlatformData::getExposureNum(mCameraId,
                                  CameraUtils::isMultiExposureCase(param.tuningMode));
    setAeManualLimits(param);

    switch(param.antibandingMode) {
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

    mAeParams.manual_exposure_time_us = nullptr;
    mAeParams.manual_analog_gain = nullptr;
    mAeParams.manual_iso = nullptr;

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

    mAeParams.exposure_coordinate = nullptr;
    if (param.blcAreaMode == BLC_AREA_MODE_ON && !param.aeRegions.empty()) {
        // Current only one AE metering window is supported, so use the latest one
        camera_window_t window = param.aeRegions.back();

        if (window.right > window.left && window.bottom > window.top) {
            camera_coordinate_t coordinate;
            CLEAR(coordinate);
            coordinate.x = window.left + (window.right - window.left) / 2;
            coordinate.y = window.top + (window.bottom - window.top) / 2;
            camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width, param.resolution.height};
            LOG3A("%s: frame resolution %dx%d", __func__, param.resolution.width, param.resolution.height);

            coordinate = AiqUtils::convertToIaCoordinate(frameCoord, coordinate);
            mExposureCoordinate.x = coordinate.x;
            mExposureCoordinate.y = coordinate.y;
            mAeParams.exposure_coordinate = &mExposureCoordinate;
            LOG3A("%s, exposure coordinate = [%d,%d], region = [%d,%d,%d,%d]", __func__,
                    mAeParams.exposure_coordinate->x, mAeParams.exposure_coordinate->y,
                    window.left, window.top, window.right, window.bottom);
        }
    }
}

void Intel3AParameter::updatePaResult(ia_aiq_pa_results_v1* paResult) {
    CheckError((paResult == nullptr), VOID_VALUE, "No Pa result provided.");

    if (!mUseManualColorMatrix) return;

    // Override color_conversion_matrix and color_gains
    // when application requires manual color transform.
    MEMCPY_S(&(paResult->color_conversion_matrix), sizeof(paResult->color_conversion_matrix),
             &(mColorMatrix.color_transform), sizeof(mColorMatrix.color_transform));

    if (paResult->preferred_acm) {
        for (unsigned int i = 0; i < paResult->preferred_acm->sector_count; i++) {
            MEMCPY_S(&(paResult->preferred_acm->advanced_color_conversion_matrices[i]),
                     sizeof(paResult->preferred_acm->advanced_color_conversion_matrices[0]),
                     &(mColorMatrix.color_transform), sizeof(mColorMatrix.color_transform));
        }
    }
}

/**
 * Override awb result by AWB gains or gain shift provided by application
 */
void Intel3AParameter::updateAwbResult(ia_aiq_awb_results* awbResult)
{
    CheckError((awbResult == nullptr), VOID_VALUE, "No Awb result provided.");

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

    //Only override final results when manual gain or gain shift applied.
    if (mUseManualColorMatrix || mUseManualAwbGain || gains.r_gain != 0
        || gains.g_gain != 0 || gains.b_gain != 0) {
        LOG3A("%s: override final awb results", __func__);
        awbResult->final_b_per_g = awbResult->accurate_b_per_g;
        awbResult->final_r_per_g = awbResult->accurate_r_per_g;
    }

    LOG3A("%s (r,g,b): (%d,%d,%d) -> (b/g, r/g): (%f,%f)",
          mUseManualColorMatrix ? "Color gain" : (mUseManualAwbGain ? "Manual gain" : "Gain shift"),
          gains.r_gain, gains.g_gain, gains.b_gain,
          awbResult->accurate_b_per_g, awbResult->accurate_r_per_g);
}

void Intel3AParameter::updateAwbParameter(const aiq_parameter_t& param)
{
    mUseManualAwbGain = false;
    mUseManualColorMatrix = false;
    mAwbParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);

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
            mManualCctRange.min_cct = std::min(param.cctRange.min, param.cctRange.max);
            mManualCctRange.max_cct = std::max(param.cctRange.min, param.cctRange.max);
            mAwbParams.manual_cct_range = &mManualCctRange;
            break;

        case AWB_MODE_MANUAL_WHITE_POINT:
        {
            mAwbParams.scene_mode = ia_aiq_awb_operation_mode_manual_white;
            mAwbParams.manual_white_coordinate = &mManualWhiteCoordinate;
            camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width, param.resolution.height};
            LOG3A("%s: frame resolution %dx%d", __func__, param.resolution.width, param.resolution.height);
            camera_coordinate_t iaCoord = AiqUtils::convertToIaCoordinate(frameCoord, param.whitePoint);

            mManualWhiteCoordinate.x = iaCoord.x;
            mManualWhiteCoordinate.y = iaCoord.y;
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

        mAwbParams.manual_convergence_time = AiqUtils::convertSpeedModeToTime(param.awbConvergeSpeed);
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

void Intel3AParameter::updateAfParameter(const aiq_parameter_t& param)
{
    mAfParams.lens_position = param.lensPosition;
    mAfParams.lens_movement_start_timestamp = param.lensMovementStartTimestamp;

    LOG3A("%s, Focus position %d, timestamp %llu, afMode %d", __func__, param.lensPosition,
            param.lensMovementStartTimestamp, param.afMode);

    // Mode
    if (mAfMode != param.afMode) {
        // Reset af parameter
        initAfParameter();

        mAfMode = param.afMode;
        if (mAfMode == AF_MODE_CONTINUOUS_PICTURE
            || mAfMode == AF_MODE_CONTINUOUS_VIDEO) {
            mAfParams.focus_mode = ia_aiq_af_operation_mode_auto;
        }
        mAfTrigger = AF_TRIGGER_IDLE;
        mAfForceLock = false;
        mDuringAfTriggerScan = false;
    }
    mAfParams.frame_use = AiqUtils::convertFrameUsageToIaFrameUsage(param.frameUsage);

    // Trigger
    mAfParams.trigger_new_search = false;
    if (mAfTrigger != AF_TRIGGER_START && param.afTrigger == AF_TRIGGER_START) {
        LOG3A("%s: Trigger AF scan, mode %d", __func__, mAfMode);
        updateAfParameterForAfTriggerStart();
    } else if (mAfTrigger != AF_TRIGGER_CANCEL && param.afTrigger == AF_TRIGGER_CANCEL) {
        LOG3A("%s: Cancel AF scan, mode %d", __func__, mAfMode);
        updateAfParameterForAfTriggerCancel();
    }
    mAfTrigger = param.afTrigger;

    // Region
    mAfParams.focus_rect = nullptr;
    if (!param.afRegions.empty()) {
        // Current only one AF metering window is supported, so use the latest one
        camera_window_t window = param.afRegions.back();
        if (window.right > window.left && window.bottom > window.top) {
            camera_coordinate_system_t frameCoord = {0, 0, param.resolution.width, param.resolution.height};
            window = AiqUtils::convertToIaWindow(frameCoord, window);
            mFocusRect.left   = window.left;
            mFocusRect.top    = window.top;
            mFocusRect.right  = window.right;
            mFocusRect.bottom = window.bottom;
            mAfParams.focus_rect = &mFocusRect;
            LOG3A("%s, af region = [%d,%d, %d, %d], window = [%d,%d,%d,%d]", __func__,
                    mFocusRect.left, mFocusRect.top, mFocusRect.right, mFocusRect.bottom,
                    window.left, window.top, window.right, window.bottom);
        }
    }

    // Manual lens position
    if (mAfMode == AF_MODE_OFF) {
        mAfParams.manual_focus_parameters = &mManualFocusParams;
        mAfParams.focus_mode = ia_aiq_af_operation_mode_manual;
        mAfParams.focus_range = ia_aiq_af_range_extended;
        mAfParams.focus_metering_mode = ia_aiq_af_metering_mode_auto;

        // Set AIQ manual action to 'none' by default
        mAfParams.manual_focus_parameters->manual_focus_action = ia_aiq_manual_focus_action_none;

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
            mAfParams.manual_focus_parameters->manual_focus_action =
                ia_aiq_manual_focus_action_set_distance;
        } else {
            // 0.0f focus distance means infinity
            mAfParams.focus_mode = ia_aiq_af_operation_mode_infinity;
        }

        mAfParams.manual_focus_parameters->manual_focus_distance = focusInMm;
    } else {
        mAfParams.manual_focus_parameters = nullptr;
    }

    LOG3A("%s, afForceLock %d, duringAfTriggerScan %d", __func__, mAfForceLock, mDuringAfTriggerScan);
}

void Intel3AParameter::updateAfParameterForAfTriggerStart()
{
    mDuringAfTriggerScan = true;
    mAfForceLock = false;

    switch (mAfMode) {
        case AF_MODE_AUTO:
        case AF_MODE_MACRO:
            // Start user af scan in this frame.
            mAfParams.frame_use = ia_aiq_frame_use_still;
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

void Intel3AParameter::updateAfParameterForAfTriggerCancel()
{
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

void Intel3AParameter::fillAfTriggerResult(ia_aiq_af_results *afResults)
{
    if (!afResults || !mAfForceLock) {
        return;
    }

    // Check the result of autofocus triggered by user
    switch (mAfMode) {
        case AF_MODE_CONTINUOUS_PICTURE:
        case AF_MODE_AUTO:
        case AF_MODE_MACRO:
            // Lock AF after current scan
            mAfForceLock = (afResults->status != ia_aiq_af_status_local_search
                          && afResults->status != ia_aiq_af_status_extended_search);
            break;
        default:
            break;
    }

    LOG3A("%s, %d update afForceLock %d", __func__, afResults->status, mAfForceLock);
}

} /* namespace icamera */
