/*
 * Copyright (C) 2019 Intel Corporation.
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

#include <memory>
#include <unordered_map>

#include "modules/algowrapper/IntelPGParam.h"
#include "modules/sandboxing/IPCIntelPGParam.h"

namespace icamera {

class IntelPGParamServer {
 public:
    IntelPGParamServer();
    ~IntelPGParamServer();

    int init(void* pData, int dataSize);
    int prepare(void* pData, int dataSize, void* palDataAddr);
    int allocatePGBuffer(void* pData, int dataSize);
    int getFragmentDescriptors(void* pData, int dataSize);
    int setPGAndPrepareProgram(void* pData, int dataSize);
    int registerPayloads(void* pData, int dataSize);
    int updatePALAndEncode(void* pData, int dataSize, void* palDataAddr);
    int decode(void* pData, int dataSize, void* statsAddr);
    void deinit(void* pData, int dataSize);

 private:
    struct PGParamPackage {
        int pgId;
        std::shared_ptr<IntelPGParam> mPGParamAdapt;
        ia_binary_data mPayloads[IPU_MAX_TERMINAL_COUNT];  // match terminals for encode/decode
        int mPayloadCount;
        ia_css_process_group_t* mPGBuffer;

        // <client addr, server payload>
        std::unordered_map<void*, ia_binary_data> mAllocatedPayloads;
    };

 private:
    int findPayloads(int32_t payloadCount, ia_binary_data* clientPayloads,
                     std::unordered_map<void*, ia_binary_data>* allocated,
                     ia_binary_data* serverPayloads);

    IPCIntelPGParam mIpc;
    std::unordered_map<uintptr_t, PGParamPackage> mPGParamPackages;
};

}  // namespace icamera
