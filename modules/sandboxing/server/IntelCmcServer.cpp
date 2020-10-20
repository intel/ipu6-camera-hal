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

#define LOG_TAG "IntelCmcServer"

#include "modules/sandboxing/server/IntelCmcServer.h"

#include <ia_cmc_parser.h>
#include <string.h>

#include <utility>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelCmcServer::IntelCmcServer() {
    LOGIPC("@%s", __func__);
}

IntelCmcServer::~IntelCmcServer() {
    LOGIPC("@%s", __func__);

    mIntelCmcs.clear();
}

status_t IntelCmcServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(cmc_init_params), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    cmc_init_params* params = static_cast<cmc_init_params*>(pData);
    ia_binary_data aiqbData = {nullptr, 0};

    bool ret = mIpc.serverUnflattenInit(*params, &aiqbData);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    std::unique_ptr<IntelCmc> intelCmc = std::make_unique<IntelCmc>();

    ret = intelCmc->init(&aiqbData, nullptr);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, intelCmc->init fails", __func__);

    ia_cmc_t* cmc = intelCmc->getCmc();
    CheckError(!cmc, UNKNOWN_ERROR, "@%s, cmc is nullptr", __func__);
    LOGIPC("@%s, cmc:%p", __func__, cmc);

    ret = mIpc.serverFlattenInit(*cmc, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenInit fails", __func__);

    mIntelCmcs[cmc] = std::move(intelCmc);

    return OK;
}

status_t IntelCmcServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(!pData, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(cmc_deinit_params), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    cmc_deinit_params* params = static_cast<cmc_deinit_params*>(pData);

    ia_cmc_t* cmc = reinterpret_cast<ia_cmc_t*>(params->cmc_handle);
    LOGIPC("@%s, cmc:%p", __func__, cmc);

    if (mIntelCmcs.find(cmc) == mIntelCmcs.end()) {
        LOGE("@%s, cmc:%p doesn't exist", __func__, cmc);
        return UNKNOWN_ERROR;
    }

    mIntelCmcs[cmc]->deinit();

    mIntelCmcs.erase(cmc);

    return OK;
}
} /* namespace icamera */
