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

namespace icamera {

constexpr static uint32_t UF_MODE_OFF = 0;
constexpr static uint32_t UF_MODE_ON = 1;

enum class ICBMResult {
    OK,
    InternalError,
    FrameError,
    GBMMappingError,
    SHMError,
    IPCError,
};

static inline const char* getResultStr(const ICBMResult r) {
    switch (r) {
    case ICBMResult::OK:                return "OK";
    case ICBMResult::InternalError:     return "InternalError";
    case ICBMResult::FrameError:        return "FrameError";
    case ICBMResult::GBMMappingError:   return "GBMMappingError";
    case ICBMResult::SHMError:          return "SHMError";
    case ICBMResult::IPCError:          return "IPCError";
    };
}

struct ImageInfo {
    uint32_t gfxHandle;
    void* bufAddr;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
};

struct ICBMReqInfo {
    uint32_t usrFrmEnabled;
};

struct ICBMInitInfo {
    uint32_t height;
    uint32_t width;
};

struct ICBMRunInfo {
    ImageInfo inII;
    ImageInfo outII;
    uint32_t inHandle;
    uint32_t outHandle;
    ICBMReqInfo icbmReqInfo;
};

}  // namespace icamera
