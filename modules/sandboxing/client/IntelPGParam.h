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

#include <memory>
#include <vector>

#include "IntelAlgoCommon.h"
#include "modules/sandboxing/IPCIntelPGParam.h"

namespace icamera {

class IntelPGParam {
 public:
    explicit IntelPGParam(int pgId);
    ~IntelPGParam();

    int init(ia_p2p_platform_t platform, const PgConfiguration& Pgconfiguration);
    int prepare(const ia_binary_data* ipuParameters, const ia_css_rbm_t* rbm,
                ia_css_kernel_bitmap_t* bitmap, uint32_t* maxStatsSize = nullptr);
    void* allocatePGBuffer(int pgSize);
    int getFragmentDescriptors(int terminalIdx, ia_p2p_fragment_desc* desc);
    int setPGAndPrepareProgram(ia_css_process_group_t* pg);
    int getPayloadSizes(int payloadCount, ia_binary_data* payloads);
    int allocatePayloads(int payloadCount, ia_binary_data* payloads);
    int updatePALAndEncode(const ia_binary_data* ipuParams, int payloadCount,
                           ia_binary_data* payloads);
    int decode(int payloadCount, ia_binary_data* payload, ia_binary_data* statistics);
    void deinit();

 private:
    IPCIntelPGParam mIpc;
    IntelAlgoCommon mCommon;
    bool mInitialized;

    ShmMemInfo mMemInit;
    ShmMemInfo mMemPrepare;
    ShmMemInfo mMemGetFragDescs;
    ShmMemInfo mMemAllocatePGBuffer;
    ShmMemInfo mMemPrepareProgram;
    ShmMemInfo mMemEncode;
    ShmMemInfo mMemDecode;
    ShmMemInfo mMemDeinit;
    ShmMemInfo mMemStatistics;
    std::vector<ShmMem> mMems;

    std::vector<ShmMemInfo> mMemAllocatePayloads;

    int mPgId;
    uintptr_t mClient;

    // Shared memory in client, to avoid memory copy
    int mPayloadCount;
    ia_binary_data mPayloads[IPU_MAX_TERMINAL_COUNT];  // save sizes
    ia_css_process_group_t* mPGBuffer;
    uint32_t mMaxStatsSize;
};

}  // namespace icamera
