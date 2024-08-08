/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#define LOG_TAG CameraStream

#include "CameraStream.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

using std::shared_ptr;

namespace icamera {

CameraStream::CameraStream(int cameraId, int streamId, const stream_t& stream)
        : mCameraId(cameraId),
          mStreamId(streamId),
          mPort(MAIN_PORT),
          mBufferProducer(nullptr),
          mBufferInProcessing(0) {
    LOG2("<id%d>@%s: automation checkpoint: WHF: %d,%d,%s", mCameraId, __func__, stream.width,
         CameraUtils::getInterlaceHeight(stream.field, stream.height),
         CameraUtils::pixelCode2String(stream.format));
}

CameraStream::~CameraStream() {}

int CameraStream::start() {
    LOG1("<id%d>@%s, %p", mCameraId, __func__, this);

    return OK;
}

int CameraStream::stop() {
    LOG1("<id%d>@%s, %p", mCameraId, __func__, this);

    mBufferInProcessing = 0;
    if (mBufferProducer != nullptr) mBufferProducer->removeFrameAvailableListener(this);

    AutoMutex poolLock(mBufferPoolLock);
    mUserBuffersPool.clear();

    return OK;
}

/*
 * Allocate memory to the stream processor which should be
 * set by the CameraDevice
 */
int CameraStream::allocateMemory(camera_buffer_t* ubuffer) {
    LOG1("<id%d>@%s, ubuffer %p", mCameraId, __func__, ubuffer);

    int ret = BAD_VALUE;
    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    CheckAndLogError(!camBuffer, ret, "@%s: fail to alloc CameraBuffer", __func__);

    // mBufferProducer will not change after start
    if (mBufferProducer) ret = mBufferProducer->allocateMemory(mPort, camBuffer);

    return ret;
}

shared_ptr<CameraBuffer> CameraStream::userBufferToCameraBuffer(camera_buffer_t* ubuffer) {
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

    if (!camBuffer) {  // Not found in the pool, so create a new CameraBuffer for it.
        v4l2_buf_type v4l2BufType = PlatformData::getV4L2BufType(mCameraId);
        ubuffer->index = mUserBuffersPool.size();
        camBuffer =
            std::make_shared<CameraBuffer>(mCameraId, BUFFER_USAGE_GENERAL, ubuffer->s.memType,
                                           ubuffer->s.size, ubuffer->index, ubuffer->s.format,
                                           v4l2BufType);
        CheckAndLogError(!camBuffer, nullptr, "@%s: fail to alloc CameraBuffer", __func__);
        mUserBuffersPool.push_back(camBuffer);
    }
    camBuffer->setUserBufferInfo(ubuffer);

    // Update the v4l2 flags
    camBuffer->updateFlags();

    return camBuffer;
}

// Q buffers to the stream processor which should be set by the CameraDevice
int CameraStream::qbuf(camera_buffer_t* ubuffer, int64_t sequence) {
    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    if (camBuffer) {
        camBuffer->setSettingSequence(sequence);
        LOG2("<id%d>@%s, mStreamId:%d, CameraBuffer:%p for port:%d, ubuffer:%p, addr:%p", mCameraId,
             __func__, mStreamId, camBuffer.get(), mPort, ubuffer, ubuffer->addr);
    }

    int ret = BAD_VALUE;
    // mBufferProducer will not change after start, no lock
    if (mBufferProducer != nullptr) {
        ret = mBufferProducer->qbuf(mPort, camBuffer);
        if (ret == OK) {
            mBufferInProcessing++;
        }
    }
    return ret;
}

// This function is called in stop status, no lock
void CameraStream::setBufferProducer(BufferProducer* producer) {
    mBufferProducer = producer;

    if (producer != nullptr) producer->addFrameAvailableListener(this);
}

// PRIVACY_MODE_S
shared_ptr<CameraBuffer> CameraStream::getPrivacyBuffer() {
    AutoMutex l(mBufferPoolLock);
    shared_ptr<CameraBuffer> buf = nullptr;
    if (!mPrivacyBuffer.empty()) {
        buf = mPrivacyBuffer.front();
        mPrivacyBuffer.pop();
    }
    return buf;
}
// PRIVACY_MODE_E

int CameraStream::onFrameAvailable(Port port, const shared_ptr<CameraBuffer>& camBuffer) {
    // Ignore if the buffer is not for this stream.
    if (mPort != port) return OK;
    if (camBuffer->getStreamId() != mStreamId) return OK;

    LOG2("<id%d>@%s: mStreamId:%d, CameraBuffer:%p for port:%d", mCameraId, __func__, mStreamId,
         camBuffer.get(), port);

    std::shared_ptr<CameraBuffer> buf = camBuffer;
    // PRIVACY_MODE_S
    if (PlatformData::getSupportPrivacy(mCameraId) != NO_PRIVACY_MODE) {
        AutoMutex l(mBufferPoolLock);
        mPrivacyBuffer.push(camBuffer);
        if (mPrivacyBuffer.size() <= 1) return OK;

        buf = mPrivacyBuffer.front();
        mPrivacyBuffer.pop();
    }
    // PRIVACY_MODE_E
    return doFrameAvailable(port, buf);
}

int CameraStream::doFrameAvailable(Port port, const shared_ptr<CameraBuffer>& camBuffer) {
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
    LOG2("ubuffer:%p, addr:%p, timestamp:%lu, sequence:%ld", ubuffer, ubuffer->addr,
         ubuffer->timestamp, ubuffer->sequence);

    PERF_CAMERA_ATRACE_PARAM3("sequence", camBuffer->getSequence(), "csi2_port",
                              camBuffer->getCsi2Port(), "virtual_channel",
                              camBuffer->getVirtualChannel());

    AutoMutex l(mBufferPoolLock);
    if (mBufferInProcessing > 0) {
        mBufferInProcessing--;
    }
    LOG2("%s, buffer in processing: %d for stream: %p", __func__, mBufferInProcessing, this);

    return OK;
}
}  // namespace icamera
