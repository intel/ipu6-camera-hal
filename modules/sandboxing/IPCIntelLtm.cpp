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

#define LOG_TAG "IPC_INTEL_LTM"

#include "modules/sandboxing/IPCIntelLtm.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIntelLtm::IPCIntelLtm() {
    LOGIPC("@%s", __func__);
}

IPCIntelLtm::~IPCIntelLtm() {
    LOGIPC("@%s", __func__);
}

bool IPCIntelLtm::clientFlattenInit(const ia_binary_data& inData, uintptr_t mkn_hanlde,
                                    LtmInitParams* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    CheckError(sizeof(params->data) < inData.size, false, "@%s, buffer is small", __func__);
    params->binary_data.data = params->data;
    MEMCPY_S(params->data, sizeof(params->data), inData.data, inData.size);
    params->binary_data.size = inData.size;
    params->mkn_handle = mkn_hanlde;

    return true;
}

bool IPCIntelLtm::serverUnflattenInit(LtmInitParams* params, ia_binary_data* inData,
                                      uintptr_t* mkn_hanlde) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!inData, false, "@%s, inData is nullptr", __func__);
    CheckError(!mkn_hanlde, false, "@%s, mkn_hanlde is nullptr", __func__);

    params->binary_data.data = params->data;
    *inData = params->binary_data;
    *mkn_hanlde = params->mkn_handle;

    return true;
}

bool IPCIntelLtm::serverFlattenInit(LtmInitParams* params, ia_ltm* ltm_handle) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!ltm_handle, false, "@%s, ltm_handle is nullptr", __func__);

    params->results = reinterpret_cast<uintptr_t>(ltm_handle);

    return true;
}

bool IPCIntelLtm::clientUnflattenInit(LtmInitParams* params, ia_ltm** ltm_handle) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);
    CheckError(!ltm_handle, false, "@%s, ltm_handle is nullptr", __func__);

    *ltm_handle = reinterpret_cast<ia_ltm*>(params->results);
    return true;
}

int IPCIntelLtm::deepCopyAeResults(const ia_aiq_ae_results& src, LtmInputAeResult* params) {
    LOGIPC("@%s", __func__);

    params->aeResultsBase = src;

    if (src.exposures) {
        CheckError(src.num_exposures > MAX_NUM_EXPOSURES_LTM, BAD_VALUE,
                   "exposure buffer is small");
        for (unsigned int i = 0; i < src.num_exposures; i++) {
            params->exposures[i].exposure_index = src.exposures[i].exposure_index;
            params->exposures[i].distance_from_convergence =
                src.exposures[i].distance_from_convergence;
            params->exposures[i].converged = src.exposures[i].converged;
            if (src.exposures[i].exposure) {
                params->exposure[i] = *src.exposures[i].exposure;
            }
            if (src.exposures[i].sensor_exposure) {
                params->sensor_exposure[i] = *src.exposures[i].sensor_exposure;
            }
            params->exposures[i].num_exposure_plan = src.exposures[i].num_exposure_plan;
            if (src.exposures[i].exposure_plan_ids) {
                CheckError(src.exposures[i].num_exposure_plan > MAX_NUM_OF_EXPOSURE_PLANS_LTM,
                           BAD_VALUE, "exposures[%d].exposure_plan_ids buffer is small", i);
                MEMCPY_S(params->exposure_plan_ids[i],
                         MAX_NUM_OF_EXPOSURE_PLANS_LTM * sizeof(unsigned int),
                         src.exposures[i].exposure_plan_ids,
                         src.exposures[i].num_exposure_plan * sizeof(unsigned int));
            }
        }
    }

    if (src.weight_grid) {
        params->weightGrid = *src.weight_grid;
        unsigned int gridElements = src.weight_grid->width * src.weight_grid->height;
        CheckError(MAX_SIZE_WEIGHT_GRID_LTM < gridElements, false, "weight_grid buffer is small");
        MEMCPY_S(params->weights, MAX_SIZE_WEIGHT_GRID_LTM * sizeof(char), src.weight_grid->weights,
                 gridElements * sizeof(char));
    }

    if (src.flashes) {
        CheckError(src.num_flashes > MAX_NUM_FLASHES_LTM, BAD_VALUE, "flash buffer is small");
        MEMCPY_S(params->flashes, MAX_NUM_FLASHES_LTM * sizeof(ia_aiq_flash_parameters),
                 src.flashes, src.num_flashes * sizeof(ia_aiq_flash_parameters));
    }

    if (src.aperture_control) {
        params->apertureControl = *src.aperture_control;
    }
    return OK;
}

int IPCIntelLtm::deepCopyRgbsGridData(const ia_aiq_rgbs_grid& src, ia_aiq_rgbs_grid* dst) {
    LOGIPC("%s", __func__);

    CheckError(!src.blocks_ptr || src.grid_width == 0 || src.grid_height == 0, BAD_VALUE,
               "Failed to deep copy Rgbs grid data - invalid source");
    CheckError(!dst || !dst->blocks_ptr, BAD_VALUE,
               "Failed to deep copy Rgbs grid data - invalid destination");

    size_t gridSize = src.grid_width * src.grid_height;
    size_t gridSizeLocal = MAX_WIDTH_STATISTICS * MAX_HEIGHT_STATISTICS;
    CheckError(gridSizeLocal < gridSize, false, "rgbs_grid buffer is small");
    dst->grid_width = src.grid_width;
    dst->grid_height = src.grid_height;
    dst->shading_correction = src.shading_correction;
    MEMCPY_S(dst->blocks_ptr, gridSizeLocal * sizeof(rgbs_grid_block), src.blocks_ptr,
             gridSize * sizeof(rgbs_grid_block));

    LOGIPC("%s, grid size=[%dx%d]", __func__, dst->grid_width, dst->grid_height);
    return OK;
}

int IPCIntelLtm::deepCopyHdrRgbsGridData(const ia_aiq_hdr_rgbs_grid& src,
                                         ia_aiq_hdr_rgbs_grid* dst) {
    LOGIPC("%s", __func__);

    CheckError(!src.blocks_ptr || src.grid_width == 0 || src.grid_height == 0, BAD_VALUE,
               "Failed to deep copy HDR Rgbs grid data - invalid source");
    CheckError(!dst || !dst->blocks_ptr, BAD_VALUE,
               "Failed to deep copy HDR Rgbs grid data - invalid destination");

    size_t gridSize = src.grid_width * src.grid_height;
    size_t gridSizeLocal = BXT_RGBS_GRID_MAX_WIDTH * BXT_RGBS_GRID_MAX_HEIGHT;
    CheckError(gridSizeLocal < gridSize, false, "hdr_rgbs_grid buffer is small");

    dst->grid_width = src.grid_width;
    dst->grid_height = src.grid_height;
    dst->grid_data_bit_depth = src.grid_data_bit_depth;
    dst->shading_correction = src.shading_correction;
    MEMCPY_S(dst->blocks_ptr, gridSizeLocal * sizeof(hdr_rgbs_grid_block), src.blocks_ptr,
             gridSize * sizeof(hdr_rgbs_grid_block));

    LOGIPC("%s, grid size=[%dx%d]", __func__, dst->grid_width, dst->grid_height);
    return OK;
}

bool IPCIntelLtm::clientFlattenRun(uintptr_t ltm, const ia_ltm_input_params& ltmParams,
                                   int imageDataSize, void* paramsAddr) {
    LOGIPC("@%s, paramsAddr:%p", __func__, paramsAddr);
    CheckError(!paramsAddr, false, "@%s, paramsAddr is nullptr", __func__);

    LtmRunParams* params = static_cast<LtmRunParams*>(paramsAddr);
    params->ltm_handle = ltm;
    params->inputParamsBase = ltmParams;
    LtmRunInputParams* inputParams = &params->inputParams;

    int ret = UNKNOWN_ERROR;
    if (ltmParams.ae_results) {
        ret = deepCopyAeResults(*ltmParams.ae_results, &inputParams->aeResult);
        CheckError(ret != OK, false, "@%s, Failed to deepCopyAeResult", __func__);
    }

    if (ltmParams.yv_grid) {
        MEMCPY_S(&inputParams->yvGrid, sizeof(ia_isp_bxt_hdr_yv_grid_t), ltmParams.yv_grid,
                 sizeof(ia_isp_bxt_hdr_yv_grid_t));
    }

    if (ltmParams.rgbs_grid_ptr) {
        inputParams->rgbsGrid.rgbsGridbase.blocks_ptr = inputParams->rgbsGrid.blocks;
        ret = deepCopyRgbsGridData(*ltmParams.rgbs_grid_ptr, &inputParams->rgbsGrid.rgbsGridbase);
        CheckError(ret != OK, false, "@%s, Failed to deepCopyRgbsGridData", __func__);
    }

    if (ltmParams.hdr_rgbs_grid_ptr) {
        inputParams->hdrRgbsGrid.hdrRgbsGridBase.blocks_ptr = inputParams->hdrRgbsGrid.blocks;
        ret = deepCopyHdrRgbsGridData(*ltmParams.hdr_rgbs_grid_ptr,
                                      &inputParams->hdrRgbsGrid.hdrRgbsGridBase);
        CheckError(ret != OK, false, "@%s, Failed to deepCopyHdrRgbsGridData", __func__);
    }

    if (ltmParams.input_image_ptr) {
        inputParams->inputImage = *ltmParams.input_image_ptr;
        if (ltmParams.input_image_ptr->image_data) {
            CheckError(imageDataSize < ltmParams.input_image_ptr->image_data->size, false,
                       "image_data buffer is small");
            CheckError(ltmParams.input_image_ptr->image_data->size != 0 &&
                           !ltmParams.input_image_ptr->image_data->data,
                       false, "image_data error");
            inputParams->imageData = *ltmParams.input_image_ptr->image_data;
            void* imageData =
                static_cast<void*>(static_cast<char*>(paramsAddr) + sizeof(LtmRunParams));
            MEMCPY_S(imageData, imageDataSize, ltmParams.input_image_ptr->image_data->data,
                     ltmParams.input_image_ptr->image_data->size);
        }
    }

    if (ltmParams.gtm_input_params_ptr) {
        MEMCPY_S(&inputParams->gtmInputParams, sizeof(ia_ltm_gtm_input_params),
                 ltmParams.gtm_input_params_ptr, sizeof(ia_ltm_gtm_input_params));
    }

    return true;
}

bool IPCIntelLtm::serverUnflattenRun(void* pData, ia_ltm** ltm, ia_ltm_input_params** inputParams) {
    LOGIPC("@%s, pData:%p", __func__, pData);
    CheckError(!pData, false, "@%s, params is nullptr", __func__);
    CheckError(!ltm, false, "@%s, ltm is nullptr", __func__);
    CheckError(!inputParams, false, "@%s, inputParams is nullptr", __func__);

    LtmRunParams* params = static_cast<LtmRunParams*>(pData);
    LtmRunInputParams* paramsTmp = &params->inputParams;
    if (params->inputParamsBase.ae_results) {
        LtmInputAeResult* aeResult = &paramsTmp->aeResult;
        params->inputParamsBase.ae_results = &aeResult->aeResultsBase;
        if (aeResult->aeResultsBase.exposures) {
            aeResult->aeResultsBase.exposures = aeResult->exposures;
            for (unsigned int i = 0; i < aeResult->aeResultsBase.num_exposures; i++) {
                if (aeResult->exposures[i].exposure) {
                    aeResult->exposures[i].exposure = &aeResult->exposure[i];
                }
                if (aeResult->exposures[i].sensor_exposure) {
                    aeResult->exposures[i].sensor_exposure = &aeResult->sensor_exposure[i];
                }
                if (aeResult->exposures[i].exposure_plan_ids) {
                    aeResult->exposures[i].exposure_plan_ids = aeResult->exposure_plan_ids[i];
                }
            }
        }

        if (aeResult->aeResultsBase.weight_grid) {
            aeResult->aeResultsBase.weight_grid = &aeResult->weightGrid;
            aeResult->weightGrid.weights = aeResult->weights;
        }

        if (aeResult->aeResultsBase.flashes) {
            aeResult->aeResultsBase.flashes = aeResult->flashes;
        }
        if (aeResult->aeResultsBase.aperture_control) {
            aeResult->aeResultsBase.aperture_control = &aeResult->apertureControl;
        }
    }

    if (params->inputParamsBase.yv_grid) {
        params->inputParamsBase.yv_grid = &paramsTmp->yvGrid;
    }

    if (params->inputParamsBase.rgbs_grid_ptr) {
        paramsTmp->rgbsGrid.rgbsGridbase.blocks_ptr = paramsTmp->rgbsGrid.blocks;
        params->inputParamsBase.rgbs_grid_ptr = &paramsTmp->rgbsGrid.rgbsGridbase;
    }

    if (params->inputParamsBase.hdr_rgbs_grid_ptr) {
        paramsTmp->hdrRgbsGrid.hdrRgbsGridBase.blocks_ptr = paramsTmp->hdrRgbsGrid.blocks;
        params->inputParamsBase.hdr_rgbs_grid_ptr = &paramsTmp->hdrRgbsGrid.hdrRgbsGridBase;
    }

    if (params->inputParamsBase.input_image_ptr) {
        params->inputParamsBase.input_image_ptr = &paramsTmp->inputImage;
        if (paramsTmp->inputImage.image_data) {
            paramsTmp->inputImage.image_data = &paramsTmp->imageData;
            if (paramsTmp->imageData.size != 0) {
                const_cast<ia_binary_data*>(paramsTmp->inputImage.image_data)->data =
                    static_cast<void*>(static_cast<char*>(pData) + sizeof(LtmRunParams));
            }
        }
    }

    if (params->inputParamsBase.gtm_input_params_ptr) {
        params->inputParamsBase.gtm_input_params_ptr = &paramsTmp->gtmInputParams;
    }

    *ltm = reinterpret_cast<ia_ltm*>(params->ltm_handle);
    *inputParams = &params->inputParamsBase;

    return true;
}

bool IPCIntelLtm::serverFlattenRun(const ia_ltm_results& ltmResults,
                                   const ia_ltm_drc_params& drcResults, ia_ltm* ltm_handle,
                                   LtmRunParams* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    MEMCPY_S(&params->ltmResult, sizeof(ia_ltm_results), &ltmResults, sizeof(ia_ltm_results));
    MEMCPY_S(&params->drcResult, sizeof(ia_ltm_drc_params), &drcResults, sizeof(ia_ltm_results));

    return true;
}

bool IPCIntelLtm::clientUnflattenRun(LtmRunParams* params, ia_ltm_results** ltmResults,
                                     ia_ltm_drc_params** drcResults) {
    LOGIPC("@%s", __func__);
    CheckError(!params, false, "@%s, params is nullptr", __func__);

    *ltmResults = &params->ltmResult;
    *drcResults = &params->drcResult;
    return true;
}

} /* namespace icamera */
