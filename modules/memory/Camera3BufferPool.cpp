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

#define LOG_TAG "Camera3BufferPool"

#include "Camera3BufferPool.h"

#include "HALv3Utils.h"
#include "iutils/Utils.h"

namespace camera3 {

Camera3BufferPool::Camera3BufferPool() {
    LOG1("@%s", __func__);
}

Camera3BufferPool::~Camera3BufferPool() {
    LOG1("@%s", __func__);
    destroyBufferPool();
}

// Create the buffer pool with HEAP buffer
icamera::status_t Camera3BufferPool::createBufferPool(int cameraId, uint32_t numBufs,
                                                      const icamera::stream_t& stream) {
    LOG1("@%s number of buffers:%d", __func__, numBufs);
    std::lock_guard<std::mutex> l(mLock);
    mBuffers.clear();

    for (uint32_t i = 0; i < numBufs; i++) {
        std::shared_ptr<Camera3Buffer> buffer = MemoryUtils::allocateHeapBuffer(
            stream.width, stream.height, stream.stride, stream.format, cameraId, stream.size);
        if (!buffer) {
            mBuffers.clear();
            LOGE("failed to alloc %d internal buffers", i);
            return icamera::NO_MEMORY;
        }

        // Initialize the buffer status to free
        mBuffers[buffer] = false;
    }

    return icamera::OK;
}

// Create the buffer pool with GFX handle buffer
icamera::status_t Camera3BufferPool::createBufferPool(int cameraId, uint32_t numBufs, int width,
                                                      int height, int gfxFmt, int usage) {
    LOG1("@%s number of buffers:%d", __func__, numBufs);
    std::lock_guard<std::mutex> l(mLock);
    mBuffers.clear();

    for (uint32_t i = 0; i < numBufs; i++) {
        std::shared_ptr<Camera3Buffer> buffer =
            MemoryUtils::allocateHandleBuffer(width, height, gfxFmt, usage, cameraId);
        if (!buffer || buffer->lock() != icamera::OK) {
            mBuffers.clear();
            LOGE("failed to alloc %d internal buffers", i);
            return icamera::NO_MEMORY;
        }

        // Initialize the buffer status to free
        mBuffers[buffer] = false;
    }

    return icamera::OK;
}

void Camera3BufferPool::destroyBufferPool() {
    LOG1("@%s Internal buffers size:%zu", __func__, mBuffers.size());

    std::lock_guard<std::mutex> l(mLock);
    mBuffers.clear();
}

std::shared_ptr<Camera3Buffer> Camera3BufferPool::acquireBuffer() {
    std::lock_guard<std::mutex> l(mLock);
    for (auto& buf : mBuffers) {
        if (!buf.second) {
            buf.second = true;
            LOG2("%s addr:%p", __func__, buf.first->data());
            return buf.first;
        }
    }

    LOGE("%s all the internal buffers are busy", __func__);
    return nullptr;
}

void Camera3BufferPool::returnBuffer(std::shared_ptr<Camera3Buffer> buffer) {
    std::lock_guard<std::mutex> l(mLock);
    for (auto& buf : mBuffers) {
        if (buf.second && buf.first == buffer) {
            LOG2("%s addr:%p", __func__, buffer->data());
            buf.second = false;
            return;
        }
    }

    LOGE("%s, the internal buffer addr:%p not found", __func__, buffer->data());
}

std::shared_ptr<Camera3Buffer> Camera3BufferPool::findBuffer(void* memAddr) {
    std::lock_guard<std::mutex> l(mLock);
    for (auto& buf : mBuffers) {
        if (buf.second && buf.first->data() == memAddr) {
            LOG2("%s addr:%p", __func__, memAddr);
            return buf.first;
        }
    }

    LOGE("%s, Failed to find the internal buffer addr: %p", __func__, memAddr);
    return nullptr;
}
}  // namespace camera3
