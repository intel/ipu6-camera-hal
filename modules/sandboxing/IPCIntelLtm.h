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

#include <ia_ltm.h>

#include "IPCCommon.h"
#include "iutils/Utils.h"

namespace icamera {
struct LtmInitParams {
    ia_binary_data binary_data;
    unsigned char data[MAX_IA_BINARY_DATA_SIZE];
    uintptr_t mkn_handle;
    uintptr_t results;
};

struct LtmDeinitParams {
    uintptr_t ltm_handle;
};

#define MAX_NUM_EXPOSURES_LTM 10
#define MAX_NUM_FLASHES_LTM 2
#define MAX_NUM_OF_EXPOSURE_PLANS_LTM 4
#define MAX_SIZE_WEIGHT_GRID_LTM 2048

struct LtmInputAeResult {
    ia_aiq_ae_results aeResultsBase;

    ia_aiq_ae_exposure_result exposures[MAX_NUM_EXPOSURES_LTM];
    ia_aiq_hist_weight_grid weightGrid;
    ia_aiq_flash_parameters flashes[MAX_NUM_FLASHES_LTM];
    ia_aiq_aperture_control apertureControl;

    // the below is in ia_aiq_ae_exposure_result exposures[MAX_NUM_EXPOSURES];
    ia_aiq_exposure_parameters exposure[MAX_NUM_EXPOSURES_LTM];
    ia_aiq_exposure_sensor_parameters sensor_exposure[MAX_NUM_EXPOSURES_LTM];
    unsigned int exposure_plan_ids[MAX_NUM_EXPOSURES_LTM][MAX_NUM_OF_EXPOSURE_PLANS_LTM];

    // the below is in ia_aiq_hist_weight_grid weight_grid;
    unsigned char weights[MAX_SIZE_WEIGHT_GRID_LTM];
};

#define MAX_WIDTH_STATISTICS 24
#define MAX_HEIGHT_STATISTICS 16
struct LtmInputRgbSGrid {
    ia_aiq_rgbs_grid rgbsGridbase;
    rgbs_grid_block blocks[MAX_WIDTH_STATISTICS * MAX_HEIGHT_STATISTICS];
};

#define BXT_RGBS_GRID_MAX_WIDTH 96
#define BXT_RGBS_GRID_MAX_HEIGHT 72
struct LtmInputHdrRgbsGrid {
    ia_aiq_hdr_rgbs_grid hdrRgbsGridBase;
    hdr_rgbs_grid_block blocks[BXT_RGBS_GRID_MAX_WIDTH * BXT_RGBS_GRID_MAX_HEIGHT];
};

struct LtmRunInputParams {
    LtmInputAeResult aeResult;
    ia_isp_bxt_hdr_yv_grid_t yvGrid;
    LtmInputRgbSGrid rgbsGrid;
    LtmInputHdrRgbsGrid hdrRgbsGrid;
    ia_ltm_input_image inputImage;
    ia_binary_data imageData;
    ia_ltm_gtm_input_params gtmInputParams;
};

struct LtmRunParams {
    uintptr_t ltm_handle;
    ia_ltm_input_params inputParamsBase;
    LtmRunInputParams inputParams;

    ia_ltm_results ltmResult;
    ia_ltm_drc_params drcResult;
};

class IPCIntelLtm {
 public:
    IPCIntelLtm();
    virtual ~IPCIntelLtm();

    // for init
    bool clientFlattenInit(const ia_binary_data& inData, uintptr_t mkn_hanlde,
                           LtmInitParams* params);
    bool serverUnflattenInit(LtmInitParams* params, ia_binary_data* inData, uintptr_t* mkn_hanlde);
    bool serverFlattenInit(LtmInitParams* params, ia_ltm* ltm_handle);
    bool clientUnflattenInit(LtmInitParams* params, ia_ltm** ltm_handle);

    // for run
    bool clientFlattenRun(uintptr_t ltm, const ia_ltm_input_params& ltmParams, int imageDataSize,
                          void* paramsAddr);
    bool serverUnflattenRun(void* pData, ia_ltm** ltm, ia_ltm_input_params** inputParams);
    bool serverFlattenRun(const ia_ltm_results& ltmResults, const ia_ltm_drc_params& drcResults,
                          ia_ltm* ltm_handle, LtmRunParams* params);
    bool clientUnflattenRun(LtmRunParams* params, ia_ltm_results** ltmResults,
                            ia_ltm_drc_params** drcResults);

 private:
    int deepCopyAeResults(const ia_aiq_ae_results& src, LtmInputAeResult* params);
    int deepCopyRgbsGridData(const ia_aiq_rgbs_grid& src, ia_aiq_rgbs_grid* dst);
    int deepCopyHdrRgbsGridData(const ia_aiq_hdr_rgbs_grid& src, ia_aiq_hdr_rgbs_grid* dst);
};
} /* namespace icamera */
