/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <ia_aiq.h>

#include "FaceBase.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
struct aiq_init_params {
    unsigned int aiqb_size;
    unsigned int nvm_size;
    unsigned int aiqd_size;
    unsigned int stats_max_width;
    unsigned int stats_max_height;
    unsigned int max_num_stats_in;
    uintptr_t ia_mkn;
    uintptr_t cmcRemoteHandle;
    uintptr_t results;
};

struct aiq_deinit_params {
    uintptr_t aiq_handle;
};

struct af_run_params {
    uintptr_t aiq_handle;

    ia_aiq_af_input_params base;
    ia_rectangle focus_rect;
    ia_aiq_manual_focus_parameters manual_focus_parameters;

    ia_aiq_af_results results;
};

#define MAX_NUM_GAMMA_LUTS 2048
#define MAX_NUM_TOME_MAP_LUTS 2048
struct gbce_results_params {
    ia_aiq_gbce_results base;

    float r_gamma_lut[MAX_NUM_GAMMA_LUTS];
    float b_gamma_lut[MAX_NUM_GAMMA_LUTS];
    float g_gamma_lut[MAX_NUM_GAMMA_LUTS];
    float tone_map_lut[MAX_NUM_TOME_MAP_LUTS];
};
struct gbce_run_params {
    uintptr_t aiq_handle;

    ia_aiq_gbce_input_params base;

    gbce_results_params res;
};

#define MAX_NUM_EXPOSURES 3
#define MAX_NUM_FLASHES 1
#define MAX_NUM_OF_EXPOSURE_PLANS 4
#define MAX_SIZE_WEIGHT_GRID (128 * 128)
struct ae_run_params_results {
    ia_aiq_ae_results base;

    ia_aiq_ae_exposure_result exposures[MAX_NUM_EXPOSURES];
    ia_aiq_hist_weight_grid weight_grid;
    ia_aiq_flash_parameters flashes[MAX_NUM_FLASHES];
    ia_aiq_aperture_control aperture_control;

    // the below is in ia_aiq_ae_exposure_result exposures[MAX_NUM_EXPOSURES];
    ia_aiq_exposure_parameters exposure[MAX_NUM_EXPOSURES];
    ia_aiq_exposure_sensor_parameters sensor_exposure[MAX_NUM_EXPOSURES];
    unsigned int exposure_plan_ids[MAX_NUM_EXPOSURES][MAX_NUM_OF_EXPOSURE_PLANS];

    // the below is in ia_aiq_hist_weight_grid weight_grid;
    unsigned char weights[MAX_SIZE_WEIGHT_GRID];
};

struct ae_run_params {
    uintptr_t aiq_handle;

    ia_aiq_ae_input_params base;
    ia_aiq_exposure_sensor_descriptor sensor_descriptor;
    ia_rectangle exposure_window;
    ia_coordinate exposure_coordinate;
    long manual_exposure_time_us;
    float manual_analog_gain;
    short manual_iso;
    ia_aiq_ae_features aec_features;
    ia_aiq_ae_manual_limits manual_limits;

    ae_run_params_results res;
};

struct awb_run_params {
    uintptr_t aiq_handle;

    ia_aiq_awb_input_params base;
    ia_aiq_awb_manual_cct_range manual_cct_range;
    ia_coordinate manual_white_coordinate;

    ia_aiq_awb_results results;
};

#define MAX_NUM_LUTS 128
#define MAX_SECTOR_COUNT 128
#define MAX_IR_WIDTH 128
#define MAX_IR_HEIGHT 128
#define MAX_NUM_IR_BLOCKS (MAX_IR_WIDTH * MAX_IR_HEIGHT)
#define MAX_NUM_IR_MODES 5
struct pa_run_params_results_v1 {
    ia_aiq_pa_results_v1 base;

    ia_aiq_advanced_ccm_t preferred_acm;
    ia_aiq_ir_weight_t ir_weight;
    ia_aiq_rgbir_t rgbir;

    // for ia_aiq_color_channels_lut linearization
    float gr[MAX_NUM_LUTS];
    float r[MAX_NUM_LUTS];
    float b[MAX_NUM_LUTS];
    float gb[MAX_NUM_LUTS];

    // for ia_aiq_advanced_ccm_t *preferred_acm
    unsigned int hue_of_sectors[MAX_SECTOR_COUNT];
    float advanced_color_conversion_matrices[MAX_SECTOR_COUNT][3][3];

    // for ia_aiq_ir_weight_t *ir_weight
    uint16_t ir_weight_grid_R[MAX_NUM_IR_BLOCKS];
    uint16_t ir_weight_grid_G[MAX_NUM_IR_BLOCKS];
    uint16_t ir_weight_grid_B[MAX_NUM_IR_BLOCKS];

    // for ia_aiq_rgbir_t *rgbir
    ia_aiq_rgbir_model_t models[MAX_NUM_IR_MODES];
};

struct ia_atbx_face_state_data {
    ia_atbx_face_state base;

    ia_atbx_face faces[MAX_FACES_DETECTABLE];
};

struct pa_run_v1_params {
    uintptr_t aiq_handle;

    ia_aiq_pa_input_params base;
    ia_aiq_awb_results awb_results;
    ia_aiq_exposure_parameters exposure_params;
    ia_aiq_color_channels color_gains;

    pa_run_params_results_v1 res;
};

#define LSC_MAX_BAYER_ORDER_NUM 4
#define LSC_TABLE_MAX_WIDTH 100
#define LSC_TABLE_MAX_HEIGHT 100
#define LSC_TABLE_MAX_SIZE (LSC_TABLE_MAX_WIDTH * LSC_TABLE_MAX_HEIGHT)
struct lsc_grid_content {
    uint16_t content[LSC_TABLE_MAX_SIZE];
};
struct sa_run_v2_params_results {
    ia_aiq_sa_results_v1 base;

    lsc_grid_content lsc_grid[LSC_MAX_BAYER_ORDER_NUM][LSC_MAX_BAYER_ORDER_NUM];
};

struct sa_run_v2_params {
    uintptr_t aiq_handle;

    ia_aiq_sa_input_params_v1 base;
    ia_aiq_frame_params sensor_frame_params;
    ia_aiq_awb_results awb_results;

    sa_run_v2_params_results res;
};

#define MAX_IA_BINARY_DATA_PARAMS_SIZE 500000
struct ia_binary_data_params {
    uintptr_t aiq_handle;
    uint8_t data[MAX_IA_BINARY_DATA_PARAMS_SIZE];
    unsigned int size;
};

#define MAX_IA_AIQ_VERSION_PARAMS_DATA_SIZE 100
struct ia_aiq_version_params {
    uintptr_t aiq_handle;
    char data[MAX_IA_AIQ_VERSION_PARAMS_DATA_SIZE];
    unsigned int size;
};

#define MAX_WIDTH 96
#define MAX_HEIGHT 72
#define MAX_NUM_BLOCKS (MAX_WIDTH * MAX_HEIGHT)
struct ia_aiq_rgbs_grid_data {
    ia_aiq_rgbs_grid base;

    rgbs_grid_block blocks_ptr[MAX_NUM_BLOCKS];
};

struct ia_aiq_hdr_rgbs_grid_data {
    ia_aiq_hdr_rgbs_grid base;

    hdr_rgbs_grid_block blocks_ptr[MAX_NUM_BLOCKS];
};

#define MAX_AF_GRID_WIDTH 96
#define MAX_AF_GRID_HEIGHT 72
#define MAX_AF_GRID_SIZE (MAX_AF_GRID_HEIGHT * MAX_AF_GRID_WIDTH)
struct ia_aiq_af_grid_data {
    ia_aiq_af_grid base;

    int filter_response_1[MAX_AF_GRID_SIZE];
    int filter_response_2[MAX_AF_GRID_SIZE];
};

#define MAX_DEPTH_GRID_WIDHT 128
#define MAX_DEPTH_GRID_HEIGHT 128
#define MAX_DEPTH_GRID_SIZE (MAX_DEPTH_GRID_WIDHT * MAX_DEPTH_GRID_HEIGHT)
struct ia_aiq_depth_grid_data {
    ia_aiq_depth_grid base;

    ia_rectangle grid_rect[MAX_DEPTH_GRID_SIZE];
    int depth_data[MAX_DEPTH_GRID_SIZE];
    unsigned char confidence[MAX_DEPTH_GRID_SIZE];
};

#define MAX_NUMBER_OF_GRIDS 1
#define MAX_NUMBER_OF_AF_GRIDS 1
#define MAX_NUMBER_OF_HISTROGRAMS 1
#define MAX_NUMBER_OF_DEPTH_GRIDS 1
#define MAX_IR_WEIGHT_GRID_DATA_SIZE 480
struct set_statistics_params_data {
    ia_aiq_statistics_input_params_v4 base;

    ae_run_params_results frame_ae_parameters;

    ia_aiq_af_results frame_af_parameters;

    const ia_aiq_rgbs_grid* rgbs_grids_array[MAX_NUMBER_OF_GRIDS];
    ia_aiq_rgbs_grid_data rgbs_grids[MAX_NUMBER_OF_GRIDS];

    ia_aiq_hdr_rgbs_grid_data hdr_rgbs_grid;

    const ia_aiq_af_grid* af_grids_array[MAX_NUMBER_OF_AF_GRIDS];
    ia_aiq_af_grid_data af_grids[MAX_NUMBER_OF_AF_GRIDS];

    pa_run_params_results_v1 frame_pa_parameters;

    ia_atbx_face_state_data faces;

    ia_aiq_awb_results awb_results;

    sa_run_v2_params_results frame_sa_parameters;

    const ia_aiq_depth_grid* depth_grids_array[MAX_NUMBER_OF_DEPTH_GRIDS];
    ia_aiq_depth_grid_data depth_grids[MAX_NUMBER_OF_DEPTH_GRIDS];

    ia_aiq_grid ir_grid;
    unsigned short ir_grid_data[MAX_IR_WEIGHT_GRID_DATA_SIZE];
};

struct set_statistics_set_v4_params {
    uintptr_t ia_aiq;
    set_statistics_params_data input;
};

class IPCIntelAiq {
 public:
    IPCIntelAiq();
    virtual ~IPCIntelAiq();

    // for init
    bool clientFlattenInit(const ia_binary_data* aiqbData, const ia_binary_data* nvmData,
                           const ia_binary_data* aiqdData, unsigned int statsMaxWidth,
                           unsigned int statsMaxHeight, unsigned int maxNumStatsIn, uintptr_t cmc,
                           uintptr_t mkn, uint8_t* pData, unsigned int size);
    bool serverUnflattenInit(const void* pData, int dataSize, ia_binary_data* aiqbData,
                             ia_binary_data* nvmData, ia_binary_data* aiqdData);

    // for ae
    bool clientFlattenAe(uintptr_t aiq, const ia_aiq_ae_input_params& inParams,
                         ae_run_params* params);
    bool clientUnflattenAe(ae_run_params* params, ia_aiq_ae_results** results);
    bool serverUnflattenAe(ae_run_params* inParams, ia_aiq_ae_input_params** params);
    bool serverFlattenAe(const ia_aiq_ae_results& aeResults, ae_run_params* params);

    bool flattenAeResults(const ia_aiq_ae_results& aeResults, ae_run_params_results* res);
    bool unflattenAeResults(ae_run_params_results* res);

    // for af
    bool clientFlattenAf(uintptr_t aiq, const ia_aiq_af_input_params& inParams,
                         af_run_params* params);
    bool clientUnflattenAf(const af_run_params& params, ia_aiq_af_results** results);
    bool serverUnflattenAf(af_run_params* inParams, ia_aiq_af_input_params** params);
    bool serverFlattenAf(const ia_aiq_af_results& afResults, af_run_params* params);

    // for awb
    bool clientFlattenAwb(uintptr_t aiq, const ia_aiq_awb_input_params& inParams,
                          awb_run_params* params);
    bool clientUnflattenAwb(const awb_run_params& inParams, ia_aiq_awb_results** results);
    bool serverUnflattenAwb(awb_run_params* inParams, ia_aiq_awb_input_params** params);
    bool serverFlattenAwb(const ia_aiq_awb_results& awbResults, awb_run_params* params);

    // for gbce
    bool clientFlattenGbce(uintptr_t aiq, const ia_aiq_gbce_input_params& inParams,
                           gbce_run_params* params);
    bool clientUnflattenGbce(gbce_run_params* params, ia_aiq_gbce_results** results);
    bool serverFlattenGbce(const ia_aiq_gbce_results& gbceResults, gbce_run_params* params);

    bool flattenGbceResults(const ia_aiq_gbce_results& gbceResults, gbce_results_params* res);
    bool unflattenGbceResults(gbce_results_params* res);

    // for pa
    bool clientFlattenPaV1(uintptr_t aiq, const ia_aiq_pa_input_params& inParams,
                           pa_run_v1_params* params);
    bool clientUnflattenPaV1(pa_run_v1_params* params, ia_aiq_pa_results_v1** results);
    bool serverUnflattenPaV1(pa_run_v1_params* inParams, ia_aiq_pa_input_params** params);
    bool serverFlattenPaV1(const ia_aiq_pa_results_v1& paResults, pa_run_v1_params* params);

    bool flattenPaResultsV1(const ia_aiq_pa_results_v1& paResults, pa_run_params_results_v1* res);
    bool unflattenPaResultsV1(pa_run_params_results_v1* res);

    // for sa
    bool clientFlattenSaV2(uintptr_t aiq, const ia_aiq_sa_input_params_v1& inParams,
                           sa_run_v2_params* params);
    bool clientUnflattenSaV2(sa_run_v2_params* params, ia_aiq_sa_results_v1** results);
    bool serverUnflattenSaV2(const sa_run_v2_params& inParams, ia_aiq_sa_input_params_v1** params);
    bool serverFlattenSaV2(const ia_aiq_sa_results_v1& saResults, sa_run_v2_params* params);

    bool flattenSaResultsV2(const ia_aiq_sa_results_v1& saResults, sa_run_v2_params_results* res);
    bool unflattenSaResultsV2(sa_run_v2_params_results* res);

    // for statistics
    bool clientFlattenStatSetV4(uintptr_t aiq, const ia_aiq_statistics_input_params_v4& inParams,
                                set_statistics_set_v4_params* params);
    bool serverUnflattenStatSetV4(set_statistics_set_v4_params* inParams,
                                  ia_aiq_statistics_input_params_v4** params);
};
} /* namespace icamera */
