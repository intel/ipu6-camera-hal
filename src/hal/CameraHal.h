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

#pragma once

#include "CameraDevice.h"
#include "Parameters.h"

#include "iutils/CameraShm.h"

namespace icamera {

/**
 * CameraHal class is the real HAL API.
 * There is only one instance of CameraHal which is created when HAL loading
 * It creates the CameraDevice based on the camera ID to support multi-cameras.
 *
 * The main jobs of the Class are
 * 1. Maintain a list of CameraDevice
 * 2. Pass the Camera HAL API to the correct CameraDevice based on CameraId
 *
 * If open dual cameras in different process, the shared memory must be used to
 * keep the account of the open times.
 *
 * The CameraHal creates and maintains following singleton instances
 * 1. MediaControl Instance
 * 2. PlatformData Instance
 */

class CameraHal {
 public:
    // HAL API
    CameraHal();
    virtual ~CameraHal();
    virtual int init();
    virtual int deinit();

    // Device API
    virtual int deviceOpen(int cameraId, int vcNum = 0);
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
    DISALLOW_COPY_AND_ASSIGN(CameraHal);

    CameraDevice* mCameraDevices[MAX_CAMERA_NUMBER];
    int mInitTimes;
    // Guard for CameraHal public API.
    Mutex mLock;
    // VIRTUAL_CHANNEL_S
    int mTotalVirtualChannelCamNum[MAX_VC_GROUP_NUMBER];
    int mConfigTimes[MAX_VC_GROUP_NUMBER];
    Condition mVirtualChannelSignal[MAX_VC_GROUP_NUMBER];
    static const nsecs_t mWaitDuration = 500000000;  // 500ms
    // VIRTUAL_CHANNEL_E

    enum { HAL_UNINIT, HAL_INIT } mState;

    // Used to store variables in different process
    CameraSharedMemory mCameraShm;
    int mCameraOpenNum;
};

}  // namespace icamera
