/*
 * Copyright (C) 2015-2020 Intel Corporation.
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
#include "iutils/Thread.h"
#include "iutils/Errors.h"

#include "CameraEvent.h"
#include "CameraBuffer.h"

/**
 * These are the abstract Classes for buffer communication between different class of HAL
 */
namespace icamera {

class BufferProducer;

/**
 * BufferConsumer listens on the onFrameAvailable event from the producer by
 * calling setBufferProducer
 */
class BufferConsumer {
public:
    virtual ~BufferConsumer() {};
    virtual int onFrameAvailable(Port port, const std::shared_ptr<CameraBuffer> &camBuffer) = 0;
    virtual void setBufferProducer(BufferProducer *producer) = 0;
};

/**
 * BufferProcuder get the buffers from consumer by "qbuf".
 * Notfiy the consumer by calling the onFramAvaible interface of consumer.
 * The consumer must be registered by "addFrameAvailableListener" before getting
 * any buffer done notification.
 */
class BufferProducer : public EventSource {
public:
    BufferProducer(int memType = V4L2_MEMORY_USERPTR);
    virtual ~BufferProducer() {};
    virtual int qbuf(Port port, const std::shared_ptr<CameraBuffer> &camBuffer) = 0;
    virtual int allocateMemory(Port port, const std::shared_ptr<CameraBuffer> &camBuffer) = 0;
    virtual void addFrameAvailableListener(BufferConsumer *listener) = 0;
    virtual void removeFrameAvailableListener(BufferConsumer *listener) = 0;
    int getMemoryType(void) const {return mMemType;}

private:
    int mMemType;
};

class BufferQueue: public BufferConsumer, public BufferProducer, public EventListener {
public:
    BufferQueue();
    virtual ~BufferQueue();

    /**
     * \brief the notify when poll one frame buffer
     *
     * Push the CameraBuffer to InputQueue and send a signal if needed
     */
    virtual int onFrameAvailable(Port port, const std::shared_ptr<CameraBuffer> &camBuffer);

    /**
     * \brief Register the BufferProducer
     *
     * Register the BufferProducer: Psys, software, or captureUnit
     */
    virtual void setBufferProducer(BufferProducer *producer);

    /**
     * \brief Queue one buffer to producer
     *
     * Push this buffer to output queue
     */
    virtual int qbuf(Port port, const std::shared_ptr<CameraBuffer> &camBuffer);

    /**
     * \brief allocate memory
     *
     * Not support this function in Psys and SWProcessor
     */
    virtual int allocateMemory(Port port,
                               const std::shared_ptr<CameraBuffer> &camBuffer) { return -1; }

    /**
     * \brief Add the get frame listener
     */
    virtual void addFrameAvailableListener(BufferConsumer *listener);

    /**
     * \brief Remove the get frame listener
     */
    virtual void removeFrameAvailableListener(BufferConsumer *listener);

    /**
     * \brief Set all frames configuration
     *
     * Must be called before configure which needs use frame configuration.
     */
    virtual void setFrameInfo(const std::map<Port, stream_t>& inputInfo,
                              const std::map<Port, stream_t>& outputInfo);

    /*
     * \brief Get all frames configuration
     */
    virtual void getFrameInfo(std::map<Port, stream_t>& inputInfo,
                              std::map<Port, stream_t>& outputInfo) const;

    /**
     * \brief Register user buffers to processor(PSys)
     */
    virtual int registerUserOutputBufs(Port port,
            const std::shared_ptr<CameraBuffer> &camBuffer) { return OK; }

    /**
     * \brief Common Interface
     */
    virtual int start() = 0;
    virtual void stop() = 0;
    virtual int setParameters(const Parameters& param) { return OK; }
    virtual int getParameters(Parameters& param) { return OK; }
    virtual int configure(const std::vector<ConfigMode>& configModes) { return OK; }

protected:
    virtual int processNewFrame() = 0;

    /**
     * \brief Clear and initialize input and output buffer queues.
     */
    void clearBufferQueues();
    /**
     * \brief Wait for available input and output buffers.
     *
     * Only fetch buffer from the buffer queue, need pop buffer from
     * the queue after the buffer is used, and need to be protected by mBufferQueueLock.
     */
    int waitFreeBuffersInQueue(ConditionLock& lock,
                               std::map<Port, std::shared_ptr<CameraBuffer> > &cInBuffer,
                               std::map<Port, std::shared_ptr<CameraBuffer> > &cOutBuffer,
                               int64_t timeout = 0);
    /**
     * \brief Buffers allocation for producer
     */
    int allocProducerBuffers(int camId, int bufNum);

protected:
    /**
     * \brief The process new frame buffer thread
     *
     * Use this thread listen to the input queue and output queue.
     * And do process if these two queues are not empty
     */
    class ProcessThread: public Thread {
        BufferQueue *mProcessor;
        public:
            ProcessThread(BufferQueue *p)
                : mProcessor(p) { }

            virtual bool threadLoop() {
                int ret = mProcessor->processNewFrame();
                return (ret == 0);
            }
    };
    static const nsecs_t kWaitDuration = 10000000000; //10000ms

    BufferProducer *mBufferProducer;
    std::vector<BufferConsumer*> mBufferConsumerList;

    std::map<Port, stream_t> mInputFrameInfo;
    std::map<Port, stream_t> mOutputFrameInfo;

    std::map<Port, CameraBufQ> mInputQueue;
    std::map<Port, CameraBufQ> mOutputQueue;

    // For internal buffers allocation for producer
    std::map<Port, CameraBufVector> mInternalBuffers;

    // Guard for BufferQueue public API
    Mutex  mBufferQueueLock;
    Condition mFrameAvailableSignal;
    Condition mOutputAvailableSignal;

    //for the thread loop
    ProcessThread* mProcessThread;
    bool mThreadRunning;   //state of the processor. true after start and false after stop

private:
    int queueInputBuffer(Port port, const std::shared_ptr<CameraBuffer> &camBuffer);

};

} //namespace icamera
