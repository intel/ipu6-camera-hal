/*
 * Copyright (C) 2022-2023 Intel Corporation
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

#define LOG_TAG IntelICBMClient

#include "modules/sandboxing/client/IntelICBMClient.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

int IntelICBM::shutdown(const ICBMReqInfo& reqInfo) {
    ICBMReqInfo* runInfo = reinterpret_cast<ICBMReqInfo*>(mRunInfoMem.mAddr);
    *runInfo = reqInfo;

    bool ret = mCommon.requestSync(IPC_ICBM_DEINIT, mRunInfoMem.mHandle);
    if (!ret) LOGE("%s, Intel ICBM shutdown failed", __func__);

    mCommon.freeShmMem(mRunInfoMem, GPU_ALGO_SHM);
    return ret;
}

int IntelICBM::setup(ICBMInitInfo* initParam) {
    auto ret = initRunInfoBuffer();
    if (ret != OK) return ret;

    ShmMemInfo initMems;
    initMems.mName = "/IntelICBMInitMem";

    ICBMInitInfo dummy{};

    if (initParam == nullptr) initParam = &dummy;

    initMems.mSize = sizeof(*initParam);

    auto re = mCommon.allocShmMem(initMems.mName, initMems.mSize, &initMems, GPU_ALGO_SHM);
    CheckAndLogError(!re, UNKNOWN_ERROR, "%s, Alloc init allocShmMem failed", __func__);

    ICBMInitInfo* im = reinterpret_cast<ICBMInitInfo*>(initMems.mAddr);
    *im = *initParam;

    re = mCommon.requestSync(IPC_ICBM_INIT, initMems.mHandle);
    mCommon.freeShmMem(initMems, GPU_ALGO_SHM);

    return re ? OK : UNKNOWN_ERROR;
}

int IntelICBM::initRunInfoBuffer() {
    std::string name =
        "/IntelICBMRuninfo" + std::to_string(reinterpret_cast<uintptr_t>(this)) + SHM_NAME;
    mRunInfoMem.mName = name.c_str();
    mRunInfoMem.mSize = sizeof(ICBMReqInfo);

    auto ret =
        mCommon.allocShmMem(mRunInfoMem.mName, mRunInfoMem.mSize, &mRunInfoMem, GPU_ALGO_SHM);

    CheckAndLogError(!ret, UNKNOWN_ERROR, "%s, RunInfo SHM alloc error!", __func__);

    return OK;
}

int IntelICBM::processFrame(const ICBMReqInfo& reqInfo) {
    ICBMReqInfo* runInfo = reinterpret_cast<ICBMReqInfo*>(mRunInfoMem.mAddr);

    *runInfo = reqInfo;
    runInfo->inHandle = mCommon.registerGbmBuffer(reqInfo.inII.gfxHandle, GPU_ALGO_SHM);
    CheckAndLogError(runInfo->inHandle < 0, UNKNOWN_ERROR, "%s, Cannot register in GBM buffers.",
                     __func__);

    runInfo->outHandle = mCommon.registerGbmBuffer(reqInfo.outII.gfxHandle, GPU_ALGO_SHM);
    CheckAndLogError(runInfo->outHandle < 0, UNKNOWN_ERROR, "%s, Cannot register out GBM buffers.",
                     __func__);

    auto runInfoHandle = mCommon.getShmMemHandle(reinterpret_cast<void*>(runInfo), GPU_ALGO_SHM);
    if (runInfoHandle < 0) {
        LOGE("%s, Cannot get GBMSHM handle.", __func__);
        mCommon.deregisterGbmBuffer(runInfo->inHandle, GPU_ALGO_SHM);
        mCommon.deregisterGbmBuffer(runInfo->outHandle, GPU_ALGO_SHM);
        return UNKNOWN_ERROR;
    }

    bool ret = mCommon.requestSync(IPC_ICBM_RUN_FRAME, runInfoHandle);
    mCommon.deregisterGbmBuffer(runInfo->inHandle, GPU_ALGO_SHM);
    mCommon.deregisterGbmBuffer(runInfo->outHandle, GPU_ALGO_SHM);

    CheckAndLogError(!ret, UNKNOWN_ERROR, "%s, Run frame IPC error!", __func__);
    return OK;
}

int IntelICBM::runTnrFrame(const ICBMReqInfo& reqInfo) {
    ICBMReqInfo* runInfo = reinterpret_cast<ICBMReqInfo*>(mRunInfoMem.mAddr);
    *runInfo = reqInfo;

    runInfo->inHandle = reqInfo.inII.gfxHandle;
    runInfo->outHandle = reqInfo.outII.gfxHandle;
    return mCommon.requestSync(IPC_ICBM_RUN_FRAME, mRunInfoMem.mHandle);
}

}  // namespace icamera
