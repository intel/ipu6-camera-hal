/*
 * Copyright (C) 2020 Intel Corporation.
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

#include "modules/ia_cipr/include/Utils.h"

#include <unistd.h>

#define LOG_TAG "IA_CIPR_UTILS"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using icamera::CAMERA_DEBUG_LOG_DBG;
using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_VERBOSE;
using icamera::CAMERA_DEBUG_LOG_WARNING;

namespace icamera {
namespace CIPR {
void* mallocMemory(size_t sz) {
    return ::malloc(sz);
}

void* callocMemory(size_t cnt, size_t sz) {
    return ::calloc(cnt, sz);
}

void* mallocAlignedMemory(size_t sz, size_t alignment) {
    void* p = nullptr;
    int ret = ::posix_memalign(&p, alignment, sz);

    return ret ? nullptr : p;
}

void freeMemory(void* p) {
    ::free(p);
}

void memoryCopy(void* dst, size_t dsz, const void* src, size_t sz) {
    if (dsz < sz) {
        icamera::LOG2("%s: destination size < source size, potential coding error", __func__);
    }

    MEMCPY_S(dst, dsz, src, sz);
}

size_t getPageSize() {
    return static_cast<size_t>(sysconf(_SC_PAGESIZE));
}
}  // namespace CIPR
}  // namespace icamera
