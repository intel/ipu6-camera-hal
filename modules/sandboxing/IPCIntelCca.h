/*
 * Copyright (C) 2020 Intel Corporation.
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

#include "IntelCCA.h"

namespace icamera {
struct intel_cca_run_aec_data {
    uint64_t frameId;
    cca::cca_ae_input_params inParams;

    cca::cca_ae_results results;
};

struct intel_cca_run_aiq_data {
    uint64_t frameId;
    cca::cca_aiq_params inParams;

    cca::cca_aiq_results results;
};

struct intel_cca_run_ltm_data {
    uint64_t frameId;
    cca::cca_ltm_input_params inParams;
};

struct intel_cca_run_aic_data {
    uint64_t frameId;
    cca::cca_pal_input_params* inParams;
    int32_t inParamsHandle;

    ia_binary_data palOutData;
    int32_t palDataHandle;
};

struct intel_cca_mkn_data {
    ia_mkn_trg type;

    cca::cca_mkn results;
};

struct intel_cca_update_tuning_data {
    uint8_t lardTags;
    ia_lard_input_params lardParams;
};

#define MAX_INTEL_CCA_VERSION_PARAMS_DATA_SIZE 100
struct intel_cca_version_data {
    char data[MAX_INTEL_CCA_VERSION_PARAMS_DATA_SIZE];
    uint32_t size;
};

struct intel_cca_decode_stats_data {
    ia_binary_data statsBuffer;
    int32_t statsHandle;

    ia_isp_bxt_statistics_query_results_t results;
};

struct intel_cca_get_pal_data_size {
    cca::cca_program_group pg;

    uint32_t returnSize;
};
} /* namespace icamera */
