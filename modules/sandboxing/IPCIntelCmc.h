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

#include <ia_cmc_types.h>
#include <ia_types.h>

#include "modules/sandboxing/IPCCommon.h"

namespace icamera {
typedef struct {
    cmc_black_level_t cmc_black_level;
    cmc_black_level_lut_t cmc_black_level_luts;
} cmc_parsed_black_level_data;

typedef struct {
    cmc_lens_shading_t cmc_lens_shading;
    cmc_lsc_grid_t cmc_lsc_grids;
    uint16_t lsc_grids;
    chromaticity_t cmc_lsc_rg_bg_ratios;
} cmc_parsed_lens_shading_data;

typedef struct {
    cmc_optomechanics_t cmc_optomechanics;
    uint16_t lut_apertures;
} cmc_parsed_optics_data;

typedef struct {
    cmc_color_matrices_t cmc_color_matrices;
    cmc_color_matrix_t cmc_color_matrix;
    uint16_t ccm_estimate_method;
} cmc_parsed_color_matrices_data;

#define MAX_NUM_SEGMENTS 512
#define MAX_NUM_ANALOG_PAIRS 1024
typedef struct {
    cmc_analog_gain_conversion_t cmc_analog_gain_conversion;
    cmc_analog_gain_segment_t cmc_analog_gain_segments[MAX_NUM_SEGMENTS];
    cmc_analog_gain_pair_t cmc_analog_gain_pairs[MAX_NUM_ANALOG_PAIRS];
} cmc_parsed_analog_gain_conversion_data;

struct ia_cmc_data {
    ia_cmc_t base;

    cmc_general_data_t cmc_general_data;
    cmc_parsed_black_level_data cmc_parsed_black_level;
    cmc_saturation_level_t cmc_saturation_level;
    cmc_sensitivity_t cmc_sensitivity;
    cmc_parsed_lens_shading_data cmc_parsed_lens_shading;
    cmc_parsed_optics_data cmc_parsed_optics;
    cmc_parsed_color_matrices_data cmc_parsed_color_matrices;
    cmc_parsed_analog_gain_conversion_data cmc_parsed_analog_gain_conversion;

    uintptr_t cmcRemoteHandle;  // it stores the remote cmc pointer.
};

struct cmc_init_params {
    ia_binary_data_mod input;
    ia_cmc_data results;
};

struct cmc_deinit_params {
    uintptr_t cmc_handle;
};

class IPCIntelCmc {
 public:
    IPCIntelCmc();
    virtual ~IPCIntelCmc();

    // for init
    bool clientFlattenInit(const ia_binary_data& aiqb, cmc_init_params* params);
    bool clientUnflattenInit(const cmc_init_params& params, ia_cmc_t** cmc,
                             uintptr_t* cmcRemoteHandle);
    bool serverFlattenInit(const ia_cmc_t& cmc, cmc_init_params* params);
    bool serverUnflattenInit(const cmc_init_params& pData, ia_binary_data* aiqb);

    bool flattenCmcData(const ia_cmc_t* cmc, ia_cmc_data* results);
    bool unflattenCmcData(ia_cmc_data* results);
};
} /* namespace icamera */
