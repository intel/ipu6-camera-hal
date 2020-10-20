/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "IPC_INTEL_CMC"

#include "modules/sandboxing/IPCIntelCmc.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIntelCmc::IPCIntelCmc() {
    LOGIPC("@%s", __func__);
}

IPCIntelCmc::~IPCIntelCmc() {
    LOGIPC("@%s", __func__);
}

bool IPCIntelCmc::clientFlattenInit(const ia_binary_data& aiqb, cmc_init_params* params) {
    LOGIPC("@%s, aiqb: data:%p, size:%d, params:%p", __func__, aiqb.data, aiqb.size, params);

    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!aiqb.data, false, "@%s, aiqb.data is nullptr", __func__);
    CheckError(aiqb.size == 0, false, "@%s, aiqb.size is 0", __func__);
    CheckError(aiqb.size > sizeof(params->input.data), false, "@%s, aiqb:%d is too big", __func__,
               aiqb.size);

    ia_binary_data_mod* input = &params->input;
    MEMCPY_S(input->data, sizeof(input->data), aiqb.data, aiqb.size);
    input->size = aiqb.size;

    return true;
}

bool IPCIntelCmc::clientUnflattenInit(const cmc_init_params& params, ia_cmc_t** cmc,
                                      uintptr_t* cmcRemoteHandle) {
    LOGIPC("@%s, cmc:%p", __func__, cmc);
    CheckError(!cmc, false, "@%s, cmc is nullptr", __func__);

    int ret = unflattenCmcData(const_cast<ia_cmc_data*>(&params.results));
    CheckError(ret == false, false, "%s, unflattenCmcData fails", __func__);

    *cmc = const_cast<ia_cmc_t*>(&params.results.base);
    *cmcRemoteHandle = params.results.cmcRemoteHandle;

    return true;
}

bool IPCIntelCmc::serverUnflattenInit(const cmc_init_params& params, ia_binary_data* aiqb) {
    LOGIPC("@%s, aiqb:%p", __func__, aiqb);
    CheckError(aiqb == nullptr, false, "@%s, aiqb is nullptr", __func__);

    ia_binary_data_mod* input = const_cast<ia_binary_data_mod*>(&params.input);
    aiqb->data = input->data;
    aiqb->size = input->size;

    return true;
}

bool IPCIntelCmc::serverFlattenInit(const ia_cmc_t& cmc, cmc_init_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    int ret = flattenCmcData(&cmc, &params->results);
    CheckError(ret == false, false, "%s flattenCmcData fails", __func__);

    return true;
}

bool IPCIntelCmc::flattenCmcData(const ia_cmc_t* cmc, ia_cmc_data* results) {
    CheckError(!cmc || !results, false, "%s, cmc or results is nullptr", __func__);

    results->base = *cmc;
    results->cmcRemoteHandle = reinterpret_cast<uintptr_t>(cmc);

    ia_cmc_t* base = &results->base;
    if (base->cmc_general_data) {
        results->cmc_general_data = *base->cmc_general_data;
    }

    cmc_parsed_black_level_t* baseBL = &base->cmc_parsed_black_level;
    cmc_parsed_black_level_data* retBL = &results->cmc_parsed_black_level;
    if (baseBL->cmc_black_level) {
        retBL->cmc_black_level = *baseBL->cmc_black_level;
    }
    if (baseBL->cmc_black_level_luts) {
        retBL->cmc_black_level_luts = *baseBL->cmc_black_level_luts;
    }

    if (base->cmc_saturation_level) {
        results->cmc_saturation_level = *base->cmc_saturation_level;
    }

    if (base->cmc_sensitivity) {
        results->cmc_sensitivity = *base->cmc_sensitivity;
    }

    cmc_parsed_lens_shading_t* baseLS = &base->cmc_parsed_lens_shading;
    cmc_parsed_lens_shading_data* retLS = &results->cmc_parsed_lens_shading;
    if (baseLS->cmc_lens_shading) {
        retLS->cmc_lens_shading = *baseLS->cmc_lens_shading;
    }
    if (baseLS->cmc_lsc_grids) {
        retLS->cmc_lsc_grids = *baseLS->cmc_lsc_grids;
    }
    if (baseLS->lsc_grids) {
        retLS->lsc_grids = *baseLS->lsc_grids;
    }
    if (baseLS->cmc_lsc_rg_bg_ratios) {
        retLS->cmc_lsc_rg_bg_ratios = *baseLS->cmc_lsc_rg_bg_ratios;
    }

    cmc_parsed_optics_t* baseOptics = &base->cmc_parsed_optics;
    cmc_parsed_optics_data* retOptics = &results->cmc_parsed_optics;
    if (baseOptics->cmc_optomechanics) {
        retOptics->cmc_optomechanics = *baseOptics->cmc_optomechanics;
    }
    if (baseOptics->lut_apertures) {
        retOptics->lut_apertures = *baseOptics->lut_apertures;
    }

    cmc_parsed_color_matrices_t* baseCM = &base->cmc_parsed_color_matrices;
    cmc_parsed_color_matrices_data* retCM = &results->cmc_parsed_color_matrices;
    if (baseCM->cmc_color_matrices) {
        retCM->cmc_color_matrices = *baseCM->cmc_color_matrices;
    }
    if (baseCM->cmc_color_matrix) {
        // fix asan issue:base->cmc_parsed_color_matrices.cmc_color_matrix is not 4 aligned
        // use memcpy instead of *
        MEMCPY_S(&retCM->cmc_color_matrix, sizeof(retCM->cmc_color_matrix),
                 baseCM->cmc_color_matrix, sizeof(cmc_color_matrix_t));
    }
    if (baseCM->ccm_estimate_method) {
        retCM->ccm_estimate_method = *baseCM->ccm_estimate_method;
    }

    cmc_parsed_analog_gain_conversion_t* baseAGC = &base->cmc_parsed_analog_gain_conversion;
    cmc_parsed_analog_gain_conversion_data* retAGC = &results->cmc_parsed_analog_gain_conversion;
    if (baseAGC->cmc_analog_gain_conversion) {
        retAGC->cmc_analog_gain_conversion = *baseAGC->cmc_analog_gain_conversion;

        CheckError(baseAGC->cmc_analog_gain_conversion->num_segments > MAX_NUM_SEGMENTS, false,
                   "@%s, num_segments:%d is too big", __func__,
                   baseAGC->cmc_analog_gain_conversion->num_segments);
        CheckError(baseAGC->cmc_analog_gain_conversion->num_pairs > MAX_NUM_ANALOG_PAIRS, false,
                   "@%s, num_pairs:%d is too big", __func__,
                   baseAGC->cmc_analog_gain_conversion->num_pairs);

        if (baseAGC->cmc_analog_gain_segments) {
            MEMCPY_S(retAGC->cmc_analog_gain_segments, sizeof(retAGC->cmc_analog_gain_segments),
                     baseAGC->cmc_analog_gain_segments,
                     (sizeof(*baseAGC->cmc_analog_gain_segments) *
                      baseAGC->cmc_analog_gain_conversion->num_segments));
        }
        if (baseAGC->cmc_analog_gain_pairs) {
            MEMCPY_S(retAGC->cmc_analog_gain_pairs, sizeof(retAGC->cmc_analog_gain_pairs),
                     baseAGC->cmc_analog_gain_pairs,
                     (sizeof(*baseAGC->cmc_analog_gain_pairs) *
                      baseAGC->cmc_analog_gain_conversion->num_pairs));
        }
    }

    return true;
}

bool IPCIntelCmc::unflattenCmcData(ia_cmc_data* results) {
    LOGIPC("@%s", __func__);
    CheckError(!results, false, "%s, results is nullptr", __func__);

    ia_cmc_t* base = &results->base;

    if (base->cmc_general_data) {
        base->cmc_general_data = &results->cmc_general_data;
    }

    cmc_parsed_black_level_t* baseBL = &base->cmc_parsed_black_level;
    cmc_parsed_black_level_data* retBL = &results->cmc_parsed_black_level;
    if (baseBL->cmc_black_level) {
        baseBL->cmc_black_level = &retBL->cmc_black_level;
    }
    if (baseBL->cmc_black_level_luts) {
        baseBL->cmc_black_level_luts = &retBL->cmc_black_level_luts;
    }

    if (base->cmc_saturation_level) {
        base->cmc_saturation_level = &results->cmc_saturation_level;
    }

    if (base->cmc_sensitivity) {
        base->cmc_sensitivity = &results->cmc_sensitivity;
    }

    cmc_parsed_lens_shading_t* baseLS = &base->cmc_parsed_lens_shading;
    cmc_parsed_lens_shading_data* retLS = &results->cmc_parsed_lens_shading;
    if (baseLS->cmc_lens_shading) {
        baseLS->cmc_lens_shading = &retLS->cmc_lens_shading;
    }
    if (baseLS->cmc_lsc_grids) {
        baseLS->cmc_lsc_grids = &retLS->cmc_lsc_grids;
    }
    if (baseLS->lsc_grids) {
        baseLS->lsc_grids = &retLS->lsc_grids;
    }
    if (baseLS->cmc_lsc_rg_bg_ratios) {
        baseLS->cmc_lsc_rg_bg_ratios = &retLS->cmc_lsc_rg_bg_ratios;
    }

    cmc_parsed_optics_t* baseOptics = &base->cmc_parsed_optics;
    cmc_parsed_optics_data* retOptics = &results->cmc_parsed_optics;
    if (baseOptics->cmc_optomechanics) {
        baseOptics->cmc_optomechanics = &retOptics->cmc_optomechanics;
    }
    if (baseOptics->lut_apertures) {
        baseOptics->lut_apertures = &retOptics->lut_apertures;
    }

    cmc_parsed_color_matrices_t* baseCM = &base->cmc_parsed_color_matrices;
    cmc_parsed_color_matrices_data* retCM = &results->cmc_parsed_color_matrices;
    if (baseCM->cmc_color_matrices) {
        baseCM->cmc_color_matrices = &retCM->cmc_color_matrices;
    }
    if (baseCM->cmc_color_matrix) {
        baseCM->cmc_color_matrix = &retCM->cmc_color_matrix;
    }
    if (baseCM->ccm_estimate_method) {
        baseCM->ccm_estimate_method = &retCM->ccm_estimate_method;
    }

    cmc_parsed_analog_gain_conversion_t* baseAGC = &base->cmc_parsed_analog_gain_conversion;
    cmc_parsed_analog_gain_conversion_data* retAGC = &results->cmc_parsed_analog_gain_conversion;
    if (baseAGC->cmc_analog_gain_conversion) {
        baseAGC->cmc_analog_gain_conversion = &retAGC->cmc_analog_gain_conversion;

        if (baseAGC->cmc_analog_gain_segments) {
            baseAGC->cmc_analog_gain_segments = retAGC->cmc_analog_gain_segments;
        }
        if (baseAGC->cmc_analog_gain_pairs) {
            baseAGC->cmc_analog_gain_pairs = retAGC->cmc_analog_gain_pairs;
        }
    }

    return true;
}
} /* namespace icamera */
