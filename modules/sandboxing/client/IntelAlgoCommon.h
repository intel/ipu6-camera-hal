/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <string>
#include <vector>

#include "IntelAlgoClient.h"

namespace icamera {
typedef struct ShmMemInfo {
    std::string mName;
    int mSize;
    int mFd;
    void* mAddr;
    int32_t mHandle;
    ShmMemInfo() : mName(""), mSize(0), mFd(-1), mAddr(nullptr), mHandle(-1) {}
} ShmMemInfo;

typedef struct ShmMem {
    std::string name;
    int size;
    ShmMemInfo* mem;
    bool allocated;
} ShmMem;

class IntelAlgoCommon {
 public:
    IntelAlgoCommon();
    virtual ~IntelAlgoCommon();

    bool allocShmMem(const std::string& name, int size, ShmMemInfo* shm,
                     ShmMemUsage usage = CPU_ALGO_SHM);
    int32_t registerGbmBuffer(int bufferFd);
    void deregisterGbmBuffer(int32_t bufferHandle);
    bool requestSync(IPC_CMD cmd, int32_t handle);
    bool requestSync(IPC_CMD cmd);
    void freeShmMem(const ShmMemInfo& shm, ShmMemUsage usage = CPU_ALGO_SHM);

    bool allocateAllShmMems(std::vector<ShmMem>* mems);
    void releaseAllShmMems(const std::vector<ShmMem>& mems);

    int32_t getShmMemHandle(void* addr, ShmMemUsage usage = CPU_ALGO_SHM);

 private:
    IntelAlgoClient* mClient;
};

} /* namespace icamera */
