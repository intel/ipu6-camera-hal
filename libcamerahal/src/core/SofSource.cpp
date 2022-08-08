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
#define LOG_TAG SofSource

#include "SofSource.h"

#include <poll.h>

#include "PlatformData.h"
#include "V4l2DeviceFactory.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

SofSource::SofSource(int cameraId)
        : mPollThread(nullptr),
          mCameraId(cameraId),
          mIsysReceiverSubDev(nullptr),
          mExitPending(false) {
    LOG1("%s: SofSource is constructed", __func__);

    mSofDisabled = !PlatformData::isIsysEnabled(cameraId);
    // FILE_SOURCE_S
    mSofDisabled = mSofDisabled || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E
}

SofSource::~SofSource() {
    LOG1("%s: SofSource is distructed.", __func__);
}

int SofSource::init() {
    if (mSofDisabled) {
        return OK;
    }

    mPollThread = new PollThread(this);

    return OK;
}

int SofSource::deinit() {
    if (mSofDisabled) {
        return OK;
    }

    int status = deinitDev();
    mPollThread->join();
    delete mPollThread;
    return status;
}

int SofSource::initDev() {
    // Create and open receiver subdevice.
    std::string subDeviceNodeName;

    if (PlatformData::getDevNameByType(mCameraId, VIDEO_ISYS_RECEIVER, subDeviceNodeName) == OK) {
        LOG1("%s: found ISYS receiver subdevice %s", __func__, subDeviceNodeName.c_str());
    }

    deinitDev();

    mIsysReceiverSubDev = V4l2DeviceFactory::getSubDev(mCameraId, subDeviceNodeName);

#ifdef CAL_BUILD
    int status = mIsysReceiverSubDev->SubscribeEvent(V4L2_EVENT_FRAME_SYNC);
    CheckAndLogError(status != OK, status, "Failed to subscribe sync event 0");
    LOG1("%s: Using SOF event id 0 for sync", __func__);
#else
    int id = 0;
    // VIRTUAL_CHANNEL_S
    /* The value of virtual channel sequence is 1, 2, 3, ... if virtual channel supported.
       The value of SOF event id is 0, 1, 2, ... (sequence -1)  when virtual channel supported. */
    int sequence = PlatformData::getVirtualChannelSequence(mCameraId);
    if (sequence > 0) {
        id = sequence - 1;
    }
    // VIRTUAL_CHANNEL_E
    int status = mIsysReceiverSubDev->SubscribeEvent(V4L2_EVENT_FRAME_SYNC, id);
    CheckAndLogError(status != OK, status, "Failed to subscribe sync event %d", id);
    LOG1("%s: Using SOF event id %d for sync", __func__, id);
#endif

    return OK;
}

int SofSource::deinitDev() {
    if (mIsysReceiverSubDev == nullptr) return OK;

    int status = 0;
#ifdef CAL_BUILD
    status = mIsysReceiverSubDev->UnsubscribeEvent(V4L2_EVENT_FRAME_SYNC);
    if (status == OK) {
        LOG1("%s: Unsubscribe SOF event id 0 done", __func__);
    } else {
        LOGE("Failed to unsubscribe SOF event 0, status: %d", status);
    }
#else
    int id = 0;
    // VIRTUAL_CHANNEL_S
    /* The value of virtual channel sequence is 1, 2, 3, ... if virtual channel supported.
       The value of SOF event id is 0, 1, 2, ... (sequence -1)  when virtual channel supported. */
    int sequence = PlatformData::getVirtualChannelSequence(mCameraId);
    if (sequence > 0) {
        id = sequence - 1;
    }
    // VIRTUAL_CHANNEL_E
    status = mIsysReceiverSubDev->UnsubscribeEvent(V4L2_EVENT_FRAME_SYNC, id);
    if (status == OK) {
        LOG1("%s: Unsubscribe SOF event id %d done", __func__, id);
    } else {
        LOGE("Failed to unsubscribe SOF event %d", id);
    }
#endif

    return status;
}

int SofSource::configure() {
    if (mSofDisabled) {
        return OK;
    }

    return initDev();
}

int SofSource::start() {
    LOG1("%s", __func__);
    if (mSofDisabled) {
        return OK;
    }

    int status = mPollThread->run("SofSource", PRIORITY_URGENT_AUDIO);
    mExitPending = false;
    return status;
}

int SofSource::stop() {
    LOG1("%s", __func__);
    if (mSofDisabled) {
        return OK;
    }

    mExitPending = true;
    int status = mPollThread->requestExitAndWait();
    return status;
}

int SofSource::poll() {
    int ret = 0;
    const int pollTimeoutCount = 10;
    const int pollTimeout = 1000;

    std::vector<V4L2Device*> pollDevs;
    pollDevs.push_back(mIsysReceiverSubDev);
    V4L2DevicePoller poller{pollDevs, -1};

    std::vector<V4L2Device*> readyDevices;

    int timeOutCount = pollTimeoutCount;

    while (timeOutCount-- && ret == 0) {
        ret = poller.Poll(pollTimeout, POLLPRI | POLLIN | POLLOUT | POLLERR, &readyDevices);

        if (ret == 0 && mExitPending) {
            // timed out
            LOGI("Time out or thread is not running, ret = %d", ret);
            return BAD_VALUE;
        }
    }

    // handle the poll error
    if (ret < 0) {
        if (mExitPending) {
            // Exiting, no error
            return 0;
        }

        LOGE("Poll error");
        return ret;
    } else if (ret == 0) {
        LOGI("Sof poll time out.");
        return 0;
    }

    struct v4l2_event event;
    CLEAR(event);
    mIsysReceiverSubDev->DequeueEvent(&event);

    EventDataSync syncData;
    syncData.sequence = event.u.frame_sync.frame_sequence;
    syncData.timestamp.tv_sec = event.timestamp.tv_sec;
    syncData.timestamp.tv_usec = (event.timestamp.tv_nsec / 1000);
    LOG2("<seq%ld> %s:sof event, event.id %u", syncData.sequence, __func__, event.id);
    TRACE_LOG_POINT("SofSource", "receive sof event", MAKE_COLOR(syncData.sequence),
                    syncData.sequence);
    EventData eventData;
    eventData.type = EVENT_ISYS_SOF;
    eventData.buffer = nullptr;
    eventData.data.sync = syncData;
    notifyListeners(eventData);

    return 0;
}

}  // namespace icamera
