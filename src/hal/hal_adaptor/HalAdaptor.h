/*
 * Copyright (C) 2023-2024 Intel Corporation.
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

#include "ICamera.h"

extern "C" {
namespace icamera {

struct HalApiHandle {
#define _DEF_HAL_FUNC(ret, name, ...)      \
    typedef ret (*pFn##name)(__VA_ARGS__); \
    pFn##name name

    _DEF_HAL_FUNC(int, getNumberOfCameras);
    _DEF_HAL_FUNC(int, getCameraInfo, int camera_id, camera_info_t& info);
    _DEF_HAL_FUNC(int, cameraHalInit);
    _DEF_HAL_FUNC(int, cameraHalDeinit);
    _DEF_HAL_FUNC(void, cameraCallbackRegister, int camera_id,
                  const camera_callback_ops_t* callback);
    _DEF_HAL_FUNC(int, cameraDeviceOpen, int camera_id, int vc_num);
    _DEF_HAL_FUNC(void, cameraDeviceClose, int camera_id);
    _DEF_HAL_FUNC(int, cameraDeviceConfigSensorInput, int camera_id, const stream_t* inputConfig);
    _DEF_HAL_FUNC(int, cameraDeviceConfigStreams, int camera_id, stream_config_t* stream_list);
    _DEF_HAL_FUNC(int, cameraDeviceStart, int camera_id);
    _DEF_HAL_FUNC(int, cameraDeviceStop, int camera_id);
    _DEF_HAL_FUNC(int, cameraDeviceAllocateMemory, int camera_id, camera_buffer_t* buffer);
    _DEF_HAL_FUNC(int, cameraStreamQbuf, int camera_id, camera_buffer_t** buffer,
                  int num_buffers, const Parameters* settings);
    _DEF_HAL_FUNC(int, cameraStreamDqbuf, int camera_id, int stream_id, camera_buffer_t** buffer,
                  Parameters* settings);
    _DEF_HAL_FUNC(int, cameraSetParameters, int camera_id, const Parameters& param);
    _DEF_HAL_FUNC(int, cameraGetParameters, int camera_id, Parameters& param, int64_t sequence);
    _DEF_HAL_FUNC(int, getHalFrameSize, int camera_id, int format, int width, int height,
                  int field, int* bpp);
};
}  // namespace icamera
}  // extern "C"

