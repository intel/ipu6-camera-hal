/*
 * Copyright (C) 2020-2021 Intel Corporation
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

#include <base/bind.h>
#include <base/threading/thread.h>

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#define CM_SURFACE_ALIGN_SIZE 0x1000

namespace icamera {

IntelTNR7US::IntelTNR7US(int cameraId)
        : mCameraId(cameraId),
          mWidth(0),
          mHeight(0),
          mTnrType(TNR_INSTANCE_MAX),
          mTnrParam(nullptr),
          mParamUpdating(false) {
}

IntelTNR7US::~IntelTNR7US() {
    for (auto surface : mCMSurfaceMap) {
        destroyCMSurface(surface.second);
    }
    mCMSurfaceMap.clear();

    int ret = pthread_cond_destroy(&mUpdateDoneCondition);
    if (ret != 0) {
        LOGE("@%s, call pthread_cond_destroy fails, ret:%d", __func__, ret);
    }

    ret = pthread_mutex_destroy(&mLock);
    CheckAndLogError(ret != 0, VOID_VALUE, "@%s, call pthread_mutex_destroy fails, ret:%d",
                     __func__, ret);
}

int IntelTNR7US::init(int width, int height, TnrType type) {
    LOG1("<id:%d>@%s size %dx%d, type %d", mCameraId, __func__, width, height, type);
    mWidth = width;
    mHeight = height;
    mTnrType = type;

    pthread_condattr_t attr;
    int ret = pthread_condattr_init(&attr);
    CheckAndLogError(ret != 0, UNKNOWN_ERROR, "@%s, call pthread_condattr_init fails, ret:%d",
                     __func__, ret);

    ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (ret != 0) {
        LOGE("@%s, call pthread_condattr_setclock fails, ret:%d", __func__, ret);
        pthread_condattr_destroy(&attr);
        return UNKNOWN_ERROR;
    }

    ret = pthread_cond_init(&mUpdateDoneCondition, &attr);
    if (ret != 0) {
        LOGE("@%s, call pthread_cond_init fails, ret:%d", __func__, ret);
        pthread_condattr_destroy(&attr);
        return UNKNOWN_ERROR;
    }

    pthread_condattr_destroy(&attr);

    ret = pthread_mutex_init(&mLock, NULL);
    CheckAndLogError(ret != 0, UNKNOWN_ERROR, "@%s, call pthread_mutex_init fails, ret:%d",
                     __func__, ret);

    std::string threadName = "IntelTNR7US" + std::to_string(type + (mCameraId << 1));
    mThread = std::unique_ptr<base::Thread>(new base::Thread(threadName));
    mThread->Start();
    return OK;
}

Tnr7Param* IntelTNR7US::allocTnr7ParamBuf() {
    LOG1("<%d>@%s, type %d", mCameraId, __func__, mTnrType);
    mTnrParam = new Tnr7Param;
    return mTnrParam;
}

void* IntelTNR7US::allocCamBuf(uint32_t bufSize, int id) {
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

void IntelTNR7US::freeAllBufs() {
    LOG1("<%d>@%s, type %d", mCameraId, __func__, mTnrType);
    for (auto surface : mCMSurfaceMap) {
        ::free(surface.first);
    }
    if (mTnrParam) {
        delete mTnrParam;
    }
}

int IntelTNR7US::prepareSurface(void* bufAddr, int size) {
    CheckAndLogError(size < mWidth * mHeight * 3 / 2, UNKNOWN_ERROR, "%s, invalid buffer size:%d",
                     __func__, size);
    CmSurface2DUP* surface = createCMSurface(bufAddr);
    CheckAndLogError(!surface, UNKNOWN_ERROR, "Failed to create CMSurface");
    mCMSurfaceMap[bufAddr] = surface;

    return OK;
}

int IntelTNR7US::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                             uint32_t outBufSize, Tnr7Param* tnrParam, bool syncUpdate, int fd) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelTNR7US", "runTnrFrame");
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

    /* the frame N gpu tnr run should wait for parameters of frame N-1 update completed */
    nsecs_t startTime = CameraUtils::systemTime();
    pthread_mutex_lock(&mLock);
    if (mParamUpdating) {
        int ret = 0;
        struct timespec ts = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += kMaxDuration;
        while (mParamUpdating && !ret) {
            ret = pthread_cond_timedwait(&mUpdateDoneCondition, &mLock, &ts);
        }

        if (ret != 0) {
            LOGE("@%s, call pthread_cond_timedwait fail, ret:%d, it takes %" PRId64 "ms", __func__,
                 ret, (CameraUtils::systemTime() - startTime) / 1000000);
            pthread_mutex_unlock(&mLock);
            return UNKNOWN_ERROR;
        }
    }
    pthread_mutex_unlock(&mLock);

    /* call Tnr api to run tnr for the inSurface and store the result in outSurface */
    int ret =
        run_tnr7us_frame(mWidth, mHeight, mWidth, inSurface, outSurface, &tnrParam->scale,
                         &tnrParam->ims, &tnrParam->bc, &tnrParam->blend, syncUpdate, mTnrType);
    if (fd >= 0) {
        destroyCMSurface(outSurface);
    }
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "tnr7us process failed");

    return OK;
}

int IntelTNR7US::asyncParamUpdate(int gain, bool forceUpdate) {
    pthread_mutex_lock(&mLock);
    mParamUpdating = true;
    pthread_mutex_unlock(&mLock);

    if (mThread->task_runner()) {
        mThread->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(&IntelTNR7US::handleParamUpdate, base::Unretained(this), gain, forceUpdate));
    }
    return OK;
}

void IntelTNR7US::handleParamUpdate(int gain, bool forceUpdate) {
    PERF_CAMERA_ATRACE();
    LOG2("@%s gain: %d", __func__, gain);
    // gain value is from AE expore analog_gain * digital_gain

    tnr7usParamUpdate(gain, forceUpdate, mTnrType);
    pthread_mutex_lock(&mLock);
    mParamUpdating = false;
    int ret = pthread_cond_signal(&mUpdateDoneCondition);
    pthread_mutex_unlock(&mLock);

    CheckAndLogError(ret != 0, VOID_VALUE, "@%s, call pthread_cond_signal fails, ret:%d", __func__,
                     ret);
}

CmSurface2DUP* IntelTNR7US::getBufferCMSurface(void* bufAddr) {
    if (mCMSurfaceMap.find(bufAddr) != mCMSurfaceMap.end()) {
        return mCMSurfaceMap[bufAddr];
    }

    return nullptr;
}

CmSurface2DUP* IntelTNR7US::createCMSurface(void* bufAddr) {
    PERF_CAMERA_ATRACE();
    CmSurface2DUP* cmSurface = nullptr;
    int32_t ret = createCmSurface2DUP(mWidth, mHeight, CM_SURFACE_FORMAT_NV12, bufAddr, cmSurface);
    CheckAndLogError(ret != 0, nullptr, "failed to create CmSurface2DUP object");
    return cmSurface;
}

int32_t IntelTNR7US::destroyCMSurface(CmSurface2DUP* surface) {
    return destroyCMSurface2DUP(surface);
}
}  // namespace icamera
