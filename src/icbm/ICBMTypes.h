/*
 * Copyright (C) 2023 Intel Corporation
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

#include <cstdint>
#include "src/iutils/Errors.h"

#include "subway_autogen.h"
namespace icamera {

struct ImageInfo {
    int32_t gfxHandle;
    void* bufAddr;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
    ImageInfo() : gfxHandle(-1), bufAddr(nullptr), width(0), height(0), stride(0), size(0) {}
};

#define ICBM_REQUEST_MAX_SHIFT 15
enum ICBMFeatureType {
    REQUEST_NONE = 0,
    LEVEL0_TNR = 1 << 1,
// LEVEL0_ICBM_S
    USER_FRAMING = 1 << 2,
    BC_MODE_BB = 1 << 3,
// LEVEL0_ICBM_E
    REQUEST_MAX = 1 << ICBM_REQUEST_MAX_SHIFT
};

struct ICBMInitInfo {
    int cameraId;
    uint32_t sessionType;
    uint32_t height;
    uint32_t width;
    int32_t libPathHandle;
    char* libPath;
};

struct ICBMReqInfo {
    int cameraId;
    // all ICBM features supported by this ICBM session, used to identify session object
    uint32_t sessionType;
    // set per-frame, 1 or several features in sessionType list
    uint32_t reqType;
    ImageInfo inII;
    ImageInfo outII;
    int32_t inHandle;
    int32_t outHandle;
    int32_t paramHandle;
    void* paramAddr;
    ICBMReqInfo()
            : cameraId(-1),
              sessionType(REQUEST_NONE),
              reqType(REQUEST_NONE),
              inHandle(-1),
              outHandle(-1),
              paramHandle(-1),
              paramAddr(nullptr) {}
};

struct IC2ApiHandle {
#define _DEF_IC2_FUNC(ret, name, ...)      \
    typedef ret (*pFn##name)(__VA_ARGS__); \
    pFn##name name

    _DEF_IC2_FUNC(void, query_version, int* major, int* minor, int* patch);
    _DEF_IC2_FUNC(void, startup);
    _DEF_IC2_FUNC(void, shutdown);
    _DEF_IC2_FUNC(void, query_features, char* future_d, size_t* fsize);
    _DEF_IC2_FUNC(void, set_loglevel, iaic_log_level leve);
    _DEF_IC2_FUNC(void, create_session, iaic_session uid, const char* feature, iaic_options opt);
    _DEF_IC2_FUNC(void, close_session, iaic_session uid, const char* feature);
    _DEF_IC2_FUNC(bool, execute, iaic_session uid, iaic_memory in, iaic_memory out);
    _DEF_IC2_FUNC(void, set_data, iaic_session uid, iaic_memory& data);
    _DEF_IC2_FUNC(void, get_data, iaic_session uid, iaic_memory& data);
};

}  // namespace icamera
