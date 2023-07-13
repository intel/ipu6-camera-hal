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

#pragma once

#include <mutex>
#include <unordered_map>

#include "Camera3Buffer.h"

namespace camera3 {

/**
 * \class Camera3BufferPool
 *
 * This class is used to manage a memory pool based on Camera3Buffer
 * It needs to follow the calling sequence:
 * createBufferPool -> acquireBuffer -> findBuffer -> returnBuffer
 */
class Camera3BufferPool {
 public:
    Camera3BufferPool();
    ~Camera3BufferPool();

    icamera::status_t createBufferPool(int cameraId, uint32_t numBufs,
                                       const icamera::stream_t& stream);
    icamera::status_t createBufferPool(int cameraId, uint32_t numBufs, int width, int height,
                                       int gfxFmt, int usage);

    void destroyBufferPool();
    std::shared_ptr<Camera3Buffer> acquireBuffer();
    void returnBuffer(std::shared_ptr<Camera3Buffer> buffer);
    std::shared_ptr<Camera3Buffer> findBuffer(void* memAddr);

 private:
    std::unordered_map<std::shared_ptr<Camera3Buffer>, bool> mBuffers;
    // first: camera3Buffer, second: true as buffer in used
    std::mutex mLock;  // lock the mBuffers
};
}  // namespace camera3
