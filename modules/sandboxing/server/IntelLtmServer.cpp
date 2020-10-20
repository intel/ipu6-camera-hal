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

#define LOG_TAG "IntelLtmServer"

#include "modules/sandboxing/server/IntelLtmServer.h"

#include <utility>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLtmServer::IntelLtmServer() {
    LOGIPC("@%s", __func__);
}

IntelLtmServer::~IntelLtmServer() {
    LOGIPC("@%s", __func__);
}

int IntelLtmServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(LtmInitParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    LtmInitParams* params = static_cast<LtmInitParams*>(pData);
    ia_binary_data inData;
    uintptr_t mkn_hanlde;
    bool ret = mIpc.serverUnflattenInit(params, &inData, &mkn_hanlde);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    std::unique_ptr<IntelLtm> intelLtm = std::unique_ptr<IntelLtm>(new IntelLtm());
    ia_mkn* mkn = reinterpret_cast<ia_mkn*>(mkn_hanlde);
    ia_ltm* ltm = intelLtm->init(&inData, mkn);

    ret = mIpc.serverFlattenInit(params, ltm);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenInit fails", __func__);

    mIntelLtms[ltm] = std::move(intelLtm);

    return OK;
}

int IntelLtmServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(LtmDeinitParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    LtmDeinitParams* params = static_cast<LtmDeinitParams*>(pData);
    ia_ltm* ltm = reinterpret_cast<ia_ltm*>(params->ltm_handle);
    LOGIPC("@%s, params->ltm_handle:%p", __func__, ltm);

    if (mIntelLtms.find(ltm) == mIntelLtms.end()) {
        LOGE("@%s, ltm:%p doesn't exist", __func__, ltm);
        return UNKNOWN_ERROR;
    }

    mIntelLtms[ltm]->deinit(ltm);
    mIntelLtms.erase(ltm);

    return OK;
}

int IntelLtmServer::run(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(LtmRunParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    ia_ltm_input_params* inputParams = nullptr;
    ia_ltm* ltm = nullptr;
    bool ret = mIpc.serverUnflattenRun(pData, &ltm, &inputParams);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, serverUnflattenRun fails", __func__);

    ia_ltm_results* ltmResults = nullptr;
    ia_ltm_drc_params* drcResults = nullptr;

    if (mIntelLtms.find(ltm) == mIntelLtms.end()) {
        LOGE("@%s, ltm:%p doesn't exist", __func__, ltm);
        return UNKNOWN_ERROR;
    }

    ia_err retErr = mIntelLtms[ltm]->run(ltm, inputParams, &ltmResults, &drcResults);
    CheckError(retErr != ia_err_none, UNKNOWN_ERROR, "@%s, mIntelLtms->run fails", __func__);

    LtmRunParams* params = static_cast<LtmRunParams*>(pData);
    ret = mIpc.serverFlattenRun(*ltmResults, *drcResults, ltm, params);
    CheckError(!ret, UNKNOWN_ERROR, "@%s, serverFlattenRun fails", __func__);

    return OK;
}
} /* namespace icamera */
