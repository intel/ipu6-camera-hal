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

#pragma once

#include <unordered_set>

#include "Types.h"
#include "Utils.h"
#include "iutils/Utils.h"

namespace icamera {
namespace CIPR {
class Buffer {
 public:
    Buffer(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory);
    Buffer(Buffer* parent, uint32_t offset, uint32_t size);
    ~Buffer();

    bool isRegion() const;
    Buffer* getParent();
    Result getMemoryCpuPtr(void** ptr);
    Result getMemorySize(int* size);
    Result attatchDevice(Context* ctx);

 private:
    MemoryDesc mMemoryDesc;
    uint32_t mOffset = 0;
    std::unordered_set<Buffer*> mRegions;
    bool mInitialized = false;
    Context* mContext;

 private:
    Result createWithUserMemory(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory);
    Result createWithUserMemoryWithCpuPtr(MemoryFlag flags, const MemoryDesc* userMemory);
    Result createWithUserMemoryCommon(MemoryFlag flags, const MemoryDesc* userMemory);

    Result validateBuffer(const MemoryDesc* memory);
    Result allocateCommon();
    Result allocate();
    Result getMemory(MemoryDesc* mem);
    Result getMemoryCommon(MemoryDesc* mem);
    void destroy();
    friend Command;

    DISALLOW_COPY_AND_ASSIGN(Buffer);
};
}  // namespace CIPR
}  // namespace icamera
