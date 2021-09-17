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

int MockCameraHal::deviceOpen(int cameraId)
{
    LOG2("@%s, camera id:%d", __func__, cameraId);

    CheckAndLogError(mState == HAL_UNINIT, NO_INIT, "HAL is not initialized");
    if (mCameraDevices[cameraId]) {
        LOG2("@%s: open multi times", __func__);
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
    LOG2("@%s, camera id:%d", __func__, cameraId);
    std::unique_lock<std::mutex> lock(mLock);

    if (mCameraDevices[cameraId]) {
        mCameraDevices[cameraId] = 0;
        mCameraOpenNum--;
    }
}

void MockCameraHal::deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback) {
    LOG2("@%s, camera id:%d", __func__, cameraId);
    std::unique_lock<std::mutex> lock(mLock);

    mCallback[cameraId] = const_cast<camera_callback_ops_t*>(callback);
    return;
}

int MockCameraHal::deviceConfigInput(int cameraId, const stream_t* inputConfig) {
    LOG2("@%s, camera id:%d", __func__, cameraId);

    return OK;
}

int MockCameraHal::deviceConfigStreams(int cameraId, stream_config_t* streamList) {
    LOG2("@%s, camera id:%d", __func__, cameraId);

    for (int i = 0; i < streamList->num_streams; i++) {
        streamList->streams[i].max_buffers = 6;
        streamList->streams[i].id = i;
    }

    return OK;
}

int MockCameraHal::deviceStart(int cameraId) {
    std::unique_lock<std::mutex> lock(mLock);
    if (mCameraDevices[cameraId] && !Thread::isRunning()) {
        LOG2("@%s, cameraId is %d", __func__, cameraId);
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
    LOG2("@%s, cameraId is %d", __func__, cameraId);

    Thread::requestExitAndWait();
    std::unique_lock<std::mutex> lock(mLock);
    while (!mCaptureRequest[cameraId].empty()) generateFrames(cameraId);
    return OK;
}

int MockCameraHal::deviceAllocateMemory(int cameraId, camera_buffer_t* ubuffer) {
    LOG2("@%s, cameraId is %d", __func__, cameraId);

    return OK;
}

int MockCameraHal::streamQbuf(int cameraId, camera_buffer_t** ubuffer, int bufferNum,
                              const Parameters* settings) {
    LOG2("@%s, cameraId is %d buffer Num %d", __func__, cameraId, bufferNum);

    std::unique_lock<std::mutex> lock(mLock);

    std::shared_ptr<CaptureRequest> req = std::make_shared<CaptureRequest>();
    req->streamNum = bufferNum;
    req->sequence = mFrameSequence[cameraId];
    for (int i = 0; i < bufferNum; i++) {
        req->ubuffer[i] = ubuffer[i];
        LOG2("@%s, stream id %d req buffer %p", __func__, ubuffer[i]->s.id, ubuffer[i]->addr);
    }
    mCaptureRequest[cameraId].push_back(req);
    mFrameSequence[cameraId]++;
    return OK;
}

int MockCameraHal::streamDqbuf(int cameraId, int streamId, camera_buffer_t** ubuffer,
                               Parameters* settings) {
    LOG2("@%s, cameraId is %d streamId %d", __func__, cameraId, streamId);

    std::unique_lock<std::mutex> lock(mLock);
    while (mCaptureResult[cameraId][streamId].empty()) {
        LOG2("@%s, camera %d stream %d no buffer ready, wait", __func__, cameraId, streamId);
        mBufferReadyCondition[cameraId].wait_for(
            lock, std::chrono::nanoseconds(kMaxDuration * SLOWLY_MULTIPLIER));
    }

    *ubuffer = mCaptureResult[cameraId][streamId].at(0);
    mCaptureResult[cameraId][streamId].pop_front();
    *settings = mParameter[cameraId];
    LOG2("@%s, cameraId return buffer %p", __func__, ubuffer[0]->addr);

    return OK;
}

int MockCameraHal::getParameters(int cameraId, Parameters& param, long sequence) {
    LOG2("@%s, cameraId is %d", __func__, cameraId);
    param = mParameter[cameraId];
    return OK;
}

int MockCameraHal::setParameters(int cameraId, const Parameters& param) {
    LOG2("@%s, cameraId is %d", __func__, cameraId);
    return OK;
}

void MockCameraHal::generateFrames(int cameraId) {
    if (mCaptureRequest[cameraId].empty()) return;

    std::shared_ptr<CaptureRequest> req = nullptr;
    req = mCaptureRequest[cameraId].at(0);

    if (req) {
        if (mCallback[cameraId]) {
            camera_msg_data_t data;
            data.type = CAMERA_ISP_BUF_READY;
            data.data.buffer_ready.sequence = req->sequence;
            data.data.buffer_ready.timestamp = mTimestamp;
            data.data.buffer_ready.frameNumber = static_cast<uint32_t>(req->sequence);
            mCallback[cameraId]->notify(mCallback[cameraId], data);
        }

        for (int i = 0; i < req->streamNum; i++) {
            int streamId = req->ubuffer[i]->s.id;
            LOG2("@%s, generate result seq %d id %d buffers %p", __func__, req->sequence, streamId,
                 req->ubuffer[i]->addr);
            req->ubuffer[i]->timestamp = mTimestamp;
            mCaptureResult[cameraId][streamId].push_back(req->ubuffer[i]);
        }

        mCaptureRequest[cameraId].pop_front();
        mBufferReadyCondition[cameraId].notify_all();
    }
}

bool MockCameraHal::threadLoop() {
    LOG2("@%s, ", __func__);

    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    uint64_t currentTimestamp = currentTime.tv_sec * 1000000000 + currentTime.tv_nsec;
    if (!Thread::isExiting()) usleep(33000);

    // assume frame is generated 10ms earlier
    mTimestamp = currentTimestamp - 100000000;
    for (int cameraId = 0; cameraId < MAX_CAMERA_NUMBER; cameraId++) {
        std::unique_lock<std::mutex> lock(mLock);
        if (mCaptureRequest[cameraId].empty()) continue;
        generateFrames(cameraId);
    }
    return true;
}

int MockCameraHal::initDefaultParameters(int cameraId) {
    LOG1("@%s cameraId:%d", __func__, cameraId);

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
