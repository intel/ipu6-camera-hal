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

#pragma once

#include <deque>
#include <memory>

#include "CameraDevice.h"
#include "CameraHal.h"
#include "ParameterGenerator.h"
#include "Parameters.h"

namespace icamera {

/**
 * CameraHal class is the real HAL API.
 * There is only one instance of CameraHal which is created when HAL loading
 * It creates the CameraDevice based on the camera ID to support multi-cameras.
 *
 * The main job of the Class is
 * 1. Maintain a list of CameraDevice
 * 2. Pass the Camera HAL API to the correct CameraDevice based on CameraId
 *
 * If open dual cameras in different process, the shared memory must be used to
 * keep the account of the open times.
 *
 * The CameraHal create and maintains followings singleton instancs
 * 1. MediaControl Instance
 * 2. PlatformData Instance
 */

class MockCameraHal : public CameraHal, public Thread {
    // HAL API
 public:
    MockCameraHal();
    virtual ~MockCameraHal();
    virtual int init();
    virtual int deinit();

    // Device API
 public:
#ifdef NO_VIRTUAL_CHANNEL
    virtual int deviceOpen(int cameraId);
#else
    virtual int deviceOpen(int cameraId, int vcNum = 0);
#endif
    virtual void deviceClose(int cameraId);

    virtual void deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback);
    virtual int deviceConfigInput(int cameraId, const stream_t* inputConfig);
    virtual int deviceConfigStreams(int cameraId, stream_config_t* streamList);
    virtual int deviceStart(int cameraId);
    virtual int deviceStop(int cameraId);
    virtual int deviceAllocateMemory(int cameraId, camera_buffer_t* ubuffer);
    // Stream API
    virtual int streamQbuf(int cameraId, camera_buffer_t** ubuffer, int bufferNum = 1,
                           const Parameters* settings = nullptr);
    virtual int streamDqbuf(int cameraId, int streamId, camera_buffer_t** ubuffer,
                            Parameters* settings = nullptr);
    virtual int setParameters(int cameraId, const Parameters& param);
    virtual int getParameters(int cameraId, Parameters& param, int64_t sequence);

 private:
    virtual bool threadLoop();
    int initDefaultParameters(int cameraId);
    void generateFrames(int cameraId);

 private:
    static const int kMaxStreamNum = 6;
    const uint64_t kMaxDuration = 2000000000;  // 2000ms
    struct CaptureRequest {
        int sequence;
        int streamNum;
        camera_buffer_t* ubuffer[kMaxStreamNum];
    };

    int mCameraDevices[MAX_CAMERA_NUMBER];
    camera_callback_ops_t* mCallback[MAX_CAMERA_NUMBER];

    Parameters mParameter[MAX_CAMERA_NUMBER];
    int mInitTimes;
    std::mutex mLock;

    enum { HAL_UNINIT, HAL_INIT } mState;

    int mFrameSequence[MAX_CAMERA_NUMBER];

    std::deque<std::shared_ptr<CaptureRequest>> mCaptureRequest[MAX_CAMERA_NUMBER];
    std::deque<camera_buffer_t*> mCaptureResult[MAX_CAMERA_NUMBER][kMaxStreamNum];
    std::condition_variable mBufferReadyCondition[MAX_CAMERA_NUMBER][kMaxStreamNum];
    int mCameraOpenNum;
    uint64_t mTimestamp;

 private:
    DISALLOW_COPY_AND_ASSIGN(MockCameraHal);
};

}  // namespace icamera
