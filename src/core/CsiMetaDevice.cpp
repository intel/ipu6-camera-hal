/*
 * Copyright (C) 2016-2023 Intel Corporation.
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

#define LOG_TAG CsiMetaDevice

#include "CsiMetaDevice.h"

#include <poll.h>

#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "linux/ipu-isys.h"

using std::map;
using std::shared_ptr;
using std::vector;

namespace icamera {

CsiMetaDevice::CsiMetaDevice(int cameraId)
        : mCameraId(cameraId),
          mCsiMetaDevice(nullptr),
          mIsCsiMetaEnabled(false),
          mCsiMetaBufferDQIndex(0),
          mBuffersInCsiMetaDevice(0),
          mState(CSI_META_DEVICE_UNINIT),
          mExitPending(false) {
    mPollThread = new PollThread(this);
    CLEAR(mEmbeddedMetaData);
}

CsiMetaDevice::~CsiMetaDevice() {
    delete mPollThread;
}

int CsiMetaDevice::init() {
    return OK;
}

void CsiMetaDevice::deinit() {
    AutoMutex l(mCsiMetaDeviceLock);
    if (!mIsCsiMetaEnabled) {
        return;
    }
    deinitLocked();
}

void CsiMetaDevice::deinitLocked() {
    LOG1("@%s", __func__);

    mCsiMetaCameraBuffers.clear();
    deinitDev();
    mPollThread->join();
    mState = CSI_META_DEVICE_UNINIT;
}

int CsiMetaDevice::resetState() {
    LOG1("@%s", __func__);

    mExitPending = false;
    mCsiMetaBufferDQIndex = 0;
    mBuffersInCsiMetaDevice = 0;
    mCsiMetaCameraBuffers.reserve(CSI_META_BUFFER_NUM);
    mState = CSI_META_DEVICE_INIT;

    return OK;
}

int CsiMetaDevice::initDev() {
    // close the previous device if any
    deinitDev();

    std::string devName;
    int ret = PlatformData::getDevNameByType(mCameraId, VIDEO_CSI_META, devName);
    CheckAndLogError(ret != OK, BAD_VALUE, "failed to init device node");

    mCsiMetaDevice = new V4L2VideoNode(devName);
    ret = mCsiMetaDevice->Open(O_RDWR);
    if (ret != OK) {
        delete mCsiMetaDevice;
        mCsiMetaDevice = nullptr;

        LOGE("open csi meta dev failed. ret %d", ret);
        return BAD_VALUE;
    }

    mConfiguredDevices.push_back(mCsiMetaDevice);

    return OK;
}

void CsiMetaDevice::deinitDev() {
    mConfiguredDevices.clear();
    if (mCsiMetaDevice) {
        // Release V4L2 buffers
        mCsiMetaDevice->Stop(true);
        mCsiMetaDevice->Close();
        delete mCsiMetaDevice;
        mCsiMetaDevice = nullptr;
    }
}

int CsiMetaDevice::initEmdMetaData() {
    // initialize mEmbeddedMetaData
    std::string videoNodeName;
    int ret = PlatformData::getVideoNodeNameByType(mCameraId, VIDEO_ISYS_RECEIVER, videoNodeName);
    CheckAndLogError(ret != OK, ret, "failed to get device node name");

    McFormat format;

    ret = PlatformData::getFormatByDevName(mCameraId, videoNodeName, format);
    CheckAndLogError(ret != OK, ret, "failed to get format, sub Dev name = %s",
                     videoNodeName.c_str());

    int bpp = CameraUtils::getBpp(format.pixelCode);
    CheckAndLogError(bpp < 0, BAD_VALUE, "failed to get bpp (%d)", bpp);

    int bpl = format.width * bpp / 8;

    mEmbeddedMetaData.csiMetaFormat = V4L2_FMT_IPU_ISYS_META;
    // for embedded meta data, width is equal with bytes per line
    mEmbeddedMetaData.width = bpl;
    // for compatible, set 1 as default, if the *.xml have configed it, use the config value repalce
    mEmbeddedMetaData.height = 1;
    ret = PlatformData::getVideoNodeNameByType(mCameraId, VIDEO_CSI_META, videoNodeName);
    if (ret == OK) {
        ret = PlatformData::getFormatByDevName(mCameraId, videoNodeName, format);
        if (ret == OK) mEmbeddedMetaData.height = format.height;
    }

    mEmbeddedMetaData.bpl = bpl;
    mEmbeddedMetaData.planesNum = CameraUtils::getNumOfPlanes(V4L2_FMT_IPU_ISYS_META);

    return OK;
}

int CsiMetaDevice::configure() {
    LOG1("@%s", __func__);
    AutoMutex l(mCsiMetaDeviceLock);

    CheckAndLogError(mState == CSI_META_DEVICE_START, BAD_VALUE, "@%s: Configure in wrong state %d",
                     __func__, mState);

    deinitLocked();
    if (PlatformData::isCsiMetaEnabled(mCameraId)) mIsCsiMetaEnabled = true;

    int ret = resetState();
    CheckAndLogError(ret != OK, ret, "@%s: init csi meta device failed", __func__);

    if (!mIsCsiMetaEnabled) {
        return OK;
    }

    ret = initDev();
    CheckAndLogError(ret != 0, BAD_VALUE, "init CSI meta device failed. ret %d", ret);

    ret = initEmdMetaData();
    CheckAndLogError(ret != 0, BAD_VALUE, "init embedded metadata failed. ret %d", ret);

    ret = setFormat();
    CheckAndLogError(ret != 0, BAD_VALUE, "set format failed, ret = %d", ret);

    ret = allocCsiMetaBuffers();
    CheckAndLogError(ret != 0, BAD_VALUE, "failed to alloc CSI buffer, ret = %d", ret);

    mState = CSI_META_DEVICE_CONFIGURED;

    return OK;
}

int CsiMetaDevice::start() {
    LOG1("@%s", __func__);
    AutoMutex l(mCsiMetaDeviceLock);
    if (!mIsCsiMetaEnabled) {
        return OK;
    }

    CheckWarning(mState != CSI_META_DEVICE_CONFIGURED && mState != CSI_META_DEVICE_STOP, OK,
                 "%s: start in wrong state: %d", __func__, mState);

    int ret = mCsiMetaDevice->Start();
    CheckAndLogError(ret < 0, ret, "failed to stream on csi meta device, ret = %d", ret);

    mExitPending = false;
    mPollThread->run("CsiMetaDevice", PRIORITY_URGENT_AUDIO);
    mState = CSI_META_DEVICE_START;

    return OK;
}

int CsiMetaDevice::stop() {
    LOG1("@%s", __func__);
    AutoMutex l(mCsiMetaDeviceLock);
    if (!mIsCsiMetaEnabled) {
        return OK;
    }

    CheckWarning(mState != CSI_META_DEVICE_START, OK, "%s: device not started", __func__);

    mExitPending = true;
    mPollThread->requestExit();

    int ret = mCsiMetaDevice->Stop(false);

    CheckAndLogError(ret < 0, ret, "failed to stream off csi meta device, ret = %d", ret);

    mPollThread->requestExitAndWait();

    mState = CSI_META_DEVICE_STOP;
    return OK;
}

int CsiMetaDevice::poll() {
    const int poll_timeout_count = 10;
    const int poll_timeout = 1000;

    std::vector<V4L2Device*> pollDevs;

    int ret = 0;
    int timeOutCount = poll_timeout_count;

    LOG2("@%s before poll number buffer in devices: %d", __func__, mBuffersInCsiMetaDevice.load());

    if (!hasBufferIndevice()) {
        LOG2("@%s there is no buffers in device, skip this poll", __func__);
        return OK;
    }

    for (const auto& device : mConfiguredDevices) {
        pollDevs.push_back(device);
    }

    std::vector<V4L2Device*> readyDevices;
    while (timeOutCount-- && ret == 0) {
        V4L2DevicePoller poller{pollDevs, -1};
        ret = poller.Poll(poll_timeout, POLLPRI | POLLIN | POLLOUT | POLLERR, &readyDevices);

        LOG2("@%s ing poll number buffer in devices: %d", __func__, mBuffersInCsiMetaDevice.load());
        if (mExitPending) {
            return NO_INIT;
        }
    }

    if (mExitPending) {
        // Exiting, no error
        return NO_INIT;
    }

    // handle the poll error
    CheckAndLogError(ret < 0, ret, "%s: Poll error", __func__);

    if (ret > 0) {
        handleCsiMetaBuffer();
    }

    LOG2("@%s after poll number buffer in devices: %d", __func__, mBuffersInCsiMetaDevice.load());
    return OK;
}

int CsiMetaDevice::hasBufferIndevice() {
    return mBuffersInCsiMetaDevice.load();
}

int CsiMetaDevice::queueCsiMetaBuffer(const shared_ptr<CameraBuffer>& camBuffer) {
    int ret = mCsiMetaDevice->PutFrame(&camBuffer->getV4L2Buffer());
    CheckAndLogError(ret < 0, BAD_VALUE, "%s: Failed to queue CSI meta buffer, ret=%d", __func__,
                     ret);

    LOG2("%s: queue CSI meta buffer, camBuffer index: %d", __func__, camBuffer->getIndex());

    mBuffersInCsiMetaDevice++;

    return OK;
}

void CsiMetaDevice::handleCsiMetaBuffer() {
    AutoMutex l(mCsiMetaBuffersLock);

    shared_ptr<CameraBuffer> camBuffer = mCsiMetaCameraBuffers[mCsiMetaBufferDQIndex];
    CheckAndLogError(!camBuffer, VOID_VALUE, "Csi meta camera buffer %d is null",
                     mCsiMetaBufferDQIndex);

    V4L2Buffer& vbuf = camBuffer->getV4L2Buffer();
    int index = mCsiMetaDevice->GrabFrame(&vbuf);

    if (mExitPending) return;
    CheckAndLogError(index < 0, VOID_VALUE, "grab frame failed. index %d", index);

    LOG2("%s: grab meta data buffer sequence %d timestamp %ld", __func__, camBuffer->getSequence(),
         TIMEVAL2USECS(camBuffer->getTimestamp()));
    PERF_CAMERA_ATRACE_PARAM3("grabFrame SeqID", camBuffer->getSequence(), "csi2_port",
                              camBuffer->getCsi2Port(), "virtual_channel",
                              camBuffer->getVirtualChannel());

    mBuffersInCsiMetaDevice--;
    mCsiMetaBufferDQIndex = (mCsiMetaBufferDQIndex + 1) % mCsiMetaCameraBuffers.size();

    if (CameraDump::isDumpTypeEnable(DUMP_EMBEDDED_METADATA)) {
        BinParam_t bParam;
        bParam.bType = BIN_TYPE_SENSOR_METADATA;
        bParam.mType = M_ISYS;
        bParam.sequence = camBuffer->getSequence();
        bParam.mParam.width = mEmbeddedMetaData.width;
        bParam.mParam.height = mEmbeddedMetaData.height;
        bParam.mParam.metaFormat = mEmbeddedMetaData.csiMetaFormat;
        for (int i = 0; i < mEmbeddedMetaData.planesNum; i++) {
            bParam.mParam.planeIdx = i;
            LOG2("@%s, dump sensor meta data, addr = %p, w:%d, h:%d, len:%d", __func__,
                 camBuffer->getBufferAddr(i), mEmbeddedMetaData.width, mEmbeddedMetaData.height,
                 camBuffer->getBufferSize(i));
            CameraDump::dumpBinary(mCameraId, camBuffer->getBufferAddr(i),
                                   camBuffer->getBufferSize(i), &bParam);
        }
    }

    EventData eventData;
    EventDataMeta metaData;
    CLEAR(metaData);
    metaData.timestamp = camBuffer->getTimestamp();
    metaData.sequence = camBuffer->getSequence();

    // DOL_FEATURE_S
    int status = decodeMetaData(metaData, camBuffer);
    if (status == OK) {
        eventData.type = EVENT_META;
        eventData.buffer = nullptr;
        eventData.data.meta = metaData;

        notifyListeners(eventData);
    }
    // DOL_FEATURE_E

    queueCsiMetaBuffer(camBuffer);
}

// DOL_FEATURE_S
int CsiMetaDevice::decodeMetaData(EventDataMeta& metaData,
                                  const shared_ptr<CameraBuffer>& camBuffer) {
    vector<int> vbpOffset;
    PlatformData::getDolVbpOffset(mCameraId, vbpOffset);

    if (vbpOffset.size() == 0) {
        LOG2("%s: skip vbp meta data decode, no vbpOffset set.", __func__);
        return INVALID_OPERATION;
    } else if (vbpOffset.size() != 2) {
        LOGE("Invalid vbpOffset value size %lu, skip vpb meta data decode.", vbpOffset.size());
        return BAD_VALUE;
    }

    if (mEmbeddedMetaData.planesNum > 0) {
        char* metaPtr = reinterpret_cast<char*>(camBuffer->getBufferAddr(0));
        metaData.vbp = metaPtr[vbpOffset[0]] | (metaPtr[vbpOffset[1]] << 8);
        LOG2("<seq%ld>%s: offset %d/%d, value is %d", metaData.sequence, __func__, vbpOffset[0],
             vbpOffset[1], metaData.vbp);
    }

    return OK;
}
// DOL_FEATURE_E

int CsiMetaDevice::setFormat() {
    struct v4l2_format v4l2fmt;
    CLEAR(v4l2fmt);

#ifdef CAL_BUILD
    v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
#else
    int dev_caps = mCsiMetaDevice->GetDeviceCaps();
    if (dev_caps & V4L2_CAP_META_CAPTURE) {
        v4l2fmt.type = V4L2_BUF_TYPE_META_CAPTURE;
    } else {
        v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }
#endif
    v4l2fmt.fmt.pix_mp.width = mEmbeddedMetaData.width;
    v4l2fmt.fmt.pix_mp.height = mEmbeddedMetaData.height;
    v4l2fmt.fmt.pix_mp.num_planes = mEmbeddedMetaData.planesNum;
    v4l2fmt.fmt.pix_mp.pixelformat = mEmbeddedMetaData.csiMetaFormat;
    for (int i = 0; i < v4l2fmt.fmt.pix_mp.num_planes; i++) {
        v4l2fmt.fmt.pix_mp.plane_fmt[i].bytesperline = mEmbeddedMetaData.bpl;
        v4l2fmt.fmt.pix_mp.plane_fmt[i].sizeimage = 0;
    }
    v4l2fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;

    V4L2Format tmpbuf{v4l2fmt};
    int ret = mCsiMetaDevice->SetFormat(tmpbuf);
    v4l2fmt = *tmpbuf.Get();

    CheckAndLogError(ret != OK, ret, "set v4l2 format failed ret=%d", ret);

    for (int i = 0; i < v4l2fmt.fmt.pix_mp.num_planes; i++) {
        mEmbeddedMetaData.size[i] = v4l2fmt.fmt.pix_mp.plane_fmt[i].sizeimage;
    }

    LOG2("VIDIOC_S_FMT type %d : width: %d, height: %d, bpl: %d, fourcc: %d, field: %d",
         v4l2fmt.type, v4l2fmt.fmt.pix.width, v4l2fmt.fmt.pix.height, v4l2fmt.fmt.pix.bytesperline,
         v4l2fmt.fmt.pix.pixelformat, v4l2fmt.fmt.pix.field);

    return OK;
}

int CsiMetaDevice::allocCsiMetaBuffers() {
    AutoMutex l(mCsiMetaBuffersLock);

    mCsiMetaCameraBuffers.clear();

    std::vector<V4L2Buffer> bufs;
    int bufNum = mCsiMetaDevice->SetupBuffers(MAX_BUFFER_COUNT, false, V4L2_MEMORY_MMAP, &bufs);

    CheckAndLogError(bufNum < 0, bufNum, "request CSI meta buffers failed return=%d", bufNum);

    for (int i = 0; i < bufNum; i++) {
        shared_ptr<CameraBuffer> camBuffer = std::make_shared<CameraBuffer>(
            mCameraId, BUFFER_USAGE_METADATA, V4L2_MEMORY_MMAP, 0, i, V4L2_FMT_IPU_ISYS_META);
        CheckAndLogError(!camBuffer, NO_MEMORY, "@%s: fail to alloc CameraBuffer", __func__);

        for (int j = 0; j < mEmbeddedMetaData.planesNum; j++) {
            LOG2("@%s, buffer size [%d] = %d", __func__, j, mEmbeddedMetaData.size[j]);
            camBuffer->setBufferSize(mEmbeddedMetaData.size[j], j);
        }

        // MMap user space buffer
        camBuffer->allocateMemory(mCsiMetaDevice);
        mCsiMetaCameraBuffers.push_back(camBuffer);

        // Enqueue CSI meta buffer to ISP
        queueCsiMetaBuffer(camBuffer);

        LOG2("%s: store csi meta, index: %d, vbuff index: %d camBuffer->getBufferAddr() = %p",
             __func__, i, camBuffer->getIndex(), camBuffer->getBufferAddr());
    }

    return OK;
}

} /* namespace icamera */
