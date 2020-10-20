/*
 * Copyright (C) 2018-2020 Intel Corporation.
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

#define LOG_TAG "DeviceBase"

#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Utils.h"
#include "linux/ipu-isys.h"

#include "SyncManager.h"
#include "PlatformData.h"
#include "DeviceBase.h"
#include "CameraEventType.h"
#include "V4l2DeviceFactory.h"

using std::shared_ptr;

namespace icamera {

DeviceBase::DeviceBase(int cameraId, VideoNodeType nodeType, VideoNodeDirection nodeDirection,
                       DeviceCallback* deviceCB) :
        mCameraId(cameraId),
        mPort(INVALID_PORT),
        mNodeType(nodeType),
        mNodeDirection(nodeDirection),
        mName(GetNodeName(nodeType)),
        mDevice(nullptr),
        mLatestSequence(-1),
        mNeedSkipFrame(false),
        mDeviceCB(deviceCB),
        mMaxBufferNumber(MAX_BUFFER_COUNT)
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);

    std::string devName;
    int ret = PlatformData::getDevNameByType(cameraId, nodeType, devName);
    CheckError(ret != OK, VOID_VALUE,
               "Failed to get video device name for cameraId: %d, node type: %d",
               cameraId, nodeType);

    mDevice = new V4L2VideoNode(devName);
}

DeviceBase::~DeviceBase()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    delete mDevice;
}

int DeviceBase::openDevice()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    if (PlatformData::isEnableFrameSyncCheck(mCameraId))
        SyncManager::getInstance()->updateSyncCamNum();

    return mDevice->Open(O_RDWR);
}

void DeviceBase::closeDevice()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);
    {
        AutoMutex l(mBufferLock);

        std::vector<V4L2Buffer> bufs;
        mDevice->SetupBuffers(0, true, mDevice->GetMemoryType(), &bufs);

        mPendingBuffers.clear();
        mBuffersInDevice.clear();
        mAllocatedBuffers.clear();
    }

    mDevice->Close();
}

int DeviceBase::configure(Port port, const stream_t& config, uint32_t bufferNum)
{
    LOG1("%s, camera id:%d device:%s, port:%d", __func__, mCameraId, mName, port);

    mPort = port;
    mMaxBufferNumber = bufferNum;

    int ret = createBufferPool(config);
    CheckError(ret != OK, NO_MEMORY, "Failed to create buffer pool:%d", ret);

    resetBuffers();

    return OK;
}

int DeviceBase::streamOn()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);

    return mDevice->Start();
}

int DeviceBase::streamOff()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    mDevice->Stop(false);

    return OK;
}

int DeviceBase::queueBuffer(long sequence)
{
    LOG2("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    shared_ptr<CameraBuffer> buffer;
    AutoMutex l(mBufferLock);
    if (mPendingBuffers.empty()) {
        LOG2("Device:%s has no pending buffer to be queued.", mName);
        return OK;
    }
    buffer = mPendingBuffers.front();

    int ret = onQueueBuffer(sequence, buffer);
    CheckError(ret != OK, ret, "Device:%s failed to preprocess the buffer with ret=%d", mName, ret);

    ret = mDevice->PutFrame(&buffer->getV4L2Buffer());

    if (ret >= 0) {
        mPendingBuffers.pop_front();
        mBuffersInDevice.push_back(buffer);
    }

    return ret;
}

int DeviceBase::dequeueBuffer()
{
    LOG2("%s, camera id:%d device:%s", __func__, mCameraId, mName);

    shared_ptr<CameraBuffer> camBuffer = getFirstDeviceBuffer();
    CheckError(!camBuffer, UNKNOWN_ERROR, "No buffer in device:%s.", mName);

    int ret = OK;
    int targetIndex = camBuffer->getIndex();

    V4L2Buffer &vbuf = camBuffer->getV4L2Buffer();
    int actualIndex = mDevice->GrabFrame(&vbuf);

    CheckError(actualIndex < 0, BAD_VALUE, "Device grabFrame failed:%d", actualIndex);
    if (actualIndex != targetIndex) {
        LOGE("%s, CamBuf index isn't same with index used by kernel", __func__);
        ret = BAD_VALUE;
    }

    mNeedSkipFrame = needQueueBack(camBuffer);
    popBufferFromDevice();

    // TODO: Will add device name info to distinguish different devices.
    PERF_CAMERA_ATRACE_PARAM3("grabFrame SeqID", camBuffer->getSequence(),
                              "csi2_port",       camBuffer->getCsi2Port(),
                              "virtual_channel", camBuffer->getVirtualChannel());

    ret |= onDequeueBuffer(camBuffer);

    // Skip initial frames if needed.
    if (mFrameSkipNum > 0) {
        mFrameSkipNum--;
    }
    return ret;
}

int DeviceBase::getBufferNumInDevice()
{
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.size();
}

void DeviceBase::resetBuffers()
{
    AutoMutex l(mBufferLock);

    mBuffersInDevice.clear();
    mPendingBuffers.clear();

    for (const auto& buffer : mAllocatedBuffers) {
        mPendingBuffers.push_back(buffer);
    }
}

bool DeviceBase::hasPendingBuffer()
{
    AutoMutex l(mBufferLock);

    return !mPendingBuffers.empty();
}

void DeviceBase::addPendingBuffer(const shared_ptr<CameraBuffer>& buffer)
{
    AutoMutex l(mBufferLock);

    mPendingBuffers.push_back(buffer);
}

long DeviceBase::getPredictSequence()
{
    AutoMutex l(mBufferLock);

    return mLatestSequence + mFrameSkipNum + mBuffersInDevice.size();
}

shared_ptr<CameraBuffer> DeviceBase::getFirstDeviceBuffer()
{
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.empty() ? nullptr : mBuffersInDevice.front();
}

bool DeviceBase::skipFrameAfterSyncCheck(long sequence)
{
    //For multi-camera sensor, to check whether the frame synced or not
    int count = 0;
    const int timeoutDuration = gSlowlyRunRatio ? (gSlowlyRunRatio * 1000000) : 1000;
    const int maxCheckTimes = 10;  //10 times
    while (!SyncManager::getInstance()->isSynced(mCameraId, sequence)) {
        usleep(timeoutDuration);
        count++;
        if (count > maxCheckTimes) {
            return true;
        }
    }
    return false;
}

void DeviceBase::popBufferFromDevice()
{
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

void DeviceBase::dumpFrame(const shared_ptr<CameraBuffer>& buffer)
{
    if (!CameraDump::isDumpTypeEnable(DUMP_ISYS_BUFFER)) return;

    LOGD("@%s, ISYS: fmt:%s(%dx%d), stride:%d, len:%d", __func__,
         CameraUtils::format2string(buffer->getFormat()).c_str(),
         buffer->getWidth(), buffer->getHeight(), buffer->getStride(), buffer->getBufferSize());

    CameraDump::dumpImage(mCameraId, buffer, M_ISYS, mPort);
}

MainDevice::MainDevice(int cameraId, VideoNodeType nodeType, DeviceCallback* deviceCB) :
        DeviceBase(cameraId, nodeType, INPUT_VIDEO_NODE, deviceCB)
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);
}

MainDevice::~MainDevice()
{
    LOG1("%s, camera id:%d device:%s", __func__, mCameraId, mName);
}

int MainDevice::createBufferPool(const stream_t& config)
{
    LOG1("%s, camera id:%d, fmt:%s(%dx%d) field:%d", __func__, mCameraId,
         CameraUtils::pixelCode2String(config.format), config.width, config.height, config.field);

    // Pass down ISYS compression flag to driver, which is CSI-BE output compression
    bool isISYSCompression = PlatformData::getISYSCompression(mCameraId);
    if (PlatformData::isCSIBackEndCapture(mCameraId)) {
        std::string csiBEDeviceNodeName;
        int ret = PlatformData::getDevNameByType(mCameraId, VIDEO_GENERIC, csiBEDeviceNodeName);
        CheckError(ret != OK, ret, "failed to get CSI-BE device node name, ret=%d", ret);
        LOG1("csiBEDeviceNodeName is %s", csiBEDeviceNodeName.c_str());

        V4L2Subdevice* csiBESubDev = V4l2DeviceFactory::getSubDev(mCameraId, csiBEDeviceNodeName);
        ret = csiBESubDev->SetControl(V4L2_CID_IPU_ISYS_COMPRESSION, isISYSCompression);
        LOG2("@%s, set control compression for BE capture, node name: %s, ret:%d",
                __func__, csiBEDeviceNodeName.c_str(), ret);
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
            LOG2("@%s, set frame without header for format: %s",
                    __func__, CameraUtils::pixelCode2String(config.format));
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
        CheckError(ret != OK, ret, "set v4l2 store csi2 header failed, ret=%d", ret);
    }

    v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    V4L2Format tmpbuf {v4l2fmt};
    int ret = mDevice->SetFormat(tmpbuf);
    CheckError(ret != OK, ret, "set v4l2 format failed ret=%d", ret);
    v4l2fmt = *tmpbuf.Get();

    int realBufferSize = v4l2fmt.fmt.pix.sizeimage;
    int calcBufferSize = 0;
    if (isISYSCompression) {
        calcBufferSize = CameraUtils::getFrameSize(config.format, config.width, config.height, false, true, true);
    } else {
        calcBufferSize = CameraUtils::getFrameSize(config.format, config.width, config.height);
    }

    CheckError(calcBufferSize < realBufferSize, BAD_VALUE,
        "realBufferSize %d is larger than calcBufferSize %d.", realBufferSize, calcBufferSize);

    LOG2("@%s: compression:%d, realBufSize:%d, calcBufSize:%d",
                __func__, isISYSCompression, realBufferSize, calcBufferSize);

    std::vector<V4L2Buffer> bufs;
    int bufNum = mDevice->SetupBuffers(mMaxBufferNumber, true,
                                       static_cast<enum v4l2_memory>(config.memType), &bufs);

    CheckError(bufNum < 0, BAD_VALUE, "request buffers failed return=%d", bufNum);

    return OK;
}

int MainDevice::onDequeueBuffer(shared_ptr<CameraBuffer> buffer)
{
    mDeviceCB->onDequeueBuffer();

    if (mNeedSkipFrame) return OK;

    LOG2("@%s, sequence:%ld, field:%d, timestamp: sec=%ld, usec=%ld",
          __func__, buffer->getSequence(), buffer->getField(),
          buffer->getTimestamp().tv_sec, buffer->getTimestamp().tv_usec);

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

bool MainDevice::needQueueBack(shared_ptr<CameraBuffer> buffer)
{
    bool needSkipFrame = (mFrameSkipNum > 0);

    const V4L2Buffer& vbuf = buffer->getV4L2Buffer();
    // Check for STR2MMIO Error from kernel space
    if((vbuf.Flags() & V4L2_BUF_FLAG_ERROR) && PlatformData::isSkipFrameOnSTR2MMIOErr(mCameraId)) {
        // On STR2MMIO error, enqueue this buffer back to V4L2 before notifying the
        // listener/consumer and return
        needSkipFrame = true;
    }
    if (PlatformData::isEnableFrameSyncCheck(mCameraId)) {
        struct camera_buf_info sharedCamBufInfo;
        sharedCamBufInfo.sequence = buffer->getSequence();
        sharedCamBufInfo.sof_ts = buffer->getTimestamp();
        SyncManager::getInstance()->updateCameraBufInfo(mCameraId, &sharedCamBufInfo);
        if (skipFrameAfterSyncCheck(buffer->getSequence())) {
            LOG1("@%s: CameraID:%d sequence %ld been dropped due to frame not sync",
                  __func__, mCameraId, buffer->getSequence());
            needSkipFrame = true;
        }
    }
    return needSkipFrame;
}

} // namespace icamera
