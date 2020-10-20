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

#include "modules/sandboxing/IPCCommon.h"
#include "src/core/psysprocessor/PGUtils.h"

namespace icamera {

#define MAX_PROCESS_GROUP_SIZE 8192
#define MAX_PAL_SIZE 0x800000  // 8M
#define MAX_STATISTICS_SIZE MAX_IA_BINARY_DATA_SIZE

struct pg_param_init_params {
    int pgId;
    uintptr_t client;
    ia_p2p_platform_t platform;
    PgConfiguration pgConfig;
    unsigned char pgManifestData[MAX_PROCESS_GROUP_SIZE];
    int disableDataTermialsData[IPU_MAX_TERMINAL_COUNT];
    int disableDataTermialsCount;
};

struct pg_param_prepare_params {
    uintptr_t client;
    uint32_t ipuParamSize;
    int32_t ipuParamHandle;
    ia_css_rbm_t* rbm;
    ia_css_rbm_t rbmData;

    // Output
    ia_css_kernel_bitmap_t bitmapData;
    uint32_t maxStatsSize;
};

// Shared memory: pg + struct
// as pg memory addr should be page size aligned
struct pg_param_allocate_pg_params {
    uintptr_t client;
    int pgSize;
};

struct pg_param_get_fragment_desc_params {
    uintptr_t client;
    int descCount;

    // Output
    int returnCount;
    ia_p2p_fragment_desc descsData[IPU_MAX_TERMINAL_COUNT * IA_P2P_MAX_FRAGMENTS];
};

struct pg_param_prepare_program_params {
    uintptr_t client;

    // Output
    int payloadCount;
    ia_binary_data payloads[IPU_MAX_TERMINAL_COUNT];  // save size of payloads
};

// Shared memory: payloads + struct
// as payload memory addr should be page size aligned
struct pg_param_register_payloads_params {
    uintptr_t client;
    int payloadCount;
    ia_binary_data cPayloads[IPU_MAX_TERMINAL_COUNT];  // save payloads of client
    ia_binary_data sPayloads[IPU_MAX_TERMINAL_COUNT];  // save payloads of server

    int32_t payloadOffsets[IPU_MAX_TERMINAL_COUNT];
};

struct pg_param_encode_params {
    uintptr_t client;
    uint32_t ipuParamSize;
    int32_t ipuParamHandle;
    int32_t payloadCount;
    ia_binary_data payloads[IPU_MAX_TERMINAL_COUNT];  // save payloads of client
};

struct pg_param_decode_params {
    uintptr_t client;
    int32_t payloadCount;
    ia_binary_data payloads[IPU_MAX_TERMINAL_COUNT];  // save payloads of client
    uint32_t clientStatsSize;
    int32_t clientStatsHandle;

    // Output
    uint32_t statsSize;
};

struct pg_param_deinit_params {
    uintptr_t client;
};

class IPCIntelPGParam {
 public:
    IPCIntelPGParam() {}
    virtual ~IPCIntelPGParam() {}

    bool clientFlattenInit(void* pData, int dataSize, int pgId, uintptr_t client,
                           ia_p2p_platform_t platform, const PgConfiguration& pgConfig);
    bool serverUnflattenInit(void* pData, int dataSize, int* pgId, uintptr_t* client,
                             ia_p2p_platform_t* platform, PgConfiguration* pgConfig);

    bool clientFlattenPrepare(void* pData, int dataSize, uintptr_t client,
                              unsigned int ipuParamSize, int32_t ipuParamHandle,
                              const ia_css_rbm_t* rbm);
    bool serverUnflattenPrepare(void* pData, int dataSize, uintptr_t* client, void* palDataAddr,
                                ia_binary_data* ipuParameters, ia_css_rbm_t** rbm,
                                ia_css_kernel_bitmap_t** bitmap, uint32_t** maxStatsSize);
    bool clientUnflattenPrepare(const void* pData, int dataSize, ia_css_kernel_bitmap_t* bitmap,
                                uint32_t* maxStatsSize = nullptr);

    int getTotalPGBufferSize(int pgSize);
    bool assignPGBuffer(void* pData, int dataSize, int pgSize, void** pgBuffer);
    bool clientFlattenAllocatePGBuffer(void* pData, int dataSize, uintptr_t client, int pgSize);
    bool serverUnflattenAllocatePGBuffer(const void* pData, int dataSize, uintptr_t* client,
                                         int* pgSize);

    bool clientFlattenGetFragDescs(void* pData, int dataSize, uintptr_t client, int descCount);
    bool serverUnflattenGetFragDescs(void* pData, int dataSize, uintptr_t* client, int* descCount,
                                     ia_p2p_fragment_desc** descs);
    bool serverFlattenGetFragDescs(void* pData, int dataSize, int count);
    bool clientUnflattenGetFragDescs(const void* pData, int dataSize, int* count,
                                     ia_p2p_fragment_desc* descs);

    bool clientFlattenPrepareProgram(void* pData, int dataSize, uintptr_t client);
    bool serverUnflattenPrepareProgram(const void* pData, int dataSize, uintptr_t* client);
    bool serverFlattenPrepareProgram(void* pData, int dataSize, int payloadCount,
                                     const ia_binary_data* payloads);
    bool clientUnflattenPrepareProgram(const void* pData, int dataSize, int* payloadCount,
                                       ia_binary_data* payloads);

    int getTotalPayloadSize(int payloadCount, const ia_binary_data* payloads);
    bool assignPayloads(void* pData, int dataSize, int payloadCount, ia_binary_data* payloads);
    bool clientFlattenRegisterPayloads(void* pData, int dataSize, uintptr_t client,
                                       int payloadCount, const ia_binary_data* payloads);
    bool serverUnflattenRegisterPayloads(void* pData, int dataSize, uintptr_t* client,
                                         int* payloadCount, ia_binary_data** cPayloads,
                                         ia_binary_data** sPayloads);

    bool clientFlattenEncode(void* pData, int dataSize, uintptr_t client,
                             unsigned int ipuParamSize, int32_t ipuParamHandle,
                             int32_t payloadCount, const ia_binary_data* payloads);
    bool serverUnflattenEncode(void* pData, int dataSize, uintptr_t* client,
                               void* palDataAddr, ia_binary_data* ipuParameters,
                               int32_t* payloadCount, ia_binary_data** payloads);

    bool clientFlattenDecode(void* pData, int dataSize, uintptr_t client, int32_t payloadCount,
                             const ia_binary_data* payloads, int32_t statsHandle);
    bool serverUnflattenDecode(void* pData, int dataSize, uintptr_t* client, int32_t* payloadCount,
                               ia_binary_data** payloads);
    bool serverFlattenDecode(void* pData, int dataSize, const ia_binary_data& statistics);
    bool clientUnflattenDecode(void* pData, int dataSize, ia_binary_data* statistics);

    bool clientFlattenDeinit(void* pData, int dataSize, uintptr_t client);
    bool serverUnflattenDeinit(const void* pData, int dataSize, uintptr_t* client);

 private:
    bool getPayloadOffsets(const void* pData, int dataSize, int payloadCount,
                           const ia_binary_data* payloads, int32_t* payloadOffsets);
    bool getPayloadData(void* pData, int dataSize, int payloadCount,
                        const int32_t* payloadOffsets, ia_binary_data* payloads);
};

}  // namespace icamera
