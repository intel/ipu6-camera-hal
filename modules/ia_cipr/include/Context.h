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

#include <vector>

#include "Buffer.h"
#include "Types.h"
#include "Utils.h"
#include "iutils/Utils.h"

namespace icamera {
namespace CIPR {
class ContextPoller;
class Context : public MemoryOper {
 public:
    Context();
    virtual ~Context();

    Result getCapabilities(PSYSCapability* cap);
    Result getManifest(uint32_t index, uint32_t* mainfestSize, void* manifest);
    Result allocate(MemoryDesc* mem) final;
    Result migrate(MemoryDesc* mem) final;
    Result getMemory(MemoryDesc* mem, MemoryDesc* out) final;
    Result destroy(MemoryDesc* mem) final;
    Result doIoctl(int request, void* ptr);
    ContextPoller getPoller(int event, int timeout);

 private:
    enum class Flags { DEBUG, NONE };
    int mFd = -1;
    Flags mInitFlag = Flags::NONE;
    bool mInitialized;

 private:
    Result registerBuffer(MemoryDesc* mem);
    Result unregisterBuffer(MemoryDesc* mem);
    static Result psysClose(int fd);

    DISALLOW_COPY_AND_ASSIGN(Context);
};

class ContextPoller {
 public:
    int poll();

 private:
    int mTimeout;
    int mEvents;
    int mFd;

    friend Context;
};
}  // namespace CIPR
}  // namespace icamera
