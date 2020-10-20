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

#define LOG_TAG "IPC_ISP_PARAM_ADAPTOR"

#include "modules/sandboxing/IPCIspParamAdaptor.h"

#include <memory>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIspParamAdaptor::IPCIspParamAdaptor() {
    LOGIPC("@%s", __func__);
}

IPCIspParamAdaptor::~IPCIspParamAdaptor() {
    LOGIPC("@%s", __func__);
}

bool IPCIspParamAdaptor::clientFlattenInit(void* pData, uint32_t size,
                                           const ia_binary_data* ispData, const ia_cmc_t* iaCmc,
                                           uint32_t maxStatsWidth, uint32_t maxStatsHeight,
                                           uint32_t maxNumStatsIn, ia_mkn* iaMkn) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData || !ispData || !iaCmc, false, "@%s, pData, ispData or iaCmc is nullptr",
               __func__);
    CheckError(size < sizeof(IspBxtInitParam), false, "@%s, buffer is small", __func__);
    CheckError(ispData->size > MAX_IA_BINARY_DATA_SIZE, false,
               "%s, the buffer of isp data is too small", __func__);

    IspBxtInitParam* params = static_cast<IspBxtInitParam*>(pData);
    params->iaIsp = *ispData;
    if (params->iaIsp.data) {
        MEMCPY_S(params->ispAiqbData, sizeof(params->ispAiqbData), ispData->data, ispData->size);
    }

    params->iaCmcHandle = reinterpret_cast<uintptr_t>(iaCmc);
    params->maxStatsWidth = maxStatsWidth;
    params->maxStatsHeight = maxStatsHeight;
    params->maxStatsIn = maxNumStatsIn;
    params->iaMkn = nullptr;  // Not used in current

    return true;
}

bool IPCIspParamAdaptor::serverUnflattenInit(IspBxtInitParam* pData, uint32_t size,
                                             ia_binary_data** Isp, ia_cmc_t** Cmc) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(IspBxtInitParam), false, "@%s, buffer is small", __func__);
    CheckError(!Isp || !Cmc, false, "@%s, Isp or Cmc is nullptr", __func__);

    if (pData->iaIsp.data) {
        CheckError(pData->iaIsp.size > MAX_IA_BINARY_DATA_SIZE, false,
                   "%s, the buffer of isp data is too small", __func__);
        pData->iaIsp.data = pData->ispAiqbData;
    }
    *Isp = &pData->iaIsp;
    *Cmc = reinterpret_cast<ia_cmc_t*>(pData->iaCmcHandle);

    return true;
}

bool IPCIspParamAdaptor::clientFlattenDeInit(void* pData, uint32_t size,
                                             const ia_isp_bxt* ispHandle) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle, false, "@%s, ispHandle is nullptr", __func__);
    CheckError(size < sizeof(IspBxtDeInitParam), false, "@%s, buffer is small", __func__);

    IspBxtDeInitParam* params = static_cast<IspBxtDeInitParam*>(pData);
    params->ispRemoteHandle = reinterpret_cast<uintptr_t>(ispHandle);

    return true;
}

bool IPCIspParamAdaptor::serverUnflattenDeInit(const void* pData, uint32_t size,
                                               ia_isp_bxt** ispHandle) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle, false, "@%s, ispHandle is nullptr", __func__);
    CheckError(size < sizeof(IspBxtDeInitParam), false, "@%s, buffer is small", __func__);

    const IspBxtDeInitParam* params = static_cast<const IspBxtDeInitParam*>(pData);
    *ispHandle = reinterpret_cast<ia_isp_bxt*>(params->ispRemoteHandle);

    return true;
}

bool IPCIspParamAdaptor::clientFlattenGetPalSize(void* pData, uint32_t size,
                                                 const ia_isp_bxt_program_group* programGroup) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!programGroup, false, "@%s, programGroup is nullptr", __func__);
    CheckError(size < sizeof(PalDataSizeParam), false, "@%s, buffer is small", __func__);

    PalDataSizeParam* params = static_cast<PalDataSizeParam*>(pData);

    bool ret = flattenProgramGroup(programGroup, &params->programGroup);
    CheckError(ret == false, false, "%s, flattenProgramGroup fails", __func__);

    return true;
}

bool IPCIspParamAdaptor::serverUnflattenGetPalSize(void* pData, uint32_t size,
                                                   ia_isp_bxt_program_group** programGroup) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!programGroup, false, "@%s, programGroup is nullptr", __func__);
    CheckError(size < sizeof(PalDataSizeParam), false, "@%s, buffer is small", __func__);

    PalDataSizeParam* params = static_cast<PalDataSizeParam*>(pData);

    bool ret = unflattenProgramGroup(&params->programGroup);
    CheckError(ret == false, false, "%s, unflattenProgramGroup fails", __func__);

    *programGroup = &params->programGroup.group;

    return true;
}

bool IPCIspParamAdaptor::clientFlattenConvertStats(void* pData, uint32_t size,
                                                   const ia_isp_bxt* ispHandle,
                                                   const ConvertInputParam* inputParams,
                                                   int32_t statsHandle) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle || !inputParams, false, "@%s, ispHandle or inputParams is nullptr",
               __func__);
    CheckError(size < sizeof(ConvertStatsParam), false, "@%s, buffer is small", __func__);

    ConvertStatsParam* params = static_cast<ConvertStatsParam*>(pData);
    params->ispRemoteHandle = reinterpret_cast<uintptr_t>(ispHandle);
    params->multiExpo = inputParams->multiExpo;

    params->statsBuffer = *inputParams->statsBuffer;
    params->statsHandle = statsHandle;

    bool ret = mIpcAiq.flattenAeResults(*inputParams->aeResults, &params->aeResults);
    CheckError(ret == false, false, "%s, flattenAeResults fails", __func__);

    params->dvsReso = *inputParams->dvsReso;

    return true;
}

bool IPCIspParamAdaptor::serverUnflattenConvertStats(void* pData, uint32_t size,
                                                     ia_isp_bxt** ispHandle,
                                                     ConvertInputParam* inputParams,
                                                     ConvertResult* result, void* statsAddr) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle || !inputParams || !result, false,
               "@%s, ispHandle, inputParams or result is nullptr", __func__);
    CheckError(size < sizeof(ConvertStatsParam), false, "@%s, buffer is small", __func__);
    CheckError(!statsAddr, false, "@%s, statsAddr is nullptr", __func__);

    ConvertStatsParam* params = static_cast<ConvertStatsParam*>(pData);
    *ispHandle = reinterpret_cast<ia_isp_bxt*>(params->ispRemoteHandle);
    inputParams->multiExpo = params->multiExpo;

    inputParams->statsBuffer = &params->statsBuffer;
    if (params->statsBuffer.size > 0) {
        params->statsBuffer.data = statsAddr;
    }

    inputParams->dvsReso = &params->dvsReso;
    ae_run_params_results* res = &params->aeResults;

    bool ret = mIpcAiq.unflattenAeResults(res);
    CheckError(ret == false, false, "%s, unflattenAeResults fails", __func__);
    inputParams->aeResults = &res->base;

    CLEAR(params->queryResults);
    result->queryResults = &params->queryResults;

    return true;
}

bool IPCIspParamAdaptor::serverFlattenConvertStats(void* pData, uint32_t size,
                                                   const ConvertResult& result) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(ConvertStatsParam), false, "@%s, buffer is small", __func__);

    ConvertStatsParam* params = static_cast<ConvertStatsParam*>(pData);
    // the queryResults uses SHM

    // flatten rgbs grid
    CLEAR(params->rgbsGridArray);
    for (int i = 0; i < MAX_NUM_EXPOSURES; ++i) {
        ia_aiq_rgbs_grid* rgbs = result.rgbsGrid[i];
        if (rgbs) {
            params->rgbsGrid[i].base = *rgbs;
            size_t rgbsSize = rgbs->grid_width * rgbs->grid_height * sizeof(rgbs_grid_block);
            size_t memSize = sizeof(params->rgbsGrid[i].blocks_ptr);
            CheckError(memSize < rgbsSize, false, "%s, memory for rgbs is too small", __func__);
            MEMCPY_S(params->rgbsGrid[i].blocks_ptr, memSize, rgbs->blocks_ptr, rgbsSize);
            params->rgbsGridArray[i] = &params->rgbsGrid[i].base;
        }
    }

    // flatten af grid
    params->afGridPtr = nullptr;
    if (result.afGrid) {
        params->afGrid.base = *result.afGrid;
        size_t afSize = result.afGrid->grid_width * result.afGrid->grid_height * sizeof(int);
        size_t memSize = sizeof(params->afGrid.filter_response_1);
        CheckError(memSize < afSize, false, "%s, memory for af grid is too small", __func__);
        MEMCPY_S(params->afGrid.filter_response_1, memSize, result.afGrid->filter_response_1,
                 afSize);
        MEMCPY_S(params->afGrid.filter_response_2, memSize, result.afGrid->filter_response_2,
                 afSize);
        params->afGridPtr = &params->afGrid.base;
    }

    // flatten dvs grid
    params->dvsStatsPtr = nullptr;
    if (result.dvsStats) {
        params->dvsStats.base = *result.dvsStats;
        CheckError(MV_ENTRIE_COUNT < result.dvsStats->vector_count, false,
                   "%s, memory for dvs statistics is too small", __func__);
        MEMCPY_S(params->dvsStats.motion_vectors, MV_ENTRIE_COUNT * sizeof(ia_dvs_motion_vector),
                 result.dvsStats->motion_vectors,
                 result.dvsStats->vector_count * sizeof(ia_dvs_motion_vector));
        params->dvsStatsPtr = &params->dvsStats.base;
    }

    return true;
}

bool IPCIspParamAdaptor::clientUnflattenConvertStats(void* pData, uint32_t size,
                                                     ConvertResult* result) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData || !result, false, "@%s, pData or result is nullptr", __func__);
    CheckError(size < sizeof(ConvertStatsParam), false, "@%s, buffer is small", __func__);

    ConvertStatsParam* params = static_cast<ConvertStatsParam*>(pData);
    *result->queryResults = params->queryResults;

    for (int i = 0; i < MAX_NUM_EXPOSURES; ++i) {
        if (params->rgbsGridArray[i]) {
            params->rgbsGrid[i].base.blocks_ptr = params->rgbsGrid[i].blocks_ptr;
            result->rgbsGrid[i] = &params->rgbsGrid[i].base;
        }
    }

    if (params->afGridPtr) {
        params->afGrid.base.filter_response_1 = params->afGrid.filter_response_1;
        params->afGrid.base.filter_response_2 = params->afGrid.filter_response_2;
        result->afGrid = &params->afGrid.base;
    }

    if (params->dvsStatsPtr) {
        params->dvsStats.base.motion_vectors = params->dvsStats.motion_vectors;
        result->dvsStats = &params->dvsStats.base;
    }

    return true;
}

bool IPCIspParamAdaptor::clientFlattenRunPal(void* pData, uint32_t size,
                                             const ia_isp_bxt* ispHandle,
                                             const ia_isp_bxt_input_params_v2* inputParams,
                                             const ia_binary_data* outputData,
                                             const int32_t palDataHandle) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle || !inputParams || !outputData, false,
               "@%s, ispHandle, inputParams or outputData is nullptr", __func__);
    CheckError(size < sizeof(RunPalParam), false, "@%s, buffer is small", __func__);

    RunPalParam* params = static_cast<RunPalParam*>(pData);
    params->ispRemoteHandle = reinterpret_cast<uintptr_t>(ispHandle);
    params->inputParamsBase = *inputParams;
    ia_isp_bxt_input_params_v2* base = &params->inputParamsBase;

    if (base->sensor_frame_params) {
        params->frameParam = *inputParams->sensor_frame_params;
    }

    if (base->awb_results) {
        params->awbResult = *inputParams->awb_results;
    }

    bool ret = true;
    if (base->gbce_results) {
        ret = mIpcAiq.flattenGbceResults(*inputParams->gbce_results, &params->gbceResult);
        CheckError(ret == false, false, "%s, flattenGbceResults fails", __func__);
    }

    if (base->ae_results) {
        ret = mIpcAiq.flattenAeResults(*inputParams->ae_results, &params->aeResults);
        CheckError(ret == false, false, "%s, flattenAeResults fails", __func__);
    }

    if (base->pa_results) {
        ret = mIpcAiq.flattenPaResultsV1(*inputParams->pa_results, &params->paResult);
        CheckError(ret == false, false, "%s, flattenPaResultsV1 fails", __func__);
    }

    if (base->sa_results) {
        mIpcAiq.flattenSaResultsV2(*inputParams->sa_results, &params->saResult);
        CheckError(ret == false, false, "%s, flattenSaResultsV2 fails", __func__);
    }

    if (base->weight_grid) {
        params->weightGrid = *inputParams->weight_grid;
        size_t weightSize = (inputParams->weight_grid->width * inputParams->weight_grid->height *
                             sizeof(unsigned char));
        CheckError(weightSize > MAX_SIZE_WEIGHT_GRID * sizeof(unsigned char), false,
                   "%s, the buffer for weight grid is too small", __func__);
        MEMCPY_S(params->weights, MAX_SIZE_WEIGHT_GRID * sizeof(unsigned char),
                 inputParams->weight_grid->weights, weightSize);
        params->weightGrid.weights = const_cast<unsigned char*>(params->weights);
    }

    if (base->program_group) {
        CLEAR(params->programGroup);
        ret = flattenProgramGroup(inputParams->program_group, &params->programGroup);
        CheckError(ret == false, false, "%s, flattenProgramGroup fails", __func__);
    }

    if (base->dvs_morph_table) {
        ret = mIpcDvs.flattenMorphTable(inputParams->dvs_morph_table, &params->dvsResult);
        CheckError(ret == false, false, "%s, flattenMorphTable fails", __func__);
    }

    if (base->custom_controls) {
        params->customControl = *inputParams->custom_controls;

        size_t customCtlSize = inputParams->custom_controls->count * sizeof(float);
        size_t maxCtlSize = MAX_CUSTOM_CONTROLS_SIZE * sizeof(float);
        CheckError(customCtlSize > maxCtlSize, false,
                   "%s, the buffer for custom controls is too small", __func__);
        if (inputParams->custom_controls->parameters) {
            MEMCPY_S(params->customCtlParams, maxCtlSize, inputParams->custom_controls->parameters,
                     customCtlSize);
            params->customControl.parameters = const_cast<float*>(params->customCtlParams);
        }
    }

    if (base->pal_override) {
        params->palOverride = *inputParams->pal_override;
        if (inputParams->pal_override->size > 0) {
            MEMCPY_S(params->overrideData, sizeof(params->overrideData),
                     inputParams->pal_override->data, inputParams->pal_override->size);
            params->palOverride.data = static_cast<void*>(params->overrideData);
        }
    }

    if (base->ltm_results) {
        params->ltmResult = *inputParams->ltm_results;
    }

    if (base->ltm_drc_params) {
        params->drcResult = *inputParams->ltm_drc_params;
    }

    if (base->gdc_transformation) {
        params->gdcTransform = *inputParams->gdc_transformation;
    }

    if (base->view_params) {
        MEMCPY_S(&params->viewConfig, sizeof(params->viewConfig), inputParams->view_params,
                 sizeof(ia_view_config_t));
    }

    if (base->bcomp_results) {
        params->bcompResult = *inputParams->bcomp_results;
    }

    if (base->gdc_mbr_limits) {
        params->gdcLimit = *inputParams->gdc_mbr_limits;
    }

    if (outputData) {
        params->palOutput = *outputData;
        params->palDataHandle = palDataHandle;
    }

    return true;
}

bool IPCIspParamAdaptor::serverUnflattenRunPal(void* pData, uint32_t size, ia_isp_bxt** ispHandle,
                                               ia_isp_bxt_input_params_v2** paramsRes,
                                               ia_binary_data** palOutput) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!ispHandle || !paramsRes || !palOutput, false,
               "@%s, ispHandle paramsRes or palOutput is nullptr", __func__);
    CheckError(size < sizeof(RunPalParam), false, "@%s, buffer is small", __func__);

    RunPalParam* params = static_cast<RunPalParam*>(pData);
    ia_isp_bxt_input_params_v2* base = &params->inputParamsBase;

    if (base->sensor_frame_params) {
        base->sensor_frame_params = &params->frameParam;
    }

    if (base->awb_results) {
        base->awb_results = &params->awbResult;
    }

    bool ret = true;
    if (base->gbce_results) {
        ret = mIpcAiq.unflattenGbceResults(&params->gbceResult);
        CheckError(ret == false, false, "%s, unflattenGbceResults fails", __func__);
        base->gbce_results = &params->gbceResult.base;
    }

    if (base->ae_results) {
        ret = mIpcAiq.unflattenAeResults(&params->aeResults);
        CheckError(ret == false, false, "%s, unflattenAeResults fails", __func__);
        base->ae_results = &params->aeResults.base;
    }

    if (base->pa_results) {
        ret = mIpcAiq.unflattenPaResultsV1(&params->paResult);
        CheckError(ret == false, false, "%s, unflattenPaResultsV1 fails", __func__);
        base->pa_results = &params->paResult.base;
    }

    if (base->sa_results) {
        ret = mIpcAiq.unflattenSaResultsV2(&params->saResult);
        CheckError(ret == false, false, "%s, unflattenSaResultsV2 fails", __func__);
        base->sa_results = &params->saResult.base;
    }

    if (base->weight_grid) {
        if (params->weightGrid.weights) {
            params->weightGrid.weights = const_cast<unsigned char*>(params->weights);
        }
        base->weight_grid = &params->weightGrid;
    }

    if (base->program_group) {
        ret = unflattenProgramGroup(&params->programGroup);
        CheckError(ret == false, false, "%s, unflattenProgramGroup fails", __func__);
        base->program_group = &params->programGroup.group;
    }

    if (base->dvs_morph_table) {
        ret = mIpcDvs.unflattenMorphTalbe(&params->dvsResult);
        CheckError(ret == false, false, "%s, unflattenMorphTalbe fails", __func__);
        base->dvs_morph_table = &params->dvsResult.morphTable;
    }

    if (base->custom_controls) {
        if (params->customControl.parameters) {
            params->customControl.parameters = const_cast<float*>(params->customCtlParams);
        }
        base->custom_controls = &params->customControl;
    }

    if (base->pal_override) {
        if (params->palOverride.data) {
            params->palOverride.data = static_cast<void*>(params->overrideData);
        }
        base->pal_override = &params->palOverride;
    }

    if (base->ltm_results) {
        base->ltm_results = &params->ltmResult;
    }

    if (base->ltm_drc_params) {
        base->ltm_drc_params = &params->drcResult;
    }

    if (base->gdc_transformation) {
        base->gdc_transformation = &params->gdcTransform;
    }

    if (base->view_params) {
        base->view_params = reinterpret_cast<ia_isp_bxt_view_params_t*>(&params->viewConfig);
    }

    if (base->bcomp_results) {
        base->bcomp_results = &params->bcompResult;
    }

    if (base->gdc_mbr_limits) {
        base->gdc_mbr_limits = &params->gdcLimit;
    }

    *ispHandle = reinterpret_cast<ia_isp_bxt*>(params->ispRemoteHandle);
    *paramsRes = &params->inputParamsBase;
    *palOutput = &params->palOutput;

    return true;
}

bool IPCIspParamAdaptor::flattenProgramGroup(const ia_isp_bxt_program_group* src,
                                             GraphKernelArray* res) {
    CheckError(!src || !res, false, "@%s, src or dst is nullptr", __func__);
    CheckError(src->kernel_count > MAX_STREAM_KERNEL_COUNT, false,
               "%s the buffer of kernel array is tool small", __func__);

    res->group = *src;
    for (unsigned int j = 0; j < res->group.kernel_count; ++j) {
        res->runKernels[j] = src->run_kernels[j];
        if (res->runKernels[j].resolution_info) {
            res->resoInfo[j] = *src->run_kernels[j].resolution_info;
        }
        if (res->runKernels[j].resolution_history) {
            res->resoHistory[j] = *src->run_kernels[j].resolution_history;
        }
    }
    if (res->group.pipe) {
        res->pipeInfo = *src->pipe;
    }

    return true;
}

bool IPCIspParamAdaptor::unflattenProgramGroup(GraphKernelArray* result) {
    CheckError(!result, false, "@%s, result is nullptr", __func__);
    CheckError(result->group.kernel_count > MAX_STREAM_KERNEL_COUNT, false,
               "%s, the buffer of kernel array is too small", __func__);

    result->group.run_kernels = result->runKernels;
    for (unsigned j = 0; j < result->group.kernel_count; ++j) {
        if (result->runKernels[j].resolution_info) {
            result->runKernels[j].resolution_info = &result->resoInfo[j];
        }

        if (result->runKernels[j].resolution_history) {
            result->runKernels[j].resolution_history = &result->resoHistory[j];
        }
    }
    if (result->group.pipe) {
        result->group.pipe = &result->pipeInfo;
    }

    return true;
}

}  // namespace icamera
