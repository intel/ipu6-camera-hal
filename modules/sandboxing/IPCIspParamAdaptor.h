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

#include "modules/algowrapper/StatsTypes.h"
#include "modules/sandboxing/IPCCommon.h"
#include "modules/sandboxing/IPCGraphConfig.h"
#include "modules/sandboxing/IPCIntelAiq.h"
#include "modules/sandboxing/IPCIntelDvs.h"

namespace icamera {

struct IspBxtInitParam {
    ia_binary_data iaIsp;
    int8_t ispAiqbData[MAX_IA_BINARY_DATA_SIZE];
    uintptr_t iaCmcHandle;
    uint32_t maxStatsWidth;
    uint32_t maxStatsHeight;
    uint32_t maxStatsIn;
    ia_mkn* iaMkn;  // hal always passes nullptr to ia_bxt_isp

    uintptr_t ispRemoteHandle;
};

struct IspBxtDeInitParam {
    uintptr_t ispRemoteHandle;
};

struct PalDataSizeParam {
    GraphKernelArray programGroup;
    int palDataSize;
};

struct DvsStatsParam {
    ia_dvs_statistics base;
    ia_dvs_motion_vector motion_vectors[MV_ENTRIE_COUNT];
};

struct ConvertStatsParam {
    uintptr_t ispRemoteHandle;

    // Input params
    bool multiExpo;
    ia_binary_data statsBuffer;
    int32_t statsHandle;
    camera_resolution_t dvsReso;
    ae_run_params_results aeResults;

    // Output result
    ia_isp_bxt_statistics_query_results_t queryResults;

    ia_aiq_rgbs_grid* rgbsGridArray[MAX_NUM_EXPOSURES];
    ia_aiq_rgbs_grid_data rgbsGrid[MAX_NUM_EXPOSURES];

    ia_aiq_af_grid* afGridPtr;
    ia_aiq_af_grid_data afGrid;

    ia_dvs_statistics* dvsStatsPtr;
    DvsStatsParam dvsStats;
};

#define MAX_CUSTOM_CONTROLS_SIZE 1024
struct RunPalParam {
    ia_isp_bxt_input_params_v2 inputParamsBase;

    // sensor frame parameters
    ia_aiq_frame_params frameParam;

    // for 3a result
    ia_aiq_awb_results awbResult;
    gbce_results_params gbceResult;

    ae_run_params_results aeResults;
    pa_run_params_results_v1 paResult;
    sa_run_v2_params_results saResult;

    ia_aiq_hist_weight_grid weightGrid;
    unsigned char weights[MAX_SIZE_WEIGHT_GRID];

    GraphKernelArray programGroup;
    DvsMorphParams dvsResult;

    // for custom control
    ia_isp_custom_controls customControl;
    float customCtlParams[MAX_CUSTOM_CONTROLS_SIZE];

    // for pal override
    ia_binary_data palOverride;
    int8_t overrideData[MAX_IA_BINARY_DATA_SIZE];

    // for ltm result
    ia_ltm_results ltmResult;
    ia_ltm_drc_params drcResult;

    ia_dvs_image_transformation gdcTransform;

    ia_view_config_t viewConfig;
    ia_bcomp_results bcompResult;
    ia_isp_bxt_gdc_limits gdcLimit;

    ia_binary_data palOutput;
    int32_t palDataHandle;

    uintptr_t ispRemoteHandle;
};

class IPCIspParamAdaptor {
 public:
    IPCIspParamAdaptor();
    virtual ~IPCIspParamAdaptor();

    // init
    bool clientFlattenInit(void* pData, uint32_t size, const ia_binary_data* ispData,
                           const ia_cmc_t* iaCmc, uint32_t maxStatsWidth, uint32_t maxStatsHeight,
                           uint32_t maxNumStatsIn, ia_mkn* iaMkn);
    bool serverUnflattenInit(IspBxtInitParam* pData, uint32_t size, ia_binary_data** Isp,
                             ia_cmc_t** Cmc);

    // deinit
    bool clientFlattenDeInit(void* pData, uint32_t size, const ia_isp_bxt* ispHandle);
    bool serverUnflattenDeInit(const void* pData, uint32_t size, ia_isp_bxt** ispHandle);

    // get pal data size
    bool clientFlattenGetPalSize(void* pData, uint32_t size,
                                 const ia_isp_bxt_program_group* programGroup);
    bool serverUnflattenGetPalSize(void* pData, uint32_t size,
                                   ia_isp_bxt_program_group** programGroup);

    // convert stats
    bool clientFlattenConvertStats(void* pData, uint32_t size, const ia_isp_bxt* ispHandle,
                                   const ConvertInputParam* inputParams, int32_t statsHandle);
    bool serverUnflattenConvertStats(void* pData, uint32_t size, ia_isp_bxt** ispHandle,
                                     ConvertInputParam* inputParams, ConvertResult* result,
                                     void* statsAddr);
    bool serverFlattenConvertStats(void* pData, uint32_t size, const ConvertResult& result);
    bool clientUnflattenConvertStats(void* pData, uint32_t size, ConvertResult* result);

    // run pal
    bool clientFlattenRunPal(void* pData, uint32_t size, const ia_isp_bxt* ispHandle,
                             const ia_isp_bxt_input_params_v2* inputParams,
                             const ia_binary_data* outputData, const int32_t palDataHandle);
    bool serverUnflattenRunPal(void* pData, uint32_t size, ia_isp_bxt** ispHandle,
                               ia_isp_bxt_input_params_v2** paramsRes, ia_binary_data** palOutput);

 private:
    bool flattenProgramGroup(const ia_isp_bxt_program_group* src, GraphKernelArray* res);
    bool unflattenProgramGroup(GraphKernelArray* result);

 private:
    IPCIntelAiq mIpcAiq;
    IPCIntelDvs mIpcDvs;

    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(IPCIspParamAdaptor);
};
}  // namespace icamera
