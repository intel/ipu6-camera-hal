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

#define LOG_TAG "AiqUtils"

#include <math.h>
#include <algorithm>

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "AiqUtils.h"
#include "AiqSetting.h"

namespace icamera {

int AiqUtils::dumpAeResults(const cca::cca_ae_results& aeResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    LOG3A("num_exposures :%d", aeResult.num_exposures);
    for (unsigned int i = 0; i < aeResult.num_exposures; i++) {
        LOG3A("AE sensor exp[%u] result ag %u dg %u coarse: %u fine: %u llp:%u fll:%u", i,
              aeResult.exposures[i].sensor_exposure[0].analog_gain_code_global,
              aeResult.exposures[i].sensor_exposure[0].digital_gain_global,
              aeResult.exposures[i].sensor_exposure[0].coarse_integration_time,
              aeResult.exposures[i].sensor_exposure[0].fine_integration_time,
              aeResult.exposures[i].sensor_exposure[0].line_length_pixels,
              aeResult.exposures[i].sensor_exposure[0].frame_length_lines);

        LOG3A("AE exp[%d] ag %f dg %f Fn %f time %uus total %u filter[%s] iso %d", i,
              aeResult.exposures[i].exposure[0].analog_gain,
              aeResult.exposures[i].exposure[0].digital_gain,
              aeResult.exposures[i].exposure[0].aperture_fn,
              aeResult.exposures[i].exposure[0].exposure_time_us,
              aeResult.exposures[i].exposure[0].total_target_exposure,
              aeResult.exposures[i].exposure[0].nd_filter_enabled? "YES": "NO",
              aeResult.exposures[i].exposure[0].iso);
        LOG3A("Distance convergence: %f, AE Converged : %s",
              aeResult.exposures[i].distance_from_convergence,
              aeResult.exposures[i].converged ? "YES" : "NO");
    }
    LOG3A("AE bracket mode = %d %s", aeResult.multiframe,
          aeResult.multiframe == ia_aiq_bracket_mode_ull ? "ULL" : "none-ULL");

    const cca::cca_hist_weight_grid& wg = aeResult.weight_grid;
    if (wg.width != 0 && wg.height != 0) {
        LOG3A("AE weight grid [%dx%d]", wg.width, wg.height);
        for (int i = 0; i < 5 && i < wg.height; i++) {
            LOG3A("AE weight_grid[%d] = %d ", wg.width/2, wg.weights[wg.width/2]);
        }
    }

    const ia_aiq_aperture_control& ac = aeResult.aperture_control;
    LOG3A("AE aperture fn = %f, iris command = %d, code = %d",
          ac.aperture_fn, ac.dc_iris_command, ac.code);

    return OK;
}

int AiqUtils::dumpAfResults(const cca::cca_af_results& afResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    LOG3A("AF results current_focus_distance %d final_position_reached %s",
          afResult.current_focus_distance,
          afResult.final_lens_position_reached ? "TRUE":"FALSE");
    LOG3A("AF results next_lens_position %d", afResult.next_lens_position);

    switch (afResult.status) {
    case ia_aiq_af_status_local_search:
        LOG3A("AF result state _local_search");
        break;
    case ia_aiq_af_status_extended_search:
        LOG3A("AF result state extended_search");
        break;
    case ia_aiq_af_status_success:
        LOG3A("AF state success");
        break;
    case ia_aiq_af_status_fail:
        LOG3A("AF state fail");
        break;
    case ia_aiq_af_status_idle:
    default:
        LOG3A("AF state idle");
    }

    return OK;
}

int AiqUtils::dumpAwbResults(const cca::cca_awb_results& awbResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    LOG3A("AWB result: accurate_r/g %f, accurate_b/g %f",
          awbResult.accurate_r_per_g, awbResult.accurate_b_per_g);
    LOG3A("AWB result: distance_from_convergence %f", awbResult.distance_from_convergence);

    return OK;
}

int AiqUtils::dumpGbceResults(const cca::cca_gbce_params& gbceResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    LOG3A("gamma_lut_size: %u, tone_map_lut_size: %u",
          gbceResult.gamma_lut_size, gbceResult.tone_map_lut_size);

    LOG3A("gamma table: R: 0(%f), %u(%f), %u(%f)", gbceResult.r_gamma_lut[0],
          (gbceResult.gamma_lut_size / 2), gbceResult.r_gamma_lut[gbceResult.gamma_lut_size / 2],
          (gbceResult.gamma_lut_size - 1),  gbceResult.r_gamma_lut[gbceResult.gamma_lut_size - 1]);

    LOG3A("gamma table: G: 0(%f), %u(%f), %u(%f)", gbceResult.g_gamma_lut[0],
          (gbceResult.gamma_lut_size / 2), gbceResult.g_gamma_lut[gbceResult.gamma_lut_size / 2],
          (gbceResult.gamma_lut_size - 1),  gbceResult.g_gamma_lut[gbceResult.gamma_lut_size - 1]);

    LOG3A("gamma table: B: 0(%f), %u(%f), %u(%f)", gbceResult.b_gamma_lut[0],
          (gbceResult.gamma_lut_size / 2), gbceResult.b_gamma_lut[gbceResult.gamma_lut_size / 2],
          (gbceResult.gamma_lut_size - 1),  gbceResult.b_gamma_lut[gbceResult.gamma_lut_size - 1]);

    LOG3A("tonemap table: 0(%f), %u(%f), %u(%f)", gbceResult.tone_map_lut[0],
          (gbceResult.tone_map_lut_size / 2),
          gbceResult.tone_map_lut[gbceResult.tone_map_lut_size / 2],
          (gbceResult.tone_map_lut_size - 1),
          gbceResult.tone_map_lut[gbceResult.tone_map_lut_size - 1]);

    return OK;
}

int AiqUtils::dumpPaResults(const cca::cca_pa_params& paResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    for (int i = 0; i < 3; i++) {
        LOG3A("color_conversion_matrix  [%.3f %.3f %.3f] ",
              paResult.color_conversion_matrix[i][0],
              paResult.color_conversion_matrix[i][1],
              paResult.color_conversion_matrix[i][2]);
    }

    LOG3A("color_gains, gr:%f, r:%f, b:%f, gb:%f",
          paResult.color_gains.gr, paResult.color_gains.r,
          paResult.color_gains.b, paResult.color_gains.gb);

    return OK;
}

int AiqUtils::dumpSaResults(const cca::cca_sa_results& saResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) return OK;
    LOG3A("@%s", __func__);

    LOG3A("SA results color_order %d size %dx%d",
          saResult.color_order, saResult.width,  saResult.height);

    return OK;
}

int AiqUtils::convertError(ia_err iaErr)
{
    LOG3A("%s, iaErr = %d", __func__, iaErr);
    switch (iaErr) {
    case ia_err_none:
        return OK;
    case ia_err_general:
        return UNKNOWN_ERROR;
    case ia_err_nomemory:
        return NO_MEMORY;
    case ia_err_data:
        return BAD_VALUE;
    case ia_err_internal:
        return INVALID_OPERATION;
    case ia_err_argument:
        return BAD_VALUE;
    default:
        return UNKNOWN_ERROR;
    }
}

/**
 * Convert SensorFrameParams defined in PlatformData to ia_aiq_frame_params in aiq
 */
void AiqUtils::convertToAiqFrameParam(const SensorFrameParams &sensor, ia_aiq_frame_params &aiq)
{
    aiq.cropped_image_height = sensor.cropped_image_height;
    aiq.cropped_image_width = sensor.cropped_image_width;
    aiq.horizontal_crop_offset = sensor.horizontal_crop_offset;
    aiq.horizontal_scaling_denominator = sensor.horizontal_scaling_denominator;
    aiq.horizontal_scaling_numerator = sensor.horizontal_scaling_numerator;
    aiq.vertical_crop_offset = sensor.vertical_crop_offset;
    aiq.vertical_scaling_denominator = sensor.vertical_scaling_denominator;
    aiq.vertical_scaling_numerator = sensor.vertical_scaling_numerator;
}

camera_coordinate_t AiqUtils::convertCoordinateSystem(const camera_coordinate_system_t& srcSystem,
                                                      const camera_coordinate_system_t& dstSystem,
                                                      const camera_coordinate_t& srcCoordinate)
{
    int dstWidth = dstSystem.right - dstSystem.left;
    int dstHeight = dstSystem.bottom - dstSystem.top;
    int srcWidth = srcSystem.right - srcSystem.left;
    int srcHeight = srcSystem.bottom - srcSystem.top;

    camera_coordinate_t result;
    result.x = (srcCoordinate.x - srcSystem.left) * dstWidth / srcWidth + dstSystem.left;
    result.y = (srcCoordinate.y - srcSystem.top) * dstHeight / srcHeight + dstSystem.top;

    return result;
}

camera_coordinate_t AiqUtils::convertToIaCoordinate(const camera_coordinate_system_t& srcSystem,
                                                    const camera_coordinate_t& srcCoordinate)
{
    camera_coordinate_system_t iaCoordinate = {IA_COORDINATE_LEFT, IA_COORDINATE_TOP,
                                               IA_COORDINATE_RIGHT, IA_COORDINATE_BOTTOM};

    return convertCoordinateSystem(srcSystem, iaCoordinate, srcCoordinate);
}

camera_window_t AiqUtils::convertToIaWindow(const camera_coordinate_system_t& srcSystem,
                                            const camera_window_t& srcWindow)
{
    camera_coordinate_t leftTop;
    camera_coordinate_t rightBottom;
    leftTop.x     = srcWindow.left;
    leftTop.y     = srcWindow.top;
    rightBottom.x = srcWindow.right;
    rightBottom.y = srcWindow.bottom;
    leftTop       = convertToIaCoordinate(srcSystem, leftTop);
    rightBottom   = convertToIaCoordinate(srcSystem, rightBottom);

    camera_window_t result;
    result.left   = leftTop.x;
    result.top    = leftTop.y;
    result.right  = rightBottom.x;
    result.bottom = rightBottom.y;
    result.weight = srcWindow.weight;
    return result;
}

/**
 * Map user input manual gain(0, 255) to (AWB_GAIN_NORMALIZED_START, AWB_GAIN_NORMALIZED_END)
 */
float AiqUtils::normalizeAwbGain(int gain)
{
    gain = CLIP(gain, AWB_GAIN_MAX, AWB_GAIN_MIN);
    return AWB_GAIN_NORMALIZED_START + (float)(gain - AWB_GAIN_MIN) * \
                                       AWB_GAIN_RANGE_NORMALIZED / AWB_GAIN_RANGE_USER;
}

int AiqUtils::convertToUserAwbGain(float normalizedGain)
{
    normalizedGain = CLIP(normalizedGain, AWB_GAIN_NORMALIZED_START, AWB_GAIN_NORMALIZED_END);
    return AWB_GAIN_MIN + (normalizedGain - AWB_GAIN_NORMALIZED_START) * \
                          AWB_GAIN_RANGE_USER / AWB_GAIN_RANGE_NORMALIZED;
}

float AiqUtils::convertSpeedModeToTime(camera_converge_speed_t mode)
{
    float convergenceTime = -1;
    /*
     * The unit of manual_convergence_time is second, and 3.0 means 3 seconds.
     * The default value can be changed based on customer requirement.
     */
    switch (mode) {
        case CONVERGE_MID:
            convergenceTime = 3.0;
            break;
        case CONVERGE_LOW:
            convergenceTime = 5.0;
            break;
        case CONVERGE_NORMAL:
        default:
            convergenceTime = -1;
            break;
    }
    return convergenceTime;
}

/*
 * Get ia_aiq_frame_use
 *
 * Convert frame usage to ia_aiq_frame_use
 */
ia_aiq_frame_use AiqUtils::convertFrameUsageToIaFrameUsage(int frameUsage)
{
    switch (frameUsage) {
        case FRAME_USAGE_VIDEO:
            return ia_aiq_frame_use_video;
        case FRAME_USAGE_STILL:
            return ia_aiq_frame_use_still;
        case FRAME_USAGE_CONTINUOUS:
            return ia_aiq_frame_use_continuous;
    }
    return ia_aiq_frame_use_preview;
}

void AiqUtils::applyTonemapGamma(float gamma, cca::cca_gbce_params* results) {
    CheckError(gamma < EPSILON, VOID_VALUE, "Bad gamma %f", gamma);
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < MIN_TONEMAP_POINTS, VOID_VALUE,
               "Bad gamma lut size (%d) in gbce results", lutSize);
    for (int i = 0; i < lutSize; i++) {
        results->g_gamma_lut[i] = pow(i / static_cast<float>(lutSize), 1 / gamma);
    }

    MEMCPY_S(results->b_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
    MEMCPY_S(results->r_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
}

void AiqUtils::applyTonemapSRGB(cca::cca_gbce_params* results) {
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < MIN_TONEMAP_POINTS, VOID_VALUE,
               "Bad gamma lut size (%d) in gbce results", lutSize);
    for (int i = 0; i < lutSize; i++) {
        if (i / (lutSize - 1)  < 0.0031308)
            results->g_gamma_lut[i] = 12.92 * (i / (lutSize - 1));
        else
            results->g_gamma_lut[i] =
                    1.055 * pow(i / static_cast<float>(lutSize - 1), 1 / 2.4) - 0.055;
    }

    MEMCPY_S(results->b_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
    MEMCPY_S(results->r_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
}

void AiqUtils::applyTonemapREC709(cca::cca_gbce_params* results) {
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < MIN_TONEMAP_POINTS, VOID_VALUE,
               "Bad gamma lut size (%d) in gbce results", lutSize);
    for (int i = 0; i < lutSize; i++) {
        if (i / (lutSize - 1) < 0.018)
            results->g_gamma_lut[i] = 4.5 * (i / (lutSize - 1));
        else
            results->g_gamma_lut[i] =
                    1.099 * pow(i / static_cast<float>(lutSize - 1), 0.45) - 0.099;
    }

    MEMCPY_S(results->b_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
    MEMCPY_S(results->r_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
}

void AiqUtils::applyTonemapCurve(const camera_tonemap_curves_t& curves,
                                 cca::cca_gbce_params* results) {
    CheckError(!results, VOID_VALUE, "gbce result nullptr");
    CheckError(results->gamma_lut_size <= 1, VOID_VALUE, "wrong gamma_lut_size");
    CheckError(curves.rSize != curves.gSize, VOID_VALUE, "wrong rSize");
    CheckError(curves.bSize != curves.gSize, VOID_VALUE, "wrong bSize");

    LOG2("%s: input size %d, output size %d", __func__, curves.gSize, results->gamma_lut_size);
    // User's curve is 2-d array: (in, out)
    float step = static_cast<float>(curves.gSize / 2 - 1) / (results->gamma_lut_size - 1);
    for (uint32_t i = 0; i < results->gamma_lut_size; i++) {
        float inPos = i * step;
        int32_t left = static_cast<int>(inPos) * 2 + 1;
        int32_t right = left + 2;
        float ratio = inPos - static_cast<int32_t>(inPos);
        if (right > curves.gSize - 1) {
            results->r_gamma_lut[i] = curves.rCurve[left];
            results->g_gamma_lut[i] = curves.gCurve[left];
            results->b_gamma_lut[i] = curves.bCurve[left];
        } else {
            results->r_gamma_lut[i] = curves.rCurve[left]
                                      + ratio * (curves.rCurve[right] - curves.rCurve[left]);
            results->g_gamma_lut[i] = curves.gCurve[left]
                                      + ratio * (curves.gCurve[right] - curves.gCurve[left]);
            results->b_gamma_lut[i] = curves.bCurve[left]
                                      + ratio * (curves.bCurve[right] - curves.bCurve[left]);
        }
    }
}

void AiqUtils::applyAwbGainForTonemapCurve(const camera_tonemap_curves_t& curves,
                                           cca::cca_awb_results* results) {
    CheckError(!results, VOID_VALUE, "pa result nullptr");
    CheckError(curves.rSize != curves.gSize, VOID_VALUE, "wrong rSize");
    CheckError(curves.bSize != curves.gSize, VOID_VALUE, "wrong bSize");

    // Use awb gains to implement different tone map curves
    float sumR = 0;
    float sumG = 0;
    float sumB = 0;
    for (int32_t i = 1; i < curves.gSize; i += 2) {
        sumR += curves.rCurve[i];
        sumG += curves.gCurve[i];
        sumB += curves.bCurve[i];
    }

    float averageR = sumR / (curves.rSize / 2);
    float averageG = sumG / (curves.gSize / 2);
    float averageB = sumB / (curves.bSize / 2);
    LOG2("%s: curve average: %f %f %f", __func__, averageR, averageG, averageB);

    float minAverage = std::min(averageR, averageG);
    minAverage = std::min(minAverage, averageB);
    float maxAverage = std::max(averageR, averageG);
    maxAverage = std::max(maxAverage, averageB);
    if (maxAverage - minAverage > EPSILON) {
        averageR =  AWB_GAIN_NORMALIZED_START + (averageR - minAverage) * \
                                           AWB_GAIN_RANGE_NORMALIZED / (maxAverage - minAverage);
        averageG =  AWB_GAIN_NORMALIZED_START + (averageG - minAverage) * \
                                           AWB_GAIN_RANGE_NORMALIZED / (maxAverage - minAverage);
        averageB =  AWB_GAIN_NORMALIZED_START + (averageB - minAverage) * \
                                           AWB_GAIN_RANGE_NORMALIZED / (maxAverage - minAverage);
        results->accurate_r_per_g = averageR / averageG;
        results->accurate_b_per_g = averageB / averageG;
        LOG2("%s: overwrite awb gain %f %f", __func__,
             results->accurate_r_per_g, results->accurate_b_per_g);
    }
}

float AiqUtils::calculateHyperfocalDistance(const cca::cca_cmc &cmc) {
    LOG2("@%s", __func__);

    const float DEFAULT_HYPERFOCAL_DISTANCE = 5000.0f;

    // Pixel size is stored in CMC in hundreds of micrometers
    float pixelSizeMicro = cmc.optics.sensor_pix_size_h / 100;
    // focal length is stored in CMC in hundreds of millimeters
    float focalLengthMillis = static_cast<float>(cmc.optics.effect_focal_length) / 100;

    // fixed aperture, the fn should be divided 100 because the value
    // is multiplied 100 in cmc avoid division by 0
    if (cmc.lut_apertures == 0) {
        LOG2("lut apertures is not provided or zero in the cmc. Using default");
        return DEFAULT_HYPERFOCAL_DISTANCE;
    }

    float fNumber = static_cast<float>(cmc.lut_apertures) / 100;
    // assuming square pixel
    const int CIRCLE_OF_CONFUSION_IN_PIXELS = 2;
    float cocMicros = pixelSizeMicro * CIRCLE_OF_CONFUSION_IN_PIXELS;
    float hyperfocalDistanceMillis = 1000 * (focalLengthMillis * focalLengthMillis) /
                                     (fNumber * cocMicros);

    return (hyperfocalDistanceMillis == 0.0f) ? DEFAULT_HYPERFOCAL_DISTANCE :
                                                hyperfocalDistanceMillis;
}
} /* namespace icamera */
