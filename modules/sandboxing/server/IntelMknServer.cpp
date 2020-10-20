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

#define LOG_TAG "IntelMknServer"

#include "modules/sandboxing/server/IntelMknServer.h"

#include <utility>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelMknServer::IntelMknServer() {
    LOGIPC("@%s", __func__);
}

IntelMknServer::~IntelMknServer() {
    LOGIPC("@%s", __func__);

    mIntelMkns.clear();
}

int IntelMknServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(MknInitParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    MknInitParams* params = static_cast<MknInitParams*>(pData);

    std::unique_ptr<IntelMkn> intelMnk = std::make_unique<IntelMkn>();
    ia_mkn* mkn = intelMnk->init(params->mkn_config_bits, params->mkn_section_1_size,
                                 params->mkn_section_2_size);
    CheckError(mkn == nullptr, UNKNOWN_ERROR, "@%s, mkn.init fails", __func__);

    params->results = reinterpret_cast<uintptr_t>(mkn);
    LOGIPC("@%s, mkn:%p, params->results:%", __func__, mkn, params->results);

    mIntelMkns[mkn] = std::move(intelMnk);

    return OK;
}

int IntelMknServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(MknDeinitParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    MknDeinitParams* params = static_cast<MknDeinitParams*>(pData);
    ia_mkn* mkn = reinterpret_cast<ia_mkn*>(params->mkn_handle);
    LOGIPC("@%s, params->mkn_handle:%p", __func__, mkn);

    IntelMkn* intelMnk = getIntelMkn(mkn);
    CheckError(intelMnk == nullptr, UNKNOWN_ERROR, "@%s, mkn:%p doesn't exist", __func__, mkn);

    intelMnk->deinit(mkn);
    mIntelMkns.erase(mkn);

    return OK;
}

int IntelMknServer::prepare(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(MknPrepareParams), UNKNOWN_ERROR, "@%s, buffer is small",
               __func__);

    MknPrepareParams* params = static_cast<MknPrepareParams*>(pData);

    ia_mkn* mkn = reinterpret_cast<ia_mkn*>(params->mkn_handle);
    ia_binary_data data;

    IntelMkn* intelMnk = getIntelMkn(mkn);
    CheckError(intelMnk == nullptr, UNKNOWN_ERROR, "@%s, mkn:%p doesn't exist", __func__, mkn);

    int ret = intelMnk->prepare(mkn, params->data_target, &data);
    CheckError(ret != OK, NO_MEMORY, "Failed to prepare makernote");
    LOGIPC("@%s, data.size:%d, data.data:%p", __func__, data.size, data.data);

    bool retFlag = mIpc.serverFlattenPrepare(data, params);
    CheckError(retFlag == false, UNKNOWN_ERROR, "@%s, serverFlattenPrepare fails", __func__);

    return OK;
}

int IntelMknServer::enable(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(MknEnableParams), UNKNOWN_ERROR, "@%s, buffer is small", __func__);

    MknEnableParams* params = static_cast<MknEnableParams*>(pData);
    ia_mkn* mkn = reinterpret_cast<ia_mkn*>(params->mkn_handle);

    IntelMkn* intelMnk = getIntelMkn(mkn);
    CheckError(intelMnk == nullptr, UNKNOWN_ERROR, "@%s, mkn:%p doesn't exist", __func__, mkn);

    return intelMnk->enable(mkn, params->enable_data_collection);
}

IntelMkn* IntelMknServer::getIntelMkn(ia_mkn* mkn) {
    LOGIPC("@%s, mkn:%p", __func__, mkn);

    if (mIntelMkns.find(mkn) == mIntelMkns.end()) {
        LOGE("@%s, mkn:%p doesn't exist", __func__, mkn);
        return nullptr;
    }

    return mIntelMkns[mkn].get();
}

}  // namespace icamera
