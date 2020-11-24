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

#define LOG_TAG "AiqUtils"

#include <math.h>
#include <algorithm>

#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "AiqUtils.h"
#include "AiqSetting.h"

namespace icamera {

#define TONEMAP_MIN_POINTS 64

int AiqUtils::dumpAeResults(const ia_aiq_ae_results &aeResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) {
        return OK;
    }

    LOG3A("@%s", __func__);

    if (aeResult.exposures) {
        for (unsigned int i = 0; i < aeResult.num_exposures; i++) {
            if (aeResult.exposures[i].exposure) {
                LOG3A("AE exp[%d] ag %f dg %f Fn %f time %dus total %d filter[%s] iso %d", i,
                      aeResult.exposures[i].exposure->analog_gain,
                      aeResult.exposures[i].exposure->digital_gain,
                      aeResult.exposures[i].exposure->aperture_fn,
                      aeResult.exposures[i].exposure->exposure_time_us,
                      aeResult.exposures[i].exposure->total_target_exposure,
                      aeResult.exposures[i].exposure->nd_filter_enabled? "YES": "NO",
                      aeResult.exposures[i].exposure->iso);
            }
            if (aeResult.exposures[i].sensor_exposure) {
                LOG3A("AE sensor exp[%d] result ag %d dg %d coarse: %d fine: %d llp:%d fll:%d", i,
                      aeResult.exposures[i].sensor_exposure->analog_gain_code_global,
                      aeResult.exposures[i].sensor_exposure->digital_gain_global,
                      aeResult.exposures[i].sensor_exposure->coarse_integration_time,
                      aeResult.exposures[i].sensor_exposure->fine_integration_time,
                      aeResult.exposures[i].sensor_exposure->line_length_pixels,
                      aeResult.exposures[i].sensor_exposure->frame_length_lines);
            }
            LOG3A(" AE Converged : %s", aeResult.exposures[i].converged ? "YES" : "NO");
        }
    } else {
        LOGE("nullptr in StatsInputParams->frame_ae_parameters->exposures");
    }
    LOG3A("AE bracket mode = %d %s", aeResult.multiframe,
          aeResult.multiframe == ia_aiq_bracket_mode_ull ? "ULL" : "none-ULL");

    if (aeResult.weight_grid &&
        aeResult.weight_grid->width != 0 &&
        aeResult.weight_grid->height != 0) {
        LOG3A("AE weight grid [%dx%d]", aeResult.weight_grid->width, aeResult.weight_grid->height);
        if (aeResult.weight_grid->weights) {
            for (int i = 0; i < 5 && i < aeResult.weight_grid->height; i++) {
                LOG3A("AE weight_grid[%d] = %d ", aeResult.weight_grid->width/2,
                      aeResult.weight_grid->weights[aeResult.weight_grid->width/2]);
            }
        }
    }

    if (aeResult.aperture_control) {
        LOG3A("AE aperture fn = %f, iris command = %d, code = %d",
              aeResult.aperture_control->aperture_fn,
              aeResult.aperture_control->dc_iris_command,
              aeResult.aperture_control->code);
    }

    return OK;
}

int AiqUtils::dumpAfResults(const ia_aiq_af_results &afResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) {
        return OK;
    }

    LOG3A("@%s", __func__);

    LOG3A("AF results current_focus_distance %d final_position_reached %s",
          afResult.current_focus_distance,
          afResult.final_lens_position_reached ? "TRUE":"FALSE");
    LOG3A("AF results driver_action %d, next_lens_position %d",
          afResult.lens_driver_action,
          afResult.next_lens_position);
    LOG3A("AF results use_af_assist %s",
          afResult.use_af_assist? "TRUE":"FALSE");

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

int AiqUtils::dumpAwbResults(const ia_aiq_awb_results &awbResult)
{
    if (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_AIQ)) {
        return OK;
    }

    LOG3A("@%s", __func__);

    LOG3A("AWB result: accurate_r/g %f, accurate_b/g %f final_r/g %f final_b/g %f",
          awbResult.accurate_r_per_g,
          awbResult.accurate_b_per_g,
          awbResult.final_r_per_g,
          awbResult.final_b_per_g);
    LOG3A("AWB result: cct_estimate %d, distance_from_convergence %f",
          awbResult.cct_estimate,
          awbResult.distance_from_convergence);

    return OK;
}

int AiqUtils::deepCopyAeResults(const ia_aiq_ae_results& src, ia_aiq_ae_results* dst)
{
    LOG3A("@%s", __func__);
    dumpAeResults(src);

    /**
     * lets check that all the pointers are there
     * in the source and in the destination
     */
    CheckError(!dst||!dst->exposures||!dst->flashes||!dst->weight_grid||!dst->weight_grid->weights
        ,BAD_VALUE ,"Failed to deep copy AE result- invalid destination");

    CheckError(!src.exposures||!src.flashes||!src.weight_grid||!src.weight_grid->weights
        ,BAD_VALUE ,"Failed to deep copy AE result- invalid source");

    dst->lux_level_estimate = src.lux_level_estimate;
    dst->flicker_reduction_mode = src.flicker_reduction_mode;
    dst->multiframe = src.multiframe;
    dst->num_flashes = src.num_flashes;
    dst->num_exposures = src.num_exposures;
    if (src.aperture_control) {
        *dst->aperture_control = *src.aperture_control;
    }
    for (unsigned int i = 0; i < dst->num_exposures; i++)
    {
        dst->exposures[i].converged = src.exposures[i].converged;
        dst->exposures[i].distance_from_convergence = src.exposures[i].distance_from_convergence;
        dst->exposures[i].exposure_index = src.exposures[i].exposure_index;
        if (src.exposures[i].exposure) {
            *dst->exposures[i].exposure = *src.exposures[i].exposure;
        }
        if (src.exposures[i].sensor_exposure) {
            *dst->exposures[i].sensor_exposure = *src.exposures[i].sensor_exposure;
        }
    }

    // Copy weight grid
    dst->weight_grid->width = src.weight_grid->width;
    dst->weight_grid->height = src.weight_grid->height;

    unsigned int gridElements  = src.weight_grid->width *
                                 src.weight_grid->height;
    gridElements = CLIP(gridElements, MAX_AE_GRID_SIZE, 1);
    MEMCPY_S(dst->weight_grid->weights, gridElements*sizeof(char),
             src.weight_grid->weights, gridElements*sizeof(char));

    // Copy the flash info structure
    MEMCPY_S(dst->flashes, NUM_FLASH_LEDS*sizeof(ia_aiq_flash_parameters),
             src.flashes, NUM_FLASH_LEDS*sizeof(ia_aiq_flash_parameters));

    return OK;
}

int AiqUtils::deepCopyAfResults(const ia_aiq_af_results& src, ia_aiq_af_results* dst)
{
    LOG3A("@%s", __func__);
    dumpAfResults(src);

    CheckError(!dst, BAD_VALUE, "Failed to deep copy Af result- invalid destination or Source");

    MEMCPY_S(dst, sizeof(ia_aiq_af_results), &src, sizeof(ia_aiq_af_results));
    return OK;
}

int AiqUtils::deepCopyAwbResults(const ia_aiq_awb_results& src, ia_aiq_awb_results* dst)
{
    LOG3A("@%s", __func__);
    dumpAwbResults(src);

    CheckError(!dst, BAD_VALUE, "Failed to deep copy Awb result- invalid destination or Source");

    MEMCPY_S(dst, sizeof(ia_aiq_awb_results), &src, sizeof(ia_aiq_awb_results));
    return OK;
}

int AiqUtils::deepCopyGbceResults(const ia_aiq_gbce_results& src, ia_aiq_gbce_results* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst||!dst->r_gamma_lut||!dst->g_gamma_lut||!dst->b_gamma_lut||!dst->tone_map_lut
        ,BAD_VALUE ,"Failed to deep copy GBCE result- invalid destination");
    CheckError(!src.r_gamma_lut||!src.g_gamma_lut||!src.b_gamma_lut
        ,BAD_VALUE ,"Failed to deep copy GBCE result- invalid source");

    MEMCPY_S(dst->r_gamma_lut, src.gamma_lut_size*sizeof(float),
             src.r_gamma_lut, src.gamma_lut_size*sizeof(float));

    MEMCPY_S(dst->g_gamma_lut, src.gamma_lut_size*sizeof(float),
             src.g_gamma_lut, src.gamma_lut_size*sizeof(float));

    MEMCPY_S(dst->b_gamma_lut, src.gamma_lut_size*sizeof(float),
             src.b_gamma_lut, src.gamma_lut_size*sizeof(float));

    dst->gamma_lut_size = src.gamma_lut_size;

    // Copy tone mapping table
    if (src.tone_map_lut != nullptr)
    {
        MEMCPY_S(dst->tone_map_lut, src.tone_map_lut_size * sizeof(float),
                 src.tone_map_lut, src.tone_map_lut_size * sizeof(float));

    }
    dst->tone_map_lut_size = src.tone_map_lut_size; // zero indicates GBCE is ineffective.

    return OK;
}

int AiqUtils::deepCopyPaResults(const ia_aiq_pa_results_v1& src, ia_aiq_pa_results_v1* dst,
                                ia_aiq_advanced_ccm_t* preferredAcm)
{
    LOG3A("%s", __func__);

    CheckError(!dst, BAD_VALUE ,"Failed to deep copy PA result- invalid destination");

    MEMCPY_S(dst->color_conversion_matrix, sizeof(dst->color_conversion_matrix),
             src.color_conversion_matrix, sizeof(src.color_conversion_matrix));
    for (unsigned int i = 0; i < 4; i++)
        for (unsigned int j = 0; j < 4; j++)
            dst->black_level_4x4[i][j] = src.black_level_4x4[i][j];
    dst->color_gains = src.color_gains;
    dst->saturation_factor = src.saturation_factor;
    dst->brightness_level = src.brightness_level;

    if (src.ir_weight) {
        unsigned long int irSize = src.ir_weight->width * src.ir_weight->height;
        if (irSize) {
            LOG3A("%s irSize = %ld", __func__, irSize);
            MEMCPY_S(dst->ir_weight->ir_weight_grid_R, irSize * sizeof(unsigned short),
                     src.ir_weight->ir_weight_grid_R, irSize * sizeof(unsigned short));
            MEMCPY_S(dst->ir_weight->ir_weight_grid_G, irSize * sizeof(unsigned short),
                     src.ir_weight->ir_weight_grid_G, irSize * sizeof(unsigned short));
            MEMCPY_S(dst->ir_weight->ir_weight_grid_B, irSize * sizeof(unsigned short),
                     src.ir_weight->ir_weight_grid_B, irSize * sizeof(unsigned short));
            dst->ir_weight->width = src.ir_weight->width;
            dst->ir_weight->height = src.ir_weight->height;
        }
    }

    if (src.preferred_acm && src.preferred_acm->sector_count) {
        dst->preferred_acm = preferredAcm;

        LOG3A("%s advanced ccm sector count = %d", __func__, src.preferred_acm->sector_count);
        MEMCPY_S(dst->preferred_acm->hue_of_sectors,
                 src.preferred_acm->sector_count * sizeof(unsigned int),
                 src.preferred_acm->hue_of_sectors,
                 src.preferred_acm->sector_count * sizeof(unsigned int));
        MEMCPY_S(dst->preferred_acm->advanced_color_conversion_matrices,
                 src.preferred_acm->sector_count * sizeof(float[3][3]),
                 src.preferred_acm->advanced_color_conversion_matrices,
                 src.preferred_acm->sector_count  * sizeof(float[3][3]));
        dst->preferred_acm->sector_count = src.preferred_acm->sector_count;
    } else {
        dst->preferred_acm = nullptr;
    }

    /* current linearization.size is zero, set related pointers to nullptr */
    dst->linearization.r = nullptr;
    dst->linearization.gr = nullptr;
    dst->linearization.gb = nullptr;
    dst->linearization.b = nullptr;
    dst->linearization.size = 0;

    return OK;
}

int AiqUtils::deepCopyLtmResults(const ia_ltm_results& src, ia_ltm_results* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst, BAD_VALUE ,"Failed to deep copy LTM result- invalid destination");

    MEMCPY_S(dst, sizeof(ia_ltm_results), &src, sizeof(ia_ltm_results));

    return OK;
}

int AiqUtils::deepCopyLtmDRCParams(const ia_ltm_drc_params& src, ia_ltm_drc_params* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst, BAD_VALUE ,"Failed to deep copy LTM DRC params- invalid destination");

    MEMCPY_S(dst, sizeof(ia_ltm_drc_params), &src, sizeof(ia_ltm_drc_params));

    return OK;
}

int AiqUtils::deepCopySaResults(const ia_aiq_sa_results_v1& src, ia_aiq_sa_results_v1* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst, BAD_VALUE, "Failed to deep copy SA result- invalid destination");

    const size_t gridSize = src.width * src.height;
    if ((size_t)(dst->width * dst->height) < gridSize) {
        LOG3A("%s: increases the size of LSC table from %dx%d to %dx%d.",
              __func__, dst->width, dst->height, src.width, src.height);

        // allocated buffer is too small to accomodate what SA returns.
        for (int i = 0; i < MAX_BAYER_ORDER_NUM; ++i) {
            for (int j = 0; j < MAX_BAYER_ORDER_NUM; ++j) {
                // re-allocate
                delete [] dst->lsc_grid[i][j];
                dst->lsc_grid[i][j] = new unsigned short[gridSize];

                // copy a table
                if (src.lsc_grid[i][j]) {
                    MEMCPY_S(dst->lsc_grid[i][j], gridSize * sizeof(unsigned short),
                             src.lsc_grid[i][j], gridSize * sizeof(unsigned short));
                }
            }
        }
    } else {
        // copy tables
        for (int i = 0; i < MAX_BAYER_ORDER_NUM; i++) {
            for (int j = 0; j < MAX_BAYER_ORDER_NUM; j++) {
                if (dst->lsc_grid[i][j] && src.lsc_grid[i][j]) {
                    MEMCPY_S(dst->lsc_grid[i][j], gridSize * sizeof(unsigned short),
                             src.lsc_grid[i][j], gridSize * sizeof(unsigned short));
                }
            }
        }
    }

    dst->width = src.width;
    dst->height = src.height;
    dst->lsc_update = src.lsc_update;
    dst->fraction_bits = src.fraction_bits;
    dst->color_order = src.color_order;

    MEMCPY_S(dst->light_source, sizeof(dst->light_source), src.light_source, sizeof(src.light_source));
    MEMCPY_S(&dst->frame_params, sizeof(dst->frame_params), &src.frame_params, sizeof(src.frame_params));

    return OK;
}

int AiqUtils::deepCopyDvsResults(const ia_dvs_morph_table& src, ia_dvs_morph_table* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst || !dst->xcoords_y || !dst->ycoords_y
          || !dst->xcoords_uv || !dst->ycoords_uv
          || !dst->xcoords_uv_float || !dst->ycoords_uv_float
          ,BAD_VALUE ,"Failed to deep copy DVS result- invalid destination");

    CheckError(!src.xcoords_y || !src.ycoords_y
          || !src.xcoords_uv || !src.ycoords_uv
          || !src.xcoords_uv_float || !src.ycoords_uv_float
          ,BAD_VALUE ,"Failed to deep copy DVS result- invalid source");

    CheckError(src.width_y == 0 || src.height_y == 0 || src.width_uv == 0 || src.height_uv == 0
          ,BAD_VALUE ,"Failed to deep copy DVS result- invalid source size y[%dx%d] uv[%dx%d]",
          src.width_y, src.height_y, src.width_uv, src.height_uv);

    dst->width_y = src.width_y;
    dst->height_y = src.height_y;
    dst->width_uv = src.width_uv;
    dst->height_uv = src.height_uv;
    dst->morph_table_changed = src.morph_table_changed;
    unsigned int SizeY = dst->width_y  * dst->height_y * sizeof(int32_t);
    unsigned int SizeUV = dst->width_uv * dst->height_uv * sizeof(int32_t);
    MEMCPY_S(dst->xcoords_y, SizeY, src.xcoords_y, SizeY);
    MEMCPY_S(dst->ycoords_y, SizeY, src.ycoords_y, SizeY);
    MEMCPY_S(dst->xcoords_uv, SizeUV, src.xcoords_uv, SizeUV);
    MEMCPY_S(dst->ycoords_uv, SizeUV, src.ycoords_uv, SizeUV);

    SizeUV = dst->width_uv * dst->height_uv * sizeof(float);
    MEMCPY_S(dst->xcoords_uv_float, SizeUV, src.xcoords_uv_float, SizeUV);
    MEMCPY_S(dst->ycoords_uv_float, SizeUV, src.ycoords_uv_float, SizeUV);

    return OK;
}

int AiqUtils::deepCopyDvsResults(const ia_dvs_image_transformation& src, ia_dvs_image_transformation* dst)
{
    LOG3A("%s", __func__);

    CheckError(!dst,BAD_VALUE ,"Failed to deep copy DVS result- invalid destination");

    dst->num_homography_matrices = src.num_homography_matrices;
    MEMCPY_S(dst->matrices, sizeof(dst->matrices), src.matrices, sizeof(src.matrices));

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

void AiqUtils::applyTonemapGamma(float gamma, ia_aiq_gbce_results* results) {
    CheckError(gamma < EPSILON, VOID_VALUE, "Bad gamma %f", gamma);
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < TONEMAP_MIN_POINTS, VOID_VALUE, "Bad gamma lut size (%d) in gbce results", lutSize);
    for (int i = 0; i < lutSize; i++) {
        results->g_gamma_lut[i] = pow(i / static_cast<float>(lutSize), 1 / gamma);
    }

    MEMCPY_S(results->b_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
    MEMCPY_S(results->r_gamma_lut, lutSize * sizeof(float),
             results->g_gamma_lut, lutSize * sizeof(float));
}

void AiqUtils::applyTonemapSRGB(ia_aiq_gbce_results* results) {
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < TONEMAP_MIN_POINTS, VOID_VALUE, "Bad gamma lut size (%d) in gbce results", lutSize);
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

void AiqUtils::applyTonemapREC709(ia_aiq_gbce_results* results) {
    CheckError(!results, VOID_VALUE, "gbce results nullptr");

    int lutSize = results->gamma_lut_size;
    CheckError(lutSize < TONEMAP_MIN_POINTS, VOID_VALUE, "Bad gamma lut size (%d) in gbce results", lutSize);
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

void AiqUtils::applyTonemapCurve(const camera_tonemap_curves_t& curves, ia_aiq_gbce_results* results) {
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
                                           ia_aiq_awb_results* results) {
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

float AiqUtils::calculateHyperfocalDistance(const ia_cmc_t &cmcData) {
    LOG2("@%s", __func__);

    float pixelSizeMicro = 100.0f;  // size of pixels in um, default to avoid division by 0
    float focalLengthMillis = 0.0f;
    const float DEFAULT_HYPERFOCAL_DISTANCE = 5000.0f;

    cmc_optomechanics_t *optoInfo = cmcData.cmc_parsed_optics.cmc_optomechanics;
    if (optoInfo) {
        // Pixel size is stored in CMC in hundreds of micrometers
        pixelSizeMicro = optoInfo->sensor_pix_size_h / 100;
        // focal length is stored in CMC in hundreds of millimeters
        focalLengthMillis = static_cast<float>(optoInfo->effect_focal_length) / 100;
    }

    // fixed aperture, the fn should be divided 100 because the value
    // is multiplied 100 in cmc avoid division by 0
    if (!cmcData.cmc_parsed_optics.lut_apertures ||
        cmcData.cmc_parsed_optics.lut_apertures[0] == 0) {
        LOG2("lut apertures is not provided or zero in the cmc. Using default");
        return DEFAULT_HYPERFOCAL_DISTANCE;
    }

    float fNumber = static_cast<float>(cmcData.cmc_parsed_optics.lut_apertures[0]) / 100;
    // assuming square pixel
    const int CIRCLE_OF_CONFUSION_IN_PIXELS = 2;
    float cocMicros = pixelSizeMicro * CIRCLE_OF_CONFUSION_IN_PIXELS;
    float hyperfocalDistanceMillis = 1000 * (focalLengthMillis * focalLengthMillis) /
                                     (fNumber * cocMicros);

    return (hyperfocalDistanceMillis == 0.0f) ? DEFAULT_HYPERFOCAL_DISTANCE :
                                                hyperfocalDistanceMillis;
}

} /* namespace icamera */
