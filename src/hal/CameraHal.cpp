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

#define LOG_TAG CameraHal

#include <vector>

#include "iutils/CameraLog.h"

#include "ICamera.h"
#include "PlatformData.h"
#include "SyncManager.h"
#include "CameraHal.h"
#include "Parameters.h"

namespace icamera {

#define checkCameraDevice(device, err_code) \
    do { \
        if (mState == HAL_UNINIT) { \
            LOGE("HAL is not init."); \
            return err_code; \
        } \
        if (!(device)) { \
            LOGE("device is not open."); \
            return err_code; \
        } \
    } while (0)

CameraHal::CameraHal() :
    mInitTimes(0),
    mState(HAL_UNINIT),
    mCameraOpenNum(0)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s", __func__);

    CLEAR(mCameraDevices);
    // VIRTUAL_CHANNEL_S
    CLEAR(mTotalVirtualChannelCamNum);
    CLEAR(mConfigTimes);
    // VIRTUAL_CHANNEL_E
}

CameraHal::~CameraHal()
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s", __func__);
}

int CameraHal::init()
{
    LOG1("@%s", __func__);
    AutoMutex lock(mLock);

    if (mInitTimes++ > 0) {
        LOGD("@%s, mInitTimes:%d, return without running", __func__, mInitTimes);
        return OK;
    }

    int ret = PlatformData::init();
    CheckAndLogError(ret != OK, NO_INIT, "PlatformData init failed");

    // VIRTUAL_CHANNEL_S
    for (int i = 0; i < MAX_VC_GROUP_NUMBER; i++) {
        mTotalVirtualChannelCamNum[i] = 0;
        mConfigTimes[i] = 0;
    }
    // VIRTUAL_CHANNEL_E

    mState = HAL_INIT;

    return OK;
}

int CameraHal::deinit()
{
    LOG1("@%s", __func__);
    AutoMutex l(mLock);

    if (--mInitTimes > 0) {
        LOGD("@%s, mInitTimes:%d, return without set state", __func__, mInitTimes);
        return OK;
    }

    // VIRTUAL_CHANNEL_S
    for (int i = 0; i < MAX_VC_GROUP_NUMBER; i++) {
        mTotalVirtualChannelCamNum[i] = 0;
        mConfigTimes[i] = 0;
    }
    // VIRTUAL_CHANNEL_E

    // SyncManager is used to do synchronization with multi-devices.
    // Release it when the last device exit
    SyncManager::releaseInstance();
    // Release the PlatformData instance here due to it was
    // created in init() period
    PlatformData::releaseInstance();

#ifdef CAMERA_TRACE
    CameraTrace::closeDevice();
#endif

    mState = HAL_UNINIT;

    return OK;
}

#ifdef NO_VIRTUAL_CHANNEL
int CameraHal::deviceOpen(int cameraId)
{
    LOG1("@%s, camera id:%d", __func__, cameraId);
    PERF_CAMERA_ATRACE();

    AutoMutex l(mLock);
    CheckAndLogError(mState == HAL_UNINIT, NO_INIT,"HAL is not initialized");

    //Create the camera device that will be freed in close
    if (mCameraDevices[cameraId]) {
        LOGD("@%s: open multi times", __func__);
        return INVALID_OPERATION;
    }

#ifdef SUPPORT_MULTI_PROCESS
    if (mCameraShm.CameraDeviceOpen(cameraId) != OK)
        return INVALID_OPERATION;
#endif

    mCameraDevices[cameraId] = new CameraDevice(cameraId);

#ifdef SUPPORT_MULTI_PROCESS
    // The if check is to handle dual camera cases
    mCameraOpenNum = mCameraShm.cameraDeviceOpenNum();
    CheckAndLogError(mCameraOpenNum == 0, INVALID_OPERATION, "camera open num couldn't be 0");
#else
    mCameraOpenNum++;
#endif

    if (mCameraOpenNum == 1) {
        MediaControl *mc = MediaControl::getInstance();
        CheckAndLogError(!mc, UNKNOWN_ERROR, "%s, MediaControl init failed", __func__);
        mc->resetAllLinks();
    }

    return mCameraDevices[cameraId]->init();
}
#else
int CameraHal::deviceOpen(int cameraId, int vcNum)
{
    LOG1("@%s, camera id:%d, vcNum:%d", __func__, cameraId, vcNum);
    LOG2("SENSORCTRLINFO: channel_id=%d", cameraId);
    PERF_CAMERA_ATRACE();

    AutoMutex l(mLock);
    CheckAndLogError(mState == HAL_UNINIT, NO_INIT,"HAL is not initialized");

    //Create the camera device that will be freed in close
    if (mCameraDevices[cameraId]) {
        LOGD("@%s: open multi times", __func__);
        return INVALID_OPERATION;
    }

#ifdef SUPPORT_MULTI_PROCESS
    if (mCameraShm.CameraDeviceOpen(cameraId) != OK)
        return INVALID_OPERATION;
#endif

    mCameraDevices[cameraId] = new CameraDevice(cameraId);

    // VIRTUAL_CHANNEL_S
    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(cameraId, info);
    int groupId = info.vc.group >= 0? info.vc.group: 0;
    mTotalVirtualChannelCamNum[groupId] = vcNum;
    // VIRTUAL_CHANNEL_E

#ifdef SUPPORT_MULTI_PROCESS
    // The check is to handle dual camera cases
    mCameraOpenNum = mCameraShm.cameraDeviceOpenNum();
    CheckAndLogError(mCameraOpenNum == 0, INVALID_OPERATION, "camera open num couldn't be 0");
#else
    mCameraOpenNum++;
#endif

    if (mCameraOpenNum == 1) {
        MediaControl *mc = MediaControl::getInstance();
        CheckAndLogError(!mc, UNKNOWN_ERROR, "%s, MediaControl init failed", __func__);
        mc->resetAllLinks();

        // VIRTUAL_CHANNEL_S
        if (info.vc.total_num) {
            // when the sensor belongs to virtual channel, reset the routes
            mc->resetAllRoutes(cameraId);
        }
        // VIRTUAL_CHANNEL_E
    }

    return mCameraDevices[cameraId]->init();
}
#endif

void CameraHal::deviceClose(int cameraId)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, camera id:%d", __func__, cameraId);
    AutoMutex l(mLock);

    if (mCameraDevices[cameraId]) {
        mCameraDevices[cameraId]->deinit();
        delete mCameraDevices[cameraId];
        mCameraDevices[cameraId] = nullptr;

#ifdef SUPPORT_MULTI_PROCESS
        mCameraShm.CameraDeviceClose(cameraId);
#else
        mCameraOpenNum--;
#endif
    }
}

void CameraHal::deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback)
{
    LOG1("@%s", __func__);
    AutoMutex l(mLock);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, VOID_VALUE);
#ifdef ENABLE_SANDBOXING
    IntelAlgoClient::getInstance()->registerErrorCallback(callback);
#endif
    device->callbackRegister(callback);
}

// Assume the inputConfig is already checked in upper layer
int CameraHal::deviceConfigInput(int cameraId, const stream_t *inputConfig)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, camera id:%d", __func__, cameraId);
    AutoMutex lock(mLock);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    device->configureInput(inputConfig);

    return OK;
}

// Assume the streamList is already checked in upper layer
int CameraHal::deviceConfigStreams(int cameraId, stream_config_t *streamList)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, camera id:%d", __func__, cameraId);
    AutoMutex lock(mLock);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    int ret = device->configure(streamList);
    if (ret != OK) {
        LOGE("failed to config streams.");
        return INVALID_OPERATION;
    }

    // VIRTUAL_CHANNEL_S
    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(cameraId, info);
    int groupId = info.vc.group >= 0? info.vc.group: 0;
    if (mTotalVirtualChannelCamNum[groupId] > 0) {
        mConfigTimes[groupId]++;
        LOG1("@%s, camera id:%d, mConfigTimes:%d, before signal", __func__, cameraId, mConfigTimes[groupId]);
        mVirtualChannelSignal[groupId].signal();
    }
    // VIRTUAL_CHANNEL_E

    return ret;
}

int CameraHal::deviceStart(int cameraId)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, cameraId is %d", __func__, cameraId);

    ConditionLock lock(mLock);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    // VIRTUAL_CHANNEL_S
    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(cameraId, info);
    int groupId = info.vc.group >= 0? info.vc.group: 0;
    LOG1("@%s, cameraId is %d, mConfigTimes:%d, mTotalVirtualChannelCamNum:%d",
        __func__, cameraId, mConfigTimes[groupId], mTotalVirtualChannelCamNum[groupId]);

    if (mTotalVirtualChannelCamNum[groupId] > 0) {
        int timeoutCnt = 10;
        while (mConfigTimes[groupId] < mTotalVirtualChannelCamNum[groupId]) {
            mVirtualChannelSignal[groupId].waitRelative(lock, mWaitDuration * SLOWLY_MULTIPLIER);
            LOG1("@%s, cameraId is %d, mConfigTimes:%d, timeoutCnt:%d",
                  __func__, cameraId, mConfigTimes[groupId], timeoutCnt);
            --timeoutCnt;
            CheckAndLogError(!timeoutCnt, TIMED_OUT, "@%s, cameraId is %d, mConfigTimes:%d, wait time out",
                             __func__, cameraId, mConfigTimes[groupId]);
        }
    }
    // VIRTUAL_CHANNEL_E

    return device->start();
}

int CameraHal::deviceStop(int cameraId)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, cameraId is %d", __func__, cameraId);

    AutoMutex lock(mLock);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->stop();
}

int CameraHal::deviceAllocateMemory(int cameraId, camera_buffer_t *ubuffer)
{
    PERF_CAMERA_ATRACE();
    LOG1("@%s, cameraId is %d", __func__, cameraId);

    CameraDevice *device = mCameraDevices[cameraId];

    checkCameraDevice(device, BAD_VALUE);

    return device->allocateMemory(ubuffer);
}

int CameraHal::streamQbuf(int cameraId, camera_buffer_t **ubuffer,
                          int bufferNum, const Parameters* settings)
{
    PERF_CAMERA_ATRACE();
    LOG2("@%s, cameraId is %d, fd:%d", __func__, cameraId, (*ubuffer)->dmafd);

    CameraDevice *device = mCameraDevices[cameraId];

    checkCameraDevice(device, BAD_VALUE);

    return device->qbuf(ubuffer, bufferNum, settings);
}

int CameraHal::streamDqbuf(int cameraId, int streamId, camera_buffer_t **ubuffer,
                           Parameters* settings)
{
    PERF_CAMERA_ATRACE();
    LOG2("@%s, cameraId is %d, streamId is %d", __func__, cameraId, streamId);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->dqbuf(streamId, ubuffer, settings);
}

int CameraHal::getParameters(int cameraId, Parameters& param, long sequence)
{
    LOG1("@%s, cameraId is %d", __func__, cameraId);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->getParameters(param, sequence);
}

int CameraHal::setParameters(int cameraId, const Parameters& param)
{
    LOG1("@%s, cameraId is %d", __func__, cameraId);

    CameraDevice *device = mCameraDevices[cameraId];
    checkCameraDevice(device, BAD_VALUE);

    return device->setParameters(param);
}

} // namespace icamera
