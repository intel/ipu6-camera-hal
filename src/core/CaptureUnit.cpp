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

#include <poll.h>
#include <fcntl.h>

#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Utils.h"

#include "PlatformData.h"
#include "MediaControl.h"
#include "CaptureUnit.h"

using std::vector;
using std::map;

namespace icamera {

CaptureUnit::CaptureUnit(int cameraId, int memType) :
    StreamSource(memType),
    mCameraId(cameraId),
#ifndef HAS_MULTI_INPUT_DEVICE
    mDevice(nullptr),
#endif
    mMaxBufferNum(PlatformData::getMaxRawDataNum(cameraId)),
    mState(CAPTURE_UNINIT),
    mExitPending(false)
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    // USE_ISA_S
    mIsaAdaptor = new IspParamAdaptor(mCameraId, PG_PARAM_ISYS);
    // USE_ISA_E
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

CaptureUnit::~CaptureUnit()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    if (mFlushFd[0] != -1) close(mFlushFd[0]);
    if (mFlushFd[1] != -1) close(mFlushFd[1]);

    delete mPollThread;
    // USE_ISA_S
    delete mIsaAdaptor;
    // USE_ISA_E
}

int CaptureUnit::init()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    mState = CAPTURE_INIT;

    return OK;
}

void CaptureUnit::deinit()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    if (mState == CAPTURE_UNINIT) {
        LOG1("%s: deinit without init", __func__);
        return;
    }

    destroyDevices();
    mPollThread->join();

    mState = CAPTURE_UNINIT;
}

int CaptureUnit::createDevices()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    destroyDevices();

    // Default INVALID_PORT means the device isn't associated with any outside consumers.
    const Port kDefaultPort = INVALID_PORT;
    Port portOfMainDevice = findDefaultPort(mOutputFrameInfo);
    // Use the config for main port as the default one.
    const stream_t& kDefaultStream = mOutputFrameInfo.at(portOfMainDevice);

    // Use VIDEO_GENERIC by default.
    VideoNodeType nodeType = VIDEO_GENERIC;
    // USE_ISA_S
    // Update it VIDEO_ISA_SCALE is ISA scale enabled.
    if (PlatformData::isISysScaleEnabled(mCameraId)) {
        nodeType = VIDEO_ISA_SCALE;
    }
    // USE_ISA_E

#ifdef HAS_MULTI_INPUT_DEVICE
    mDevices.push_back(new MainDevice(mCameraId, nodeType, this));

    // targetPorts specifies the desired port for the device. But the real port which will be used
    // is deciced whether the port is provided by the consumer.
    vector<Port> targetPorts;
    targetPorts.push_back(portOfMainDevice);

    // DOL_FEATURE_S
    if (PlatformData::isDolShortEnabled(mCameraId)) {
        mDevices.push_back(new DolCaptureDevice(mCameraId, VIDEO_GENERIC_SHORT_EXPO));
        targetPorts.push_back(SECOND_PORT);
    }

    if (PlatformData::isDolMediumEnabled(mCameraId)) {
        mDevices.push_back(new DolCaptureDevice(mCameraId, VIDEO_GENERIC_MEDIUM_EXPO));
        targetPorts.push_back(THIRD_PORT);
    }
    // DOL_FEATURE_E

    // USE_ISA_S
    if (PlatformData::isISysScaleEnabled(mCameraId)) {
        mDevices.push_back(new IsaRawDevice(mCameraId, VIDEO_GENERIC));
        targetPorts.push_back(SECOND_PORT);
    }

    if (PlatformData::isIsaEnabled(mCameraId)) {
        int ret = mIsaAdaptor->init();
        CheckAndLogError((ret != OK), ret, "Init ISA adaptor failed with:%d", ret);

        ret = configureIsaAdaptor(kDefaultStream);
        CheckAndLogError(ret != OK, ret, "%s: Failed to configure ISA ISP adaptor.", __func__);

        mDevices.push_back(new IsaConfigDevice(mCameraId, VIDEO_ISA_CONFIG, mIsaAdaptor));
        targetPorts.push_back(kDefaultPort);

        mDevices.push_back(new IsaStatsDevice(mCameraId, VIDEO_AA_STATS, mIsaAdaptor));
        targetPorts.push_back(kDefaultPort);
    }
    // USE_ISA_E

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
#else
    mDevice = new MainDevice(mCameraId, nodeType, this);

    // Open and configure the device. The stream and port that are used by the device is
    // decided by whether consumer has provided such info, use the default one if not.
    int ret = mDevice->openDevice();
    CheckAndLogError(ret != OK, ret, "Open device(%s) failed:%d", mDevice->getName(), ret);

    bool hasPort = mOutputFrameInfo.find(portOfMainDevice) != mOutputFrameInfo.end();
    const stream_t& stream = hasPort ? mOutputFrameInfo.at(portOfMainDevice) : kDefaultStream;

    ret = mDevice->configure(hasPort ? portOfMainDevice : kDefaultPort, stream, mMaxBufferNum);
    CheckAndLogError(ret != OK, ret, "Configure device(%s) failed:%d", mDevice->getName(), ret);
#endif

    return OK;
}

void CaptureUnit::destroyDevices()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->closeDevice();
        delete device;
    }
    mDevices.clear();
#else
    if (mDevice) {
        mDevice->closeDevice();
        delete mDevice;
        mDevice = nullptr;
    }
#endif

    // USE_ISA_S
    if (PlatformData::isIsaEnabled(mCameraId)) {
        mIsaAdaptor->deinit();
    }
    // USE_ISA_E
}

/**
 * Find the device that can handle the given port.
 */
DeviceBase* CaptureUnit::findDeviceByPort(Port port)
{
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        if (device->getPort() == port) {
            return device;
        }
    }
#else
    if (mDevice && mDevice->getPort() == port) {
        return mDevice;
    }
#endif

    return nullptr;
}

int CaptureUnit::streamOn()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        int ret = device->streamOn();
        CheckAndLogError(ret < 0, INVALID_OPERATION, "Device:%s stream on failed.",
                         device->getName());
    }
#else
    if (mDevice) {
        int ret = mDevice->streamOn();
        CheckAndLogError(ret < 0, INVALID_OPERATION, "Device:%s stream on failed.",
                         mDevice->getName());
    }
#endif

    return OK;
}

int CaptureUnit::start()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

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

void CaptureUnit::streamOff()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->streamOff();
    }
#else
    if (mDevice) {
        mDevice->streamOff();
    }
#endif
}

int CaptureUnit::stop()
{
    PERF_CAMERA_ATRACE();
    LOG1("%s, mCameraId:%d", __func__, mCameraId);

    if (mState != CAPTURE_START) {
        LOGW("@%s: device not started", __func__);
        return OK;
    }

    mExitPending = true;
    if (mFlushFd[1] != -1) {
        char buf = 0xf;    // random value to write to flush fd.
        int size = write(mFlushFd[1], &buf, sizeof(char));
        LOG1("%s, write size %d", __func__, size);
    }

    mPollThread->requestExit();
    streamOff();
    mPollThread->requestExitAndWait();

    AutoMutex   l(mLock);
    mState = CAPTURE_STOP;

#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->resetBuffers();
    }
#else
    if (mDevice) {
        mDevice->resetBuffers();
    }
#endif
    LOG2("@%s: automation checkpoint: flag: poll_stopped", __func__);

    mExitPending = false; // It's already stopped.

    return OK;
}

/**
 * Check if the given outputFrames are different from the previous one.
 * Only return false when the config for each port is exactly same.
 */
bool CaptureUnit::isNewConfiguration(const map<Port, stream_t>& outputFrames)
{
    for (const auto& item : outputFrames) {
        if (mOutputFrameInfo.find(item.first) == mOutputFrameInfo.end()) {
            return true;
        }

        const stream_t& oldStream = mOutputFrameInfo[item.first];
        const stream_t& newStream = item.second;

        bool isNewConfig = (oldStream.width != newStream.width || oldStream.height != newStream.height
               || oldStream.format != newStream.format || oldStream.field != newStream.field
               || oldStream.memType != newStream.memType);
        if (isNewConfig) {
            return true;
        }
    }

    return false;
}

int CaptureUnit::configure(const map<Port, stream_t>& outputFrames,
                           const vector<ConfigMode>& configModes)
{
    PERF_CAMERA_ATRACE();

    CheckAndLogError(outputFrames.empty(), BAD_VALUE, "No frame info configured.");
    CheckAndLogError(mState != CAPTURE_CONFIGURE && mState != CAPTURE_INIT && mState != CAPTURE_STOP,
          INVALID_OPERATION, "@%s: Configure in wrong state %d", __func__, mState);

    Port port = findDefaultPort(outputFrames);
    const stream_t& mainStream = outputFrames.at(port);

#ifdef HAS_MULTI_INPUT_DEVICE
    if (!isNewConfiguration(outputFrames)) {
        LOGD("@%s: Configuration is not changed.", __func__);
        // USE_ISA_S
        configureIsaAdaptor(mainStream); // ISA adaptor needs to be re-configured.
        // USE_ISA_E
        return OK;
    }
#endif

    for (const auto& item : outputFrames) {
        LOG1("%s, mCameraId:%d, port:%d, w:%d, h:%d, f:%s", __func__, mCameraId, item.first,
              item.second.width, item.second.height,
              CameraUtils::format2string(item.second.format).c_str());
    }

    mConfigModes = configModes;
    mOutputFrameInfo = outputFrames;

    /* media ctl setup */
    MediaCtlConf *mediaCtl = PlatformData::getMediaCtlConf(mCameraId);
    CheckAndLogError(!mediaCtl, BAD_VALUE, "get format configuration failed for %s (%dx%d)",
                     CameraUtils::format2string(mainStream.format).c_str(),
                     mainStream.width, mainStream.height);

    MediaControl *mc = MediaControl::getInstance();
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

Port CaptureUnit::findDefaultPort(const map<Port, stream_t>& frames) const
{
    Port availablePorts[] = {MAIN_PORT, SECOND_PORT, THIRD_PORT, FORTH_PORT};
    for (unsigned int i = 0; i < ARRAY_SIZE(availablePorts); i++) {
        if (frames.find(availablePorts[i]) != frames.end()) {
            return availablePorts[i];
        }
    }
    return INVALID_PORT;
}

int CaptureUnit::allocateMemory(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
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

// USE_ISA_S
int CaptureUnit::configureIsaAdaptor(const stream_t &stream)
{
    if (!PlatformData::isIsaEnabled(mCameraId)) {
        return OK;
    }

    CheckAndLogError(mConfigModes.empty(), INVALID_OPERATION, "empty config modes");

    TuningMode tuningMode;
    int status = PlatformData::getTuningModeByConfigMode(mCameraId, mConfigModes[0], tuningMode);
    CheckAndLogError(status != OK, status, "%s, get tuningModes failed %d", __func__, status);

    status = mIsaAdaptor->configure(stream, mConfigModes[0], tuningMode);
    CheckAndLogError(status != OK, -1, "Failed to configure isa adaptor");

    return OK;
}
// USE_ISA_E

int CaptureUnit::qbuf(Port port, const std::shared_ptr<CameraBuffer> &camBuffer)
{
    CheckAndLogError(camBuffer == nullptr, BAD_VALUE, "Camera buffer is null");
    CheckAndLogError((mState == CAPTURE_INIT || mState == CAPTURE_UNINIT), INVALID_OPERATION,
                     "@%s: qbuf in wrong state %d", __func__, mState);

    DeviceBase* device = findDeviceByPort(port);
    CheckAndLogError(!device, BAD_VALUE, "No device available for port:%d", port);

    LOG2("@%s, mCameraId:%d, queue CameraBuffer: %p to port:%d",
         __func__, mCameraId, camBuffer.get(), port);

    device->addPendingBuffer(camBuffer);

    return processPendingBuffers();
}

int CaptureUnit::queueAllBuffers()
{
    PERF_CAMERA_ATRACE();

    if (mExitPending) return OK;

#ifdef HAS_MULTI_INPUT_DEVICE
    long predictSequence = -1;
    for (auto device : mDevices) {
        int ret = device->queueBuffer(predictSequence);
        if (mExitPending) break;
        CheckAndLogError(ret != OK, ret, "queueBuffer fails, dev:%s, ret:%d", device->getName(), ret);
        if (predictSequence == -1) {
            predictSequence = device->getPredictSequence();
        }
    }
#else
    if (mDevice) {
        int ret = mDevice->queueBuffer(-1);
        if (mExitPending) return OK;
        CheckAndLogError(ret != OK, ret, "queueBuffer fails, dev:%s, ret:%d", mDevice->getName(), ret);
        mDevice->getPredictSequence();
    }
#endif

    return OK;
}

void CaptureUnit::onDequeueBuffer()
{
    processPendingBuffers();
}

int CaptureUnit::processPendingBuffers()
{
#ifdef HAS_MULTI_INPUT_DEVICE
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
#else
    if (mDevice && mDevice->getBufferNumInDevice() < mMaxBuffersInDevice) {
        LOG2("%s: buffers in device:%d", __func__, mDevice->getBufferNumInDevice());

        if (!mDevice->hasPendingBuffer()) {
            return OK;
        }

        int ret = queueAllBuffers();
        if (mExitPending) return OK;
        CheckAndLogError(ret != OK, ret, "Failed to queue buffers, ret=%d", ret);
    }
#endif

    return OK;
}

int CaptureUnit::poll()
{
    PERF_CAMERA_ATRACE();
    int ret = 0;
    const int poll_timeout_count = 10;
    const int poll_timeout = gSlowlyRunRatio ? (gSlowlyRunRatio * 1000000) : 1000;

    LOG2("@%s, mCameraId:%d", __func__, mCameraId);

    CheckAndLogError((mState != CAPTURE_CONFIGURE && mState != CAPTURE_START), INVALID_OPERATION,
                     "@%s: poll buffer in wrong state %d", __func__, mState);

    int timeOutCount = poll_timeout_count;

    std::vector<V4L2Device*> pollDevs, readyDevices;
#ifdef HAS_MULTI_INPUT_DEVICE
    for (const auto& device : mDevices) {
        pollDevs.push_back(device->getV4l2Device());
        LOG2("@%s: device:%s has %d buffers queued.", __func__,
             device->getName(), device->getBufferNumInDevice());
    }
#else
    if (mDevice) {
        pollDevs.push_back(mDevice->getV4l2Device());
        LOG2("@%s: device:%s has %d buffers queued.", __func__,
             mDevice->getName(), mDevice->getBufferNumInDevice());
    }
#endif

    while (timeOutCount-- && ret == 0) {
        // If stream off, no poll needed.
        if (mExitPending) {
            LOG2("%s: mExitPending is true, exit", __func__);
            //Exiting, no error
            return -1;
        }

        V4L2DevicePoller poller {pollDevs, mFlushFd[0]};
        ret = poller.Poll(poll_timeout, POLLPRI | POLLIN | POLLOUT | POLLERR, &readyDevices);

        LOG2("@%s: automation checkpoint: flag: poll_buffer, ret:%d", __func__, ret);
    }

    //In case poll error after stream off
    if (mExitPending) {
        LOG2("%s: mExitPending is true, exit", __func__);
        //Exiting, no error
        return -1;
    }

    CheckAndLogError(ret < 0, UNKNOWN_ERROR, "%s: Poll error, ret:%d", __func__, ret);

    if (ret == 0) {
        LOG1("%s, cameraId: %d: timeout happens, wait recovery", __func__, mCameraId);
        return OK;
    }

    for (const auto& readyDevice : readyDevices) {
#ifdef HAS_MULTI_INPUT_DEVICE
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
#else
        if (mDevice && mDevice->getV4l2Device() == readyDevice) {
            int ret = mDevice->dequeueBuffer();
            if (mExitPending) return -1;

            if (ret != OK) {
                LOGE("Device:%s grab frame failed:%d", mDevice->getName(), ret);
            }
            break;
        }
#endif
    }

    return OK;
}

void CaptureUnit::addFrameAvailableListener(BufferConsumer *listener)
{
    LOG1("%s camera id:%d", __func__, mCameraId);

    AutoMutex   l(mLock);
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->addFrameListener(listener);
    }
#else
    if (mDevice) {
        mDevice->addFrameListener(listener);
    }
#endif
}

void CaptureUnit::removeFrameAvailableListener(BufferConsumer *listener)
{
    LOG1("%s camera id:%d", __func__, mCameraId);

    AutoMutex   l(mLock);
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->removeFrameListener(listener);
    }
#else
    if (mDevice) {
        mDevice->removeFrameListener(listener);
    }
#endif
}

void CaptureUnit::removeAllFrameAvailableListener()
{
    LOG1("%s camera id:%d", __func__, mCameraId);

    AutoMutex   l(mLock);
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->removeAllFrameListeners();
    }
#else
    if (mDevice) {
        mDevice->removeAllFrameListeners();
    }
#endif
}

void CaptureUnit::registerListener(EventType eventType, EventListener* eventListener)
{
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->registerListener(eventType, eventListener);
    }
#else
    if (mDevice) {
        mDevice->registerListener(eventType, eventListener);
    }
#endif
}

void CaptureUnit::removeListener(EventType eventType, EventListener* eventListener)
{
#ifdef HAS_MULTI_INPUT_DEVICE
    for (auto device : mDevices) {
        device->removeListener(eventType, eventListener);
    }
#else
    if (mDevice) {
        mDevice->removeListener(eventType, eventListener);
    }
#endif
}
} // namespace icamera

