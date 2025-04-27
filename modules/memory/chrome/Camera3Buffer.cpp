/*
 * Copyright (C) 2013-2025 Intel Corporation
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

#define LOG_TAG Camera3Buffer

#include "Camera3Buffer.h"

#include <sync/sync.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <unistd.h>

#include "Camera3Stream.h"
#include "HALv3Utils.h"
#include "iutils/CameraDump.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "HALv3Utils.h"

using namespace icamera;

namespace camera3 {
////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
////////////////////////////////////////////////////////////////////

/**
 * Camera3Buffer
 *
 * Default constructor
 * This constructor is used when we pre-allocate the Camera3Buffer object
 * The initialization will be done as a second stage wit the method
 * init(), where we initialize the wrapper with the gralloc buffer provided by
 * the framework
 */
Camera3Buffer::Camera3Buffer()
        : mFormat(0),
          mInit(false),
          mLocked(false),
          mUsage(0),
          mType(BUF_TYPE_HANDLE),
          mHandlePtr(nullptr),
          mCameraId(0),
          mRegistered(false),
          mGbmBufferManager(nullptr) {
    CLEAR(mHalBuffer);
    mHalBuffer.dmafd = -1;
    LOG2("%s default constructor for buf %p", __func__, this);
}

/**
 * Camera3Buffer
 *
 * Constructor for buffers allocated using MemoryUtils::allocateHeapBuffer
 *
 * \param w [IN] width
 * \param h [IN] height
 * \param s [IN] stride
 * \param v4l2fmt [IN] V4l2 format
 * \param usrPtr [IN] Data pointer
 * \param cameraId [IN] id of camera being used
 * \param dataSizeOverride [IN] buffer size input. Default is 0 and frameSize()
                                is used in that case.
 */
Camera3Buffer::Camera3Buffer(int w, int h, int stride, int v4l2fmt, void* usrPtr, int cameraId,
                             int dataSizeOverride)
        : mFormat(0),
          mInit(false),
          mLocked(true),
          mUsage(0),
          mType(BUF_TYPE_MALLOC),
          mHandlePtr(nullptr),
          mCameraId(cameraId)

{
    LOG2("%s create malloc camera buffer %p", __func__, this);

    CLEAR(mHalBuffer);
    mHalBuffer.s.format = v4l2fmt;
    mHalBuffer.s.width = w;
    mHalBuffer.s.height = h;
    mHalBuffer.s.stride = stride;
    mHalBuffer.s.memType = V4L2_MEMORY_USERPTR;
    mHalBuffer.flags =
        camera_buffer_flags_t::BUFFER_FLAG_SW_WRITE | camera_buffer_flags_t::BUFFER_FLAG_SW_READ;
    mHalBuffer.dmafd = -1;

    if (usrPtr != nullptr) {
        mHalBuffer.addr = usrPtr;
        mInit = true;
        mHalBuffer.s.size = dataSizeOverride ?
                                dataSizeOverride :
                                CameraUtils::getFrameSize(v4l2fmt, w, h, false, false, false);
    } else {
        LOGE("Tried to initialize a buffer with nullptr ptr!!");
    }
}

Camera3Buffer::~Camera3Buffer() {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);

    if (mInit) {
        switch (mType) {
            case BUF_TYPE_MALLOC:
                free(mHalBuffer.addr);
                mHalBuffer.addr = nullptr;
                break;
            case BUF_TYPE_MMAP:
                if (mHalBuffer.addr != nullptr) munmap(mHalBuffer.addr, mHalBuffer.s.size);
                mHalBuffer.addr = nullptr;
                mHalBuffer.s.size = 0;
                close(mHalBuffer.dmafd);
                break;
            case BUF_TYPE_HANDLE:
                // Allocated by the HAL
                if (!(mUserBuffer.stream)) {
                    LOG1("release internal buffer");
                    // For HAL buffer, need to unlock before free it
                    unlock();
                    mGbmBufferManager->Free(mHandle);
                }
                break;
            default:
                break;
        }
    }
    LOG2("%s destroying buf %p", __func__, this);
}

/**
 * init
 *
 * Construct to wrap a camera3_stream_buffer
 *
 * \param aBuffer [IN] camera3_stream_buffer buffer
 */
icamera::status_t Camera3Buffer::init(const camera3_stream_buffer* aBuffer, int cameraId) {
    mType = BUF_TYPE_HANDLE;
#ifdef HAVE_CHROME_OS
    mGbmBufferManager = cros::CameraBufferManager::GetInstance();
#else
    mGbmBufferManager = crosIpu6::CameraBufferManager::GetInstance();
#endif
    mHandle = *aBuffer->buffer;
    mHandlePtr = aBuffer->buffer;
    mHalBuffer.s.width = aBuffer->stream->width;
    mHalBuffer.s.height = aBuffer->stream->height;
    mFormat = aBuffer->stream->format;
    mHalBuffer.s.memType =
        IS_ZSL_USAGE(aBuffer->stream->usage) ? V4L2_MEMORY_USERPTR : V4L2_MEMORY_DMABUF;
    mHalBuffer.s.format = mGbmBufferManager->GetV4L2PixelFormat(mHandle);
    // Use actual width from platform native handle for stride
    mHalBuffer.s.stride = mGbmBufferManager->GetPlaneStride(*aBuffer->buffer, 0);
    mHalBuffer.s.size = CameraUtils::getFrameSize(mHalBuffer.s.format, mHalBuffer.s.width,
                                                  mHalBuffer.s.height, false, false, false);
    mLocked = false;
    mUsage = aBuffer->stream->usage;
    mHalBuffer.flags =
        camera_buffer_flags_t::BUFFER_FLAG_SW_WRITE | camera_buffer_flags_t::BUFFER_FLAG_SW_READ;
    mInit = true;
    mHalBuffer.addr = nullptr;
    mUserBuffer = *aBuffer;
    mUserBuffer.release_fence = -1;
    mCameraId = cameraId;
    LOG2("@%s, mHandle:%p, mFormat:%d, width:%d, height:%d, stride:%d, size: %d", __func__, mHandle,
         mFormat, mHalBuffer.s.width, mHalBuffer.s.height, mHalBuffer.s.stride, mHalBuffer.s.size);

    if (mHandle == nullptr) {
        LOGE("@%s: invalid buffer handle", __func__);
        mUserBuffer.status = CAMERA3_BUFFER_STATUS_ERROR;
        return BAD_VALUE;
    }

    mHalBuffer.dmafd = mHandle->data[0];
    int ret = registerBuffer();
    if (ret) {
        mUserBuffer.status = CAMERA3_BUFFER_STATUS_ERROR;
        return UNKNOWN_ERROR;
    }

    return icamera::OK;
}

icamera::status_t Camera3Buffer::init(const camera3_stream_t* stream, buffer_handle_t handle,
                                      int cameraId) {
    CheckAndLogError(!handle, UNKNOWN_ERROR, "%s, handle is nullptr", __func__);

    mType = BUF_TYPE_HANDLE;
#ifdef HAVE_CHROME_OS
    mGbmBufferManager = cros::CameraBufferManager::GetInstance();
#else
    mGbmBufferManager = crosIpu6::CameraBufferManager::GetInstance();
#endif
    mHandle = handle;
    mHandlePtr = &mHandle;
    mHalBuffer.s.width = stream->width;
    mHalBuffer.s.height = stream->height;
    mFormat = stream->format;
    mHalBuffer.s.memType = V4L2_MEMORY_DMABUF;
    mHalBuffer.s.format = mGbmBufferManager->GetV4L2PixelFormat(handle);
    // Use actual width from platform native handle for stride
    mHalBuffer.s.stride = mGbmBufferManager->GetPlaneStride(handle, 0);
    mHalBuffer.s.size = 0;
    mHalBuffer.flags =
        camera_buffer_flags_t::BUFFER_FLAG_SW_WRITE | camera_buffer_flags_t::BUFFER_FLAG_SW_READ;
    mLocked = false;
    mUsage = (stream->usage | GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK);
    mInit = true;
    mHalBuffer.addr = nullptr;
    mCameraId = cameraId;
    mHalBuffer.dmafd = mHandle->data[0];
    LOG2("@%s, mHandle:%p, mFormat:%d, width:%d, height:%d, stride:%d", __func__, mHandle, mFormat,
         mHalBuffer.s.width, mHalBuffer.s.height, mHalBuffer.s.stride);

    return icamera::OK;
}

icamera::status_t Camera3Buffer::deinit() {
    return deregisterBuffer();
}

icamera::status_t Camera3Buffer::waitOnAcquireFence() {
    const int BUFFER_READY = -1;
    if (mUserBuffer.acquire_fence != BUFFER_READY) {
        LOG2("%s: Fence in HAL is %d", __func__, mUserBuffer.acquire_fence);
        const int WAIT_TIME_OUT_MS = 300;
        int ret = sync_wait(mUserBuffer.acquire_fence, WAIT_TIME_OUT_MS);
        if (ret) {
            mUserBuffer.release_fence = mUserBuffer.acquire_fence;
            mUserBuffer.acquire_fence = -1;
            mUserBuffer.status = CAMERA3_BUFFER_STATUS_ERROR;
            LOGE("Buffer sync_wait fail!");
            return TIMED_OUT;
        } else {
            close(mUserBuffer.acquire_fence);
        }
        mUserBuffer.acquire_fence = BUFFER_READY;
    }

    return icamera::OK;
}

/**
 * getFence
 *
 * return the fecne to request result
 */
icamera::status_t Camera3Buffer::getFence(camera3_stream_buffer* buf) {
    if (!buf) return BAD_VALUE;

    buf->acquire_fence = mUserBuffer.acquire_fence;
    buf->release_fence = mUserBuffer.release_fence;

    return icamera::OK;
}

icamera::status_t Camera3Buffer::registerBuffer() {
    int ret = mGbmBufferManager->Register(mHandle);
    if (ret) {
        LOGE("@%s: call Register fail, mHandle:%p, ret:%d", __func__, mHandle, ret);
        return UNKNOWN_ERROR;
    }

    mRegistered = true;
    return icamera::OK;
}

icamera::status_t Camera3Buffer::deregisterBuffer() {
    if (mRegistered) {
        int ret = mGbmBufferManager->Deregister(mHandle);
        if (ret) {
            LOGE("@%s: call Deregister fail, mHandle:%p, ret:%d", __func__, mHandle, ret);
            return UNKNOWN_ERROR;
        }
        mRegistered = false;
    }

    return icamera::OK;
}

/**
 * lock
 *
 * lock the gralloc buffer with specified flags
 *
 * \param aBuffer [IN] int flags
 */
icamera::status_t Camera3Buffer::lock(int flags) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    mHalBuffer.addr = nullptr;
    mHalBuffer.s.size = 0;
    int ret = 0;
    uint32_t planeNum = mGbmBufferManager->GetNumPlanes(mHandle);
    LOG2("@%s, planeNum:%d, mHandle:%p, mFormat:%d", __func__, planeNum, mHandle, mFormat);

    if (planeNum == 1) {
        void* data = nullptr;
        ret = (mFormat == HAL_PIXEL_FORMAT_BLOB) ?
                  mGbmBufferManager->Lock(mHandle, 0, 0, 0, mHalBuffer.s.stride, 1, &data) :
                  mGbmBufferManager->Lock(mHandle, 0, 0, 0, mHalBuffer.s.width, mHalBuffer.s.height,
                                          &data);
        mHalBuffer.addr = data;
    } else if (planeNum > 1) {
        struct android_ycbcr ycbrData;
        ret = mGbmBufferManager->LockYCbCr(mHandle, 0, 0, 0, mHalBuffer.s.width,
                                           mHalBuffer.s.height, &ycbrData);
        mHalBuffer.addr = ycbrData.y;
    } else {
        LOGE("ERROR @%s: planeNum is 0", __func__);
        return UNKNOWN_ERROR;
    }

    CheckAndLogError(ret, UNKNOWN_ERROR, "@%s: Failed to lock buffer, mHandle:%p planeNum: %d",
                     __func__, mHandle, planeNum);

    for (uint32_t i = 0; i < planeNum; i++) {
        mHalBuffer.s.size += mGbmBufferManager->GetPlaneSize(mHandle, i);
    }

    LOG2("@%s, addr:%p, size:%d", __func__, mHalBuffer.addr, mHalBuffer.s.size);
    CheckAndLogError(!mHalBuffer.s.size, UNKNOWN_ERROR, "ERROR @%s: Failed to GetPlaneSize, it's 0",
                     __func__);

    mLocked = true;

    return icamera::OK;
}

icamera::status_t Camera3Buffer::lock() {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    CheckAndLogError(!mInit, INVALID_OPERATION,
                     "@%s: Error: Cannot lock now this buffer, not initialized", __func__);

    if (mType != BUF_TYPE_HANDLE) {
        mLocked = true;
        return icamera::OK;
    }

    CheckAndLogError(mLocked, INVALID_OPERATION, "@%s: Error: Cannot lock buffer, already locked",
                     __func__);

    int lockMode = mUsage & (GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK |
                             GRALLOC_USAGE_HW_CAMERA_MASK);
    if (!lockMode) {
        LOGW("@%s:trying to lock a buffer with no flags", __func__);
        return INVALID_OPERATION;
    }

    icamera::status_t status = lock(lockMode);
    if (status != icamera::OK) {
        mUserBuffer.status = CAMERA3_BUFFER_STATUS_ERROR;
    }

    return status;
}

icamera::status_t Camera3Buffer::unlock() {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL2);
    if (mLocked && mType != BUF_TYPE_HANDLE) {
        mLocked = false;
        return icamera::OK;
    }

    if (mLocked) {
        LOG2("@%s, mHandle:%p, mFormat:%d", __func__, mHandle, mFormat);
        int ret = mGbmBufferManager->Unlock(mHandle);
        if (ret) {
            LOGE("@%s: call Unlock fail, mHandle:%p, ret:%d", __func__, mHandle, ret);
            return UNKNOWN_ERROR;
        }

        mLocked = false;
        return icamera::OK;
    }
    LOG1("@%s:trying to unlock a buffer that is not locked", __func__);
    return INVALID_OPERATION;
}

void Camera3Buffer::dump() {
    if (mInit) {
        LOG1("Buffer dump: handle %p: locked :%d: dataPtr:%p", static_cast<void*>(&mHandle),
             mLocked, mHalBuffer.addr);
    } else {
        LOG1("Buffer dump: Buffer not initialized");
    }
}

void Camera3Buffer::dumpImage(int frameNumber, const int type, int format) {
    if (!CameraDump::isDumpTypeEnable(type)) {
        return;
    }

    dumpImage(mHalBuffer.addr, frameNumber, mHalBuffer.s.size, mHalBuffer.s.width,
              mHalBuffer.s.height, format);
}

void Camera3Buffer::dumpImage(const void* data, int frameNumber, const int size, int width,
                              int height, int format) const {
#ifdef DUMP_IMAGE
    static unsigned int count = 0;
    count++;

    std::string fileName(CameraDump::getDumpPath());
    fileName += "dump_" + std::to_string(width) + "x" + std::to_string(height) + "_frame#" +
                std::to_string(count) + "_req#" + std::to_string(frameNumber);

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t nowtime = tv.tv_sec;
    struct tm targetTm;
    struct tm* nowtm = localtime_r(&nowtime, &targetTm);
    if (nowtm) {
        fileName += "_time#" + std::to_string(nowtm->tm_mday) + ":" + std::to_string(nowtm->tm_min)
                    + ":" + std::to_string(nowtm->tm_sec) + ":" + std::to_string(tv.tv_usec / 1000);
    }

    fileName += "." + std::string(CameraUtils::format2string(format).c_str());
    LOG2("%s filename is %s", __func__, fileName.data());

    FILE* fp = fopen(fileName.data(), "w+");
    if (fp == nullptr) {
        LOGE("open file failed");
        return;
    }
    LOG1("Begin write image %s", fileName.data());

    if ((fwrite(data, size, 1, fp)) != 1)
        LOGW("Error or short count writing %d bytes to %s", size, fileName.data());
    fclose(fp);
#endif
}

/**
 * Utility methods to allocate Camera3Buffer from HEAP or Gfx memory
 */
namespace MemoryUtils {

/**
 * Allocates the memory needed to store the image described by the parameters
 * passed during construction
 */
std::shared_ptr<Camera3Buffer> allocateHeapBuffer(int w, int h, int stride, int v4l2Fmt,
                                                  int cameraId, int dataSizeOverride) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    void* dataPtr;

    int dataSize = dataSizeOverride ? PAGE_ALIGN(dataSizeOverride) :
                                      CameraUtils::getFrameSize(v4l2Fmt, w, h, false, false, false);
    LOG1("@%s, dataSize:%d", __func__, dataSize);

    int ret = posix_memalign(&dataPtr, sysconf(_SC_PAGESIZE), dataSize);
    if (dataPtr == nullptr || ret != 0) {
        LOGE("Could not allocate heap camera buffer of size %d", dataSize);
        return nullptr;
    }

    return std::shared_ptr<Camera3Buffer>(
        new Camera3Buffer(w, h, stride, v4l2Fmt, dataPtr, cameraId, dataSize));
}

/**
 * Allocates internal GBM buffer
 */
std::shared_ptr<Camera3Buffer> allocateHandleBuffer(int w, int h, int gfxFmt, int usage,
                                                    int cameraId) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
#ifdef HAVE_CHROME_OS
    cros::CameraBufferManager* bufManager = cros::CameraBufferManager::GetInstance();
#else
    crosIpu6::CameraBufferManager* bufManager = crosIpu6::CameraBufferManager::GetInstance();
#endif
    buffer_handle_t handle;
    uint32_t stride = 0;

    int width = w;
    int height = h;
    if (gfxFmt == HAL_PIXEL_FORMAT_BLOB) {
        width = CameraUtils::getFrameSize(
            HalV3Utils::HALFormatToV4l2Format(cameraId, gfxFmt, usage), w, h, false, false, false);
        height = 1;
    }

    LOG1("%s, [wxh] = [%dx%d], format 0x%x, usage 0x%x", __func__, width, height, gfxFmt, usage);
    int ret = bufManager->Allocate(width, height, gfxFmt, usage, &handle, &stride);
    CheckAndLogError(ret != 0, nullptr, "Allocate handle failed! ret:%d", ret);

    std::shared_ptr<Camera3Buffer> buffer(new Camera3Buffer());
    camera3_stream_t stream{};
    stream.width = w;
    stream.height = h;
    stream.format = gfxFmt;
    stream.usage = usage;
    ret = buffer->init(&stream, handle, cameraId);
    if (ret != icamera::OK) {
        // buffer handle will free in Camera3Buffer destructure function
        return nullptr;
    }

    return buffer;
}

}  // namespace MemoryUtils
}  // namespace camera3
