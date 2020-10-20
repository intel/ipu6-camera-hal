/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

class CameraHal {
//HAL API
public:
    CameraHal();
    ~CameraHal();
    int init();
    int deinit();

//Device API
public:
    int deviceOpen(int cameraId);
    void deviceClose(int cameraId);

    void deviceCallbackRegister(int cameraId, const camera_callback_ops_t* callback);
    int deviceConfigInput(int cameraId, const stream_t *inputConfig);
    int deviceConfigStreams(int cameraId, stream_config_t *streamList);
    int deviceStart(int cameraId);
    int deviceStop(int cameraId);
    int deviceAllocateMemory(int cameraId, camera_buffer_t *ubuffer);
//Stream API
    int streamQbuf(int cameraId, camera_buffer_t **ubuffer,
                   int bufferNum = 1, const Parameters* settings = nullptr);
    int streamDqbuf(int cameraId, int streamId, camera_buffer_t **ubuffer,
                    Parameters* settings = nullptr);
    int setParameters(int cameraId, const Parameters& param);
    int getParameters(int cameraId, Parameters& param, long sequence);

private:
    DISALLOW_COPY_AND_ASSIGN(CameraHal);

private:
    CameraDevice* mCameraDevices[MAX_CAMERA_NUMBER];
    int mInitTimes;
    // Guard for CameraHal public API.
    Mutex mLock;

    enum {
        HAL_UNINIT,
        HAL_INIT
    } mState;

    int mCameraOpenNum = 0;
};

} // namespace icamera
