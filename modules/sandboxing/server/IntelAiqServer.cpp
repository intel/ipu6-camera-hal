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

#define LOG_TAG "IntelAiqServer"

#include "modules/sandboxing/server/IntelAiqServer.h"

#include <ia_aiq.h>
#include <ia_cmc_parser.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace icamera {
IntelAiqServer::IntelAiqServer() {
    LOGIPC("@%s", __func__);
}

IntelAiqServer::~IntelAiqServer() {
    LOGIPC("@%s", __func__);
}

status_t IntelAiqServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(aiq_init_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    aiq_init_params* params = static_cast<aiq_init_params*>(pData);

    ia_binary_data aiqbData = {nullptr, 0};
    ia_binary_data nvmData = {nullptr, 0};
    ia_binary_data aiqdData = {nullptr, 0};
    bool ret = mIpc.serverUnflattenInit(pData, dataSize, &aiqbData, &nvmData, &aiqdData);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    std::unique_ptr<IntelAiq> intelAiq = std::make_unique<IntelAiq>();

    ia_aiq* aiq = intelAiq->init(&aiqbData, &nvmData, &aiqdData, params->stats_max_width,
                                 params->stats_max_height, params->max_num_stats_in,
                                 reinterpret_cast<ia_cmc_t*>(params->cmcRemoteHandle),
                                 reinterpret_cast<ia_mkn*>(params->ia_mkn));
    CheckError(!aiq, UNKNOWN_ERROR, "@%s, intelAiq.init fails", __func__);

    mIntelAiqs[aiq] = std::move(intelAiq);

    params->results = reinterpret_cast<uintptr_t>(aiq);

    return OK;
}

status_t IntelAiqServer::aeRun(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "aeRun");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(ae_run_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    ae_run_params* params = static_cast<ae_run_params*>(pData);

    ia_aiq_ae_input_params* aeParams = nullptr;
    bool ret = mIpc.serverUnflattenAe(params, &aeParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenAe fails", __func__);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_aiq_ae_results* aeResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->aeRun(aeParams, &aeResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, aeRun fails %d", __func__, err);

    ret = mIpc.serverFlattenAe(*aeResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenAe fails", __func__);

    return OK;
}

status_t IntelAiqServer::afRun(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "afRun");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(af_run_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    af_run_params* params = static_cast<af_run_params*>(pData);

    ia_aiq_af_input_params* afParams = nullptr;
    bool ret = mIpc.serverUnflattenAf(params, &afParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflatten fails", __func__);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_aiq_af_results* afResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->afRun(afParams, &afResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, afRun fails %d", __func__, err);

    ret = mIpc.serverFlattenAf(*afResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlatten fails", __func__);

    return OK;
}

status_t IntelAiqServer::awbRun(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "awbRun");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(awb_run_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    awb_run_params* params = static_cast<awb_run_params*>(pData);

    ia_aiq_awb_input_params* awbParams = nullptr;
    bool ret = mIpc.serverUnflattenAwb(params, &awbParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenAwb fails", __func__);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_aiq_awb_results* awbResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->awbRun(awbParams, &awbResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, awbRun fails %d", __func__, err);

    ret = mIpc.serverFlattenAwb(*awbResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenAwb fails", __func__);

    return OK;
}

status_t IntelAiqServer::gbceRun(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "gbceRun");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(gbce_run_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    gbce_run_params* params = static_cast<gbce_run_params*>(pData);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_aiq_gbce_results* gbceResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->gbceRun(&params->base, &gbceResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, gbceRun fails %d", __func__, err);

    bool ret = mIpc.serverFlattenGbce(*gbceResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGbce fails", __func__);

    return OK;
}

status_t IntelAiqServer::paRunV1(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "paRunV1");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pa_run_v1_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    pa_run_v1_params* params = static_cast<pa_run_v1_params*>(pData);

    ia_aiq_pa_input_params* paParams = nullptr;
    bool ret = mIpc.serverUnflattenPaV1(params, &paParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenPa fails", __func__);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    //    ia_aiq_pa_results
    ia_aiq_pa_results_v1* paResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->paRunV1(paParams, &paResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, paRunV1 fails %d", __func__, err);

    ret = mIpc.serverFlattenPaV1(*paResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenPa fails", __func__);

    return OK;
}

status_t IntelAiqServer::saRunV2(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelAiqServer", "saRunV2");
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(sa_run_v2_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    sa_run_v2_params* params = static_cast<sa_run_v2_params*>(pData);

    ia_aiq_sa_input_params_v1* saParams = nullptr;
    bool ret = mIpc.serverUnflattenSaV2(*params, &saParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenSaV2 fails", __func__);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_aiq_sa_results_v1* saResults = nullptr;
    ia_err err = mIntelAiqs[aiq]->saRunV2(saParams, &saResults);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, saRunV2 fails %d", __func__, err);

    ret = mIpc.serverFlattenSaV2(*saResults, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenSaV2 fails", __func__);

    return OK;
}

status_t IntelAiqServer::statisticsSetV4(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(set_statistics_set_v4_params), UNKNOWN_ERROR,
               "@%s, buffer is small", __func__);

    set_statistics_set_v4_params* params = static_cast<set_statistics_set_v4_params*>(pData);

    ia_aiq_statistics_input_params_v4* stat = nullptr;
    bool ret = mIpc.serverUnflattenStatSetV4(params, &stat);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenStatSetV4 fails", __func__);

    if (stat->faces) {
        LOGIPC("@%s, num_faces:%d", __func__, stat->faces->num_faces);
        if (stat->faces->num_faces > 0) {
            ia_rectangle& rect = stat->faces->faces[0].face_area;
            LOGIPC("@%s, left:%d, top:%d, right:%d, bottom:%d", __func__, rect.left, rect.top,
                   rect.right, rect.bottom);
        }
    }

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->ia_aiq);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_err err = mIntelAiqs[aiq]->statisticsSetV4(stat);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, statisticsSetV4 fails %d", __func__, err);

    return OK;
}

status_t IntelAiqServer::getAiqdData(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(ia_binary_data_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    ia_binary_data binaryData = {nullptr, 0};

    ia_binary_data_params* params = static_cast<ia_binary_data_params*>(pData);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    ia_err err = mIntelAiqs[aiq]->getAiqdData(&binaryData);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, getAiqdData fails %d", __func__, err);
    LOGIPC("@%s, binary_data, data:%p, size:%d", __func__, binaryData.data, binaryData.size);

    MEMCPY_S(params->data, sizeof(params->data), binaryData.data, binaryData.size);
    params->size = binaryData.size;

    return OK;
}

status_t IntelAiqServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(aiq_deinit_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    aiq_deinit_params* params = static_cast<aiq_deinit_params*>(pData);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    mIntelAiqs[aiq]->deinit();

    return OK;
}

status_t IntelAiqServer::getVersion(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(ia_aiq_version_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    ia_aiq_version_params* params = static_cast<ia_aiq_version_params*>(pData);

    ia_aiq* aiq = reinterpret_cast<ia_aiq*>(params->aiq_handle);
    if (mIntelAiqs.find(aiq) == mIntelAiqs.end()) {
        LOGE("@%s, aiq:%p doesn't exist", __func__, aiq);
        return UNKNOWN_ERROR;
    }

    std::string version;
    mIntelAiqs[aiq]->getVersion(&version);
    snprintf(params->data, sizeof(params->data), "%s", version.c_str());
    params->size = std::min(version.size(), sizeof(params->data));
    LOGIPC("@%s, aiq version:%s, size:%d", __func__, version.c_str(), params->size);

    return OK;
}
} /* namespace icamera */
