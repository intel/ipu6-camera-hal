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

#define LOG_TAG "IntelLardServer"

#include "modules/sandboxing/server/IntelLardServer.h"

#include <memory>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLardServer::IntelLardServer() {
    mLard = std::unique_ptr<IntelLard>(new IntelLard());
    LOGIPC("@%s", __func__);
}

IntelLardServer::~IntelLardServer() {
    LOGIPC("@%s", __func__);
}

status_t IntelLardServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_binary_data binaryData = {nullptr, 0};
    bool ret = mIpc.serverUnflattenInit(pData, dataSize, &binaryData);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    ia_lard* lard = mLard->init(&binaryData);
    CheckError(lard == nullptr, UNKNOWN_ERROR, "@%s, mLard.init fails", __func__);

    ret = mIpc.serverFlattenInit(pData, dataSize, lard);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverflattenInit fails", __func__);

    return OK;
}

status_t IntelLardServer::getTagList(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_lard* lard = nullptr;
    unsigned int mode_tag = LCMC_TAG;

    bool ret = mIpc.serverUnflattenGetTagList(pData, dataSize, &lard, &mode_tag);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenGetTagList fails", __func__);
    CheckError(lard == nullptr, UNKNOWN_ERROR, "@%s, serverUnflattenGetTagList fails", __func__);

    unsigned int num_tags = 0;
    const unsigned int* tags = nullptr;

    ia_err ret1 = mLard->getTagList(lard, mode_tag, &num_tags, &tags);
    CheckError(ret1 != ia_err_none, UNKNOWN_ERROR, "@%s, mLard.getTagList fails", __func__);

    mIpc.serverFlattenGetTagList(pData, dataSize, num_tags, tags);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetTagList fails", __func__);
    CheckError(lard == nullptr, UNKNOWN_ERROR, "@%s, serverFlattenGetTagList fails", __func__);

    return OK;
}

status_t IntelLardServer::run(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_lard* lard = nullptr;
    ia_lard_input_params* inputParams = nullptr;

    bool ret = mIpc.serverUnflattenRun(pData, dataSize, &lard, &inputParams);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenRun fails", __func__);
    CheckError(lard == nullptr, UNKNOWN_ERROR, "@%s, serverUnflattenRun fails", __func__);
    CheckError(inputParams == nullptr, UNKNOWN_ERROR, "@%s, serverUnflattenRun fails", __func__);

    ia_lard_results* result = nullptr;
    ia_err ret1 = mLard->run(lard, inputParams, &result);
    CheckError(ret1 != ia_err_none, UNKNOWN_ERROR, "@%s, mLard.run fails", __func__);

    ret = mIpc.serverFlattenRun(pData, dataSize, result);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenRun fails", __func__);

    return OK;
}

status_t IntelLardServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    ia_lard* lard = nullptr;
    bool ret = mIpc.serverUnflattenDeinit(pData, dataSize, &lard);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDeinit fails", __func__);
    CheckError(lard == nullptr, UNKNOWN_ERROR, "@%s, serverUnflattenDeinit fails", __func__);

    mLard->deinit(lard);

    return OK;
}
} /* namespace icamera */
