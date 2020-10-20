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

#define LOG_TAG "ClientIntelMkn"

#include "modules/sandboxing/client/IntelMkn.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IntelMkn::IntelMkn() : mInitialized(false) {
    LOGIPC("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    mMems = {
        {("/mknInit" + std::to_string(personal) + "Shm"), sizeof(MknInitParams), &mMemInit, false},
        {("/mknDeinit" + std::to_string(personal) + "Shm"), sizeof(MknDeinitParams), &mMemDeinit,
         false},
        {("/mknPrepare" + std::to_string(personal) + "Shm"), sizeof(MknPrepareParams), &mMemPrepare,
         false},
        {("/mknEnable" + std::to_string(personal) + "Shm"), sizeof(MknEnableParams), &mMemEnable,
         false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelMkn::~IntelMkn() {
    LOGIPC("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

ia_mkn* IntelMkn::init(ia_mkn_config_bits mkn_config_bits, size_t mkn_section_1_size,
                       size_t mkn_section_2_size) {
    LOGIPC("@%s, mkn_config_bits:%d, mkn_section_1_size:%zu, mkn_section_2_size:%zu", __func__,
           mkn_config_bits, mkn_section_1_size, mkn_section_2_size);
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    MknInitParams* params = static_cast<MknInitParams*>(mMemInit.mAddr);

    bool ret =
        mIpc.clientFlattenInit(mkn_config_bits, mkn_section_1_size, mkn_section_2_size, params);
    CheckError(ret == false, nullptr, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_MKN_INIT, mMemInit.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    return reinterpret_cast<ia_mkn*>(params->results);
}

void IntelMkn::deinit(ia_mkn* pMkn) {
    LOGIPC("@%s", __func__);
    CheckError(pMkn == nullptr, VOID_VALUE, "@%s, mkn is nullptr", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    MknDeinitParams* params = static_cast<MknDeinitParams*>(mMemDeinit.mAddr);
    params->mkn_handle = reinterpret_cast<uintptr_t>(pMkn);

    bool ret = mCommon.requestSync(IPC_MKN_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

int IntelMkn::prepare(ia_mkn* pMkn, ia_mkn_trg data_target, ia_binary_data* pBinaryData) {
    LOGIPC("@%s", __func__);

    *pBinaryData = {nullptr, 0};
    CheckError(pMkn == nullptr, UNKNOWN_ERROR, "@%s, mkn is nullptr", __func__);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    MknPrepareParams* params = static_cast<MknPrepareParams*>(mMemPrepare.mAddr);
    bool ret = mIpc.clientFlattenPrepare(reinterpret_cast<uintptr_t>(pMkn), data_target, params);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenPrepare fails", __func__);

    ret = mCommon.requestSync(IPC_MKN_PREPARE, mMemPrepare.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenPrepare(params, pBinaryData);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenPrepare fails", __func__);

    return OK;
}

int IntelMkn::enable(ia_mkn* pMkn, bool enable_data_collection) {
    LOGIPC("@%s, enable_data_collection:%d", __func__, enable_data_collection);

    CheckError(pMkn == nullptr, UNKNOWN_ERROR, "@%s, mkn is nullptr", __func__);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    MknEnableParams* params = static_cast<MknEnableParams*>(mMemEnable.mAddr);
    params->mkn_handle = reinterpret_cast<uintptr_t>(pMkn);
    params->enable_data_collection = enable_data_collection;

    bool ret = mCommon.requestSync(IPC_MKN_ENABLE, mMemEnable.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}
}  // namespace icamera
