/*
 * Copyright (C) 2023 Intel Corporation.
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
#define LOG_TAG HalAdaptor

#include "HalAdaptor.h"

#include <dlfcn.h>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "iutils/Errors.h"

extern "C" {
namespace icamera {

static void* gCameraHalLib = nullptr;
static HalApiHandle gCameraHalAdaptor = {};

#define CheckFuncCall(function)                                   \
    do {                                                          \
        if (((function) == nullptr)) {                            \
            LOGE("%s, function call is nullptr", __func__);       \
            return -1;                                            \
        }                                                         \
    } while (0)

#define GET_FUNC_CALL(member, fnName)                                                  \
    do {                                                                               \
        gCameraHalAdaptor.member =                                                     \
            (HalApiHandle::pFn##member)dlsym(gCameraHalLib, #fnName);                  \
        if (gCameraHalAdaptor.member == nullptr) {                                     \
            LOGE("@%s: LOADING: " #fnName "failed: %s", __func__, dlerror());          \
            return;                                                                    \
        }                                                                              \
        LOG2("@%s: LOADING: " #fnName "= %x", __func__, gCameraHalAdaptor.member);     \
    } while (0)

static void load_camera_hal_library(const char* libName) {
    CheckAndLogError(!libName, VOID_VALUE, "%s, invalid library name", __func__);

    gCameraHalLib = dlopen(libName, RTLD_NOW);
    CheckAndLogError(!gCameraHalLib, VOID_VALUE, "%s, failed to open library: %s, error: %s",
                     __func__, libName, dlerror());

    GET_FUNC_CALL(getNumberOfCameras, get_number_of_cameras);
    GET_FUNC_CALL(getCameraInfo, get_camera_info);
    GET_FUNC_CALL(cameraHalInit, camera_hal_init);
    GET_FUNC_CALL(cameraHalDeinit, camera_hal_deinit);
    GET_FUNC_CALL(cameraCallbackRegister, camera_callback_register);
    GET_FUNC_CALL(cameraDeviceOpen, camera_device_open);
    GET_FUNC_CALL(cameraDeviceClose, camera_device_close);
    GET_FUNC_CALL(cameraDeviceConfigSensorInput, camera_device_config_sensor_input);
    GET_FUNC_CALL(cameraDeviceConfigStreams, camera_device_config_streams);
    GET_FUNC_CALL(cameraDeviceStart, camera_device_start);
    GET_FUNC_CALL(cameraDeviceStop, camera_device_stop);
    GET_FUNC_CALL(cameraDeviceAllocateMemory, camera_device_allocate_memory);
    GET_FUNC_CALL(cameraStreamQbuf, camera_stream_qbuf);
    GET_FUNC_CALL(cameraStreamDqbuf, camera_stream_dqbuf);
    GET_FUNC_CALL(cameraSetParameters, camera_set_parameters);
    GET_FUNC_CALL(cameraGetParameters, camera_get_parameters);
    GET_FUNC_CALL(getHalFrameSize, get_frame_size);
}

static void close_camera_hal_library() {
    if (gCameraHalLib) {
        dlclose(gCameraHalLib);
        gCameraHalLib = nullptr;
    }
}

int get_number_of_cameras() {
    CheckFuncCall(gCameraHalAdaptor.getNumberOfCameras);
    return gCameraHalAdaptor.getNumberOfCameras();
}

int get_camera_info(int camera_id, camera_info_t& info) {
    CheckFuncCall(gCameraHalAdaptor.getCameraInfo);
    return gCameraHalAdaptor.getCameraInfo(camera_id, info);
}
int camera_hal_init() {
    CheckFuncCall(gCameraHalAdaptor.cameraHalInit);
    return gCameraHalAdaptor.cameraHalInit();
}

int camera_hal_deinit() {
    CheckFuncCall(gCameraHalAdaptor.cameraHalDeinit);
    return gCameraHalAdaptor.cameraHalDeinit();
}

void camera_callback_register(int camera_id, const camera_callback_ops_t* callback) {
    if (gCameraHalAdaptor.cameraCallbackRegister) {
        LOGE("%s, function call is nullptr", __func__);
        return VOID_VALUE;
    }
    gCameraHalAdaptor.cameraCallbackRegister(camera_id, callback);
}

int camera_device_open(int camera_id, int vc_num) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceOpen);
    return gCameraHalAdaptor.cameraDeviceOpen(camera_id, vc_num);
}

void camera_device_close(int camera_id) {
    if (gCameraHalAdaptor.cameraDeviceClose) {
        LOGE("%s, function call is nullptr", __func__);
        return VOID_VALUE;
    }
    gCameraHalAdaptor.cameraDeviceClose(camera_id);
}

int camera_device_config_sensor_input(int camera_id, const stream_t* input_config) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceConfigSensorInput);
    return gCameraHalAdaptor.cameraDeviceConfigSensorInput(camera_id, input_config);
}

int camera_device_config_streams(int camera_id, stream_config_t* stream_list) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceConfigStreams);
    return gCameraHalAdaptor.cameraDeviceConfigStreams(camera_id, stream_list);
}

int camera_device_start(int camera_id) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceStart);
    return gCameraHalAdaptor.cameraDeviceStart(camera_id);
}

int camera_device_stop(int camera_id) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceStop);
    return gCameraHalAdaptor.cameraDeviceStop(camera_id);
}

int camera_device_allocate_memory(int camera_id, camera_buffer_t* buffer) {
    CheckFuncCall(gCameraHalAdaptor.cameraDeviceAllocateMemory);
    return gCameraHalAdaptor.cameraDeviceAllocateMemory(camera_id, buffer);
}

int camera_stream_qbuf(int camera_id, camera_buffer_t** buffer, int num_buffers,
                       const Parameters* settings) {
    CheckFuncCall(gCameraHalAdaptor.cameraStreamQbuf);
    return gCameraHalAdaptor.cameraStreamQbuf(camera_id, buffer, num_buffers, settings);
}

int camera_stream_dqbuf(int camera_id, int stream_id, camera_buffer_t** buffer,
                        Parameters* settings) {
    CheckFuncCall(gCameraHalAdaptor.cameraStreamDqbuf);
    return gCameraHalAdaptor.cameraStreamDqbuf(camera_id, stream_id, buffer, settings);
}

int camera_set_parameters(int camera_id, const Parameters& param) {
    CheckFuncCall(gCameraHalAdaptor.cameraSetParameters);
    return gCameraHalAdaptor.cameraSetParameters(camera_id, param);
}

int camera_get_parameters(int camera_id, Parameters& param, int64_t sequence) {
    CheckFuncCall(gCameraHalAdaptor.cameraGetParameters);
    return gCameraHalAdaptor.cameraGetParameters(camera_id, param, sequence);
}

int get_frame_size(int camera_id, int format, int width, int height, int field, int* bpp) {
    CheckFuncCall(gCameraHalAdaptor.getHalFrameSize);
    return gCameraHalAdaptor.getHalFrameSize(camera_id, format, width, height, field, bpp);
}

__attribute__((constructor)) void initHalAdaptor() {
    Log::setDebugLevel();
    load_camera_hal_library("/usr/lib/libcamhal.so");
}

__attribute__((destructor)) void deinitHalAdaptor() {
    close_camera_hal_library();
    CLEAR(gCameraHalAdaptor);
}

}  // namespace icamera
}  // extern "C"
