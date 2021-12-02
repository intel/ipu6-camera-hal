/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#include <map>
#include <vector>

#include "CameraBuffer.h"
#include "DeviceBase.h"
#include "IspParamAdaptor.h"
#include "StreamSource.h"
#include "iutils/Thread.h"

namespace icamera {

/**
 * CaptureUnit abstract the ISYS function.
 * It implements the BufferProducer Interface and it is the source of any pipeline
 * It hides the v4l2 and media controller to the upper layer.
 */
class CaptureUnit : public StreamSource, public DeviceCallback {
 public:
    explicit CaptureUnit(int cameraId, int memType = V4L2_MEMORY_MMAP);
    virtual ~CaptureUnit();

 public:
    /**
     * \brief Queue on buffer to driver
     *
     * 1. Get the v4l2 buffer form the CameraBuffer
     * 2. Queue this v4l2 buffer to driver and save it to one queue.
     *
     * \param[in] port: Indicates the camBuffer belongs to which port
     * \param[in] camBuffer: the cameraBuffer queue to driver
     *
     * \return 0 if succeed, other value indicates failed
     */
    virtual int qbuf(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);

    /**
     * \brief allocate memory
     *
     * 1. Get the v4l2 buffer form the CameraBuffer
     * 2. Query the v4l2 buffer to get the offset
     * 3. Calling cameraBuffer class to allocate memory
     *
     * \return OK if succeed, other value indicates failed
     */
    virtual int allocateMemory(Port port, const std::shared_ptr<CameraBuffer>& camBuffer);

    /**
     * \brief Add the frame buffer listener
     *
     * \param listener: the listener need to add
     */
    virtual void addFrameAvailableListener(BufferConsumer* listener);

    /**
     * \brief Remove the frame buffer listener
     *
     * \param listener: the listener need to remove
     */
    virtual void removeFrameAvailableListener(BufferConsumer* listener);

    /**
     * \brief Remove all the listeners
     */
    virtual void removeAllFrameAvailableListener();

    /**
     * \brief CaptureUnit initialze
     */
    virtual int init();

    /**
     * \brief CaptureUnit deinit
     *
     * 1. Destory all the buffer pool
     * 2. Deinit the v4l2 device
     * 3. Destory the poll thread
     */
    virtual void deinit();

    /**
     * \brief CaptureUnit start
     *
     * 1. Stream on
     * 2. Running the pool Thread
     */
    virtual int start();

    /**
     * \brief CaptureUnit stop
     *
     * 1. Stream off
     * 3. Release all the buffer queue
     * 3. Stop the pool thread.
     */
    virtual int stop();

    /**
     * \brief configure the streams
     *
     * 1. Setup and reset the MediaControl links
     * 2. Set format to Capture Device
     *
     * \param outputFrames: The output frames' configuration for ISYS.
     * \param configModes: ConfigMode types
     *
     * \return OK if succeed, other value indicates failed
     */
    virtual int configure(const std::map<Port, stream_t>& outputFrames,
                          const std::vector<ConfigMode>& configModes);

    // Override EventSource API to delegate the listeners to DeviceBase.
    virtual void registerListener(EventType eventType, EventListener* eventListener);
    virtual void removeListener(EventType eventType, EventListener* eventListener);

    // Overwrite DeviceCallback API
    void onDequeueBuffer();

    int createDevices();
    void destroyDevices();
    DeviceBase* findDeviceByPort(Port port);
    Port findDefaultPort(const std::map<Port, stream_t>& frames) const;

    int streamOn();
    void streamOff();

    int poll();

    bool isNewConfiguration(const std::map<Port, stream_t>& outputFrames);

    int processPendingBuffers();
    int queueAllBuffers();

 private:
    /**
     * \brief The pool frame buffer thread
     */
    class PollThread : public Thread {
        CaptureUnit* mCaptureU;

     public:
        explicit PollThread(CaptureUnit* hw) : mCaptureU(hw) {}

        virtual bool threadLoop() { return (mCaptureU->poll() == 0); }
    };

    PollThread* mPollThread;
    int mFlushFd[2];  // Flush file descriptor

    // Guard for mCaptureUnit public API except dqbuf and qbuf
    Mutex mLock;

    int mCameraId;
    int mMaxBuffersInDevice;  // To control the number of buffers enqueued, for per-frame control.

    std::vector<ConfigMode> mConfigModes;
    std::map<Port, stream_t> mOutputFrameInfo;
#ifdef HAS_MULTI_INPUT_DEVICE
    std::vector<DeviceBase*> mDevices;
#else
    DeviceBase* mDevice;
#endif
    uint32_t mMaxBufferNum;

    enum {
        CAPTURE_UNINIT,
        CAPTURE_INIT,
        CAPTURE_CONFIGURE,
        CAPTURE_START,
        CAPTURE_STOP,
    } mState;
    bool mExitPending;

 private:
    DISALLOW_COPY_AND_ASSIGN(CaptureUnit);
};

}  // namespace icamera
