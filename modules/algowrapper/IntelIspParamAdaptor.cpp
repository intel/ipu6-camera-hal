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

#define LOG_TAG "IntelIspParamAdaptor"

#include "modules/algowrapper/IntelIspParamAdaptor.h"

#include "iutils/CameraLog.h"

namespace icamera {

IntelIspParamAdaptor::IntelIspParamAdaptor() {
    LOG2("@%s", __func__);
}

IntelIspParamAdaptor::~IntelIspParamAdaptor() {
    LOG2("@%s", __func__);
}

ia_isp_bxt* IntelIspParamAdaptor::init(const ia_binary_data* ispData, const ia_cmc_t* iaCmc,
                                       unsigned int maxStatsWidth, unsigned int maxStatsHeight,
                                       unsigned int maxNumStatsIn, ia_mkn* iaMkn) {
    LOG2("@%s", __func__);
    CheckError(!ispData || !iaCmc, nullptr, "%s, No CPF or CMC data", __func__);
    LOG1("%s, ispData size: %d, pointer: %p, max width: %u, max height: %u", __func__,
         ispData->size, ispData->data, maxStatsWidth, maxStatsHeight);

    return ia_isp_bxt_init(ispData, iaCmc, maxStatsWidth, maxStatsHeight, maxNumStatsIn, iaMkn);
}

void IntelIspParamAdaptor::deInit(ia_isp_bxt* ispBxtHandle) {
    LOG2("@%s", __func__);
    CheckError(!ispBxtHandle, VOID_VALUE, "%s, ispBxtHandle is nullptr", __func__);

    ia_isp_bxt_deinit(ispBxtHandle);
}

int IntelIspParamAdaptor::getPalDataSize(ia_isp_bxt_program_group* programGroup) {
    LOG2("@%s", __func__);
    CheckError(!programGroup, -1, "%s programGroup is nullptr", __func__);

    return ia_isp_bxt_get_output_size(programGroup);
}

void IntelIspParamAdaptor::freePalBuffer(void* addr) {
    LOG2("@%s addr: %p", __func__, addr);
    free(addr);
}

void* IntelIspParamAdaptor::allocatePalBuffer(int streamId, int index, int palDataSize) {
    LOG2("@%s index: %d, streamId: %d, size: %d", __func__, index, streamId, palDataSize);

    return calloc(1, palDataSize);
}

status_t IntelIspParamAdaptor::runPal(ia_isp_bxt* ispBxtHandle,
                                      const ia_isp_bxt_input_params_v2* inputParams,
                                      ia_binary_data* outputData) {
    LOG2("@%s", __func__);
    CheckError((!ispBxtHandle || !inputParams), UNKNOWN_ERROR,
               "%s, ispBxtHandle or inputParams is nullptr", __func__);
    CheckError((!outputData || !outputData->data || outputData->size <= 0), UNKNOWN_ERROR,
               "%s, Wrong pal data buffer", __func__);

    ia_err ret = ia_isp_bxt_run_v2(ispBxtHandle, inputParams, outputData);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "%s, isp parameters adaptor run failed %d",
               __func__, ret);
    LOG1("%s, The pal result size: %d", __func__, outputData->size);

    return OK;
}

status_t IntelIspParamAdaptor::queryAndConvertStats(ia_isp_bxt* ispBxtHandle,
                                                    ConvertInputParam* inputParams,
                                                    ConvertResult* result) {
    LOG2("@%s", __func__);
    CheckError(!ispBxtHandle, UNKNOWN_ERROR, "%s, ispBxtHandle is nullptr", __func__);
    CheckError(!inputParams || !result, UNKNOWN_ERROR, "%s, inputParams or result nullptr",
               __func__);
    CheckError(!inputParams->dvsReso || !inputParams->aeResults, UNKNOWN_ERROR,
               "%s, inputParams or result nullptr", __func__);
    CheckError((!inputParams->statsBuffer || !inputParams->statsBuffer->data ||
                inputParams->statsBuffer->size <= 0),
               UNKNOWN_ERROR, "%s, Wrong statistics buffer", __func__);
    CheckError(!result->queryResults, UNKNOWN_ERROR, "%s, queryResults is nullptr", __func__);

    ia_err ret =
        ia_isp_bxt_statistics_query(ispBxtHandle, inputParams->statsBuffer, result->queryResults);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "%s, Query statistice failed %d", __func__, ret);

    // Decode DVS statistics
    if (result->queryResults->dvs_stats) {
        if (inputParams->dvsReso->width == 0 || inputParams->dvsReso->height == 0) {
            LOGW("%s, The gdc resolution for DVS isn't correct", __func__);
        } else {
            ret = ia_isp_bxt_statistics_convert_dvs_from_binary(
                ispBxtHandle, inputParams->statsBuffer, inputParams->dvsReso->width,
                inputParams->dvsReso->height, &(result->dvsStats));
            CheckWarning((ret != ia_err_none || !result->dvsStats), UNKNOWN_ERROR,
                         "%s, Failed to convert DVS statistics %d", __func__, ret);
            LOG3A("%s, DVS stat vector_count: %u", __func__, result->dvsStats->vector_count);
        }
    }

    // Decode psa rgbs and af statistics
    if (result->queryResults->rgbs_grid && result->queryResults->af_grid &&
        !inputParams->multiExpo) {
        ret = ia_isp_bxt_statistics_convert_awb_from_binary_v3(
            ispBxtHandle, inputParams->statsBuffer, nullptr, inputParams->aeResults,
            inputParams->bcompResult, result->rgbsGrid, nullptr);

        ia_aiq_rgbs_grid* rgbs = *(result->rgbsGrid);
        CheckWarning((ret != ia_err_none || !rgbs), UNKNOWN_ERROR,
                     "%s, Failed to convert psa RGBS statistics %d", __func__, ret);
        LOG3A("%s, RGBS stat grid %dx%d", __func__, rgbs->grid_width, rgbs->grid_height);

        ret = ia_isp_bxt_statistics_convert_af_from_binary(ispBxtHandle, inputParams->statsBuffer,
                                                           &(result->afGrid));
        CheckWarning((ret != ia_err_none || !result->afGrid), UNKNOWN_ERROR,
                     "%s, Failed to convert psa AF statistics %d", __func__, ret);
        LOG3A("%s, AF stat grid %dx%d", __func__, result->afGrid->grid_width,
              result->afGrid->grid_height);
    }

    return OK;
}
}  // namespace icamera
