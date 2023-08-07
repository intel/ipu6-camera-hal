/*
 * Copyright (C) 2020-2023 Intel Corporation
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

#define LOG_TAG IntelTNR7USClient

#include "modules/sandboxing/client/IntelTNR7USClient.h"

#include <string>
#include <memory>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
namespace icamera {

IntelTNR7US* IntelTNR7US::createIntelTNR(int cameraId) {
    if (!PlatformData::isGpuTnrEnabled()) return nullptr;
#ifdef TNR7_CM
    return new IntelC4mTNR(cameraId);
#else
    return new IntelLevel0TNR(cameraId);
#endif
}

Tnr7Param* IntelTNR7US::allocTnr7ParamBuf() {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/TnrParam" + std::to_string(personal) + "Shm";

    mParamMems.mName = initName.c_str();
    mParamMems.mSize = sizeof(Tnr7Param);
    bool ret = mCommon.allocShmMem(mParamMems.mName, mParamMems.mSize, &mParamMems, GPU_ALGO_SHM);
    CheckAndLogError(!ret, nullptr, "@%s, allocShmMem fails", __func__);

    return reinterpret_cast<Tnr7Param*>(mParamMems.mAddr);
}

void* IntelTNR7US::allocCamBuf(uint32_t bufSize, int id) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/TnrCam" + std::to_string(personal) + std::to_string(id) + "Shm";
    ShmMemInfo shm;
    shm.mName = initName.c_str();
    shm.mSize = bufSize;
    bool ret = mCommon.allocShmMem(shm.mName, shm.mSize, &shm, GPU_ALGO_SHM);
    CheckAndLogError(!ret, nullptr, "@%s, allocShmMem fails", __func__);

    mCamBufMems.push_back(shm);
    return shm.mAddr;
}

void IntelTNR7US::freeAllBufs() {
    if (mParamMems.mAddr) {
        mCommon.freeShmMem(mParamMems, GPU_ALGO_SHM);
    }
    for (auto& camBuf : mCamBufMems) {
        if (camBuf.mAddr) {
            mCommon.freeShmMem(camBuf, GPU_ALGO_SHM);
        }
    }
}

#ifdef TNR7_CM
IntelC4mTNR::IntelC4mTNR(int cameraId)
        : IntelTNR7US(cameraId),
          mTnrType(TNR_INSTANCE_MAX),
          mTnrRequestInfo(nullptr) {
    LOG1("<id%d> %s, Construct", cameraId, __func__);
}

IntelC4mTNR::~IntelC4mTNR() {
    // the instance not initialized, don't need to free
    if (mTnrType != TNR_INSTANCE_MAX) {
        mTnrRequestInfo->type = mTnrType;
        mTnrRequestInfo->cameraId = mCameraId;

        int32_t requestHandle =
            mCommon.getShmMemHandle(static_cast<void*>(mTnrRequestInfo), GPU_ALGO_SHM);
        int ret = mCommon.requestSync(IPC_GPU_TNR_DEINIT, requestHandle);
        CheckAndLogError(!ret, VOID_VALUE, "@%s, requestSync fails", __func__);
        mCommon.freeShmMem(mTnrRequestInfoMem, GPU_ALGO_SHM);
    }
    LOG1("<id%d> %s, Destroy", mCameraId, __func__);
}

int IntelC4mTNR::init(int width, int height, TnrType type) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/TnrRun" + std::to_string(personal) + "Shm";
    mTnrRequestInfoMem.mName = initName.c_str();
    mTnrRequestInfoMem.mSize = sizeof(TnrRequestInfo);
    bool ret = mCommon.allocShmMem(mTnrRequestInfoMem.mName, mTnrRequestInfoMem.mSize,
                                   &mTnrRequestInfoMem, GPU_ALGO_SHM);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "@%s, allocShmMem fails", __func__);
    mTnrRequestInfo = static_cast<TnrRequestInfo*>(mTnrRequestInfoMem.mAddr);

    initName = "/TnrInit" + std::to_string(personal) + "Shm";
    ShmMemInfo initInfoMems;
    initInfoMems.mName = initName.c_str();
    initInfoMems.mSize = sizeof(TnrInitInfo);
    ret = mCommon.allocShmMem(initInfoMems.mName, initInfoMems.mSize, &initInfoMems, GPU_ALGO_SHM);
    if (!ret) {
        LOGE("@%s, alloc initInfo ShmMem fails", __func__);
        mCommon.freeShmMem(mTnrRequestInfoMem, GPU_ALGO_SHM);
        return UNKNOWN_ERROR;
    }

    TnrInitInfo* initInfo = static_cast<TnrInitInfo*>(initInfoMems.mAddr);
    *initInfo = {width, height, mCameraId, type};

    ret = mCommon.requestSync(IPC_GPU_TNR_INIT, initInfoMems.mHandle);
    if (!ret) {
        LOGE("@%s, IPC %d failed", __func__, IPC_GPU_TNR_INIT);
        mCommon.freeShmMem(mTnrRequestInfoMem, GPU_ALGO_SHM);
    }
    mCommon.freeShmMem(initInfoMems, GPU_ALGO_SHM);
    mTnrType = type;
    LOG1("%s, GPU TNR instance size %dx%d, type %d", __func__, width, height, mTnrType);

    return ret ? OK : UNKNOWN_ERROR;
}

int IntelC4mTNR::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                             uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate, int fd) {
    LOG2("%s, type: %d, syncUpdate: %d, fd: %d", __func__, mTnrType, syncUpdate, fd);
    CheckAndLogError(!inBufAddr || !outBufAddr || !tnrParam, UNKNOWN_ERROR,
                     "@%s, invalid data buffer or parameter buffer", __func__);
    int32_t inHandle = mCommon.getShmMemHandle(const_cast<void*>(inBufAddr), GPU_ALGO_SHM);
    CheckAndLogError(inHandle < 0, UNKNOWN_ERROR, "@%s, can't find inBuf handle", __func__);
    CheckAndLogError(mParamMems.mAddr != tnrParam, UNKNOWN_ERROR, "@%s, invalid tnr parameter",
                     __func__);

    if (fd >= 0) {
        mTnrRequestInfo->outHandle = mCommon.registerGbmBuffer(fd, GPU_ALGO_SHM);
    } else {
        mTnrRequestInfo->outHandle =
            mCommon.getShmMemHandle(static_cast<void*>(outBufAddr), GPU_ALGO_SHM);
    }
    CheckAndLogError(mTnrRequestInfo->outHandle < 0, UNKNOWN_ERROR, "@%s, can't init outBuf handle",
                     __func__);

    mTnrRequestInfo->inHandle = inHandle;
    mTnrRequestInfo->paramHandle = mParamMems.mHandle;
    mTnrRequestInfo->type = mTnrType;
    mTnrRequestInfo->cameraId = mCameraId;
    mTnrRequestInfo->outBufFd = fd;
    mTnrRequestInfo->isForceUpdate = syncUpdate;

    int32_t requestHandle =
        mCommon.getShmMemHandle(static_cast<void*>(mTnrRequestInfo), GPU_ALGO_SHM);

    IPC_CMD cmd = mTnrType > 0 ? IPC_GPU_TNR_THREAD2_RUN_FRAME : IPC_GPU_TNR_RUN_FRAME;
    bool ret = mCommon.requestSync(cmd, requestHandle);

    if (fd >= 0) {
        mCommon.deregisterGbmBuffer(mTnrRequestInfo->outHandle, GPU_ALGO_SHM);
    }

    CheckAndLogError(!ret, OK, "@%s, run tnr fails", __func__);

    return OK;
}

void* IntelC4mTNR::allocCamBuf(uint32_t bufSize, int id) {
    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/TnrCam" + std::to_string(personal) + std::to_string(id) + "Shm";
    ShmMemInfo shm;
    shm.mName = initName.c_str();
    shm.mSize = bufSize;
    bool ret = mCommon.allocShmMem(shm.mName, shm.mSize, &shm, GPU_ALGO_SHM);
    CheckAndLogError(!ret, nullptr, "@%s, allocShmMem fails", __func__);
    mTnrRequestInfo->surfaceHandle = shm.mHandle;
    mTnrRequestInfo->type = mTnrType;
    mTnrRequestInfo->cameraId = mCameraId;

    int32_t requestHandle =
        mCommon.getShmMemHandle(static_cast<void*>(mTnrRequestInfo), GPU_ALGO_SHM);
    ret = mCommon.requestSync(IPC_GPU_TNR_PREPARE_SURFACE, requestHandle);
    if (!ret) {
        mCommon.freeShmMem(shm, GPU_ALGO_SHM);
        return nullptr;
    }
    mCamBufMems.push_back(shm);

    return shm.mAddr;
}

int IntelC4mTNR::asyncParamUpdate(int gain, bool forceUpdate) {
    LOG2("%s, type: %d, gain: %d, forceUpdate: %d", __func__, mTnrType, gain, forceUpdate);
    mTnrRequestInfo->gain = gain;
    mTnrRequestInfo->type = mTnrType;
    mTnrRequestInfo->cameraId = mCameraId;
    mTnrRequestInfo->isForceUpdate = forceUpdate;

    int32_t requestHandle =
        mCommon.getShmMemHandle(static_cast<void*>(mTnrRequestInfo), GPU_ALGO_SHM);

    IPC_CMD cmd = mTnrType > 0 ? IPC_GPU_TNR_THREAD2_PARAM_UPDATE : IPC_GPU_TNR_PARAM_UPDATE;
    bool ret = mCommon.requestSync(cmd, requestHandle);

    CheckAndLogError(!ret, UNKNOWN_ERROR, "@%s, IPC_GPU_TNR_PARAM_UPDATE requestSync fails",
                     __func__);
    return OK;
}

int IntelC4mTNR::getTnrBufferSize(int width, int height, uint32_t* size) {
    mTnrRequestInfo->width = width;
    mTnrRequestInfo->height = height;
    mTnrRequestInfo->type = mTnrType;
    mTnrRequestInfo->cameraId = mCameraId;

    int32_t requestHandle =
        mCommon.getShmMemHandle(static_cast<void*>(mTnrRequestInfo), GPU_ALGO_SHM);

    bool ret = mCommon.requestSync(IPC_GPU_TNR_GET_SURFACE_INFO, requestHandle);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "@%s, IPC_GPU_TNR_GET_SURFACE_INFO requestSync fails",
                     __func__);
    if (size) *size = mTnrRequestInfo->surfaceSize;
    return OK;
}
#elif defined(TNR7_LEVEL0)
IntelLevel0TNR::~IntelLevel0TNR() {
    LOG1("<id%d> %s", mCameraId, __func__);
    if (mIntelICBM) {
        icamera::ICBMReqInfo reqInfo;
        reqInfo.cameraId = mCameraId;
        reqInfo.reqType = icamera::ICBMReqType::LEVEL0_TNR;
        mIntelICBM->shutdown(reqInfo);
        mIntelICBM = nullptr;
    }
}

int IntelLevel0TNR::init(int width, int height, TnrType type) {
    LOG1("<id%d> %s  %dx%d", mCameraId, __func__, width, height);
    mWidth = width;
    mHeight = height;
    mIntelICBM = std::make_unique<IntelICBM>();
    icamera::ICBMInitInfo initParam = {.cameraId = mCameraId,
                                       .reqType = icamera::ICBMReqType::LEVEL0_TNR};
    int ret = mIntelICBM->setup(&initParam);
    CheckAndLogError(ret != OK, ret, "%s: Init failed", __func__);

    return ret;
}

int IntelLevel0TNR::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                                uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate, int fd) {
    (void)syncUpdate;
    ImageInfo input = {};
    input.width = mWidth;
    input.height = mHeight;
    input.size = inBufSize;
    input.stride = mWidth;
    input.gfxHandle = mCommon.getShmMemHandle(const_cast<void*>(inBufAddr), GPU_ALGO_SHM);

    ImageInfo output = {};
    output.width = mWidth;
    output.height = mHeight;
    output.size = outBufSize;
    output.stride = mWidth;

    if (fd >= 0) {
        output.gfxHandle = mCommon.registerGbmBuffer(fd, GPU_ALGO_SHM);
    } else {
        output.gfxHandle = mCommon.getShmMemHandle(static_cast<void*>(outBufAddr), GPU_ALGO_SHM);
    }

    ICBMReqInfo reqInfo;
    reqInfo.cameraId = mCameraId;
    reqInfo.reqType = ICBMReqType::LEVEL0_TNR;
    reqInfo.paramHandle = mParamMems.mHandle;
    reqInfo.inII = input;
    reqInfo.outII = output;

    int ret = mIntelICBM->runTnrFrame(reqInfo);

    if (fd >= 0) {
        mCommon.deregisterGbmBuffer(output.gfxHandle, GPU_ALGO_SHM);
    }

    return ret;
}
#endif
}  // namespace icamera
