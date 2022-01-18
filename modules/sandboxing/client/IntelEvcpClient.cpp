/*
 * Copyright (C) 2021 Intel Corporation
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

#define LOG_TAG IntelEVCPClient

#include "modules/sandboxing/client/IntelEvcpClient.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelEvcp::~IntelEvcp() {
    bool ret = mCommon.requestSync(IPC_EVCP_DEINIT);
    if (!ret) LOGE("@%s, EVCP DEINIT Fails", __func__);

    mCommon.freeShmMem(mParamMems, GPU_ALGO_SHM);
    mCommon.freeShmMem(mEvcpRunInfoMem, GPU_ALGO_SHM);
}

int IntelEvcp::init(int width, int height, EvcpParam* param) {
    bool ret = initResolution(width, height);
    if (!ret) return UNKNOWN_ERROR;

    ret = initRunInfoBuffer();
    if (!ret) return UNKNOWN_ERROR;

    ret = initParamBuffer();
    if (!ret) return UNKNOWN_ERROR;

    if (param) {
        ret = updateEvcpParam(param);
    } else {
        EvcpParam defParam = {false, false, false, false};
        ret = updateEvcpParam(&defParam);
    }

    if (!ret) return UNKNOWN_ERROR;

    return OK;
}

bool IntelEvcp::initRunInfoBuffer() {
    std::string name = "/evcpRuninfo" + std::to_string(reinterpret_cast<uintptr_t>(this))
                        + SHM_NAME;
    mEvcpRunInfoMem.mName = name.c_str();
    mEvcpRunInfoMem.mSize = sizeof(EvcpRunInfo);
    return mCommon.allocShmMem(mEvcpRunInfoMem.mName, mEvcpRunInfoMem.mSize, &mEvcpRunInfoMem,
                                   GPU_ALGO_SHM);
}

bool IntelEvcp::initParamBuffer() {
    std::string name = "/evcpParam" + std::to_string(reinterpret_cast<uintptr_t>(this))
                        + SHM_NAME;
    mParamMems.mName = name.c_str();
    mParamMems.mSize = sizeof(EvcpParam);
    return mCommon.allocShmMem(mParamMems.mName, mParamMems.mSize, &mParamMems, GPU_ALGO_SHM);
}

bool IntelEvcp::initResolution(int width, int height) {
    ShmMemInfo resolutionMems;
    resolutionMems.mName = "/evcpResolutionShm";
    resolutionMems.mSize = sizeof(EvcpResolution);

    bool ret = mCommon.allocShmMem(resolutionMems.mName, resolutionMems.mSize, &resolutionMems,
                                   GPU_ALGO_SHM);
    CheckAndLogError(!ret, false, "@%s, Alloc resolution allocShmMem fails", __func__);

    EvcpResolution* res = reinterpret_cast<EvcpResolution*>(resolutionMems.mAddr);
    res->width = width;
    res->height = height;

    ret = mCommon.requestSync(IPC_EVCP_INIT, resolutionMems.mHandle);
    mCommon.freeShmMem(resolutionMems, GPU_ALGO_SHM);

    return ret;
}

bool IntelEvcp::runEvcpFrame(int dmafd, int dataSize) {
    EvcpRunInfo* runInfo = reinterpret_cast<EvcpRunInfo*>(mEvcpRunInfoMem.mAddr);

    runInfo->inHandle = mCommon.registerGbmBuffer(dmafd, GPU_ALGO_SHM);
    CheckAndLogError(runInfo->inHandle < 0, false, "@%s, Cannot register GBM buffers.", __func__);

    runInfo->bufSize = dataSize;

    auto runInfoHandle = mCommon.getShmMemHandle(reinterpret_cast<void*>(runInfo), GPU_ALGO_SHM);
    if (runInfoHandle < 0) {
        LOGE("@%s, Cannot get shm handle.", __func__);
        mCommon.deregisterGbmBuffer(runInfo->inHandle, GPU_ALGO_SHM);
        return false;
    }

    bool ret = mCommon.requestSync(IPC_EVCP_RUN_FRAME, runInfoHandle);
    mCommon.deregisterGbmBuffer(runInfo->inHandle, GPU_ALGO_SHM);

    return ret;
}

void IntelEvcp::getEvcpParam(EvcpParam* evcpParam) {
    EvcpParam* pamem = reinterpret_cast<EvcpParam*>(mParamMems.mAddr);

    mCommon.requestSync(IPC_EVCP_GETCONF, mParamMems.mHandle);
    *evcpParam = *pamem;
}

bool IntelEvcp::updateEvcpParam(EvcpParam* param) {
    EvcpParam* pamem = reinterpret_cast<EvcpParam*>(mParamMems.mAddr);
    *pamem = *param;

    return mCommon.requestSync(IPC_EVCP_UPDCONF, mParamMems.mHandle);
}

}  // namespace icamera
