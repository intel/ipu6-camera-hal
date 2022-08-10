/*
 * Copyright (C) 2019-2021 Intel Corporation.
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

#include "BufferQueue.h"
#include "iutils/Errors.h"

namespace icamera {

/**
 * \interface StreamSource
 * It's an abstract interface for buffer producers, like CaptureUnit, FileSource
 * or external source producer.
 */
class StreamSource : public BufferProducer {
 public:
    StreamSource(int memType) : BufferProducer(memType) {}
    virtual ~StreamSource() {}
    /* Initialize stream source */
    virtual int init() = 0;
    /* Deinitialize stream source */
    virtual void deinit() = 0;
    /* Configure stream source */
    virtual int configure(const std::map<Port, stream_t>& outputFrames,
                          const std::vector<ConfigMode>& configModes) = 0;
    /* Start stream source */
    virtual int start() = 0;
    /* Stop stream source */
    virtual int stop() = 0;
    /* Remove all liateners */
    virtual void removeAllFrameAvailableListener() = 0;
};

// DUMMY_SOURCE_S
class DummySource : public StreamSource {
 public:
    DummySource() : StreamSource(V4L2_MEMORY_USERPTR) {}

    int init() { return OK; }
    void deinit() {}
    int configure(const std::map<Port, stream_t>& outputFrames,
                  const std::vector<ConfigMode>& configModes) {
        return OK;
    }
    int start() { return OK; }
    int stop() { return OK; }
    void removeAllFrameAvailableListener() {}

    int qbuf(Port port, const std::shared_ptr<CameraBuffer>& camBuffer) { return OK; }
    int allocateMemory(Port port, const std::shared_ptr<CameraBuffer>& camBuffer) { return OK; }

    void addFrameAvailableListener(BufferConsumer* listener) {}
    void removeFrameAvailableListener(BufferConsumer* listener) {}
};
// DUMMY_SOURCE_E
}  // namespace icamera
