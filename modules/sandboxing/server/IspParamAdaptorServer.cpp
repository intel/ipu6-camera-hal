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

#define LOG_TAG "IspParamAdaptorServer"

#include "modules/sandboxing/server/IspParamAdaptorServer.h"

#include <utility>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IspParamAdaptorServer::IspParamAdaptorServer() {
    LOGIPC("@%s", __func__);
}

IspParamAdaptorServer::~IspParamAdaptorServer() {
    LOGIPC("@%s", __func__);
    mIspParamAdaptors.clear();
}

status_t IspParamAdaptorServer::init(void* pData, int size) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckError(size < sizeof(IspBxtInitParam), UNKNOWN_ERROR, "%s, buffer is small", __func__);

    IspBxtInitParam* params = static_cast<IspBxtInitParam*>(pData);

    ia_binary_data* ispData = nullptr;
    ia_cmc_t* cmcData = nullptr;
    bool ret = mIpc.serverUnflattenInit(params, size, &ispData, &cmcData);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    std::unique_ptr<IntelIspParamAdaptor> adaptor = std::make_unique<IntelIspParamAdaptor>();
    ia_isp_bxt* ispHandle =
        adaptor->init(ispData, cmcData, params->maxStatsWidth, params->maxStatsHeight,
                      params->maxStatsIn, params->iaMkn);
    CheckError(!ispHandle, UNKNOWN_ERROR, "@%s, init isp param adaptor failed", __func__);

    params->ispRemoteHandle = reinterpret_cast<uintptr_t>(ispHandle);
    LOGIPC("@%s ispHandle %p: %d", __func__, ispHandle, params->ispRemoteHandle);

    mIspParamAdaptors[ispHandle] = std::move(adaptor);

    return OK;
}

status_t IspParamAdaptorServer::deInit(void* pData, int size) {
    LOGIPC("@%s", __func__);
    CheckError(!pData, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckError(size < sizeof(IspBxtDeInitParam), UNKNOWN_ERROR, "%s, buffer is small", __func__);

    ia_isp_bxt* ispHandle = nullptr;
    bool ret = mIpc.serverUnflattenDeInit(pData, size, &ispHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDeInit fails", __func__);

    CheckError((mIspParamAdaptors.find(ispHandle) == mIspParamAdaptors.end()), UNKNOWN_ERROR,
               "%s, the isp handle doesn't exist in the table", __func__);
    CheckError(!mIspParamAdaptors[ispHandle], UNKNOWN_ERROR, "%s, IntelIspParamAdaptor is nullptr",
               __func__);

    mIspParamAdaptors[ispHandle]->deInit(ispHandle);
    mIspParamAdaptors.erase(ispHandle);

    return OK;
}

int IspParamAdaptorServer::getPalDataSize(void* pData, int size) {
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckError(size < sizeof(PalDataSizeParam), UNKNOWN_ERROR, "%s, buffer is small", __func__);

    ia_isp_bxt_program_group* programGroup = nullptr;
    bool ret = mIpc.serverUnflattenGetPalSize(pData, size, &programGroup);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenGetPalSize fails", __func__);

    CheckError(mIspParamAdaptors.empty(), UNKNOWN_ERROR, "%s, mIspParamAdaptors is empty",
               __func__);
    int palSize = mIspParamAdaptors.begin()->second->getPalDataSize(programGroup);
    LOGIPC("%s, The pal data size: %d", __func__, palSize);

    PalDataSizeParam* params = static_cast<PalDataSizeParam*>(pData);
    params->palDataSize = palSize;

    return OK;
}

status_t IspParamAdaptorServer::queryAndConvertStats(void* pData, int size, void* statsAddr) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IspParamAdaptorServer", "queryAndConvertStats");
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckError(size < sizeof(ConvertStatsParam), UNKNOWN_ERROR, "%s, buffer is small", __func__);

    ia_isp_bxt* ispHandle = nullptr;
    ConvertInputParam inputParams = {};
    ConvertResult result = {};

    bool ret =
        mIpc.serverUnflattenConvertStats(pData, size, &ispHandle, &inputParams, &result, statsAddr);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenConvertStats fails", __func__);

    CheckError((mIspParamAdaptors.find(ispHandle) == mIspParamAdaptors.end()), UNKNOWN_ERROR,
               "%s, the isp handle doesn't exist in the table", __func__);
    CheckError(!mIspParamAdaptors[ispHandle], UNKNOWN_ERROR, "%s, IntelIspParamAdaptor is nullptr",
               __func__);
    int res = mIspParamAdaptors[ispHandle]->queryAndConvertStats(ispHandle, &inputParams, &result);
    CheckError(res != OK, res, "%s, Failed to convert the status", __func__);

    ret = mIpc.serverFlattenConvertStats(pData, size, result);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenConvertStats fails", __func__);

    return OK;
}

status_t IspParamAdaptorServer::runPal(void* pData, int size, void* palDataAddr) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IspParamAdaptorServer", "runPal");
    LOGIPC("@%s, pData:%p, size:%d", __func__, pData, size);
    CheckError(!pData, UNKNOWN_ERROR, "%s, pData is nullptr", __func__);
    CheckError(!palDataAddr, UNKNOWN_ERROR, "%s, palDataAddr is nullptr", __func__);

    ia_isp_bxt* ispHandle = nullptr;
    ia_isp_bxt_input_params_v2* inputParams = nullptr;
    ia_binary_data* palOutput = nullptr;

    bool ret = mIpc.serverUnflattenRunPal(pData, size, &ispHandle, &inputParams, &palOutput);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenRunPal fails", __func__);
    palOutput->data = palDataAddr;
    LOGIPC("%s, palDataAddr: %p, size: %d", __func__, palDataAddr, palOutput->size);

    CheckError((mIspParamAdaptors.find(ispHandle) == mIspParamAdaptors.end()), UNKNOWN_ERROR,
               "%s, the isp handle doesn't exist in the table", __func__);
    CheckError(!mIspParamAdaptors[ispHandle], UNKNOWN_ERROR, "%s, IntelIspParamAdaptor is nullptr",
               __func__);

    int res = mIspParamAdaptors[ispHandle]->runPal(ispHandle, inputParams, palOutput);
    CheckError(res != OK, res, "%s, Failed to run pal", __func__);
    LOGIPC("%s, the pal data size is: %d after running", __func__, palOutput->size);

    return OK;
}

}  // namespace icamera
