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

#define LOG_TAG CaptureUnit

#include "CaptureUnit.h"

#include <fcntl.h>
#include <poll.h>

#include "MediaControl.h"
#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using std::map;
using std::vector;

namespace icamera {

CaptureUnit::CaptureUnit(int cameraId, int memType)
        : StreamSource(memType),
          mCameraId(cameraId),
          mMaxBufferNum(PlatformData::getMaxRawDataNum(cameraId)),
          mState(CAPTURE_UNINIT),
          mExitPending(false) {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    mPollThread = new PollThread(this);
    mFlushFd[0] = -1;
    mFlushFd[1] = -1;

    int ret = pipe(mFlushFd);
    if (ret >= 0) {
        ret = fcntl(mFlushFd[0], F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            LOG1("failed to set flush pipe flag: %s", strerror(errno));
            close(mFlushFd[0]);
            close(mFlushFd[1]);
            mFlushFd[0] = -1;
            mFlushFd[1] = -1;
        }
        LOG1("%s, mFlushFd [%d-%d]", __func__, mFlushFd[0], mFlushFd[1]);
    }
    mMaxBuffersInDevice = PlatformData::getExposureLag(mCameraId) + 1;
    if (mMaxBuffersInDevice < 2) {
        mMaxBuffersInDevice = 2;
    }
}

CaptureUnit::~CaptureUnit() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    if (mFlushFd[0] != -1) close(mFlushFd[0]);
    if (mFlushFd[1] != -1) close(mFlushFd[1]);

    delete mPollThread;
}

int CaptureUnit::init() {
    LOG1("<id%d>%s", mCameraId, __func__);
    mState = CAPTURE_INIT;

    return OK;
}

void CaptureUnit::deinit() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    if (mState == CAPTURE_UNINIT) {
        LOG1("%s: deinit without init", __func__);
        return;
    }

    destroyDevices();
    mPollThread->join();
    mState = CAPTURE_UNINIT;
}

int CaptureUnit::createDevices() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);
    destroyDevices();

    const Port kDefaultPort = INVALID_PORT;
    Port portOfMainDevice = findDefaultPort(mOutputFrameInfo);
    const stream_t& kDefaultStream = mOutputFrameInfo.at(portOfMainDevice);
    VideoNodeType nodeType = VIDEO_GENERIC;
    mDevices.push_back(new MainDevice(mCameraId, nodeType, this));

    // targetPorts specifies the desired port for the device.
    // But the real port which will be used is deciced whether
    // the port is provided by the consumer.
    vector<Port> targetPorts;
    targetPorts.push_back(portOfMainDevice);

    // Open and configure the devices. The stream and port that are used by the device is
    // decided by whether consumer has provided such info, use the default one if not.
    for (uint8_t i = 0; i < mDevices.size(); i++) {
        DeviceBase* device = mDevices[i];

        int ret = device->openDevice();
        CheckAndLogError(ret != OK, ret, "Open device(%s) failed:%d", device->getName(), ret);

        const Port kTargetPort = targetPorts[i];
        bool hasPort = mOutputFrameInfo.find(kTargetPort) != mOutputFrameInfo.end();
        const stream_t& stream = hasPort ? mOutputFrameInfo.at(kTargetPort) : kDefaultStream;

        ret = device->configure(hasPort ? kTargetPort : kDefaultPort, stream, mMaxBufferNum);
        CheckAndLogError(ret != OK, ret, "Configure device(%s) failed:%d", device->getName(), ret);
    }

    return OK;
}

void CaptureUnit::destroyDevices() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    for (auto device : mDevices) {
        device->closeDevice();
        delete device;
    }
    mDevices.clear();
}

/**
 * Find the device that can handle the given port.
 */
DeviceBase* CaptureUnit::findDeviceByPort(Port port) {
    for (auto device : mDevices) {
        if (device->getPort() == port) {
            return device;
        }
    }

    return nullptr;
}

int CaptureUnit::streamOn() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    for (auto device : mDevices) {
        int ret = device->streamOn();
        CheckAndLogError(ret < 0, INVALID_OPERATION, "Device:%s stream on failed.",
                         device->getName());
    }

    return OK;
}

int CaptureUnit::start() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mLock);
    CheckWarning(mState == CAPTURE_START, OK, "@%s: device already started", __func__);

    int ret = streamOn();
    if (ret != OK) {
        streamOff();
        LOGE("Devices stream on failed:%d", ret);
        return ret;
    }

    if (mFlushFd[0] != -1) {
        // read pipe just in case there is data in pipe.
        char readBuf;
        int readSize = read(mFlushFd[0], reinterpret_cast<void*>(&readBuf), sizeof(char));
        LOG1("%s, readSize %d", __func__, readSize);
    }
    mPollThread->run("CaptureUnit", PRIORITY_URGENT_AUDIO);
    mState = CAPTURE_START;
    mExitPending = false;
    LOG2("@%s: automation checkpoint: flag: poll_started", __func__);

    return OK;
}

void CaptureUnit::streamOff() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);

    for (auto device : mDevices) {
        device->streamOff();
    }
}

int CaptureUnit::stop() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>%s", mCameraId, __func__);
    CheckWarning(mState != CAPTURE_START, OK, "@%s: device not started", __func__);

    mExitPending = true;
    if (mFlushFd[1] != -1) {
        char buf = 0xf;  // random value to write to flush fd.
        int size = write(mFlushFd[1], &buf, sizeof(char));
        LOG1("%s, write size %d", __func__, size);
    }

    mPollThread->requestExit();
    streamOff();
    mPollThread->requestExitAndWait();

    AutoMutex l(mLock);
    mState = CAPTURE_STOP;

    for (auto device : mDevices) {
        device->resetBuffers();
    }
    LOG2("@%s: automation checkpoint: flag: poll_stopped", __func__);
    mExitPending = false;  // It's already stopped.

    return OK;
}

int CaptureUnit::configure(const map<Port, stream_t>& outputFrames,
                           const vector<ConfigMode>& configModes) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError(outputFrames.empty(), BAD_VALUE, "No frame info configured.");
    CheckAndLogError(
        mState != CAPTURE_CONFIGURE && mState != CAPTURE_INIT && mState != CAPTURE_STOP,
        INVALID_OPERATION, "@%s: Configure in wrong state %d", __func__, mState);

    Port port = findDefaultPort(outputFrames);
    const stream_t& mainStream = outputFrames.at(port);

    for (const auto& item : outputFrames) {
        LOG1("<id%d>%s, port:%d, w:%d, h:%d, f:%s", mCameraId, __func__, item.first,
             item.second.width, item.second.height,
             CameraUtils::format2string(item.second.format).c_str());
    }

    mConfigModes = configModes;
    mOutputFrameInfo = outputFrames;

    /* media ctl setup */
    MediaCtlConf* mediaCtl = PlatformData::getMediaCtlConf(mCameraId);
    CheckAndLogError(!mediaCtl, BAD_VALUE, "get format configuration failed for %s (%dx%d)",
                     CameraUtils::format2string(mainStream.format).c_str(), mainStream.width,
                     mainStream.height);

    MediaControl* mc = MediaControl::getInstance();
    CheckAndLogError(!mc, UNKNOWN_ERROR, "%s, MediaControl init failed", __func__);

    int status = mc->mediaCtlSetup(mCameraId, mediaCtl, mainStream.width, mainStream.height,
                                   mainStream.field);
    CheckAndLogError(status != OK, status, "set up mediaCtl failed");

    // Create, open, and configure all of needed devices.
    status = createDevices();
    CheckAndLogError(status != OK, status, "Create devices failed:%d", status);

    mState = CAPTURE_CONFIGURE;
    // mExitPending should also be set false in configure to make buffers queued before start
    mExitPending = false;

    return OK;
}

Port CaptureUnit::findDefaultPort(const map<Port, stream_t>& frames) const {
    Port availablePorts[] = {MAIN_PORT, SECOND_PORT, THIRD_PORT, FORTH_PORT};
    for (unsigned int i = 0; i < ARRAY_SIZE(availablePorts); i++) {
        if (frames.find(availablePorts[i]) != frames.end()) {
            return availablePorts[i];
        }
    }
    return INVALID_PORT;
}

int CaptureUnit::allocateMemory(Port port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    const struct v4l2_buffer* v = camBuffer->getV4L2Buffer().Get();
    CheckAndLogError(v->index >= mMaxBufferNum, -1, "index %d is larger than max count %d",
                     v->index, mMaxBufferNum);
    CheckAndLogError(v->memory != V4L2_MEMORY_MMAP, -1,
                     "Allocating Memory Capture device only supports MMAP mode.");

    DeviceBase* device = findDeviceByPort(port);
    CheckAndLogError(!device, BAD_VALUE, "No device available for port:%d", port);

    int ret = camBuffer->allocateMemory(device->getV4l2Device());
    CheckAndLogError(ret < 0, ret, "Failed to allocate memory ret(%d) for port:%d", ret, port);

    return OK;
}

int CaptureUnit::qbuf(Port port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    CheckAndLogError(camBuffer == nullptr, BAD_VALUE, "Camera buffer is null");
    CheckAndLogError((mState == CAPTURE_INIT || mState == CAPTURE_UNINIT), INVALID_OPERATION,
                     "@%s: qbuf in wrong state %d", __func__, mState);

    DeviceBase* device = findDeviceByPort(port);
    CheckAndLogError(!device, BAD_VALUE, "No device available for port:%d", port);

    LOG2("<id%d>@%s, queue CameraBuffer: %p to port:%d", mCameraId, __func__, camBuffer.get(),
         port);

    device->addPendingBuffer(camBuffer);
    return processPendingBuffers();
}

int CaptureUnit::queueAllBuffers() {
    PERF_CAMERA_ATRACE();

    if (mExitPending) return OK;

    int64_t predictSequence = -1;
    for (auto device : mDevices) {
        int ret = device->queueBuffer(predictSequence);
        if (mExitPending) break;
        CheckAndLogError(ret != OK, ret, "queueBuffer fails, dev:%s, ret:%d", device->getName(),
                         ret);
        if (predictSequence == -1) {
            predictSequence = device->getPredictSequence();
        }
    }

    return OK;
}

void CaptureUnit::onDequeueBuffer() {
    processPendingBuffers();
}

int CaptureUnit::processPendingBuffers() {
    LOG2("%s: buffers in device:%d", __func__, mDevices.front()->getBufferNumInDevice());

    while (mDevices.front()->getBufferNumInDevice() < mMaxBuffersInDevice) {
        bool hasPendingBuffer = true;
        for (auto device : mDevices) {
            if (!device->hasPendingBuffer()) {
                hasPendingBuffer = false;
                break;
            }
        }
        // Do not queue buffer when one of the devices has no pending buffers.
        if (!hasPendingBuffer) break;

        int ret = queueAllBuffers();
        if (mExitPending) break;
        CheckAndLogError(ret != OK, ret, "Failed to queue buffers, ret=%d", ret);
    }

    return OK;
}

int CaptureUnit::poll() {
    PERF_CAMERA_ATRACE();
    int ret = 0;
    const int poll_timeout_count = 10;
    // Normally set the timeout threshold to 1s
    const int poll_timeout = gSlowlyRunRatio ? (gSlowlyRunRatio * 100000) : 1000;

    LOG2("<id%d>%s", mCameraId, __func__);
    CheckAndLogError((mState != CAPTURE_CONFIGURE && mState != CAPTURE_START), INVALID_OPERATION,
                     "@%s: poll buffer in wrong state %d", __func__, mState);

    int timeOutCount = (PlatformData::getMaxIsysTimeout() > 0) ? PlatformData::getMaxIsysTimeout() :
                                                                 poll_timeout_count;
    std::vector<V4L2Device*> pollDevs, readyDevices;
    for (const auto& device : mDevices) {
        pollDevs.push_back(device->getV4l2Device());
        LOG2("@%s: device:%s has %d buffers queued.", __func__, device->getName(),
             device->getBufferNumInDevice());
    }

    while (timeOutCount-- && ret == 0) {
        // If stream off, no poll needed.
        if (mExitPending) {
            LOG2("%s: mExitPending is true, exit", __func__);
            // Exiting, no error
            return -1;
        }

        V4L2DevicePoller poller{pollDevs, mFlushFd[0]};
        ret = poller.Poll(poll_timeout, POLLPRI | POLLIN | POLLOUT | POLLERR, &readyDevices);
    }

    // In case poll error after stream off
    if (mExitPending) {
        LOG2("%s: mExitPending is true, exit", __func__);
        // Exiting, no error
        return -1;
    }
    CheckAndLogError(ret < 0, UNKNOWN_ERROR, "%s: Poll error, ret:%d", __func__, ret);
    if (ret == 0) {
#ifdef CAL_BUILD
        LOGI("<id%d>%s, timeout happens, buffer in device: %d. wait recovery", mCameraId, __func__,
             mDevices.front()->getBufferNumInDevice());
#else
        LOG1("<id%d>%s, timeout happens, buffer in device: %d. wait recovery", mCameraId, __func__,
             mDevices.front()->getBufferNumInDevice());
#endif
        if (PlatformData::getMaxIsysTimeout() > 0 && mDevices.front()->getBufferNumInDevice() > 0) {
            EventData errorData;
            errorData.type = EVENT_ISYS_ERROR;
            errorData.buffer = nullptr;
            notifyListeners(errorData);
        }

        return OK;
    }

    for (const auto& readyDevice : readyDevices) {
        for (auto device : mDevices) {
            if (device->getV4l2Device() == readyDevice) {
                int ret = device->dequeueBuffer();
                if (mExitPending) return -1;

                if (ret != OK) {
                    LOGE("Device:%s grab frame failed:%d", device->getName(), ret);
                }
                break;
            }
        }
    }

    return OK;
}

void CaptureUnit::addFrameAvailableListener(BufferConsumer* listener) {
    AutoMutex l(mLock);
    for (auto device : mDevices) {
        device->addFrameListener(listener);
    }
}

void CaptureUnit::removeFrameAvailableListener(BufferConsumer* listener) {
    AutoMutex l(mLock);
    for (auto device : mDevices) {
        device->removeFrameListener(listener);
    }
}

void CaptureUnit::removeAllFrameAvailableListener() {
    AutoMutex l(mLock);
    for (auto device : mDevices) {
        device->removeAllFrameListeners();
    }
}

void CaptureUnit::registerListener(EventType eventType, EventListener* eventListener) {
    for (auto device : mDevices) {
        device->registerListener(eventType, eventListener);
    }
    if (eventType == EVENT_ISYS_ERROR) EventSource::registerListener(eventType, eventListener);
}

void CaptureUnit::removeListener(EventType eventType, EventListener* eventListener) {
    for (auto device : mDevices) {
        device->removeListener(eventType, eventListener);
    }
    if (eventType == EVENT_ISYS_ERROR) EventSource::removeListener(eventType, eventListener);
}
}  // namespace icamera
