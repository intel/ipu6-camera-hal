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

#define LOG_TAG "CameraStream"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#include "CameraStream.h"
#include "PlatformData.h"

using std::shared_ptr;

namespace icamera {

CameraStream::CameraStream(int cameraId, int streamId, const stream_t& stream)
      : mCameraId(cameraId),
        mStreamId(streamId),
        mPort(MAIN_PORT),
        mBufferProducer(nullptr),
        mNumHoldingUserBuffers(0),
        mIsWaitingBufferReturn(false)
{
    LOG1("@%s: mCameraId:%d, width:%d, height:%d, format:%s", __func__,
          mCameraId, stream.width, CameraUtils::getInterlaceHeight(stream.field, stream.height),
          CameraUtils::pixelCode2String(stream.format));
    LOG2("@%s: automation checkpoint: WHF: %d,%d,%s", __func__,
          stream.width, CameraUtils::getInterlaceHeight(stream.field, stream.height),
          CameraUtils::pixelCode2String(stream.format));
}

CameraStream::~CameraStream()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);
}

int CameraStream::start()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    return OK;
}

int CameraStream::stop()
{
    LOG1("@%s, mCameraId:%d", __func__, mCameraId);

    mIsWaitingBufferReturn = false;
    mNumHoldingUserBuffers = 0;

    if (mBufferProducer != nullptr)
        mBufferProducer->removeFrameAvailableListener(this);

    AutoMutex poolLock(mBufferPoolLock);
    mUserBuffersPool.clear();

    return OK;
}

/*
 * Allocate memory to the stream processor which should be
 * set by the CameraDevice
 */
int CameraStream::allocateMemory(camera_buffer_t *ubuffer)
{
    LOG1("@%s, mCameraId:%d, ubuffer %p", __func__, mCameraId, ubuffer);

    int ret = BAD_VALUE;
    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    CheckError(!camBuffer, ret, "@%s: fail to alloc CameraBuffer", __func__);

    // mBufferProducer will not change after start
    if (mBufferProducer)
        ret = mBufferProducer->allocateMemory(mPort, camBuffer);

    return ret;
}

shared_ptr<CameraBuffer> CameraStream::userBufferToCameraBuffer(camera_buffer_t *ubuffer)
{
    if (ubuffer == nullptr) return nullptr;

    shared_ptr<CameraBuffer> camBuffer = nullptr;

    AutoMutex l(mBufferPoolLock);
    for (auto buffer = mUserBuffersPool.begin(); buffer != mUserBuffersPool.end(); buffer++) {
        /* check if the buffer is already in the pool, when the userBuffer is same as ubuffer
         * will continue to check the buffer, because the data addr in ubuffer may change */
        if ((*buffer)->getUserBuffer() == ubuffer) {
            /* when memType matches, the dmafd or the addr should match */
            if (((*buffer)->getMemory() == static_cast<uint32_t>(ubuffer->s.memType)) &&
                ((ubuffer->addr != nullptr && (*buffer)->getUserBuffer()->addr == ubuffer->addr) ||
                (ubuffer->dmafd >= 0 && (*buffer)->getUserBuffer()->dmafd == ubuffer->dmafd))) {
                camBuffer = *buffer;
            } else {
                mUserBuffersPool.erase(buffer);
            }
            break;
        }
    }

    if (!camBuffer) { // Not found in the pool, so create a new CameraBuffer for it.
        ubuffer->index = mUserBuffersPool.size();
        camBuffer = std::make_shared<CameraBuffer>(mCameraId, BUFFER_USAGE_GENERAL,
                ubuffer->s.memType, ubuffer->s.size, ubuffer->index, ubuffer->s.format);
        CheckError(!camBuffer, nullptr, "@%s: fail to alloc CameraBuffer", __func__);
        mUserBuffersPool.push_back(camBuffer);
    }
    camBuffer->setUserBufferInfo(ubuffer);

    // Update the v4l2 flags
    camBuffer->updateFlags();

    return camBuffer;
}

void CameraStream::waitToReturnAllUserBufffers()
{
    LOG1("%s: wait for all user buffers to be returned to user", __func__);

    ConditionLock lock(mBufferPoolLock);

    if (mNumHoldingUserBuffers > 0){
        // mIsWaitingBufferReturn flag is used to prevent situation that signal goes before wait
        mIsWaitingBufferReturn = true;
        int ret = mAllBuffersReturnedSignal.waitRelative(lock,
                                                         kWaitDuration * SLOWLY_MULTIPLIER);

        if (ret == TIMED_OUT) {
            LOGW("@%s, mCameraId:%d, time out happens when waiting return user buffers",
                 __func__, mCameraId);
            return;
        }
        mIsWaitingBufferReturn = false;
    }

    LOG1("%s: all buffers have been returned to user", __func__);

    return;
}

// Q buffers to the stream processor which should be set by the CameraDevice
int CameraStream::qbuf(camera_buffer_t *ubuffer, long sequence)
{
    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    if (camBuffer) {
        camBuffer->setSettingSequence(sequence);
        LOG2("@%s, mCameraId:%d, mStreamId:%d, CameraBuffer:%p for port:%d, ubuffer:%p, addr:%p",
             __func__, mCameraId, mStreamId, camBuffer.get(), mPort, ubuffer, ubuffer->addr);
    }

    int ret = BAD_VALUE;
    // mBufferProducer will not change after start, no lock
    if (mBufferProducer != nullptr) {
        ret = mBufferProducer->qbuf(mPort, camBuffer);
        if (ret == OK) {
            mNumHoldingUserBuffers++;
        }
    }
    return ret;
}

//This function is called in stop status, no lock
void CameraStream::setBufferProducer(BufferProducer *producer)
{
    LOG1("@%s, mCameraId:%d, producer %p", __func__, mCameraId, producer);

    mBufferProducer = producer;

    if (producer != nullptr)
        producer->addFrameAvailableListener(this);
}

int CameraStream::onFrameAvailable(Port port, const shared_ptr<CameraBuffer> &camBuffer)
{
    // Ignore if the buffer is not for this stream.
    if (mPort != port) return OK;
    if (camBuffer->getStreamId() != mStreamId) return OK;

    LOG2("@%s: mCameraId:%d, mStreamId:%d, CameraBuffer:%p for port:%d",
         __func__, mCameraId, mStreamId, camBuffer.get(), port);

    // Update the user buffer info before return back
    camBuffer->updateUserBuffer();

    EventFrameAvailable frameData;
    frameData.streamId = mStreamId;
    EventData eventData;
    eventData.type = EVENT_FRAME_AVAILABLE;
    eventData.buffer = camBuffer;
    eventData.data.frameDone = frameData;
    notifyListeners(eventData);

    camera_buffer_t* ubuffer = camBuffer->getUserBuffer();
    LOG2("ubuffer:%p, addr:%p, timestamp:%lu, sequence:%ld",
         ubuffer, ubuffer->addr, ubuffer->timestamp, ubuffer->sequence);

    LOGVCSYNC("[onFrameDone], CPU-timestamp:%lu, sequence:%ld, vc:%d, kernel-timestamp:%luus, endl",
        CameraUtils::systemTime(),
        ubuffer->sequence,
        camBuffer->getVirtualChannel(),
        ubuffer->timestamp);

    PERF_CAMERA_ATRACE_PARAM3("sequence", camBuffer->getSequence(),
                              "csi2_port", camBuffer->getCsi2Port(),
                              "virtual_channel", camBuffer->getVirtualChannel());

    AutoMutex l(mBufferPoolLock);

    if (mNumHoldingUserBuffers > 0) {
        mNumHoldingUserBuffers--;
    }

    LOG2("mNumHoldingUserBuffers has already been counted down to %d", mNumHoldingUserBuffers);

    // mIsWaitingBufferReturn is used to prevent signal before wait
    if (mIsWaitingBufferReturn && mNumHoldingUserBuffers == 0) {
        LOG2("%s: all user buffer returned, trigger signal", __func__);
        mAllBuffersReturnedSignal.signal();
    }

    return OK;
}

} //namespace icamera
