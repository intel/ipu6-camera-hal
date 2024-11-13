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

#define LOG_TAG CameraHal

#include "CameraHal.h"

#include <vector>

#include "ICamera.h"
#include "Parameters.h"
#include "PlatformData.h"
// FRAME_SYNC_S
#include "SyncManager.h"
// FRAME_SYNC_E
#include "iutils/CameraLog.h"

namespace icamera {

#define checkCameraDevice(device, err_code) \
    do {                                    \
        if (mState == HAL_UNINIT) {         \
            LOGE("HAL is not init.");       \
            return err_code;                \
        }                                   \
        if (!(device)) {                    \
            LOGE("device is not open.");    \
            return err_code;                \
        }                                   \
    } while (0)

CameraHal::CameraHal() : mInitTimes(0), mState(HAL_UNINIT), mCameraOpenNum(0) {
    LOG1("@%s", __func__);

    CLEAR(mCameraDevices);
    // VIRTUAL_CHANNEL_S
    mCurrentGroupId = -1;
    mVcNum = 0;
    mConfigTimes = 0;
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        mDeviceClosing[i] = false;
    }
    // VIRTUAL_CHANNEL_E
}

CameraHal::~CameraHal() {
    LOG1("@%s", __func__);
}

int CameraHal::init() {
    LOG1("@%s", __func__);
    PERF_CAMERA_ATRACE();
    AutoMutex lock(mLock);

    if (mInitTimes++ > 0) {
        LOGI("already initialized, mInitTimes:%d", mInitTimes);
        return OK;
    }

    int ret = PlatformData::init();
    CheckAndLogError(ret != OK, NO_INIT, "PlatformData init failed");

    // VIRTUAL_CHANNEL_S
    mCurrentGroupId = -1;
    mVcNum = 0;
    mConfigTimes = 0;
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        mDeviceClosing[i] = false;
    }
    // VIRTUAL_CHANNEL_E
    mState = HAL_INIT;

    return OK;
}

int CameraHal::deinit() {
    LOG1("@%s", __func__);
    PERF_CAMERA_ATRACE();
    AutoMutex l(mLock);

    if (--mInitTimes > 0) {
        LOGI("CameraHal still running, mInitTimes:%d", mInitTimes);
        return OK;
    }

    // VIRTUAL_CHANNEL_S
    mVcNum = 0;
    mCurrentGroupId = -1;
    mConfigTimes = 0;
    for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
        mDeviceClosing[i] = false;
    }
    // VIRTUAL_CHANNEL_E

    // FRAME_SYNC_S
    // SyncManager is used to do synchronization with multi-devices.
    // Release it when the last device exit
    SyncManager::releaseInstance();
    // FRAME_SYNC_E
    // Release the PlatformData instance here due to it was
    // created in init() period
    PlatformData::releaseInstance();

#ifdef CAMERA_TRACE
    CameraTrace::closeDevice();
#endif

    mState = HAL_UNINIT;

    return OK;
}

int CameraHal::deviceOpen(int cameraId, int vcNum) {
    LOG1("<id%d> @%s SENSORCTRLINFO: vcNum %d", cameraId, __func__, vcNum);
    AutoMutex l(mLock);
    CheckAndLogError(mState == HAL_UNINIT, NO_INIT, "HAL is not initialized");

    // Create the camera device that will be freed in close
    if (mCameraDevices[cameraId]) {
        LOGI("<id%d> has already opened", cameraId);
        return INVALID_OPERATION;
    }

    // VIRTUAL_CHANNEL_S
    CheckAndLogError(mCameraOpenNum && vcNum != mVcNum, INVALID_OPERATION,
                     "New vcNum %d dismatch the previous %d", vcNum, mVcNum);

    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(cameraId, info);
    if (info.vc.total_num) {
        // Open as vc sensor
        int groupId = info.vc.group >= 0 ? info.vc.group : 0;
        CheckAndLogError(mCurrentGroupId >= 0 && groupId != mCurrentGroupId, INVALID_OPERATION,
                         "Open group %d fail because group %d already opened!", groupId,
                         mCurrentGroupId);
        mCurrentGroupId = groupId;
    }
    mVcNum = vcNum;
    // VIRTUAL_CHANNEL_E

    if (mCameraShm.CameraDeviceOpen(cameraId) != OK) return INVALID_OPERATION;
    mCameraDevices[cameraId] = new CameraDevice(cameraId);
    // The check is to handle dual camera cases
    mCameraOpenNum = mCameraShm.cameraDeviceOpenNum();
    CheckAndLogError(mCameraOpenNum == 0, INVALID_OPERATION, "camera open num couldn't be 0");

    if (mCameraOpenNum == 1) {
        MediaControl* mc = MediaControl::getInstance();
        CheckAndLogError(!mc, UNKNOWN_ERROR, "MediaControl init failed");

        if (PlatformData::isResetLinkRoute(cameraId)) {
            int ret = mc->resetAllLinks();
            CheckAndLogError(ret != OK, DEV_BUSY, "resetAllLinks failed");
        }
        // VIRTUAL_CHANNEL_S
        if (info.vc.total_num) {
            // when the sensor belongs to virtual channel, reset the routes
            if (PlatformData::isResetLinkRoute(cameraId)) mc->resetAllRoutes(cameraId);
        }
        // VIRTUAL_CHANNEL_E
    }

    return mCameraDevices[cameraId]->init();
}

void CameraHal::deviceClose(int cameraId) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex l(mLock);

    if (mCameraDevices[cameraId]) {
        if (mVcNum <= 0) {
            mCameraDevices[cameraId]->deinit();
            delete mCameraDevices[cameraId];
            mCameraDevices[cameraId] = nullptr;
            mCameraOpenNum--;
        } else if (!mDeviceClosing[cameraId]) {
            // only deinit vc camera here
            mCameraDevices[cameraId]->deinit();
            mCameraOpenNum--;
            mDeviceClosing[cameraId] = true;
        }
        mCameraShm.CameraDeviceClose(cameraId);
    }
    // VIRTUAL_CHANNEL_S
    // Destroy all closed vc cameras
    if (mVcNum > 0 && mCameraOpenNum == 0) {
        for (int i = 0; i < MAX_CAMERA_NUMBER; i++) {
            if (mDeviceClosing[i]) {
                delete mCameraDevices[i];
                mCameraDevices[i] = nullptr;
                mDeviceClosing[i] = false;
            }
        }
        mVcNum = 0;
    }
    // VIRTUAL_CHANNEL_E
}

void CameraHal::deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex l(mLock);

    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, VOID_VALUE);
#ifdef ENABLE_SANDBOXING
    IntelAlgoClient::getInstance()->registerErrorCallback(callback);
#endif
    device->callbackRegister(callback);
}

// Assume the inputConfig is already checked in upper layer
int CameraHal::deviceConfigInput(int cameraId, const stream_t* inputConfig) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex lock(mLock);

    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    device->configureInput(inputConfig);

    return OK;
}

// Assume the streamList is already checked in upper layer
int CameraHal::deviceConfigStreams(int cameraId, stream_config_t* streamList) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex lock(mLock);

    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    int ret = device->configure(streamList);
    if (ret != OK) {
        LOGE("failed to config streams.");
        return INVALID_OPERATION;
    }

    // VIRTUAL_CHANNEL_S
    if (mVcNum > 0) {
        mConfigTimes++;
        LOG1("<id%d> @%s, mConfigTimes:%d, before signal", cameraId, __func__, mConfigTimes);
        mVirtualChannelSignal.signal();
    }
    // VIRTUAL_CHANNEL_E

    return ret;
}

int CameraHal::deviceStart(int cameraId) {
    LOG1("<id%d> @%s", cameraId, __func__);
    ConditionLock lock(mLock);

    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    // VIRTUAL_CHANNEL_S
    if (mVcNum > 0) {
        LOG1("<id%d>@%s, mConfigTimes:%d, mVcNum:%d", cameraId, __func__, mConfigTimes, mVcNum);
        int timeoutCnt = 10;
        while (mConfigTimes < mVcNum) {
            mVirtualChannelSignal.waitRelative(lock, mWaitDuration * SLOWLY_MULTIPLIER);
            LOG1("<id%d> @%s, mConfigTimes:%d, timeoutCnt:%d", cameraId, __func__,
                 mConfigTimes, timeoutCnt);
            --timeoutCnt;
            CheckAndLogError(!timeoutCnt, TIMED_OUT, "<id%d> mConfigTimes:%d, wait time out",
                             cameraId, mConfigTimes);
        }
    }
    // VIRTUAL_CHANNEL_E

    return device->start();
}

int CameraHal::deviceStop(int cameraId) {
    LOG1("<id%d> @%s", cameraId, __func__);
    AutoMutex lock(mLock);

    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->stop();
}

int CameraHal::deviceAllocateMemory(int cameraId, camera_buffer_t* ubuffer) {
    LOG1("<id%d> @%s", cameraId, __func__);
    CameraDevice* device = mCameraDevices[cameraId];

    checkCameraDevice(device, BAD_VALUE);

    return device->allocateMemory(ubuffer);
}

int CameraHal::streamQbuf(int cameraId, camera_buffer_t** ubuffer, int bufferNum,
                          const Parameters* settings) {
    LOG2("<id%d> @%s, fd:%d", cameraId, __func__, (*ubuffer)->dmafd);
    CameraDevice* device = mCameraDevices[cameraId];

    checkCameraDevice(device, BAD_VALUE);

    return device->qbuf(ubuffer, bufferNum, settings);
}

int CameraHal::streamDqbuf(int cameraId, int streamId, camera_buffer_t** ubuffer,
                           Parameters* settings) {
    LOG2("<id%d> @%s, streamId is %d", cameraId, __func__, streamId);
    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->dqbuf(streamId, ubuffer, settings);
}

int CameraHal::getParameters(int cameraId, Parameters& param, int64_t sequence) {
    LOG2("<id%d> @%s", cameraId, __func__);
    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->getParameters(param, sequence);
}

int CameraHal::setParameters(int cameraId, const Parameters& param) {
    LOG2("<id%d> @%s", cameraId, __func__);
    CameraDevice* device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->setParameters(param);
}

}  // namespace icamera
