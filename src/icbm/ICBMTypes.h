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
    USER_FRAMING = 1 << 2,
    BC_MODE_BB = 1 << 3,
    REQUEST_MAX = 1 << ICBM_REQUEST_MAX_SHIFT
};

struct ICBMInitInfo {
    int cameraId;
    uint32_t sessionType;
    uint32_t height;
    uint32_t width;
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

}  // namespace icamera
