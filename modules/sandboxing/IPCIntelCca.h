/*
 * Copyright (C) 2020-2023 Intel Corporation.
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

struct intel_cca_struct_data {
    int cameraId;
    TuningMode tuningMode;
};

struct intel_cca_init_data {
    int cameraId;
    TuningMode tuningMode;

    cca::cca_init_params inParams;
};

struct intel_cca_decode_stats_data {
    int cameraId;
    TuningMode tuningMode;

    ia_binary_data statsBuffer;
    int32_t statsHandle;
    uint32_t bitmap;
    cca::cca_out_stats outStats;

    ia_isp_bxt_statistics_query_results_t results;
};

struct intel_cca_run_aec_data {
    int cameraId;
    TuningMode tuningMode;

    uint64_t frameId;
    cca::cca_ae_input_params inParams;

    bool hasStats;
    cca::cca_stats_params inStatsParams;

    bool hasDecodeStats;
    intel_cca_decode_stats_data decodeStatsParams;

    cca::cca_ae_results results;
};

struct intel_cca_run_aiq_data {
    int cameraId;
    TuningMode tuningMode;

    uint64_t frameId;
    cca::cca_aiq_params inParams;

    cca::cca_aiq_results results;

    ia_mkn_trg type;

    cca::cca_mkn* mknResults;
    int32_t mknResultsHandle;
};

struct intel_cca_run_ltm_data {
    int cameraId;
    TuningMode tuningMode;

    uint64_t frameId;
    cca::cca_ltm_input_params inParams;
};

struct intel_cca_update_zoom_data {
    int cameraId;
    TuningMode tuningMode;

    cca::cca_dvs_zoom inParams;
    uint32_t streamId;
};

struct intel_cca_run_dvs_data {
    int cameraId;
    TuningMode tuningMode;

    uint64_t frameId;
    uint32_t streamId;
};

struct intel_cca_run_aic_data {
    int cameraId;
    TuningMode tuningMode;

    uint64_t frameId;
    cca::cca_pal_input_params* inParams;
    int32_t inParamsHandle;

    ia_binary_data palOutData;
    int32_t palDataHandle;
};

struct intel_cca_get_cmc_data {
    int cameraId;
    TuningMode tuningMode;
    cca::cca_cpf cpf;

    cca::cca_cmc results;
};

struct intel_cca_get_aiqd_data {
    int cameraId;
    TuningMode tuningMode;

    cca::cca_aiqd results;
};

struct intel_cca_update_tuning_data {
    int cameraId;
    TuningMode tuningMode;

    uint8_t lardTags;
    ia_lard_input_params lardParams;
    cca::cca_nvm nvmParams;
    int32_t streamId;
};

struct intel_cca_deinit_data {
    int cameraId;
    TuningMode tuningMode;
};

struct intel_cca_get_pal_data_size {
    int cameraId;
    TuningMode tuningMode;

    cca::cca_program_group pg;

    uint32_t returnSize;
};
} /* namespace icamera */
