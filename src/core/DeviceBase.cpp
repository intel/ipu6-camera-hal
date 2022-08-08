/*
 * Copyright (C) 2018-2022 Intel Corporation.
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

#include <string>
#include <vector>

#define LOG_TAG DeviceBase

#include "DeviceBase.h"

#include "CameraEventType.h"
#include "PlatformData.h"
// FRAME_SYNC_S
#include "SyncManager.h"
// FRAME_SYNC_E
#include "V4l2DeviceFactory.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "linux/ipu-isys.h"

using std::shared_ptr;

namespace icamera {

DeviceBase::DeviceBase(int cameraId, VideoNodeType nodeType, VideoNodeDirection nodeDirection,
                       DeviceCallback* deviceCB)
        : mCameraId(cameraId),
          mPort(INVALID_PORT),
          mNodeType(nodeType),
          mNodeDirection(nodeDirection),
          mName(GetNodeName(nodeType)),
          mDevice(nullptr),
          mLatestSequence(-1),
          mNeedSkipFrame(false),
          mDeviceCB(deviceCB),
          mMaxBufferNumber(MAX_BUFFER_COUNT),
          mBufferQueuing(false) {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);
    std::string devName;
    int ret = PlatformData::getDevNameByType(cameraId, nodeType, devName);
    CheckAndLogError(ret != OK, VOID_VALUE,
                     "Failed to get video device name for cameraId: %d, node type: %d", cameraId,
                     nodeType);

    mDevice = new V4L2VideoNode(devName);
}

DeviceBase::~DeviceBase() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);
    delete mDevice;
}

int DeviceBase::openDevice() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    // FRAME_SYNC_S
    if (PlatformData::isEnableFrameSyncCheck(mCameraId))
        SyncManager::getInstance()->updateSyncCamNum();
    // FRAME_SYNC_E

    return mDevice->Open(O_RDWR);
}

void DeviceBase::closeDevice() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    // Release V4L2 buffers
    mDevice->Stop(true);
    {
        AutoMutex l(mBufferLock);
        mPendingBuffers.clear();
        mBuffersInDevice.clear();
        mAllocatedBuffers.clear();
    }
    mDevice->Close();
}

int DeviceBase::configure(Port port, const stream_t& config, uint32_t bufferNum) {
    LOG1("<id%d>%s, device:%s, port:%d", mCameraId, __func__, mName, port);

    mPort = port;
    mMaxBufferNumber = bufferNum;

    int ret = createBufferPool(config);
    CheckAndLogError(ret != OK, NO_MEMORY, "Failed to create buffer pool:%d", ret);

    resetBuffers();

    return OK;
}

int DeviceBase::streamOn() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);

    return mDevice->Start();
}

int DeviceBase::streamOff() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mDevice->Stop(false);

    return OK;
}

int DeviceBase::queueBuffer(int64_t sequence) {
    LOG2("<id%d>%s, device:%s", mCameraId, __func__, mName);

    shared_ptr<CameraBuffer> buffer;
    {
        AutoMutex l(mBufferLock);
        if (mBufferQueuing) {
            LOG2("buffer is queuing");
            return OK;
        }

        if (mPendingBuffers.empty()) {
            LOG2("Device:%s has no pending buffer to be queued.", mName);
            return OK;
        }
        buffer = mPendingBuffers.front();
        mBufferQueuing = true;
    }

    int ret = onQueueBuffer(sequence, buffer);
    if (ret == OK) {
        ret = mDevice->PutFrame(&buffer->getV4L2Buffer());

        if (ret >= 0) {
            AutoMutex l(mBufferLock);
            mPendingBuffers.pop_front();
            mBuffersInDevice.push_back(buffer);
        } else {
            LOGE("%s, index:%u size:%u, memory:%u, used:%u", __func__, buffer->getIndex(),
                 buffer->getBufferSize(), buffer->getMemory(), buffer->getBytesused());
        }
    } else {
        LOGE("Device:%s failed to preprocess the buffer with ret=%d", mName, ret);
    }

    {
        AutoMutex l(mBufferLock);
        mBufferQueuing = false;
    }

    return ret;
}

int DeviceBase::dequeueBuffer() {
    LOG2("<id%d>%s, device:%s", mCameraId, __func__, mName);

    shared_ptr<CameraBuffer> camBuffer = getFirstDeviceBuffer();
    CheckAndLogError(!camBuffer, UNKNOWN_ERROR, "No buffer in device:%s.", mName);

    int ret = OK;
    int targetIndex = camBuffer->getIndex();

    V4L2Buffer& vbuf = camBuffer->getV4L2Buffer();
    int actualIndex = mDevice->GrabFrame(&vbuf);

    CheckAndLogError(actualIndex < 0, BAD_VALUE, "Device grabFrame failed:%d", actualIndex);
    if (actualIndex != targetIndex) {
        LOGE("%s, CamBuf index isn't same with index used by kernel", __func__);
        ret = BAD_VALUE;
    }

    mNeedSkipFrame = needQueueBack(camBuffer);
    popBufferFromDevice();

    PERF_CAMERA_ATRACE_PARAM3("grabFrame SeqID", camBuffer->getSequence(), "csi2_port",
                              camBuffer->getCsi2Port(), "virtual_channel",
                              camBuffer->getVirtualChannel());

    ret |= onDequeueBuffer(camBuffer);

    // Skip initial frames if needed.
    if (mFrameSkipNum > 0) {
        mFrameSkipNum--;
    }
    return ret;
}

int DeviceBase::getBufferNumInDevice() {
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.size();
}

void DeviceBase::resetBuffers() {
    AutoMutex l(mBufferLock);

    mBuffersInDevice.clear();
    mPendingBuffers.clear();
    for (const auto& buffer : mAllocatedBuffers) {
        mPendingBuffers.push_back(buffer);
    }
}

bool DeviceBase::hasPendingBuffer() {
    AutoMutex l(mBufferLock);

    return !mPendingBuffers.empty();
}

void DeviceBase::addPendingBuffer(const shared_ptr<CameraBuffer>& buffer) {
    AutoMutex l(mBufferLock);

    mPendingBuffers.push_back(buffer);
}

int64_t DeviceBase::getPredictSequence() {
    AutoMutex l(mBufferLock);

    return mLatestSequence + mFrameSkipNum + mBuffersInDevice.size();
}

shared_ptr<CameraBuffer> DeviceBase::getFirstDeviceBuffer() {
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.empty() ? nullptr : mBuffersInDevice.front();
}

// FRAME_SYNC_S
bool DeviceBase::skipFrameAfterSyncCheck(int64_t sequence) {
    // For multi-camera sensor, to check whether the frame synced or not
    int count = 0;
    const int timeoutDuration = gSlowlyRunRatio ? (gSlowlyRunRatio * 1000000) : 1000;
    const int maxCheckTimes = 10;  // 10 times
    while (!SyncManager::getInstance()->isSynced(mCameraId, sequence)) {
        usleep(timeoutDuration);
        count++;
        if (count > maxCheckTimes) {
            return true;
        }
    }
    return false;
}
// FRAME_SYNC_E

void DeviceBase::popBufferFromDevice() {
    AutoMutex l(mBufferLock);
    if (mBuffersInDevice.empty()) {
        return;
    }

    shared_ptr<CameraBuffer> camBuffer = mBuffersInDevice.front();
    mBuffersInDevice.pop_front();
    mLatestSequence = camBuffer->getSequence();

    if (mNeedSkipFrame) {
        mPendingBuffers.push_back(camBuffer);
    }
}

void DeviceBase::dumpFrame(const shared_ptr<CameraBuffer>& buffer) {
    if (!CameraDump::isDumpTypeEnable(DUMP_ISYS_BUFFER)) return;

    LOG2("@%s, ISYS: fmt:%s(%dx%d), stride:%d, len:%d", __func__,
         CameraUtils::format2string(buffer->getFormat()).c_str(), buffer->getWidth(),
         buffer->getHeight(), buffer->getStride(), buffer->getBufferSize());

    CameraDump::dumpImage(mCameraId, buffer, M_ISYS, mPort);
}

MainDevice::MainDevice(int cameraId, VideoNodeType nodeType, DeviceCallback* deviceCB)
        : DeviceBase(cameraId, nodeType, INPUT_VIDEO_NODE, deviceCB) {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);
}

MainDevice::~MainDevice() {}

int MainDevice::createBufferPool(const stream_t& config) {
    LOG1("<id%d>%s, fmt:%s(%dx%d) field:%d", mCameraId, __func__,
         CameraUtils::pixelCode2String(config.format), config.width, config.height, config.field);

    // Pass down ISYS compression flag to driver, which is CSI-BE output compression
    bool isISYSCompression = PlatformData::getISYSCompression(mCameraId);
    if (PlatformData::isCSIBackEndCapture(mCameraId)) {
        std::string csiBEDeviceNodeName;
        int ret = PlatformData::getDevNameByType(mCameraId, VIDEO_GENERIC, csiBEDeviceNodeName);
        CheckAndLogError(ret != OK, ret, "failed to get CSI-BE device node name, ret=%d", ret);
        LOG1("csiBEDeviceNodeName is %s", csiBEDeviceNodeName.c_str());

        V4L2Subdevice* csiBESubDev = V4l2DeviceFactory::getSubDev(mCameraId, csiBEDeviceNodeName);
        ret = csiBESubDev->SetControl(V4L2_CID_IPU_ISYS_COMPRESSION, isISYSCompression);
        LOG2("@%s, set control compression for BE capture, node name: %s, ret:%d", __func__,
             csiBEDeviceNodeName.c_str(), ret);
    }

    bool setWithHeaderCtl = true;
    std::string subDeviceNodeName;

    if (PlatformData::getDevNameByType(mCameraId, VIDEO_ISYS_RECEIVER, subDeviceNodeName) == OK) {
        LOG1("%s: found ISYS receiver subdevice %s", __func__, subDeviceNodeName.c_str());
        if (PlatformData::isTPGReceiver(mCameraId)) {
            LOG1("%s: no need to set csi header ctrl for tpg", __func__);
            setWithHeaderCtl = false;
        }
    } else {
        setWithHeaderCtl = false;
    }

    int withHeader = 1;
    struct v4l2_format v4l2fmt;
    v4l2fmt.fmt.pix_mp.field = config.field;

    if (PlatformData::isCSIFrontEndCapture(mCameraId)) {
        int planesNum = CameraUtils::getNumOfPlanes(config.format);
        LOG1("@%s Num of planes: %d, mCameraId:%d", __func__, planesNum, mCameraId);

        v4l2fmt.fmt.pix_mp.width = config.width;
        v4l2fmt.fmt.pix_mp.height = config.height;
        v4l2fmt.fmt.pix_mp.num_planes = planesNum;
        v4l2fmt.fmt.pix_mp.pixelformat = config.format;
        for (int i = 0; i < v4l2fmt.fmt.pix_mp.num_planes; i++) {
            v4l2fmt.fmt.pix_mp.plane_fmt[i].bytesperline = config.width;
            v4l2fmt.fmt.pix_mp.plane_fmt[i].sizeimage = 0;
        }
        // The frame data is without header(MIPI STORE MODE) when
        // format is YUV/RGB and frame output from CSI-Front-End entity.
        if (!CameraUtils::isRaw(config.format)) {
            LOG2("@%s, set frame without header for format: %s", __func__,
                 CameraUtils::pixelCode2String(config.format));
            withHeader = 0;
        }
    } else {
        v4l2fmt.fmt.pix.width = config.width;
        v4l2fmt.fmt.pix.height = config.height;
        v4l2fmt.fmt.pix.pixelformat = config.format;
        v4l2fmt.fmt.pix.bytesperline = config.width;
        v4l2fmt.fmt.pix.sizeimage = 0;
    }

    if (setWithHeaderCtl) {
        V4L2Subdevice* receiverSubDev = V4l2DeviceFactory::getSubDev(mCameraId, subDeviceNodeName);
        int ret = receiverSubDev->SetControl(V4L2_CID_IPU_STORE_CSI2_HEADER, withHeader);
        CheckAndLogError(ret != OK, ret, "set v4l2 store csi2 header failed, ret=%d", ret);
    }

    v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    V4L2Format tmpbuf{v4l2fmt};
    int ret = mDevice->SetFormat(tmpbuf);
    CheckAndLogError(ret != OK, ret, "set v4l2 format failed ret=%d", ret);
    v4l2fmt = *tmpbuf.Get();

    int realBufferSize = v4l2fmt.fmt.pix.sizeimage;
    int calcBufferSize = 0;
    if (isISYSCompression) {
        calcBufferSize = CameraUtils::getFrameSize(config.format, config.width, config.height,
                                                   false, true, true);
    } else {
        calcBufferSize = CameraUtils::getFrameSize(config.format, config.width, config.height);
    }
    CheckAndLogError(calcBufferSize < realBufferSize, BAD_VALUE,
                     "realBufferSize %d is larger than calcBufferSize %d.", realBufferSize,
                     calcBufferSize);

    LOG2("@%s: compression:%d, realBufSize:%d, calcBufSize:%d", __func__, isISYSCompression,
         realBufferSize, calcBufferSize);

    std::vector<V4L2Buffer> bufs;
    int bufNum = mDevice->SetupBuffers(mMaxBufferNumber, true,
                                       static_cast<enum v4l2_memory>(config.memType), &bufs);
    CheckAndLogError(bufNum < 0, BAD_VALUE, "request buffers failed return=%d", bufNum);

    return OK;
}

int MainDevice::onDequeueBuffer(shared_ptr<CameraBuffer> buffer) {
    mDeviceCB->onDequeueBuffer();

    if (mNeedSkipFrame) return OK;

    LOG2("<seq%ld>@%s, field:%d, timestamp: sec=%ld, usec=%ld", buffer->getSequence(),
         __func__, buffer->getField(), buffer->getTimestamp().tv_sec,
         buffer->getTimestamp().tv_usec);

    for (auto& consumer : mConsumers) {
        consumer->onFrameAvailable(mPort, buffer);
    }

    EventData frameData;
    frameData.type = EVENT_ISYS_FRAME;
    frameData.buffer = nullptr;
    frameData.data.frame.sequence = buffer->getSequence();
    frameData.data.frame.timestamp.tv_sec = buffer->getTimestamp().tv_sec;
    frameData.data.frame.timestamp.tv_usec = buffer->getTimestamp().tv_usec;
    notifyListeners(frameData);

    dumpFrame(buffer);

    return OK;
}

bool MainDevice::needQueueBack(shared_ptr<CameraBuffer> buffer) {
    bool needSkipFrame = (mFrameSkipNum > 0);

    const V4L2Buffer& vbuf = buffer->getV4L2Buffer();
    // Check for STR2MMIO Error from kernel space
    if ((vbuf.Flags() & V4L2_BUF_FLAG_ERROR) && PlatformData::isSkipFrameOnSTR2MMIOErr(mCameraId)) {
        // On STR2MMIO error, enqueue this buffer back to V4L2 before notifying the
        // listener/consumer and return
        needSkipFrame = true;
        LOGW("<seq%ld>%s: buffer error", buffer->getSequence(), __func__);
    }
    // FRAME_SYNC_S
    if (PlatformData::isEnableFrameSyncCheck(mCameraId)) {
        struct camera_buf_info sharedCamBufInfo;
        sharedCamBufInfo.sequence = buffer->getSequence();
        sharedCamBufInfo.sof_ts = buffer->getTimestamp();
        SyncManager::getInstance()->updateCameraBufInfo(mCameraId, &sharedCamBufInfo);
        if (skipFrameAfterSyncCheck(buffer->getSequence())) {
            LOG1("<id%d:seq%ld>@%s: dropped due to frame not sync", mCameraId,
                 buffer->getSequence(), __func__);
            needSkipFrame = true;
        }
    }
    // FRAME_SYNC_E
    return needSkipFrame;
}

// DOL_FEATURE_S
DolCaptureDevice::DolCaptureDevice(int cameraId, VideoNodeType nodeType)
        : DeviceBase(cameraId, nodeType, INPUT_VIDEO_NODE) {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);
}

DolCaptureDevice::~DolCaptureDevice() {}

int DolCaptureDevice::createBufferPool(const stream_t& config) {
    LOG1("<id%d>%s, fmt:%s(%dx%d) field:%d", mCameraId, __func__,
         CameraUtils::pixelCode2String(config.format), config.width, config.height, config.field);

    CheckAndLogError(mPort == INVALID_PORT, NO_MEMORY, "@%s: consumer does not provide DOL buffers",
                     __func__);

    struct v4l2_format v4l2fmt;
    v4l2fmt.fmt.pix.width = config.width;
    v4l2fmt.fmt.pix.height = config.height;
    v4l2fmt.fmt.pix.pixelformat = config.format;
    v4l2fmt.fmt.pix.bytesperline = config.width;
    v4l2fmt.fmt.pix.sizeimage = 0;
    v4l2fmt.fmt.pix_mp.field = 0;

    v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    V4L2Format tmpbuf{v4l2fmt};
    int ret = mDevice->SetFormat(tmpbuf);
    CheckAndLogError(ret != OK, ret, "set DOL v4l2 format failed ret=%d", ret);
    v4l2fmt = *tmpbuf.Get();

    std::vector<V4L2Buffer> bufs;
    int bufNum = mDevice->SetupBuffers(mMaxBufferNumber, false,
                                       static_cast<enum v4l2_memory>(config.memType), &bufs);
    CheckAndLogError(bufNum < 0, bufNum, "request DOL buffers failed return=%d", bufNum);

    return OK;
}

int DolCaptureDevice::onDequeueBuffer(shared_ptr<CameraBuffer> buffer) {
    if (mNeedSkipFrame) {
        return OK;  // Do nothing if the buffer needs to be skipped.
    }

    for (auto& consumer : mConsumers) {
        consumer->onFrameAvailable(mPort, buffer);
    }

    dumpFrame(buffer);
    return OK;
}

bool DolCaptureDevice::needQueueBack(shared_ptr<CameraBuffer> buffer) {
    /**
     * needNotifyBufferDone is used to check if the buffer needs to be returned to its consumer.
     * It is only true when: 1. ISA raw buffers are queued from its consumer;
     *                       2. there is no frame needs to be skipped;
     *                       3. there is no STR2MMIOErr happened.
     * needNotifyBufferDone is false means the buffer needs to be queued back immediately.
     */
    bool needNotifyBufferDone = (mPort != INVALID_PORT);
    if ((buffer->getV4L2Buffer().Flags() & V4L2_BUF_FLAG_ERROR) &&
        PlatformData::isSkipFrameOnSTR2MMIOErr(mCameraId)) {
        needNotifyBufferDone = false;
    } else if (mFrameSkipNum > 0) {
        needNotifyBufferDone = false;
    }

    return !needNotifyBufferDone;
}
// DOL_FEATURE_E
}  // namespace icamera
