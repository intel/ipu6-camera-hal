/*
 * Copyright (C) 2021 Intel Corporation.
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

#define LOG_TAG MockCameraHal

#include "src/hal/MockCameraHal.h"

#include <sys/mman.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "ICamera.h"
#include "Parameters.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"

namespace icamera {

MockCameraHal::MockCameraHal()
        : mInitTimes(0),
          mState(HAL_UNINIT),
          mCameraOpenNum(0),
          mTimestamp(0) {
    LOG2("@%s", __func__);
    CLEAR(mCameraDevices);
    CLEAR(mFrameSequence);
    CLEAR(mCallback);
}

MockCameraHal::~MockCameraHal() {
    LOG2("@%s", __func__);
}

int MockCameraHal::init() {
    LOG2("@%s", __func__);
    std::unique_lock<std::mutex> lock(mLock);

    if (mInitTimes++ > 0) {
        LOG2("@%s, mInitTimes:%d, return without running", __func__, mInitTimes);
        return OK;
    }

    int ret = PlatformData::init();
    CheckAndLogError(ret != OK, NO_INIT, "PlatformData init failed");

    mState = HAL_INIT;

    return OK;
}

int MockCameraHal::deinit() {
    LOG2("@%s", __func__);
    std::unique_lock<std::mutex> lock(mLock);

    if (--mInitTimes > 0) {
        LOG2("@%s, mInitTimes:%d, return without set state", __func__, mInitTimes);
        return OK;
    }

    // Release the PlatformData instance here due to it was
    // created in init() period
    PlatformData::releaseInstance();

    mState = HAL_UNINIT;

    return OK;
}

#ifdef NO_VIRTUAL_CHANNEL
int MockCameraHal::deviceOpen(int cameraId)
#else
int MockCameraHal::deviceOpen(int cameraId, int vcNum)
#endif
{
    LOG1("<id%d>@%s", cameraId, __func__);

    CheckAndLogError(mState == HAL_UNINIT, NO_INIT, "HAL is not initialized");
    if (mCameraDevices[cameraId]) {
        LOG1("@%s: open multi times", __func__);
        return INVALID_OPERATION;
    }

    std::unique_lock<std::mutex> lock(mLock);
    for (int streamId = 0; streamId < kMaxStreamNum; streamId++)
        mCaptureResult[cameraId][streamId].clear();
    mCaptureRequest[cameraId].clear();
    mFrameSequence[cameraId] = 0;

    mCameraOpenNum++;
    mCameraDevices[cameraId] = 1;

    initDefaultParameters(cameraId);
    return OK;
}

void MockCameraHal::deviceClose(int cameraId) {
    std::unique_lock<std::mutex> lock(mLock);

    if (mCameraDevices[cameraId]) {
        mCameraDevices[cameraId] = 0;
        mCameraOpenNum--;
    }
    LOG1("<id%d>@%s", cameraId, __func__);
}

void MockCameraHal::deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback) {
    LOG1("<id%d>@%s, callback:%p", cameraId, __func__, callback);
    std::unique_lock<std::mutex> lock(mLock);

    // report unfinished frame shutterDone and metadataDone when user request stop camera
    if (callback == nullptr) {
        uint64_t timeStamp = mTimestamp;
        for (auto request = mCaptureRequest[cameraId].begin();
             request != mCaptureRequest[cameraId].end(); request++) {
            LOG2("<req%d> @%s, ISP ready", (*request)->sequence, __func__);
            camera_msg_data_t shutter;
            shutter.type = CAMERA_ISP_BUF_READY;
            shutter.data.buffer_ready.timestamp = timeStamp;
            shutter.data.buffer_ready.frameNumber = static_cast<uint32_t>((*request)->sequence);
            mCallback[cameraId]->notify(mCallback[cameraId], shutter);
            camera_msg_data_t metadata;
            metadata.type = CAMERA_METADATA_READY;
            metadata.data.metadata_ready.sequence = (*request)->sequence;
            metadata.data.metadata_ready.frameNumber = static_cast<uint32_t>((*request)->sequence);
            mCallback[cameraId]->notify(mCallback[cameraId], metadata);
            timeStamp += 33000000;
        }
    }
    mCallback[cameraId] = const_cast<camera_callback_ops_t*>(callback);
    return;
}

int MockCameraHal::deviceConfigInput(int cameraId, const stream_t* inputConfig) {
    return OK;
}

int MockCameraHal::deviceConfigStreams(int cameraId, stream_config_t* streamList) {
    LOG1("<id%d>@%s", cameraId, __func__);

    for (int i = 0; i < streamList->num_streams; i++) {
        streamList->streams[i].max_buffers = 6;
        streamList->streams[i].id = i;
    }

    return OK;
}

int MockCameraHal::deviceStart(int cameraId) {
    std::unique_lock<std::mutex> lock(mLock);

    if (mCameraDevices[cameraId] && !Thread::isRunning()) {
        LOG1("<id%d>@%s", cameraId, __func__);
        std::string threadName = "MockStream";
        threadName += std::to_string(cameraId);

        run(threadName);
    }

    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    mTimestamp = currentTime.tv_sec * 1000000000 + currentTime.tv_nsec;
    return OK;
}

int MockCameraHal::deviceStop(int cameraId) {
    Thread::requestExitAndWait();
    while (!mCaptureRequest[cameraId].empty()) generateFrames(cameraId);
    LOG1("<id%d>@%s", cameraId, __func__);
    return OK;
}

int MockCameraHal::deviceAllocateMemory(int cameraId, camera_buffer_t* ubuffer) {
    return OK;
}

int MockCameraHal::streamQbuf(int cameraId, camera_buffer_t** ubuffer, int bufferNum,
                              const Parameters* settings) {
    LOG2("<id%d:req%d>@%s, buffer Num %d", cameraId, mFrameSequence[cameraId], __func__, bufferNum);

    std::unique_lock<std::mutex> lock(mLock);

    std::shared_ptr<CaptureRequest> req = std::make_shared<CaptureRequest>();
    req->streamNum = bufferNum;
    req->sequence = mFrameSequence[cameraId];
    for (int i = 0; i < bufferNum; i++) {
        req->ubuffer[i] = ubuffer[i];
    }
    mCaptureRequest[cameraId].push_back(req);
    mFrameSequence[cameraId]++;
    return OK;
}

int MockCameraHal::streamDqbuf(int cameraId, int streamId, camera_buffer_t** ubuffer,
                               Parameters* settings) {
    std::unique_lock<std::mutex> lock(mLock);
    while (mCaptureResult[cameraId][streamId].empty()) {
        mBufferReadyCondition[cameraId][streamId].wait_for(lock,
                                                           std::chrono::nanoseconds(kMaxDuration));

        if (mCaptureResult[cameraId][streamId].empty())
            LOGW("<id%d>@%s, wait buffer %d ready timeout", cameraId, __func__, streamId);
    }

    *ubuffer = mCaptureResult[cameraId][streamId].at(0);
    LOG2("<id%d>@%s, stream %d", cameraId, __func__, streamId);
    mCaptureResult[cameraId][streamId].pop_front();
    if (settings) *settings = mParameter[cameraId];

    return OK;
}

int MockCameraHal::getParameters(int cameraId, Parameters& param, int64_t sequence) {
    param = mParameter[cameraId];
    return OK;
}

int MockCameraHal::setParameters(int cameraId, const Parameters& param) {
    return OK;
}

void MockCameraHal::generateFrames(int cameraId) {
    std::shared_ptr<CaptureRequest> req = nullptr;
    {
        std::unique_lock<std::mutex> lock(mLock);
        if (mCaptureRequest[cameraId].empty()) return;
        req = mCaptureRequest[cameraId].at(0);
    }
    if (req) {
        if (mCallback[cameraId]) {
            camera_msg_data_t shutter;
            shutter.type = CAMERA_ISP_BUF_READY;
            shutter.data.buffer_ready.timestamp = mTimestamp;
            shutter.data.buffer_ready.frameNumber = static_cast<uint32_t>(req->sequence);
            mCallback[cameraId]->notify(mCallback[cameraId], shutter);

            camera_msg_data_t metadata;
            metadata.type = CAMERA_METADATA_READY;
            metadata.data.metadata_ready.sequence = req->sequence;
            metadata.data.metadata_ready.frameNumber = static_cast<uint32_t>(req->sequence);
            mCallback[cameraId]->notify(mCallback[cameraId], metadata);
            LOG2("<req%d> @%s, ISP ready", req->sequence, __func__);
        }

        for (int i = 0; i < req->streamNum; i++) {
            int streamId = req->ubuffer[i]->s.id;
            LOG2("<req%d> @%s, id %d", req->sequence, __func__, streamId);
            req->ubuffer[i]->timestamp = mTimestamp;
            std::unique_lock<std::mutex> lock(mLock);
            mCaptureResult[cameraId][streamId].push_back(req->ubuffer[i]);
            mBufferReadyCondition[cameraId][streamId].notify_one();
        }
        {
            std::unique_lock<std::mutex> lock(mLock);
            mCaptureRequest[cameraId].pop_front();
        }
    }
}

bool MockCameraHal::threadLoop() {
    if (Thread::isExiting()) return true;

    for (int cameraId = 0; cameraId < MAX_CAMERA_NUMBER; cameraId++) generateFrames(cameraId);

    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    uint64_t currentTimestamp = currentTime.tv_sec * 1000000000 + currentTime.tv_nsec;
    int waitTimeNs = mTimestamp + 33000000 - currentTimestamp;

    if (waitTimeNs > 1000) usleep(waitTimeNs / 1000);
    mTimestamp = currentTimestamp + waitTimeNs;
    return true;
}

int MockCameraHal::initDefaultParameters(int cameraId) {
    camera_info_t info;
    CLEAR(info);
    PlatformData::getCameraInfo(cameraId, info);

    mParameter[cameraId] = *info.capability;

    camera_range_t fps = {10, 30};
    mParameter[cameraId].setFpsRange(fps);
    mParameter[cameraId].setFrameRate(30);

    camera_image_enhancement_t enhancement;
    CLEAR(enhancement);  // All use 0 as default
    mParameter[cameraId].setImageEnhancement(enhancement);

    mParameter[cameraId].setWeightGridMode(WEIGHT_GRID_AUTO);

    mParameter[cameraId].setWdrLevel(100);

    mParameter[cameraId].setFlipMode(FLIP_MODE_NONE);

    mParameter[cameraId].setRun3ACadence(1);

    mParameter[cameraId].setYuvColorRangeMode(PlatformData::getYuvColorRangeMode(cameraId));

    mParameter[cameraId].setFocusDistance(0.0f);
    mParameter[cameraId].setTonemapMode(TONEMAP_MODE_FAST);

    return OK;
}
}  // namespace icamera
