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

#define LOG_TAG "BufferQueue"

#include "iutils/CameraLog.h"

#include "BufferQueue.h"
#include "PlatformData.h"

namespace icamera {

BufferProducer::BufferProducer(int memType) : mMemType(memType)
{
    LOG1("@%s BufferProducer created mMemType: %d", __func__, mMemType);
}

BufferQueue::BufferQueue() : mBufferProducer(nullptr),
                             mProcessThread(nullptr),
                             mThreadRunning(false)
{
    LOG1("@%s BufferQueue created", __func__);
}

BufferQueue::~BufferQueue()
{
    LOG1("@%s BufferQueue destroyed", __func__);
}

int BufferQueue::queueInputBuffer(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
    // If it's not in mInputQueue, then it's not for this processor.
    if (mInputQueue.find(port) == mInputQueue.end()) {
        return OK;
    }

    LOG2("%s CameraBuffer %p for port:%d", __func__, camBuffer.get(), port);

    CameraBufQ &input = mInputQueue[port];
    bool needSignal = input.empty();
    input.push(camBuffer);
    if (needSignal) {
        mFrameAvailableSignal.signal();
    }

    LOG2("%s Exit", __func__);
    return OK;
}

int BufferQueue::onFrameAvailable(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
    AutoMutex l(mBufferQueueLock);

    return queueInputBuffer(port, camBuffer);
}

void BufferQueue::setBufferProducer(BufferProducer *producer)
{
    LOG1("%s producer %p", __func__, producer);

    AutoMutex l(mBufferQueueLock);
    mBufferProducer = producer;

    if (producer == nullptr)
        return;

    mBufferProducer->addFrameAvailableListener(this);
}

void BufferQueue::addFrameAvailableListener(BufferConsumer *listener)
{
    LOG1("%s listener %p", __func__, listener);
    AutoMutex   l(mBufferQueueLock);
    bool isAlreadyAdded = false;
    for (auto& consumer : mBufferConsumerList) {
        if (consumer == listener) {
            isAlreadyAdded = true;
            break;
        }
    }

    // If the listener has been already added, then we don't register it again.
    if (isAlreadyAdded) {
        return;
    }
    mBufferConsumerList.push_back(listener);
}

void BufferQueue::removeFrameAvailableListener(BufferConsumer *listener)
{
    LOG1("%s listener %p", __func__, listener);
    AutoMutex   l(mBufferQueueLock);

    for (auto it = mBufferConsumerList.begin(); it != mBufferConsumerList.end(); ++it) {
        if ((*it) == listener) {
            mBufferConsumerList.erase(it);
            break;
        }
    }
}

int BufferQueue::qbuf(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
    LOG2("%s CameraBuffer %p for port:%d", __func__, camBuffer.get(), port);

    //Enqueue buffer to internal pool
    AutoMutex   l(mBufferQueueLock);
    if (camBuffer != nullptr && camBuffer->getStreamType() == CAMERA_STREAM_INPUT) {
        return queueInputBuffer(port, camBuffer);
    }

    if (mOutputQueue.find(port) == mOutputQueue.end()) {
        LOGE("Not supported port:%d", port);
        return BAD_VALUE;
    }

    CameraBufQ &output = mOutputQueue[port];
    bool needSignal = output.empty();
    output.push(camBuffer);
    if (needSignal) {
        mOutputAvailableSignal.signal();
    }

    return OK;
}

void BufferQueue::clearBufferQueues()
{
    AutoMutex l(mBufferQueueLock);

    mInputQueue.clear();
    for (const auto& input : mInputFrameInfo) {
        mInputQueue[input.first] = CameraBufQ();
    }

    mOutputQueue.clear();
    for (const auto& output : mOutputFrameInfo) {
        mOutputQueue[output.first] = CameraBufQ();
    }
}

void BufferQueue::setFrameInfo(const std::map<Port, stream_t>& inputInfo,
                               const std::map<Port, stream_t>& outputInfo)
{
    mInputFrameInfo = inputInfo;
    mOutputFrameInfo = outputInfo;

    clearBufferQueues();
}

void BufferQueue::getFrameInfo(std::map<Port, stream_t>& inputInfo,
                               std::map<Port, stream_t>& outputInfo) const
{
    inputInfo = mInputFrameInfo;
    outputInfo = mOutputFrameInfo;
}

int BufferQueue::waitFreeBuffersInQueue(ConditionLock& lock,
                                        std::map<Port, std::shared_ptr<CameraBuffer> > &cInBuffer,
                                        std::map<Port, std::shared_ptr<CameraBuffer> > &cOutBuffer,
                                        int64_t timeout)
{
    LOG2("@%s start waiting the input and output buffers", __func__);

    if (!mThreadRunning) {
        LOG1("@%s: Processor is not active.", __func__);
        return OK;
    }

    int ret = OK;
    timeout = (timeout ? timeout : kWaitDuration) * SLOWLY_MULTIPLIER;

    for (auto& input: mInputQueue) {
        Port port = input.first;
        CameraBufQ &inputQueue = input.second;
        while (inputQueue.empty()) {
            LOG2("%s: wait input port %d", __func__, port);
            ret = mFrameAvailableSignal.waitRelative(lock, timeout);

            // Thread was stopped during wait
            if (!mThreadRunning) {
                LOG1("@%s: Processor is not active while waiting for input buffers", __func__);
                return OK;
            }

            if (ret == TIMED_OUT) {
                return ret;
            }
        }
        // Wake up from the buffer available
        cInBuffer[port] = inputQueue.front();
    }

    for (auto& output: mOutputQueue) {
        Port port = output.first;
        CameraBufQ &outputQueue = output.second;
        while (outputQueue.empty()) {
            LOG2("%s: wait output port %d", __func__, port);
            ret = mOutputAvailableSignal.waitRelative(lock, timeout);

            // Thread was stopped during wait
            if (!mThreadRunning) {
                LOG1("@%s: Processor is not active while waiting for output buffers.", __func__);
                return OK;
            }

            if (ret == TIMED_OUT) {
                return ret;
            }
        }

        cOutBuffer[port] = outputQueue.front();
    }

    return ret;
}

int BufferQueue::allocProducerBuffers(int camId, int bufNum)
{
    LOG1("%s: buffer queue size %d", __func__, bufNum);

    mInternalBuffers.clear();

    CheckError(!mBufferProducer, BAD_VALUE ,"@%s: Buffer Producer is nullptr", __func__);

    for (const auto& item : mInputFrameInfo) {
        Port port = item.first;
        int srcFmt = item.second.format;
        int srcWidth = item.second.width;
        int srcHeight = item.second.height;

        LOG1("%s fmt:%s (%dx%d)", __func__,
             CameraUtils::format2string(srcFmt).c_str(), srcWidth, srcHeight);

        int32_t size = 0;
        bool isISYSCompression = PlatformData::getISYSCompression(camId);
        if (isISYSCompression)
            size = CameraUtils::getFrameSize(srcFmt, srcWidth, srcHeight, false, true, true);
        else
            size = CameraUtils::getFrameSize(srcFmt, srcWidth, srcHeight);
        int memType = mBufferProducer->getMemoryType();

        for (int i = 0; i < bufNum; i++) {
            std::shared_ptr<CameraBuffer> camBuffer;
            switch (memType) {
            case V4L2_MEMORY_USERPTR:
                camBuffer = CameraBuffer::create(camId, BUFFER_USAGE_PSYS_INPUT, V4L2_MEMORY_USERPTR,
                                                 size, i, srcFmt, srcWidth, srcHeight);
                CheckError(!camBuffer, NO_MEMORY, "Allocate producer userptr buffer failed");
                break;

            case V4L2_MEMORY_MMAP:
                camBuffer = std::make_shared<CameraBuffer>(camId, BUFFER_USAGE_PSYS_INPUT,
                                                      V4L2_MEMORY_MMAP, size, i, srcFmt);
                CheckError(!camBuffer, NO_MEMORY, "Allocate producer mmap buffer failed");
                camBuffer->setUserBufferInfo(srcFmt, srcWidth, srcHeight);
                mBufferProducer->allocateMemory(port, camBuffer);
                break;

            default:
                LOGE("Not supported v4l2 memory type:%d", memType);
                return BAD_VALUE;
            }

            mInternalBuffers[port].push_back(camBuffer);
            mBufferProducer->qbuf(port, camBuffer);
        }
    }

    return OK;
}

} //namespace icamera
