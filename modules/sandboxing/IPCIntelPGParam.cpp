/*
 * Copyright (C) 2019-2021 Intel Corporation.
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

#define LOG_TAG IPCIntelPGParam

#include "modules/sandboxing/IPCIntelPGParam.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

bool IPCIntelPGParam::clientFlattenInit(void* pData, int dataSize, int pgId, uintptr_t client,
                                        ia_p2p_platform_t platform,
                                        const PgConfiguration& pgConfig) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_init_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_init_params* params = static_cast<pg_param_init_params*>(pData);
    CheckAndLogError(pgConfig.pgManifestSize > sizeof(params->pgManifestData), false,
                     "@%s, manifest buffer size: %d is big", __func__, pgConfig.pgManifestSize);

    params->pgId = pgId;
    params->client = client;
    params->platform = platform;

    params->pgConfig.fragmentCount = pgConfig.fragmentCount;
    params->pgConfig.inputMainFrame = pgConfig.inputMainFrame;
    params->pgConfig.outputMainFrame = pgConfig.outputMainFrame;

    params->pgConfig.pgManifestSize = pgConfig.pgManifestSize;
    MEMCPY_S(params->pgManifestData, sizeof(params->pgManifestData), pgConfig.pgManifest,
             pgConfig.pgManifestSize);

    params->disableDataTermialsCount = pgConfig.disableDataTermials.size();
    CheckAndLogError(params->disableDataTermialsCount > IPU_MAX_TERMINAL_COUNT, false,
                     "@%s, disableDataTermials cound: %d is big", __func__,
                     params->disableDataTermialsCount);
    memset(params->disableDataTermialsData, -1, sizeof(params->disableDataTermialsData));
    for (int i = 0; i < params->disableDataTermialsCount; i++) {
        params->disableDataTermialsData[i] = pgConfig.disableDataTermials[i];
    }

    return true;
}

bool IPCIntelPGParam::serverUnflattenInit(void* pData, int dataSize, int* pgId, uintptr_t* client,
                                          ia_p2p_platform_t* platform, PgConfiguration* pgConfig) {
    CheckAndLogError(!pData || !pgId || !client || !platform || !pgConfig ||
                         dataSize < sizeof(pg_param_init_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, pgId: %p, client: %p, platform: %p, "
                     "pgConfig: %p, dataSize: %d",
                     __func__, pData, pgId, client, platform, pgConfig, dataSize);

    pg_param_init_params* params = static_cast<pg_param_init_params*>(pData);
    *pgId = params->pgId;
    *client = params->client;
    *platform = params->platform;

    pgConfig->fragmentCount = params->pgConfig.fragmentCount;
    pgConfig->inputMainFrame = params->pgConfig.inputMainFrame;
    pgConfig->outputMainFrame = params->pgConfig.outputMainFrame;
    pgConfig->pgManifestSize = params->pgConfig.pgManifestSize;
    void* dataPtr = static_cast<void*>(params->pgManifestData);
    pgConfig->pgManifest = static_cast<ia_css_program_group_manifest_t*>(dataPtr);

    pgConfig->disableDataTermials.clear();
    CheckAndLogError(params->disableDataTermialsCount > IPU_MAX_TERMINAL_COUNT, false,
                     "@%s, disableDataTermials cound: %d is big", __func__,
                     params->disableDataTermialsCount);
    for (int i = 0; i < params->disableDataTermialsCount; i++) {
        pgConfig->disableDataTermials.push_back(params->disableDataTermialsData[i]);
    }

    return true;
}

bool IPCIntelPGParam::clientFlattenPrepare(void* pData, int dataSize, uintptr_t client,
                                           unsigned int ipuParamSize, int32_t ipuParamHandle,
                                           const ia_css_rbm_t* rbm) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_prepare_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_prepare_params* params = static_cast<pg_param_prepare_params*>(pData);
    params->client = client;
    params->ipuParamSize = ipuParamSize;
    params->ipuParamHandle = ipuParamHandle;

    if (rbm) {
        params->rbm = &params->rbmData;
        MEMCPY_S(&params->rbmData, sizeof(params->rbmData), rbm, sizeof(*rbm));
    } else {
        params->rbm = nullptr;
    }

    return true;
}

bool IPCIntelPGParam::serverUnflattenPrepare(void* pData, int dataSize, uintptr_t* client,
                                             void* palDataAddr, ia_binary_data* ipuParameters,
                                             ia_css_rbm_t** rbm, ia_css_kernel_bitmap_t** bitmap,
                                             uint32_t** maxStatsSize) {
    CheckAndLogError(!pData || !client || !ipuParameters || !rbm || !bitmap || !palDataAddr ||
                         dataSize < sizeof(pg_param_prepare_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, ipuParameters: %p, rbm: %p, "
                     "bitmap: %p, palDataAddr: %p, dataSize: %d",
                     __func__, pData, client, ipuParameters, rbm, bitmap, palDataAddr, dataSize);

    pg_param_prepare_params* params = static_cast<pg_param_prepare_params*>(pData);
    *client = params->client;
    ipuParameters->size = params->ipuParamSize;
    ipuParameters->data = palDataAddr;
    *rbm = params->rbm ? &params->rbmData : nullptr;
    *bitmap = &params->bitmapData;
    if (maxStatsSize) *maxStatsSize = &params->maxStatsSize;
    return true;
}

bool IPCIntelPGParam::clientUnflattenPrepare(const void* pData, int dataSize,
                                             ia_css_kernel_bitmap_t* bitmap,
                                             uint32_t* maxStatsSize) {
    CheckAndLogError(!pData || !bitmap || dataSize < sizeof(pg_param_prepare_params), false,
                     "@%s, Wrong parameters, pData: %p, bitmap: %p, dataSize: %d", __func__, pData,
                     bitmap, dataSize);

    const pg_param_prepare_params* params = static_cast<const pg_param_prepare_params*>(pData);
    MEMCPY_S(bitmap, sizeof(*bitmap), &params->bitmapData, sizeof(params->bitmapData));

    if (maxStatsSize) *maxStatsSize = params->maxStatsSize;
    return true;
}

int IPCIntelPGParam::getTotalPGBufferSize(int pgSize) {
    int size = sizeof(pg_param_allocate_pg_params);
    size += PAGE_ALIGN(pgSize);
    return size;
}

bool IPCIntelPGParam::assignPGBuffer(void* pData, int dataSize, int pgSize, void** pgBuffer) {
    CheckAndLogError(!pData || !pgBuffer || dataSize < pgSize, false,
                     "@%s, Wrong parameters, pData: %p, pgBuffer: %p, dataSize: %d, pgSize: %d",
                     __func__, pData, pgBuffer, dataSize, pgSize);

    uintptr_t pgAddr = reinterpret_cast<uintptr_t>(pData);
    CheckAndLogError(pgAddr & ((getpagesize() - 1)), false, "@%s, pg addr is not aligned",
                     __func__);
    *pgBuffer = pData;
    return true;
}

bool IPCIntelPGParam::clientFlattenAllocatePGBuffer(void* pData, int dataSize, uintptr_t client,
                                                    int pgSize) {
    CheckAndLogError(!pData, false, "@%s, pData is nullptr", __func__);
    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_allocate_pg_params);
    CheckAndLogError(paramAddr < reinterpret_cast<uintptr_t>(pData) + pgSize, false,
                     "@%s, dataSize: %d is small", __func__, dataSize);

    pg_param_allocate_pg_params* params = reinterpret_cast<pg_param_allocate_pg_params*>(paramAddr);
    params->client = client;
    params->pgSize = pgSize;

    return true;
}

bool IPCIntelPGParam::serverUnflattenAllocatePGBuffer(const void* pData, int dataSize,
                                                      uintptr_t* client, int* pgSize) {
    CheckAndLogError(!pData || !client || !pgSize || dataSize < sizeof(pg_param_allocate_pg_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, pgSize: %p, dataSize: %d",
                     __func__, pData, client, pgSize, dataSize);
    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_allocate_pg_params);
    pg_param_allocate_pg_params* params = reinterpret_cast<pg_param_allocate_pg_params*>(paramAddr);
    CheckAndLogError(paramAddr < reinterpret_cast<uintptr_t>(pData) + params->pgSize, false,
                     "@%s, dataSize is small", __func__);
    *client = params->client;
    *pgSize = params->pgSize;

    return true;
}

bool IPCIntelPGParam::clientFlattenGetFragDescs(void* pData, int dataSize, uintptr_t client,
                                                int descCount) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_get_fragment_desc_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    CheckAndLogError(descCount > sizeof(params->descsData), false, "@%s, descCount: %d is big",
                     __func__, descCount);

    params->client = client;
    params->descCount = descCount;
    return true;
}

bool IPCIntelPGParam::serverUnflattenGetFragDescs(void* pData, int dataSize, uintptr_t* client,
                                                  int* descCount, ia_p2p_fragment_desc** descs) {
    CheckAndLogError(!pData || !client || !descCount || !descs ||
                         dataSize < sizeof(pg_param_get_fragment_desc_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, descCount: %p, "
                     "descs: %p, dataSize: %d",
                     __func__, pData, client, descCount, descs, dataSize);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    *client = params->client;
    *descCount = params->descCount;
    *descs = params->descsData;
    return true;
}

bool IPCIntelPGParam::serverFlattenGetFragDescs(void* pData, int dataSize, int count) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_get_fragment_desc_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    params->returnCount = count;
    return true;
}

bool IPCIntelPGParam::clientUnflattenGetFragDescs(const void* pData, int dataSize, int* count,
                                                  ia_p2p_fragment_desc* descs) {
    CheckAndLogError(
        !pData || !count || !descs || dataSize < sizeof(pg_param_get_fragment_desc_params), false,
        "@%s, Wrong parameters, pData: %p, count: %p, descs: %p, dataSize: %d", __func__, pData,
        count, descs, dataSize);

    const pg_param_get_fragment_desc_params* params =
        static_cast<const pg_param_get_fragment_desc_params*>(pData);
    *count = params->returnCount;
    MEMCPY_S(descs, sizeof(ia_p2p_fragment_desc) * params->descCount, params->descsData,
             sizeof(params->descsData));
    return true;
}

bool IPCIntelPGParam::clientFlattenPrepareProgram(void* pData, int dataSize, uintptr_t client) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_prepare_program_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_prepare_program_params* params = static_cast<pg_param_prepare_program_params*>(pData);
    params->client = client;
    return true;
}

bool IPCIntelPGParam::serverUnflattenPrepareProgram(const void* pData, int dataSize,
                                                    uintptr_t* client) {
    CheckAndLogError(!pData || !client || dataSize < sizeof(pg_param_prepare_program_params), false,
                     "@%s, Wrong parameters, pData: %p, client: %p, dataSize: %d", __func__, pData,
                     client, dataSize);

    const pg_param_prepare_program_params* params =
        static_cast<const pg_param_prepare_program_params*>(pData);
    *client = params->client;
    return true;
}

bool IPCIntelPGParam::serverFlattenPrepareProgram(void* pData, int dataSize, int payloadCount,
                                                  const ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads || dataSize < sizeof(pg_param_prepare_program_params) ||
                         payloadCount > IPU_MAX_TERMINAL_COUNT,
                     false,
                     "@%s, Wrong parameters, pData: %p, payloads: %p, dataSize: %d, count: %d",
                     __func__, pData, payloads, dataSize, payloadCount);

    pg_param_prepare_program_params* params = static_cast<pg_param_prepare_program_params*>(pData);
    params->payloadCount = payloadCount;
    MEMCPY_S(params->payloads, sizeof(ia_binary_data) * payloadCount, payloads,
             sizeof(ia_binary_data) * payloadCount);
    return true;
}

bool IPCIntelPGParam::clientUnflattenPrepareProgram(const void* pData, int dataSize,
                                                    int* payloadCount, ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads || dataSize < sizeof(pg_param_prepare_program_params),
                     false, "@%s, Wrong parameters, pData: %p, payloads: %p, dataSize: %d",
                     __func__, pData, payloads, dataSize);

    const pg_param_prepare_program_params* params =
        static_cast<const pg_param_prepare_program_params*>(pData);

    CheckAndLogError(*payloadCount < params->payloadCount, false, "@%s, payloadCount is small",
                     __func__);
    *payloadCount = params->payloadCount;
    for (int i = 0; i < params->payloadCount; i++) {
        payloads[i].size = params->payloads[i].size;
        payloads[i].data = nullptr;
    }
    return true;
}

int IPCIntelPGParam::getTotalPayloadSize(int payloadCount, const ia_binary_data* payloads) {
    CheckAndLogError(!payloads, 0, "@%s, payloads is nullptr", __func__);
    int size = sizeof(pg_param_register_payloads_params);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size) {
            size += PAGE_ALIGN(payloads[i].size);
        }
    }
    return size;
}

bool IPCIntelPGParam::assignPayloads(void* pData, int dataSize, int payloadCount,
                                     ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads, false, "@%s, pData: %p, payloads: %p is nullptr",
                     __func__, pData, payloads);

    uintptr_t payloadAddr = reinterpret_cast<uintptr_t>(pData);
    CheckAndLogError(payloadAddr & ((getpagesize() - 1)), false, "@%s, payload addr is not aligned",
                     __func__);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size > 0) {
            CheckAndLogError(payloadAddr > reinterpret_cast<uintptr_t>(pData) + dataSize, false,
                             "@%s, buffer is small", __func__);
            payloads[i].data = reinterpret_cast<void*>(payloadAddr);
            payloadAddr += PAGE_ALIGN(payloads[i].size);
        }
    }
    return true;
}

bool IPCIntelPGParam::clientFlattenRegisterPayloads(void* pData, int dataSize, uintptr_t client,
                                                    int payloadCount,
                                                    const ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads || dataSize < sizeof(pg_param_register_payloads_params) ||
                         payloadCount > IPU_MAX_TERMINAL_COUNT,
                     false,
                     "@%s, Wrong parameters, pData: %p, payloads: %p, dataSize: %d, count: %d",
                     __func__, pData, payloads, dataSize, payloadCount);

    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_register_payloads_params);
    pg_param_register_payloads_params* params =
        reinterpret_cast<pg_param_register_payloads_params*>(paramAddr);
    params->client = client;
    params->payloadCount = payloadCount;
    MEMCPY_S(params->cPayloads, sizeof(params->cPayloads), payloads,
             sizeof(ia_binary_data) * payloadCount);
    getPayloadOffsets(pData, dataSize, payloadCount, payloads, params->payloadOffsets);
    return true;
}

bool IPCIntelPGParam::serverUnflattenRegisterPayloads(void* pData, int dataSize, uintptr_t* client,
                                                      int* payloadCount, ia_binary_data** cPayloads,
                                                      ia_binary_data** sPayloads) {
    CheckAndLogError(!pData || !client || !payloadCount || !cPayloads || !sPayloads ||
                         dataSize < sizeof(pg_param_register_payloads_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, payloadCount: %p, "
                     "cPayloads: %p, sPayloads: %p, dataSize: %d",
                     __func__, pData, client, payloadCount, cPayloads, sPayloads, dataSize);

    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_register_payloads_params);
    pg_param_register_payloads_params* params =
        reinterpret_cast<pg_param_register_payloads_params*>(paramAddr);
    *client = params->client;
    MEMCPY_S(params->sPayloads, sizeof(params->sPayloads), params->cPayloads,
             sizeof(params->cPayloads));  // save size to sPayloads
    *payloadCount = params->payloadCount;
    getPayloadData(pData, dataSize, *payloadCount, params->payloadOffsets, params->sPayloads);
    *cPayloads = params->cPayloads;
    *sPayloads = params->sPayloads;
    return true;
}

bool IPCIntelPGParam::getPayloadOffsets(const void* pData, int dataSize, int payloadCount,
                                        const ia_binary_data* payloads, int32_t* payloadOffsets) {
    CheckAndLogError(!pData || !payloads || !payloadOffsets, false,
                     "@%s, pData: %p, payloads: %p, payloadOffsets: %p is nullptr", __func__, pData,
                     payloads);

    uintptr_t startAddr = reinterpret_cast<uintptr_t>(pData);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size <= 0) continue;

        int32_t offset = reinterpret_cast<uintptr_t>(payloads[i].data) - startAddr;
        CheckAndLogError((offset > (dataSize - payloads[i].size) || offset < 0), false,
                         "@%s, error offset %d", __func__, offset);
        payloadOffsets[i] = offset;
    }
    return 0;
}

bool IPCIntelPGParam::getPayloadData(void* pData, int dataSize, int payloadCount,
                                     const int32_t* payloadOffsets, ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads || !payloadOffsets, false,
                     "@%s, pData: %p, payloads: %p, payloadOffsets: %p is nullptr", __func__, pData,
                     payloads, payloadOffsets);

    unsigned char* startAddr = reinterpret_cast<unsigned char*>(pData);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size <= 0) continue;

        CheckAndLogError(payloadOffsets[i] > dataSize - payloads[i].size, false,
                         "@%s, error offset %d", __func__, payloadOffsets[i]);
        payloads[i].data = startAddr + payloadOffsets[i];
    }
    return true;
}

bool IPCIntelPGParam::clientFlattenEncode(void* pData, int dataSize, uintptr_t client,
                                          unsigned int ipuParamSize, int32_t ipuParamHandle,
                                          int32_t payloadCount, const ia_binary_data* payloads) {
    CheckAndLogError(!pData || !payloads || dataSize < sizeof(pg_param_encode_params) ||
                         payloadCount > IPU_MAX_TERMINAL_COUNT,
                     false,
                     "@%s, Wrong parameters, pData: %p, payloads: %p, dataSize: %d, count: %d",
                     __func__, pData, payloads, dataSize, payloadCount);

    pg_param_encode_params* params = static_cast<pg_param_encode_params*>(pData);
    params->client = client;
    params->ipuParamSize = ipuParamSize;
    params->ipuParamHandle = ipuParamHandle;
    params->payloadCount = payloadCount;
    MEMCPY_S(params->payloads, sizeof(params->payloads), payloads,
             sizeof(ia_binary_data) * payloadCount);
    return true;
}

bool IPCIntelPGParam::serverUnflattenEncode(void* pData, int dataSize, uintptr_t* client,
                                            void* palDataAddr, ia_binary_data* ipuParameters,
                                            int32_t* payloadCount, ia_binary_data** payloads) {
    CheckAndLogError(!pData || !client || !ipuParameters || !palDataAddr || !payloadCount ||
                         !payloads || dataSize < sizeof(pg_param_encode_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, ipuParameters: %p, "
                     "palDataAddr: %p, payloadCount: %p, payloads: %p, dataSize: %d",
                     __func__, pData, client, ipuParameters, palDataAddr, payloadCount, payloads,
                     dataSize);

    pg_param_encode_params* params = static_cast<pg_param_encode_params*>(pData);
    *client = params->client;
    ipuParameters->size = params->ipuParamSize;
    ipuParameters->data = palDataAddr;
    *payloadCount = params->payloadCount;
    *payloads = params->payloads;
    return true;
}

bool IPCIntelPGParam::clientFlattenDecode(void* pData, int dataSize, uintptr_t client,
                                          int32_t payloadCount, const ia_binary_data* payloads,
                                          int32_t statsHandle) {
    CheckAndLogError(!pData || !payloads || dataSize < sizeof(pg_param_decode_params) ||
                         payloadCount > IPU_MAX_TERMINAL_COUNT,
                     false,
                     "@%s, Wrong parameters, pData: %p, payloads: %p, dataSize: %d, count: %d",
                     __func__, pData, payloads, dataSize, payloadCount);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    params->client = client;
    params->payloadCount = payloadCount;
    MEMCPY_S(params->payloads, sizeof(params->payloads), payloads,
             sizeof(ia_binary_data) * payloadCount);
    params->clientStatsHandle = statsHandle;
    return true;
}

bool IPCIntelPGParam::serverUnflattenDecode(void* pData, int dataSize, uintptr_t* client,
                                            int32_t* payloadCount, ia_binary_data** payloads) {
    CheckAndLogError(!pData || !client || !payloadCount || !payloads ||
                         dataSize < sizeof(pg_param_decode_params),
                     false,
                     "@%s, Wrong parameters, pData: %p, client: %p, payloadCount: %p, "
                     "payloads: %p, dataSize: %d",
                     __func__, pData, client, payloadCount, payloads, dataSize);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    *client = params->client;
    *payloadCount = params->payloadCount;
    *payloads = params->payloads;
    return true;
}

bool IPCIntelPGParam::serverFlattenDecode(void* pData, int dataSize,
                                          const ia_binary_data& statistics) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_decode_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);
    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    params->clientStatsSize = statistics.size;
    return true;
}

bool IPCIntelPGParam::clientUnflattenDecode(void* pData, int dataSize, ia_binary_data* statistics) {
    CheckAndLogError(!pData || !statistics || dataSize < sizeof(pg_param_decode_params), false,
                     "@%s, Wrong parameters, pData: %p, statistics: %p, dataSize: %d", __func__,
                     pData, statistics, dataSize);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    statistics->size = params->clientStatsSize;
    return true;
}

bool IPCIntelPGParam::clientFlattenDeinit(void* pData, int dataSize, uintptr_t client) {
    CheckAndLogError(!pData || dataSize < sizeof(pg_param_deinit_params), false,
                     "@%s, Wrong parameters, pData: %p, dataSize: %d", __func__, pData, dataSize);

    pg_param_deinit_params* params = static_cast<pg_param_deinit_params*>(pData);
    params->client = client;

    return true;
}

bool IPCIntelPGParam::serverUnflattenDeinit(const void* pData, int dataSize, uintptr_t* client) {
    CheckAndLogError(!pData || !client || dataSize < sizeof(pg_param_deinit_params), false,
                     "@%s, Wrong parameters, pData: %p, client: %p, dataSize: %d", __func__, pData,
                     client, dataSize);

    const pg_param_deinit_params* params = static_cast<const pg_param_deinit_params*>(pData);
    *client = params->client;

    return true;
}

}  // namespace icamera
