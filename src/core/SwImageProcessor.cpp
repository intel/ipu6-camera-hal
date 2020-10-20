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

#define LOG_TAG "SwImageProcessor"

#include "iutils/Utils.h"
#include "iutils/SwImageConverter.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"

#include "PlatformData.h"
#include "CameraBuffer.h"

#include "SwImageProcessor.h"

namespace icamera {

SwImageProcessor::SwImageProcessor(int cameraId) : mCameraId(cameraId)
{
    LOGW("@%s: You are running the SwProcessor instead of PSYS pipeline. SwProcessor is for debug only", __func__);
    LOG1("@%s camera id:%d", __func__, mCameraId);

    mProcessThread = new ProcessThread(this);
}

SwImageProcessor::~SwImageProcessor()
{
    LOGW("@%s: You are running the SwProcessor instead of PSYS pipeline. SwProcessor is for debug only", __func__);

    mProcessThread->join();
    delete mProcessThread;
}

int SwImageProcessor::start()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s", __func__);
    AutoMutex   l(mBufferQueueLock);

    int memType = mOutputFrameInfo.begin()->second.memType;
    CheckError(memType == V4L2_MEMORY_DMABUF, BAD_VALUE,
          "@%s: DMABUF is not supported in SwProcessor as output", __func__);

    int ret = allocProducerBuffers(mCameraId, MAX_BUFFER_COUNT);
    CheckError(ret != OK, ret, "@%s: Allocate Buffer failed", __func__);
    mThreadRunning = true;
    mProcessThread->run("SwImageProcessor", PRIORITY_NORMAL);

    return 0;
}

void SwImageProcessor::stop()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s", __func__);

    mProcessThread->requestExit();
    {
        AutoMutex l(mBufferQueueLock);
        mThreadRunning = false;
        //Wakeup the thread to exit
        mFrameAvailableSignal.signal();
        mOutputAvailableSignal.signal();
    }

    mProcessThread->requestExitAndWait();

    // Thread is not running. It is safe to clear the Queue
    clearBufferQueues();
}

int SwImageProcessor::processNewFrame()
{
    PERF_CAMERA_ATRACE();
    LOG2("%s", __func__);

    int ret = OK;
    std::map<Port, std::shared_ptr<CameraBuffer> > srcBuffers, dstBuffers;
    std::shared_ptr<CameraBuffer> cInBuffer;
    Port inputPort = INVALID_PORT;

    { // Auto lock mBufferQueueLock scope
    ConditionLock lock(mBufferQueueLock);
    ret = waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers);

    if (!mThreadRunning) return -1;

    CheckError((ret < 0), -1, "@%s: wake up from the wait abnomal such as stop", __func__);

    inputPort = srcBuffers.begin()->first;
    cInBuffer = srcBuffers[inputPort];

    for (auto& output: mOutputQueue) {
        output.second.pop();
    }

    for (auto& input: mInputQueue) {
        input.second.pop();
    }
    } // End of auto lock mBufferQueueLock scope

    CheckError(!cInBuffer, BAD_VALUE, "Invalid input buffer.");

    for (auto& dst : dstBuffers) {
        Port port = dst.first;
        std::shared_ptr<CameraBuffer> cOutBuffer = dst.second;
        // If the output buffer is nullptr, that means user doesn't request that buffer,
        // so it doesn't need to be handled here.
        if (!cOutBuffer) {
            continue;
        }

        //No Lock for this function make sure buffers are not freed before the stop
        ret = SwImageConverter::convertFormat(cInBuffer->getWidth(), cInBuffer->getHeight(),
                (unsigned char *)cInBuffer->getBufferAddr(), cInBuffer->getBufferSize(), cInBuffer->getFormat(),
                (unsigned char *)cOutBuffer->getBufferAddr(), cOutBuffer->getBufferSize(), cOutBuffer->getFormat());
        CheckError((ret < 0), ret, "format convertion failed with %d", ret);

        if (CameraDump::isDumpTypeEnable(DUMP_SW_IMG_PROC_OUTPUT)) {
            CameraDump::dumpImage(mCameraId, cOutBuffer, M_SWIPOP);
        }

        //update the interlaced field, sequence, and timestamp  from the src buf to dst buf
        cOutBuffer->updateV4l2Buffer(*cInBuffer->getV4L2Buffer().Get());

        //Notify listener: No lock here: mBufferConsumerList will not updated in this state
        for (auto &it : mBufferConsumerList) {
            it->onFrameAvailable(port, cOutBuffer);
        }
    }

    {
        PERF_CAMERA_ATRACE_PARAM3("sof.sequence", cInBuffer->getSequence(), "csi2_port", cInBuffer->getCsi2Port(), \
                                    "virtual_channel", cInBuffer->getVirtualChannel());
    }

    // Return the buffers to the producer
    if (mBufferProducer) {
        mBufferProducer->qbuf(inputPort, cInBuffer);
    }

    return OK;
}

} //namespace icamera
