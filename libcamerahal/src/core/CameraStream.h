/*
 * Copyright (C) 2015-2022 Intel Corporation.
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
#include "CameraBuffer.h"
#include "Parameters.h"
#include "iutils/Thread.h"

namespace icamera {

/**
 * CameraStream: The HAL represent of the application stream.
 * CameraStream implement the BufferConsumer interface.
 *
 * CameraStream provide the buffer interface to application.
 * It gets buffers from producers and returns to the app
 *
 * Application used the DQ buffer to get the buffers from Camera
 * and Q buffer to return the buffers to camera.
 */

class CameraStream : public BufferConsumer, public EventSource {
 public:
    CameraStream(int cameraId, int streamId, const stream_t& stream);
    virtual ~CameraStream();

    /**
     * \brief Set which port this stream is linked to.
     */
    void setPort(Port port) { mPort = port; }

    /**
     * \brief get the port which the stream is linked to.
     */
    Port getPort() { return mPort; }

    /**
     * \brief Set the StreamActive state
     */
    int start();

    /**
     * \brief Clear streamActive state and clear up
     * the buffer queue
     */
    int stop();

    /**
     * \brief Push one CameraBuffer to bufferProducer
     */
    int qbuf(camera_buffer_t* ubuffer, int64_t sequence);

    /**
     * \brief Calling mBufferProducer to allocate memory
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int allocateMemory(camera_buffer_t* buffer);

    std::shared_ptr<CameraBuffer> userBufferToCameraBuffer(camera_buffer_t* ubuffer);

    /**
     * \brief Register the mBufferProducer
     */
    virtual void setBufferProducer(BufferProducer* producer);

    /**
     * \brief The notify when polled or processed one frame buffer
     */
    virtual int onFrameAvailable(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);

 private:
    int mCameraId;
    int mStreamId;
    Port mPort;
    BufferProducer* mBufferProducer;

    // Guard for member mUserBuffersPool and mBufferInProcessing
    Mutex mBufferPoolLock;
    CameraBufVector mUserBuffersPool;
    // How many user buffers are currently processing underhood.
    int mBufferInProcessing;
};

}  // namespace icamera
