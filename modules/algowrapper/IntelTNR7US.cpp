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

#define LOG_TAG IntelTNR7US

#include "modules/algowrapper/IntelTNR7US.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#define CM_SURFACE_ALIGN_SIZE 0x1000
#define CM_SURFACE_ALIGN_HEIGHT(val) ALIGN(val, 64)

namespace icamera {

IntelTNR7US* IntelTNR7US::createIntelTNR(int cameraId) {
#ifdef TNR7_CM
    return new IntelC4mTNR(cameraId);
#else
    return nullptr;
#endif
}

Tnr7Param* IntelTNR7US::allocTnr7ParamBuf() {
    LOG1("<id:%d>@%s, type %d", mCameraId, __func__, mTnrType);
    mTnrParam = new Tnr7Param;
    return mTnrParam;
}

#ifdef TNR7_CM
IntelC4mTNR::~IntelC4mTNR() {
    icamera::Thread::requestExit();
    {
        std::lock_guard<std::mutex> l(mLock);
        mThreadRunning = false;
        mRequestCondition.notify_one();
    }

    icamera::Thread::requestExitAndWait();

    for (auto surface : mCMSurfaceMap) {
        destroyCMSurface(surface.second);
    }
    mCMSurfaceMap.clear();
}

int IntelC4mTNR::init(int width, int height, TnrType type) {
    LOG1("<id:%d>@%s size %dx%d, type %d", mCameraId, __func__, width, height, type);
    mWidth = width;
    mHeight = height;
    mTnrType = type;

    std::string threadName = "IntelC4mTNR" + std::to_string(type + (mCameraId << 1));
    run(threadName);
    mThreadRunning = true;

    return OK;
}

void* IntelC4mTNR::allocCamBuf(uint32_t bufSize, int id) {
    LOG1("<%d>@%s, type %d, id: %d", mCameraId, __func__, mTnrType, id);
    void* buffer = nullptr;
    int ret = posix_memalign(&buffer, getpagesize(), bufSize);
    CheckAndLogError(ret != 0, nullptr, "%s, posix_memalign fails, ret:%d", __func__, ret);

    CmSurface2DUP* surface = createCMSurface(buffer);
    if (!surface) {
        ::free(buffer);
        return nullptr;
    }
    mCMSurfaceMap[buffer] = surface;
    return buffer;
}

void IntelC4mTNR::freeAllBufs() {
    LOG1("<%d>@%s, type %d", mCameraId, __func__, mTnrType);
    for (auto surface : mCMSurfaceMap) {
        ::free(surface.first);
    }
    if (mTnrParam) {
        delete mTnrParam;
    }
}

int IntelC4mTNR::prepareSurface(void* bufAddr, int size) {
    CheckAndLogError(size < mWidth * mHeight * 3 / 2, UNKNOWN_ERROR, "%s, invalid buffer size:%d",
                     __func__, size);
    CmSurface2DUP* surface = createCMSurface(bufAddr);
    CheckAndLogError(!surface, UNKNOWN_ERROR, "Failed to create CMSurface");
    mCMSurfaceMap[bufAddr] = surface;

    return OK;
}

int IntelC4mTNR::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                             uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate, int fd) {
    TRACE_LOG_PROCESS("IntelC4mTNR", "runTnrFrame");
    LOG2("<%d>@%s type %d", mCameraId, __func__, mTnrType);
    CheckAndLogError(inBufAddr == nullptr || outBufAddr == nullptr || tnrParam == nullptr,
                     UNKNOWN_ERROR, "@%s, buffer is nullptr", __func__);

    CmSurface2DUP* inSurface = getBufferCMSurface(const_cast<void*>(inBufAddr));
    CheckAndLogError(!inSurface, UNKNOWN_ERROR, "Failed to get CMSurface for input buffer");

    CmSurface2DUP* outSurface = nullptr;
    if (fd >= 0) {
        outSurface = createCMSurface(outBufAddr);
    } else {
        outSurface = getBufferCMSurface(outBufAddr);
    }
    CheckAndLogError(outSurface == nullptr, UNKNOWN_ERROR,
                     "Failed to get CMSurface for output buffer");

    /* call Tnr api to run tnr for the inSurface and store the result in outSurface */
    int ret =
        run_tnr7us_frame(mWidth, mHeight, mWidth, inSurface, outSurface, &tnrParam->scale,
                         &tnrParam->ims, &tnrParam->bc, &tnrParam->blend, syncUpdate, mTnrType);
    if (fd >= 0) {
        destroyCMSurface(outSurface);
    }

    return ret;
}
bool IntelC4mTNR::threadLoop() {
    {
        std::unique_lock<std::mutex> lock(mLock);
        if (!mThreadRunning) return false;
        if (mParamGainRequest.empty()) {
            std::cv_status ret = mRequestCondition.wait_for(
                lock, std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));

            // Already stopped
            if (!mThreadRunning) return false;

            if (ret == std::cv_status::timeout) {
                return true;
            }
        }
    }

    ParamGain param;
    {
        std::lock_guard<std::mutex> l(mLock);
        param = mParamGainRequest.front();
        mParamGainRequest.pop();
    }

    tnr7usParamUpdate(param.gain, param.forceUpdate, mTnrType);

    return true;
}

int IntelC4mTNR::asyncParamUpdate(int gain, bool forceUpdate) {
    std::lock_guard<std::mutex> l(mLock);
    mParamGainRequest.push({gain, forceUpdate});
    mRequestCondition.notify_one();

    return OK;
}

int32_t IntelC4mTNR::getTnrBufferSize(int width, int height, uint32_t* size) {
    uint32_t pitch = 0;
    uint32_t physicalSize = 0;
    int ret = getSurface2DInfo(uint32_t(width), uint32_t(CM_SURFACE_ALIGN_HEIGHT(height)),
                               CM_SURFACE_FORMAT_NV12, pitch, physicalSize);
    CheckAndLogError(ret != 0, ret, "Failed to get surface info");
    if (size) *size = physicalSize;
    LOG1("@%s surface size: %u", __func__, physicalSize);
    return OK;
}

CmSurface2DUP* IntelC4mTNR::getBufferCMSurface(void* bufAddr) {
    if (mCMSurfaceMap.find(bufAddr) != mCMSurfaceMap.end()) {
        return mCMSurfaceMap[bufAddr];
    }

    return nullptr;
}

CmSurface2DUP* IntelC4mTNR::createCMSurface(void* bufAddr) {
    CmSurface2DUP* cmSurface = nullptr;
    int32_t ret = createCmSurface2DUP(mWidth, mHeight, CM_SURFACE_FORMAT_NV12, bufAddr, cmSurface);
    CheckAndLogError(ret != 0, nullptr, "failed to create CmSurface2DUP object");
    return cmSurface;
}

int32_t IntelC4mTNR::destroyCMSurface(CmSurface2DUP* surface) {
    return destroyCMSurface2DUP(surface);
}
#endif

}  // namespace icamera
