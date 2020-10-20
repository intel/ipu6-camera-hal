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

#define LOG_TAG "ClientIntelLtm"

#include "modules/sandboxing/client/IntelLtm.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLtm::IntelLtm() : mInitialized(false), mRunParamsSize(0) {
    LOGIPC("@%s", __func__);

    mCaller = reinterpret_cast<uintptr_t>(this);
    mMems = {
        {("/ltmInit" + std::to_string(mCaller) + "Shm"), sizeof(LtmInitParams), &mMemInit, false},
        {("/ltmDeinit" + std::to_string(mCaller) + "Shm"), sizeof(LtmDeinitParams), &mMemDeinit,
         false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelLtm::~IntelLtm() {
    LOGIPC("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
    mCommon.freeShmMem(mMemRun);
}

ia_ltm* IntelLtm::init(const ia_binary_data* lard_data_ptr, ia_mkn* mkn) {
    LOGIPC("@%s", __func__);
    CheckError(!lard_data_ptr, nullptr, "@%s, lard_data_ptr is null", __func__);
    CheckError(!mkn, nullptr, "@%s, mkn is null", __func__);
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    LtmInitParams* params = static_cast<LtmInitParams*>(mMemInit.mAddr);

    bool ret = mIpc.clientFlattenInit(*lard_data_ptr, reinterpret_cast<uintptr_t>(mkn), params);
    CheckError(ret == false, nullptr, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_LTM_INIT, mMemInit.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    ia_ltm* ltm = nullptr;
    ret = mIpc.clientUnflattenInit(params, &ltm);
    CheckError(ret == false, nullptr, "@%s, clientUnflattenInit fails", __func__);

    return ltm;
}

void IntelLtm::deinit(ia_ltm* ltm) {
    LOGIPC("@%s", __func__);
    CheckError(!ltm, VOID_VALUE, "@%s, ltm is nullptr", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    LtmDeinitParams* params = static_cast<LtmDeinitParams*>(mMemDeinit.mAddr);
    params->ltm_handle = reinterpret_cast<uintptr_t>(ltm);

    bool ret = mCommon.requestSync(IPC_LTM_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

int IntelLtm::allocaRunImage(int dataSize) {
    mRunParamsSize = 0;
    mCommon.freeShmMem(mMemRun);
    mMemRun = {};

    mMemRun.mName = "/ltmRun" + std::to_string(mCaller) + "Shm";
    mMemRun.mSize = dataSize;
    bool ret = mCommon.allocShmMem(mMemRun.mName, mMemRun.mSize, &mMemRun);
    CheckError(!ret, mRunParamsSize, "@%s, allocShmMem fails", __func__);

    mRunParamsSize = dataSize;
    return mRunParamsSize;
}

ia_err IntelLtm::run(ia_ltm* ltm, const ia_ltm_input_params* inputParams,
                     ia_ltm_results** ltmResults, ia_ltm_drc_params** drcResults) {
    LOGIPC("@%s", __func__);

    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(!ltm, ia_err_general, "@%s, ltm is nullptr", __func__);
    CheckError(!inputParams, ia_err_general, "@%s, inputParams is nullptr", __func__);
    CheckError(!ltmResults, ia_err_general, "@%s, ltmResultsis is nullptr", __func__);
    CheckError(!drcResults, ia_err_general, "@%s, drcResults is nullptr", __func__);

    int runParamsSize = sizeof(LtmRunParams);
    if (inputParams->input_image_ptr && inputParams->input_image_ptr->image_data) {
        runParamsSize += inputParams->input_image_ptr->image_data->size;
    }

    if (mRunParamsSize < runParamsSize) {
        LOGIPC("@%s, mRunParamsSize %d, runParamsSize %d", __func__, mRunParamsSize, runParamsSize);
        int dataSize = allocaRunImage(runParamsSize);
        CheckError(dataSize <= 0, ia_err_general, "@%s, Failed to allocaRunImage", __func__);
    }

    bool ret = mIpc.clientFlattenRun(reinterpret_cast<uintptr_t>(ltm), *inputParams, mMemRun.mSize,
                                     mMemRun.mAddr);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenPrepare fails", __func__);

    ret = mCommon.requestSync(IPC_LTM_RUN, mMemRun.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    LtmRunParams* params = static_cast<LtmRunParams*>(mMemRun.mAddr);
    ret = mIpc.clientUnflattenRun(params, ltmResults, drcResults);
    CheckError(ret == false, ia_err_general, "@%s, clientUnflattenPrepare fails", __func__);

    return ia_err_none;
}

}  // namespace icamera
