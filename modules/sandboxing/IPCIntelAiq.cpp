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

#define LOG_TAG "IPCIntelAiq"

#include "modules/sandboxing/IPCIntelAiq.h"

#include <ia_types.h>

namespace icamera {
IPCIntelAiq::IPCIntelAiq() {
    LOGIPC("@%s", __func__);
}

IPCIntelAiq::~IPCIntelAiq() {
    LOGIPC("@%s", __func__);
}

// init
bool IPCIntelAiq::clientFlattenInit(const ia_binary_data* aiqbData, const ia_binary_data* nvmData,
                                    const ia_binary_data* aiqdData, unsigned int statsMaxWidth,
                                    unsigned int statsMaxHeight, unsigned int maxNumStatsIn,
                                    uintptr_t cmc, uintptr_t mkn, uint8_t* pData,
                                    unsigned int size) {
    LOGIPC("@%s, aiqbData:%p, nvmData:%p, aiqdData:%p", __func__, aiqbData, nvmData, aiqdData);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);

    uint8_t* ptr = pData;
    memset(ptr, 0, size);

    aiq_init_params* params = reinterpret_cast<aiq_init_params*>(ptr);
    params->aiqb_size = aiqbData ? aiqbData->size : 0;
    params->nvm_size = nvmData ? nvmData->size : 0;
    params->aiqd_size = aiqdData ? aiqdData->size : 0;
    params->stats_max_width = statsMaxWidth;
    params->stats_max_height = statsMaxHeight;
    params->max_num_stats_in = maxNumStatsIn;
    params->ia_mkn = mkn;
    params->cmcRemoteHandle = cmc;

    ptr += sizeof(aiq_init_params);
    if (aiqbData) {
        MEMCPY_S(ptr, aiqbData->size, aiqbData->data, aiqbData->size);
    }

    ptr += params->aiqb_size;
    if (nvmData) {
        MEMCPY_S(ptr, nvmData->size, nvmData->data, nvmData->size);
    }

    ptr += params->nvm_size;
    if (aiqdData) {
        MEMCPY_S(ptr, aiqdData->size, aiqdData->data, aiqdData->size);
    }

    return true;
}

bool IPCIntelAiq::serverUnflattenInit(const void* pData, int dataSize, ia_binary_data* aiqbData,
                                      ia_binary_data* nvmData, ia_binary_data* aiqdData) {
    LOGIPC("@%s, pData:%p, dataSize:%d, aiqbData:%p, nvmData:%p, aiqdData:%p", __func__, pData,
           dataSize, aiqbData, nvmData, aiqdData);
    CheckError(dataSize < sizeof(aiq_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!aiqbData, false, "@%s, aiqbData is nullptr", __func__);
    CheckError(!nvmData, false, "@%s, nvmData is nullptr", __func__);
    CheckError(!aiqdData, false, "@%s, aiqdData is nullptr", __func__);

    const aiq_init_params* params = static_cast<const aiq_init_params*>(pData);

    LOGIPC("@%s, aiqb_size:%d, nvm_size:%d, aiqd_size:%d", __func__, params->aiqb_size,
           params->nvm_size, params->aiqd_size);

    int totalMemSize =
        sizeof(aiq_init_params) + params->aiqb_size + params->nvm_size + params->aiqd_size;
    CheckError(dataSize < totalMemSize, false, "@%s, dataSize:%d is too small", __func__, dataSize);

    const aiq_init_params* p = static_cast<const aiq_init_params*>(pData) + 1;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(const_cast<aiq_init_params*>(p));
    aiqbData->size = params->aiqb_size;
    aiqbData->data = aiqbData->size > 0 ? ptr : nullptr;

    ptr += params->aiqb_size;
    nvmData->size = params->nvm_size;
    nvmData->data = nvmData->size > 0 ? ptr : nullptr;

    ptr += params->nvm_size;
    aiqdData->size = params->aiqd_size;
    aiqdData->data = aiqdData->size > 0 ? ptr : nullptr;

    return true;
}

// ae
bool IPCIntelAiq::clientFlattenAe(uintptr_t aiq, const ia_aiq_ae_input_params& inParams,
                                  ae_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;

    params->base = inParams;
    const ia_aiq_ae_input_params* base = &params->base;

    if (base->aec_features) {
        params->aec_features = *inParams.aec_features;
    }

    if (base->exposure_coordinate) {
        params->exposure_coordinate = *inParams.exposure_coordinate;
    }

    if (base->exposure_window) {
        params->exposure_window = *inParams.exposure_window;
    }

    if (inParams.num_exposures > 1) {
        LOGE("@%s, BUG: num_exposures:%d > 1. Copying only first.", __func__,
             inParams.num_exposures);
    }

    if (base->sensor_descriptor) {
        params->sensor_descriptor = *inParams.sensor_descriptor;
    }

    if (base->manual_exposure_time_us) {
        params->manual_exposure_time_us = *inParams.manual_exposure_time_us;
    }

    if (base->manual_analog_gain) {
        params->manual_analog_gain = *inParams.manual_analog_gain;
    }

    if (base->manual_iso) {
        params->manual_iso = *inParams.manual_iso;
    }

    if (base->manual_limits) {
        params->manual_limits = *inParams.manual_limits;
    }

    return true;
}

bool IPCIntelAiq::serverUnflattenAe(ae_run_params* inParams, ia_aiq_ae_input_params** params) {
    LOGIPC("@%s, inParams:%p, params:%p", __func__, inParams, params);
    CheckError(!inParams, false, "@%s, inParams is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_ae_input_params* base = &inParams->base;
    if (base->aec_features) {
        base->aec_features = &inParams->aec_features;
    }

    if (base->exposure_coordinate) {
        base->exposure_coordinate = &inParams->exposure_coordinate;
    }

    if (base->exposure_window) {
        base->exposure_window = &inParams->exposure_window;
    }

    if (base->sensor_descriptor) {
        base->sensor_descriptor = &inParams->sensor_descriptor;
    }

    if (base->manual_exposure_time_us) {
        base->manual_exposure_time_us = &inParams->manual_exposure_time_us;
    }

    if (base->manual_analog_gain) {
        base->manual_analog_gain = &inParams->manual_analog_gain;
    }

    if (base->manual_iso) {
        base->manual_iso = &inParams->manual_iso;
    }

    if (base->manual_limits) {
        base->manual_limits = &inParams->manual_limits;
    }

    *params = base;

    return true;
}

bool IPCIntelAiq::clientUnflattenAe(ae_run_params* params, ia_aiq_ae_results** results) {
    LOGIPC("@%s, params:%p, results:%p", __func__, params, results);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    ae_run_params_results* res = &params->res;
    bool ret = unflattenAeResults(res);
    CheckError((ret == false), false, "@%s, unflattenAeResults fails", __func__);

    *results = &res->base;

    return true;
}

bool IPCIntelAiq::serverFlattenAe(const ia_aiq_ae_results& aeResults, ae_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    bool ret = flattenAeResults(aeResults, &params->res);
    CheckError(ret == false, false, "@%s, flattenAeResults fails", __func__);

    return true;
}

bool IPCIntelAiq::flattenAeResults(const ia_aiq_ae_results& aeResults, ae_run_params_results* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    res->base = aeResults;
    const ia_aiq_ae_results* base = &res->base;

    if (base->exposures && base->num_exposures > 0) {
        CheckError(base->num_exposures > MAX_NUM_EXPOSURES, false,
                   "@%s, base->num_exposures:% is too big", __func__, base->num_exposures);

        for (unsigned int i = 0; i < base->num_exposures; i++) {
            res->exposures[i] = aeResults.exposures[i];

            if (res->exposures[i].exposure) {
                res->exposure[i] = *aeResults.exposures[i].exposure;
            }
            if (res->exposures[i].sensor_exposure) {
                res->sensor_exposure[i] = *aeResults.exposures[i].sensor_exposure;
            }
            if (res->exposures[i].exposure_plan_ids) {
                CheckError(base->exposures->num_exposure_plan > MAX_NUM_OF_EXPOSURE_PLANS, false,
                           "@%s, base->exposures->num_exposure_plan:% is too big", __func__,
                           base->exposures->num_exposure_plan);
                MEMCPY_S(res->exposure_plan_ids[i],
                         sizeof(unsigned int) * MAX_NUM_OF_EXPOSURE_PLANS,
                         aeResults.exposures[i].exposure_plan_ids,
                         sizeof(unsigned int) * aeResults.exposures[i].num_exposure_plan);
            }
        }
    }

    if (base->weight_grid) {
        res->weight_grid = *aeResults.weight_grid;

        if (res->weight_grid.weights) {
            unsigned int gridElements =
                aeResults.weight_grid->width * aeResults.weight_grid->height;
            gridElements = CLIP(gridElements, MAX_SIZE_WEIGHT_GRID, 1);
            MEMCPY_S(res->weights, sizeof(res->weights), aeResults.weight_grid->weights,
                     gridElements * sizeof(unsigned char));
        }
    }

    if (base->flashes) {
        // Valgrind will give warning from here in the first round. It should be fine.
        if (aeResults.num_flashes > 0) {
            MEMCPY_S(res->flashes, sizeof(res->flashes), aeResults.flashes,
                     MAX_NUM_FLASHES * sizeof(ia_aiq_flash_parameters));
        }
    }

    if (base->aperture_control) {
        res->aperture_control = *aeResults.aperture_control;
    }

    return true;
}

bool IPCIntelAiq::unflattenAeResults(ae_run_params_results* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    ia_aiq_ae_results* base = &res->base;

    if (base->exposures) {
        base->exposures = res->exposures;

        CheckError(base->num_exposures > MAX_NUM_EXPOSURES, false,
                   "@%s, base->num_exposures:% is too big", __func__, base->num_exposures);

        for (unsigned int i = 0; i < base->num_exposures; i++) {
            if (base->exposures[i].exposure) {
                base->exposures[i].exposure = &res->exposure[i];
            }
            if (base->exposures[i].sensor_exposure) {
                base->exposures[i].sensor_exposure = &res->sensor_exposure[i];
            }
            if (base->exposures[i].exposure_plan_ids) {
                base->exposures[i].exposure_plan_ids = res->exposure_plan_ids[i];
            }
        }
    }

    if (base->weight_grid) {
        base->weight_grid = &res->weight_grid;
        if (base->weight_grid->weights) {
            base->weight_grid->weights = res->weights;
        }
    }

    if (base->flashes) {
        base->flashes = res->flashes;
    }

    if (base->aperture_control) {
        base->aperture_control = &res->aperture_control;
    }

    return true;
}

// af
bool IPCIntelAiq::clientFlattenAf(uintptr_t aiq, const ia_aiq_af_input_params& inParams,
                                  af_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;

    params->base = inParams;
    ia_aiq_af_input_params* base = &params->base;
    if (base->focus_rect) {
        params->focus_rect = *inParams.focus_rect;
    }
    if (base->manual_focus_parameters) {
        params->manual_focus_parameters = *inParams.manual_focus_parameters;
    }

    return true;
}

bool IPCIntelAiq::clientUnflattenAf(const af_run_params& params, ia_aiq_af_results** results) {
    LOGIPC("@%s, results:%p", __func__, results);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    *results = const_cast<ia_aiq_af_results*>(&params.results);

    return true;
}

bool IPCIntelAiq::serverUnflattenAf(af_run_params* inParams, ia_aiq_af_input_params** params) {
    LOGIPC("@%s, inParams:%p, params:%p", __func__, inParams, params);
    CheckError(!inParams, false, "@%s, inParams is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_af_input_params* base = &inParams->base;
    if (base->focus_rect) {
        base->focus_rect = &inParams->focus_rect;
    }
    if (base->manual_focus_parameters) {
        base->manual_focus_parameters = &inParams->manual_focus_parameters;
    }

    *params = base;

    return true;
}

bool IPCIntelAiq::serverFlattenAf(const ia_aiq_af_results& afResults, af_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_af_results* results = &params->results;
    *results = afResults;

    LOGIPC("af results->status:%d", results->status);
    LOGIPC("af results->current_focus_distance:%d", results->current_focus_distance);
    LOGIPC("af results->next_lens_position:%d", results->next_lens_position);
    LOGIPC("af results->lens_driver_action:%d", results->lens_driver_action);
    LOGIPC("af results->use_af_assist:%d", results->use_af_assist);
    LOGIPC("af results->final_lens_position_reached:%d", results->final_lens_position_reached);

    return true;
}

// awb
bool IPCIntelAiq::clientFlattenAwb(uintptr_t aiq, const ia_aiq_awb_input_params& inParams,
                                   awb_run_params* params) {
    LOGIPC("@%s, aiq:0x%, params:%p", __func__, aiq, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;

    params->base = inParams;
    const ia_aiq_awb_input_params* base = &params->base;

    if (base->manual_cct_range) {
        params->manual_cct_range = *inParams.manual_cct_range;
    }

    if (base->manual_white_coordinate) {
        params->manual_white_coordinate = *inParams.manual_white_coordinate;
    }

    return true;
}

bool IPCIntelAiq::clientUnflattenAwb(const awb_run_params& inParams, ia_aiq_awb_results** results) {
    LOGIPC("@%s, results:%p", __func__, results);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    *results = const_cast<ia_aiq_awb_results*>(&inParams.results);

    return true;
}

bool IPCIntelAiq::serverUnflattenAwb(awb_run_params* inParams, ia_aiq_awb_input_params** params) {
    LOGIPC("@%s, inParams:%p, params:%p", __func__, inParams, params);
    CheckError(!inParams, false, "@%s, inParams is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_awb_input_params* base = &inParams->base;

    if (base->manual_cct_range) {
        base->manual_cct_range = &inParams->manual_cct_range;
    }

    if (base->manual_white_coordinate) {
        base->manual_white_coordinate = &inParams->manual_white_coordinate;
    }

    LOGIPC("@%s, manual_cct_range:%p, manual_white_coordinate:%p", __func__, base->manual_cct_range,
           base->manual_white_coordinate);

    *params = base;

    return true;
}

bool IPCIntelAiq::serverFlattenAwb(const ia_aiq_awb_results& awbResults, awb_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_awb_results* results = &params->results;
    *results = awbResults;

    LOGIPC("awb results->accurate_r_per_g:%f", results->accurate_r_per_g);
    LOGIPC("awb results->accurate_b_per_g:%f", results->accurate_b_per_g);
    LOGIPC("awb results->final_r_per_g:%f", results->final_r_per_g);
    LOGIPC("awb results->final_b_per_g:%f", results->final_b_per_g);
    LOGIPC("awb results->cct_estimate:%d", results->cct_estimate);
    LOGIPC("awb results->distance_from_convergence:%f", results->distance_from_convergence);

    return true;
}

// gbce
bool IPCIntelAiq::clientFlattenGbce(uintptr_t aiq, const ia_aiq_gbce_input_params& inParams,
                                    gbce_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;
    params->base = inParams;

    return true;
}

bool IPCIntelAiq::clientUnflattenGbce(gbce_run_params* params, ia_aiq_gbce_results** results) {
    LOGIPC("@%s, params:%p, results:%p", __func__, params, results);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    gbce_results_params* res = &params->res;
    bool ret = unflattenGbceResults(res);
    CheckError(!ret, false, "@%s, unflattenGbceResults fails", __func__);

    *results = &res->base;

    return true;
}

bool IPCIntelAiq::serverFlattenGbce(const ia_aiq_gbce_results& gbceResults,
                                    gbce_run_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    return flattenGbceResults(gbceResults, &params->res);
}

bool IPCIntelAiq::flattenGbceResults(const ia_aiq_gbce_results& gbceResults,
                                     gbce_results_params* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    size_t size = gbceResults.gamma_lut_size * sizeof(*gbceResults.g_gamma_lut);
    LOGIPC("@%s, gamma_lut_size:%d, size:%zu, tone_map_lut_size:%d", __func__,
           gbceResults.gamma_lut_size, size, gbceResults.tone_map_lut_size);

    res->base = gbceResults;

    const ia_aiq_gbce_results* base = &res->base;

    if (base->r_gamma_lut) {
        MEMCPY_S(res->r_gamma_lut, sizeof(res->r_gamma_lut), gbceResults.r_gamma_lut, size);
    }

    if (base->b_gamma_lut) {
        MEMCPY_S(res->b_gamma_lut, sizeof(res->b_gamma_lut), gbceResults.b_gamma_lut, size);
    }

    if (base->g_gamma_lut) {
        MEMCPY_S(res->g_gamma_lut, sizeof(res->g_gamma_lut), gbceResults.g_gamma_lut, size);
    }

    if (base->tone_map_lut) {
        MEMCPY_S(res->tone_map_lut, sizeof(res->tone_map_lut), gbceResults.tone_map_lut,
                 gbceResults.tone_map_lut_size * sizeof(*gbceResults.tone_map_lut));
    }

    return true;
}

bool IPCIntelAiq::unflattenGbceResults(gbce_results_params* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    ia_aiq_gbce_results* base = &res->base;

    LOGIPC("@%s, gamma_lut_size:%d", __func__, base->gamma_lut_size);
    LOGIPC("@%s, tone_map_lut_size:%d", __func__, base->tone_map_lut_size);
    CheckError(base->gamma_lut_size > MAX_NUM_GAMMA_LUTS, false,
               "@%s, gamma_lut_size:%d is too big", __func__, base->gamma_lut_size);
    CheckError((base->tone_map_lut_size > MAX_NUM_TOME_MAP_LUTS), false,
               "@%s, tone_map_lut_size:%d is too big", __func__, base->tone_map_lut_size);

    if (base->r_gamma_lut) {
        base->r_gamma_lut = res->r_gamma_lut;
    }

    if (base->b_gamma_lut) {
        base->b_gamma_lut = res->b_gamma_lut;
    }

    if (base->g_gamma_lut) {
        base->g_gamma_lut = res->g_gamma_lut;
    }

    if (base->tone_map_lut) {
        base->tone_map_lut = res->tone_map_lut;
    }

    return true;
}

// pa
bool IPCIntelAiq::clientFlattenPaV1(uintptr_t aiq, const ia_aiq_pa_input_params& inParams,
                                    pa_run_v1_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;

    params->base = inParams;
    ia_aiq_pa_input_params* base = &params->base;

    if (base->awb_results) {
        params->awb_results = *base->awb_results;
    }

    if (base->exposure_params) {
        params->exposure_params = *base->exposure_params;
    }

    if (base->color_gains) {
        params->color_gains = *base->color_gains;
    }

    return true;
}

bool IPCIntelAiq::clientUnflattenPaV1(pa_run_v1_params* params, ia_aiq_pa_results_v1** results) {
    LOGIPC("@%s, params:%p, results:%p", __func__, params, results);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    pa_run_params_results_v1* res = &params->res;
    bool ret = unflattenPaResultsV1(res);
    CheckError((ret == false), false, "@%s, unflattenPaResultsV1 fails", __func__);

    *results = &res->base;

    return true;
}

bool IPCIntelAiq::serverUnflattenPaV1(pa_run_v1_params* inParams, ia_aiq_pa_input_params** params) {
    LOGIPC("@%s, inParams:%p, params:%p", __func__, inParams, params);
    CheckError(!inParams, false, "@%s, inParams is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_pa_input_params* base = &inParams->base;

    if (base->awb_results) {
        base->awb_results = &inParams->awb_results;
    }

    if (base->exposure_params) {
        base->exposure_params = &inParams->exposure_params;
    }

    if (base->color_gains) {
        base->color_gains = &inParams->color_gains;
    }

    *params = base;

    return true;
}

bool IPCIntelAiq::serverFlattenPaV1(const ia_aiq_pa_results_v1& paResults,
                                    pa_run_v1_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    bool ret = flattenPaResultsV1(paResults, &params->res);
    CheckError(ret == false, false, "@%s, flattenPaResultsV1 fails", __func__);

    return true;
}

bool IPCIntelAiq::flattenPaResultsV1(const ia_aiq_pa_results_v1& paResults,
                                     pa_run_params_results_v1* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    res->base = paResults;
    ia_aiq_pa_results_v1* base = &res->base;

    ia_aiq_color_channels_lut* linearization = &base->linearization;
    CheckError((MAX_NUM_LUTS < linearization->size), false, "@%s, linearization:%d is too big",
               __func__, linearization->size);
    if (linearization->gr) {
        MEMCPY_S(res->gr, sizeof(res->gr), linearization->gr,
                 sizeof(*linearization->gr) * linearization->size);
    }
    if (linearization->r) {
        MEMCPY_S(res->r, sizeof(res->r), linearization->r,
                 sizeof(*linearization->r) * linearization->size);
    }
    if (linearization->b) {
        MEMCPY_S(res->b, sizeof(res->b), linearization->b,
                 sizeof(*linearization->b) * linearization->size);
    }
    if (linearization->gb) {
        MEMCPY_S(res->gb, sizeof(res->gb), linearization->gb,
                 sizeof(*linearization->gb) * linearization->size);
    }

    ia_aiq_advanced_ccm_t* preferred_acm = base->preferred_acm;
    if (preferred_acm) {
        CheckError((MAX_SECTOR_COUNT < preferred_acm->sector_count), false,
                   "@%s, sector_count:%d is too big", __func__, preferred_acm->sector_count);

        res->preferred_acm = *preferred_acm;

        if (preferred_acm->hue_of_sectors) {
            MEMCPY_S(res->hue_of_sectors, sizeof(res->hue_of_sectors),
                     preferred_acm->hue_of_sectors,
                     sizeof(*preferred_acm->hue_of_sectors) * preferred_acm->sector_count);
        }

        if (preferred_acm->advanced_color_conversion_matrices) {
            MEMCPY_S(res->advanced_color_conversion_matrices,
                     sizeof(res->advanced_color_conversion_matrices),
                     preferred_acm->advanced_color_conversion_matrices,
                     (sizeof(*preferred_acm->advanced_color_conversion_matrices) *
                      preferred_acm->sector_count));
        }
    }

    ia_aiq_ir_weight_t* ir_weight = base->ir_weight;
    if (ir_weight) {
        res->ir_weight = *ir_weight;

        if (ir_weight->ir_weight_grid_R) {
            MEMCPY_S(res->ir_weight_grid_R, sizeof(res->ir_weight_grid_R),
                     ir_weight->ir_weight_grid_R,
                     sizeof(*ir_weight->ir_weight_grid_R) * ir_weight->height * ir_weight->width);
        }

        if (ir_weight->ir_weight_grid_G) {
            MEMCPY_S(res->ir_weight_grid_G, sizeof(res->ir_weight_grid_G),
                     ir_weight->ir_weight_grid_G,
                     sizeof(*ir_weight->ir_weight_grid_G) * ir_weight->height * ir_weight->width);
        }

        if (ir_weight->ir_weight_grid_B) {
            MEMCPY_S(res->ir_weight_grid_B, sizeof(res->ir_weight_grid_B),
                     ir_weight->ir_weight_grid_B,
                     sizeof(*ir_weight->ir_weight_grid_B) * ir_weight->height * ir_weight->width);
        }
    }

    ia_aiq_rgbir_t* rgbir = base->rgbir;
    if (rgbir) {
        res->rgbir = *rgbir;

        CheckError(rgbir->n_models > MAX_NUM_IR_MODES, false, "@%s, rgbir->n_models:%d is too big",
                   __func__, rgbir->n_models);

        if (rgbir->models && rgbir->n_models > 0) {
            for (unsigned int i = 0; i < rgbir->n_models; i++) {
                res->models[i] = rgbir->models[i];
            }
        }
    }

    return true;
}

bool IPCIntelAiq::unflattenPaResultsV1(pa_run_params_results_v1* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    ia_aiq_pa_results_v1* base = &res->base;

    if (base->linearization.gr) {
        base->linearization.gr = res->gr;
    }
    if (base->linearization.r) {
        base->linearization.r = res->r;
    }
    if (base->linearization.b) {
        base->linearization.b = res->b;
    }
    if (base->linearization.gb) {
        base->linearization.gb = res->gb;
    }

    if (base->preferred_acm) {
        base->preferred_acm = &res->preferred_acm;

        if (base->preferred_acm->hue_of_sectors) {
            base->preferred_acm->hue_of_sectors = res->hue_of_sectors;
        }

        if (base->preferred_acm->advanced_color_conversion_matrices) {
            base->preferred_acm->advanced_color_conversion_matrices =
                static_cast<float(*)[3][3]>(res->advanced_color_conversion_matrices);
        }
    }

    if (base->ir_weight) {
        base->ir_weight = &res->ir_weight;

        if (base->ir_weight->ir_weight_grid_R) {
            base->ir_weight->ir_weight_grid_R = res->ir_weight_grid_R;
        }
        if (base->ir_weight->ir_weight_grid_G) {
            base->ir_weight->ir_weight_grid_G = res->ir_weight_grid_G;
        }
        if (base->ir_weight->ir_weight_grid_B) {
            base->ir_weight->ir_weight_grid_B = res->ir_weight_grid_B;
        }
    }

    if (base->rgbir) {
        base->rgbir = &res->rgbir;

        ia_aiq_rgbir_t* resRgbir = &res->rgbir;
        if (resRgbir->models && resRgbir->n_models > 0) {
            resRgbir->models = res->models;
        }
    }

    return true;
}

// sa
bool IPCIntelAiq::clientFlattenSaV2(uintptr_t aiq, const ia_aiq_sa_input_params_v1& inParams,
                                    sa_run_v2_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->aiq_handle = aiq;

    params->base = inParams;
    const ia_aiq_sa_input_params_v1* base = &params->base;

    if (base->sensor_frame_params) {
        params->sensor_frame_params = *inParams.sensor_frame_params;
    }

    if (base->awb_results) {
        params->awb_results = *inParams.awb_results;
    }

    return true;
}

bool IPCIntelAiq::clientUnflattenSaV2(sa_run_v2_params* params, ia_aiq_sa_results_v1** results) {
    LOGIPC("@%s, params:%p, results:%p", __func__, params, results);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!results, false, "@%s, results is nullptr", __func__);

    sa_run_v2_params_results* res = &params->res;
    bool ret = unflattenSaResultsV2(res);
    CheckError(ret == false, false, "@%s, unflattenSaResultsV2 fails", __func__);

    *results = &res->base;

    return true;
}

bool IPCIntelAiq::serverUnflattenSaV2(const sa_run_v2_params& inParams,
                                      ia_aiq_sa_input_params_v1** params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    ia_aiq_sa_input_params_v1* base = const_cast<ia_aiq_sa_input_params_v1*>(&inParams.base);

    if (base->sensor_frame_params) {
        base->sensor_frame_params = const_cast<ia_aiq_frame_params*>(&inParams.sensor_frame_params);
    }

    if (base->awb_results) {
        base->awb_results = const_cast<ia_aiq_awb_results*>(&inParams.awb_results);
    }

    *params = base;

    return true;
}

bool IPCIntelAiq::serverFlattenSaV2(const ia_aiq_sa_results_v1& saResults,
                                    sa_run_v2_params* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, results is nullptr", __func__);

    bool ret = flattenSaResultsV2(saResults, &params->res);
    CheckError(ret == false, false, "@%s, flattenSaResultsV2 fails", __func__);

    return true;
}

bool IPCIntelAiq::flattenSaResultsV2(const ia_aiq_sa_results_v1& saResults,
                                     sa_run_v2_params_results* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    res->base = saResults;
    ia_aiq_sa_results_v1* base = &res->base;

    LOGIPC("sa_results: width:%d, height:%d, lsc_update:%d", base->width, base->height,
           base->lsc_update);

    if (base->width && base->height) {
        size_t size = base->width * base->height * sizeof(unsigned short);
        for (int i = 0; i < LSC_MAX_BAYER_ORDER_NUM; i++) {
            for (int j = 0; j < LSC_MAX_BAYER_ORDER_NUM; j++) {
                lsc_grid_content* lgc = &res->lsc_grid[i][j];
                if (base->lsc_grid[i][j]) {
                    MEMCPY_S(lgc->content, sizeof(lgc->content), base->lsc_grid[i][j], size);
                }
            }
        }
    } else if (base->lsc_update) {
        LOGE("@%s, Error: LSC table size is 0", __func__);
    }

    return true;
}

bool IPCIntelAiq::unflattenSaResultsV2(sa_run_v2_params_results* res) {
    LOGIPC("@%s, res:%p", __func__, res);
    CheckError(!res, false, "@%s, res is nullptr", __func__);

    ia_aiq_sa_results_v1* base = &res->base;

    LOGIPC("sa_results_data:height:%d, width:%d", base->height, base->width);

    for (int i = 0; i < LSC_MAX_BAYER_ORDER_NUM; i++) {
        for (int j = 0; j < LSC_MAX_BAYER_ORDER_NUM; j++) {
            lsc_grid_content* lgc = &res->lsc_grid[i][j];
            base->lsc_grid[i][j] = lgc->content;
        }
    }

    return true;
}

// statistics
bool IPCIntelAiq::clientFlattenStatSetV4(uintptr_t aiq,
                                         const ia_aiq_statistics_input_params_v4& inParams,
                                         set_statistics_set_v4_params* params) {
    LOGIPC("@%s, aiq:0x%, params:%p", __func__, aiq, params);
    CheckError(reinterpret_cast<ia_aiq*>(aiq) == nullptr, false, "@%s, aiq is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    params->ia_aiq = aiq;

    set_statistics_params_data* input = &params->input;
    input->base = inParams;
    ia_aiq_statistics_input_params_v4* base = &input->base;

    if (base->frame_ae_parameters) {
        flattenAeResults(*base->frame_ae_parameters, &input->frame_ae_parameters);
    }

    if (base->frame_af_parameters) {
        input->frame_af_parameters = *base->frame_af_parameters;
    }

    if (base->rgbs_grids) {
        CheckError((base->num_rgbs_grids > MAX_NUMBER_OF_GRIDS), false,
                   "@%s, num_rgbs_grids:%d > MAX_NUMBER_OF_GRIDS:%d", __func__,
                   base->num_rgbs_grids, MAX_NUMBER_OF_GRIDS);

        for (int i = 0; i < MAX_NUMBER_OF_GRIDS; i++) {
            ia_aiq_rgbs_grid_data* rgbs_grids = &input->rgbs_grids[i];
            rgbs_grids->base = *base->rgbs_grids[i];

            CheckError(rgbs_grids->base.grid_width * rgbs_grids->base.grid_height > MAX_NUM_BLOCKS,
                       false, "@%s, grid_width:%d * grid_height:%d is too big", __func__,
                       rgbs_grids->base.grid_width, rgbs_grids->base.grid_height);

            MEMCPY_S(rgbs_grids->blocks_ptr, sizeof(rgbs_grids->blocks_ptr),
                     rgbs_grids->base.blocks_ptr,
                     rgbs_grids->base.grid_width * rgbs_grids->base.grid_height *
                         sizeof(*rgbs_grids->base.blocks_ptr));
        }
    }

    if (base->hdr_rgbs_grid) {
        ia_aiq_hdr_rgbs_grid_data* hdr_rgbs_grid = &input->hdr_rgbs_grid;
        hdr_rgbs_grid->base = *base->hdr_rgbs_grid;
        MEMCPY_S(hdr_rgbs_grid->blocks_ptr, sizeof(hdr_rgbs_grid->blocks_ptr),
                 hdr_rgbs_grid->base.blocks_ptr,
                 hdr_rgbs_grid->base.grid_width * hdr_rgbs_grid->base.grid_height *
                     sizeof(*hdr_rgbs_grid->base.blocks_ptr));
    }

    if (base->af_grids) {
        CheckError((base->num_af_grids > MAX_NUMBER_OF_AF_GRIDS), false,
                   "@%s, num_af_grids:%d > MAX_NUMBER_OF_AF_GRIDS:%d", __func__, base->num_af_grids,
                   MAX_NUMBER_OF_AF_GRIDS);

        for (int i = 0; i < MAX_NUMBER_OF_AF_GRIDS; i++) {
            ia_aiq_af_grid_data* af_grids = &input->af_grids[i];
            af_grids->base = *base->af_grids[i];
            MEMCPY_S(af_grids->filter_response_1, sizeof(af_grids->filter_response_1),
                     af_grids->base.filter_response_1,
                     af_grids->base.grid_width * af_grids->base.grid_height *
                         sizeof(*af_grids->base.filter_response_1));
            MEMCPY_S(af_grids->filter_response_2, sizeof(af_grids->filter_response_2),
                     af_grids->base.filter_response_2,
                     af_grids->base.grid_width * af_grids->base.grid_height *
                         sizeof(*af_grids->base.filter_response_2));
        }
    }

    if (base->frame_pa_parameters) {
        flattenPaResultsV1(*base->frame_pa_parameters, &input->frame_pa_parameters);
    }

    if (base->faces) {
        input->faces.base = *base->faces;
        for (int i = 0; i < base->faces->num_faces; i++) {
            input->faces.faces[i] = *(base->faces->faces + i);
        }
    }

    if (base->awb_results) {
        input->awb_results = *base->awb_results;
    }

    if (base->frame_sa_parameters) {
        flattenSaResultsV2(*base->frame_sa_parameters, &input->frame_sa_parameters);
    }

    if (base->depth_grids) {
        CheckError(base->num_depth_grids > MAX_NUMBER_OF_DEPTH_GRIDS, false,
                   "@%s, num_depth_grids:%d > MAX_NUMBER_OF_DEPTH_GRIDS:%d", __func__,
                   base->num_depth_grids, MAX_NUMBER_OF_DEPTH_GRIDS);

        for (int i = 0; i < MAX_NUMBER_OF_DEPTH_GRIDS; i++) {
            ia_aiq_depth_grid_data* depth_grids = &input->depth_grids[i];
            depth_grids->base = *base->depth_grids[i];
            MEMCPY_S(depth_grids->grid_rect, sizeof(depth_grids->grid_rect),
                     depth_grids->base.grid_rect,
                     depth_grids->base.grid_height * depth_grids->base.grid_width *
                         sizeof(*depth_grids->base.grid_rect));
            MEMCPY_S(depth_grids->depth_data, sizeof(depth_grids->depth_data),
                     depth_grids->base.depth_data,
                     depth_grids->base.grid_height * depth_grids->base.grid_width *
                         sizeof(*depth_grids->base.depth_data));
            MEMCPY_S(depth_grids->confidence, sizeof(depth_grids->confidence),
                     depth_grids->base.confidence,
                     depth_grids->base.grid_height * depth_grids->base.grid_width *
                         sizeof(*depth_grids->base.confidence));
        }
    }

    if (base->ir_grid) {
        input->ir_grid = *base->ir_grid;
        MEMCPY_S(input->ir_grid_data, MAX_IR_WEIGHT_GRID_DATA_SIZE * sizeof(unsigned short),
                 base->ir_grid->data,
                 base->ir_grid->width * base->ir_grid->height * sizeof(unsigned short));
    }

    return true;
}

bool IPCIntelAiq::serverUnflattenStatSetV4(set_statistics_set_v4_params* inParams,
                                           ia_aiq_statistics_input_params_v4** params) {
    LOGIPC("@%s, inParams:%p, params:%p", __func__, inParams, params);
    CheckError(!inParams, false, "@%s, inParams is nullptr", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    set_statistics_params_data* input = &inParams->input;
    ia_aiq_statistics_input_params_v4* base = &input->base;

    if (base->frame_ae_parameters) {
        unflattenAeResults(&input->frame_ae_parameters);
        base->frame_ae_parameters = &input->frame_ae_parameters.base;
    }

    if (base->frame_af_parameters) {
        base->frame_af_parameters = &input->frame_af_parameters;
    }

    if (base->rgbs_grids) {
        CheckError(base->num_rgbs_grids > MAX_NUMBER_OF_GRIDS, false,
                   "@%s, num_rgbs_grids:%d > MAX_NUMBER_OF_GRIDS:%d", __func__,
                   base->num_rgbs_grids, MAX_NUMBER_OF_GRIDS);

        for (int i = 0; i < base->num_rgbs_grids; i++) {
            ia_aiq_rgbs_grid_data* rgbs_grids = &input->rgbs_grids[i];
            rgbs_grids->base.blocks_ptr = rgbs_grids->blocks_ptr;

            input->rgbs_grids_array[i] = &rgbs_grids->base;
        }
        base->rgbs_grids = (input->rgbs_grids_array);
    }

    if (base->hdr_rgbs_grid) {
        input->hdr_rgbs_grid.base.blocks_ptr = input->hdr_rgbs_grid.blocks_ptr;
        base->hdr_rgbs_grid = &input->hdr_rgbs_grid.base;
    }

    if (base->af_grids) {
        CheckError((base->num_af_grids > MAX_NUMBER_OF_AF_GRIDS), false,
                   "@%s, num_af_grids:%d > MAX_NUMBER_OF_AF_GRIDS:%d", __func__, base->num_af_grids,
                   MAX_NUMBER_OF_AF_GRIDS);

        for (int i = 0; i < base->num_af_grids; i++) {
            ia_aiq_af_grid_data* af_grids = &input->af_grids[i];
            af_grids->base.filter_response_1 = af_grids->filter_response_1;
            af_grids->base.filter_response_2 = af_grids->filter_response_2;

            input->af_grids_array[i] = &af_grids->base;
        }
        base->af_grids = input->af_grids_array;
    }

    if (base->frame_pa_parameters) {
        unflattenPaResultsV1(&input->frame_pa_parameters);
        base->frame_pa_parameters = &input->frame_pa_parameters.base;
    }

    if (base->faces) {
        input->faces.base.faces = input->faces.faces;
        base->faces = &input->faces.base;
    }

    if (base->awb_results) {
        base->awb_results = &input->awb_results;
    }

    if (base->frame_sa_parameters) {
        unflattenSaResultsV2(&input->frame_sa_parameters);
        base->frame_sa_parameters = &input->frame_sa_parameters.base;
    }

    if (base->depth_grids) {
        CheckError((base->num_depth_grids > MAX_NUMBER_OF_DEPTH_GRIDS), false,
                   "@%s, num_depth_grids:%d > MAX_NUMBER_OF_DEPTH_GRIDS:%d", __func__,
                   base->num_depth_grids, MAX_NUMBER_OF_DEPTH_GRIDS);

        for (int i = 0; i < base->num_depth_grids; i++) {
            ia_aiq_depth_grid_data* depth_grids = &input->depth_grids[i];
            depth_grids->base.grid_rect = depth_grids->grid_rect;
            depth_grids->base.depth_data = depth_grids->depth_data;
            depth_grids->base.confidence = depth_grids->confidence;

            input->depth_grids_array[i] = &depth_grids->base;
        }
        base->depth_grids = input->depth_grids_array;
    }

    if (base->ir_grid) {
        input->ir_grid.data = input->ir_grid_data;
        base->ir_grid = &input->ir_grid;
    }

    *params = base;

    return true;
}

} /* namespace icamera */
