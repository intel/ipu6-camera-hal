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

#define LOG_TAG "IntelCmc"

#include "modules/sandboxing/client/IntelCmc.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelCmc::IntelCmc() : mInitialized(false) {
    LOGIPC("@%s", __func__);

    mHandle = nullptr;
    mCmcRemoteHandle = reinterpret_cast<uintptr_t>(nullptr);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/cmcInit" + std::to_string(personal) + "Shm";
    std::string deinitName = "/cmcDeinit" + std::to_string(personal) + "Shm";

    mMems = {{initName.c_str(), sizeof(cmc_init_params), &mMemInit, false},
             {deinitName.c_str(), sizeof(cmc_deinit_params), &mMemDeinit, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelCmc::~IntelCmc() {
    LOGIPC("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

bool IntelCmc::init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData) {
    LOGIPC("@%s, aiqbData:%p, nvmData:%p", __func__, aiqbData, nvmData);
    CheckError(nvmData, false, "@%s, nvmData should be nullptr", __func__);

    CheckError(mInitialized == false, false, "@%s, mInitialized is false", __func__);
    CheckError(!aiqbData, false, "@%s, aiqbData is nullptr", __func__);
    CheckError(!aiqbData->data, false, "@%s, aiqbData->data is nullptr", __func__);
    CheckError(aiqbData->size == 0, false, "@%s, aiqbData->size is 0", __func__);

    cmc_init_params* params = static_cast<cmc_init_params*>(mMemInit.mAddr);

    bool ret = mIpc.clientFlattenInit(*aiqbData, params);
    CheckError(ret == false, false, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_CMC_INIT, mMemInit.mHandle);
    CheckError(ret == false, false, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenInit(*params, &mHandle, &mCmcRemoteHandle);
    CheckError(ret == false, false, "@%s, clientUnflattenInit fails", __func__);

    return true;
}

ia_cmc_t* IntelCmc::getCmc() const {
    LOGIPC("@%s, mHandle:%p", __func__, mHandle);

    return mHandle;
}

uintptr_t IntelCmc::getCmcHandle() const {
    LOGIPC("@%s", __func__);

    return mCmcRemoteHandle;
}

void IntelCmc::deinit() {
    LOGIPC("@%s, mCmc:%p", __func__, mHandle);

    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(!mHandle, VOID_VALUE, "@%s, mHandle is nullptr", __func__);
    CheckError(reinterpret_cast<ia_cmc_t*>(mCmcRemoteHandle) == nullptr, VOID_VALUE,
               "@%s, mCmcRemoteHandle is nullptr", __func__);

    cmc_deinit_params* params = static_cast<cmc_deinit_params*>(mMemDeinit.mAddr);
    params->cmc_handle = mCmcRemoteHandle;

    bool ret = mCommon.requestSync(IPC_CMC_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
    mHandle = nullptr;
    mCmcRemoteHandle = reinterpret_cast<uintptr_t>(nullptr);
}

} /* namespace icamera */
