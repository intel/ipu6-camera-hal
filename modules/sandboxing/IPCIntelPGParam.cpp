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

#define LOG_TAG "IPCIntelPGParam"

#include "modules/sandboxing/IPCIntelPGParam.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

bool IPCIntelPGParam::clientFlattenInit(void* pData, int dataSize, int pgId, uintptr_t client,
                                        ia_p2p_platform_t platform,
                                        const PgConfiguration& pgConfig) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_init_params), false, "@%s, dataSize is small", __func__);

    pg_param_init_params* params = static_cast<pg_param_init_params*>(pData);
    CheckError(pgConfig.pgManifestSize > sizeof(params->pgManifestData), false,
               "@%s, manifest buffer is small", __func__);

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
    CheckError(params->disableDataTermialsCount > IPU_MAX_TERMINAL_COUNT, false,
               "@%s, disableDataTermials is big", __func__);
    memset(params->disableDataTermialsData, -1, sizeof(params->disableDataTermialsData));
    for (int i = 0; i < params->disableDataTermialsCount; i++) {
        params->disableDataTermialsData[i] = pgConfig.disableDataTermials[i];
    }

    return true;
}

bool IPCIntelPGParam::serverUnflattenInit(void* pData, int dataSize, int* pgId, uintptr_t* client,
                                          ia_p2p_platform_t* platform, PgConfiguration* pgConfig) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!pgId || !client, false, "@%s, nullptr client", __func__);
    CheckError(!platform || !pgConfig, false, "@%s, nullptr config", __func__);

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
    CheckError(params->disableDataTermialsCount > IPU_MAX_TERMINAL_COUNT, false,
               "@%s, disableDataTermials is big", __func__);
    for (int i = 0; i < params->disableDataTermialsCount; i++) {
        pgConfig->disableDataTermials.push_back(params->disableDataTermialsData[i]);
    }

    return true;
}

bool IPCIntelPGParam::clientFlattenPrepare(void* pData, int dataSize, uintptr_t client,
                                           unsigned int ipuParamSize, int32_t ipuParamHandle,
                                           const ia_css_rbm_t* rbm) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_params), false, "@%s, buffer is small", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_params), false, "@%s, buffer is small", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);
    CheckError(!ipuParameters || !rbm || !bitmap, false, "@%s, nullptr outputs", __func__);
    CheckError(!palDataAddr, false, "%s, palDataAddr is nullptr", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_params), false, "@%s, buffer is small", __func__);
    CheckError(!bitmap, false, "@%s, bitmap is nullptr", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < pgSize, false, "@%s, buffer is small", __func__);
    CheckError(!pgBuffer, false, "@%s, payloads is nullptr", __func__);

    uintptr_t pgAddr = reinterpret_cast<uintptr_t>(pData);
    CheckError(pgAddr & ((getpagesize() - 1)), false, "@%s, pg addr is not aligned", __func__);
    *pgBuffer = pData;
    return true;
}

bool IPCIntelPGParam::clientFlattenAllocatePGBuffer(void* pData, int dataSize, uintptr_t client,
                                                    int pgSize) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_allocate_pg_params);
    CheckError(paramAddr < reinterpret_cast<uintptr_t>(pData) + pgSize, false,
               "@%s, dataSize is small", __func__);

    pg_param_allocate_pg_params* params = reinterpret_cast<pg_param_allocate_pg_params*>(paramAddr);
    params->client = client;
    params->pgSize = pgSize;

    return true;
}

bool IPCIntelPGParam::serverUnflattenAllocatePGBuffer(const void* pData, int dataSize,
                                                      uintptr_t* client, int* pgSize) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_allocate_pg_params), false, "@%s, buffer is small",
               __func__);
    CheckError(!client || !pgSize, false, "@%s, nullptr input", __func__);

    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_allocate_pg_params);
    pg_param_allocate_pg_params* params = reinterpret_cast<pg_param_allocate_pg_params*>(paramAddr);
    CheckError(paramAddr < reinterpret_cast<uintptr_t>(pData) + params->pgSize, false,
               "@%s, dataSize is small", __func__);
    *client = params->client;
    *pgSize = params->pgSize;

    return true;
}

bool IPCIntelPGParam::clientFlattenGetFragDescs(void* pData, int dataSize, uintptr_t client,
                                                int descCount) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_get_fragment_desc_params), false,
               "@%s, dataSize is small", __func__);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    CheckError(descCount > sizeof(params->descsData), false, "@%s, descCount is big", __func__);

    params->client = client;
    params->descCount = descCount;
    return true;
}

bool IPCIntelPGParam::serverUnflattenGetFragDescs(void* pData, int dataSize, uintptr_t* client,
                                                  int* descCount, ia_p2p_fragment_desc** descs) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_get_fragment_desc_params), false,
               "@%s, dataSize is small", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);
    CheckError(!descCount || !descs, false, "@%s, nullptr outputs", __func__);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    *client = params->client;
    *descCount = params->descCount;
    *descs = params->descsData;
    return true;
}

bool IPCIntelPGParam::serverFlattenGetFragDescs(void* pData, int dataSize, int count) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_get_fragment_desc_params), false,
               "@%s, dataSize is small", __func__);

    pg_param_get_fragment_desc_params* params =
        static_cast<pg_param_get_fragment_desc_params*>(pData);
    params->returnCount = count;
    return true;
}

bool IPCIntelPGParam::clientUnflattenGetFragDescs(const void* pData, int dataSize, int* count,
                                                  ia_p2p_fragment_desc* descs) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_get_fragment_desc_params), false,
               "@%s, dataSize is small", __func__);
    CheckError(!count || !descs, false, "@%s, nullptr outputs", __func__);

    const pg_param_get_fragment_desc_params* params =
        static_cast<const pg_param_get_fragment_desc_params*>(pData);
    *count = params->returnCount;
    MEMCPY_S(descs, sizeof(ia_p2p_fragment_desc) * params->descCount, params->descsData,
             sizeof(params->descsData));
    return true;
}

bool IPCIntelPGParam::clientFlattenPrepareProgram(void* pData, int dataSize, uintptr_t client) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_program_params), false, "@%s, dataSize is small",
               __func__);

    pg_param_prepare_program_params* params = static_cast<pg_param_prepare_program_params*>(pData);
    params->client = client;
    return true;
}

bool IPCIntelPGParam::serverUnflattenPrepareProgram(const void* pData, int dataSize,
                                                    uintptr_t* client) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_program_params), false, "@%s, dataSize is small",
               __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);

    const pg_param_prepare_program_params* params =
        static_cast<const pg_param_prepare_program_params*>(pData);
    *client = params->client;
    return true;
}

bool IPCIntelPGParam::serverFlattenPrepareProgram(void* pData, int dataSize, int payloadCount,
                                                  const ia_binary_data* payloads) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_program_params), false, "@%s, dataSize is small",
               __func__);
    CheckError(payloadCount > IPU_MAX_TERMINAL_COUNT, false, "@%s, payloadCount is big", __func__);
    CheckError(!payloads, false, "@%s, payloads is nullptr", __func__);

    pg_param_prepare_program_params* params = static_cast<pg_param_prepare_program_params*>(pData);
    params->payloadCount = payloadCount;
    MEMCPY_S(params->payloads, sizeof(ia_binary_data) * payloadCount, payloads,
             sizeof(ia_binary_data) * payloadCount);
    return true;
}

bool IPCIntelPGParam::clientUnflattenPrepareProgram(const void* pData, int dataSize,
                                                    int* payloadCount, ia_binary_data* payloads) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_prepare_program_params), false, "@%s, dataSize is small",
               __func__);
    CheckError(!payloads, false, "@%s, payloads is nullptr", __func__);

    const pg_param_prepare_program_params* params =
        static_cast<const pg_param_prepare_program_params*>(pData);

    CheckError(*payloadCount < params->payloadCount, false, "@%s, payloadCount is small", __func__);
    *payloadCount = params->payloadCount;
    for (int i = 0; i < params->payloadCount; i++) {
        payloads[i].size = params->payloads[i].size;
        payloads[i].data = nullptr;
    }
    return true;
}

int IPCIntelPGParam::getTotalPayloadSize(int payloadCount, const ia_binary_data* payloads) {
    CheckError(!payloads, 0, "@%s, payloads is nullptr", __func__);
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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!payloads, false, "@%s, payloads is nullptr", __func__);

    uintptr_t payloadAddr = reinterpret_cast<uintptr_t>(pData);
    CheckError(payloadAddr & ((getpagesize() - 1)), false, "@%s, payload addr is not aligned",
               __func__);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size > 0) {
            CheckError(payloadAddr > reinterpret_cast<uintptr_t>(pData) + dataSize, false,
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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_register_payloads_params), false,
               "@%s, dataSize is small", __func__);
    CheckError(!payloads, false, "@%s, payloads is nullptr", __func__);
    CheckError(payloadCount > IPU_MAX_TERMINAL_COUNT, false, "@%s, payloadCount is big", __func__);

    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_register_payloads_params);
    pg_param_register_payloads_params* params =
        reinterpret_cast<pg_param_register_payloads_params*>(paramAddr);
    params->client = client;
    params->payloadCount = payloadCount;
    MEMCPY_S(params->cPayloads, sizeof(params->cPayloads),
             payloads, sizeof(ia_binary_data) * payloadCount);
    getPayloadOffsets(pData, dataSize, payloadCount, payloads, params->payloadOffsets);
    return true;
}

bool IPCIntelPGParam::serverUnflattenRegisterPayloads(void* pData, int dataSize, uintptr_t* client,
                                                      int* payloadCount,
                                                      ia_binary_data** cPayloads,
                                                      ia_binary_data** sPayloads) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_register_payloads_params), false,
               "@%s, dataSize is small", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);
    CheckError(!payloadCount, false, "@%s, zero payloadCount", __func__);
    CheckError(!cPayloads, false, "@%s, cPayloads is nullptr", __func__);
    CheckError(!sPayloads, false, "@%s, sPayloads is nullptr", __func__);

    uintptr_t paramAddr =
        reinterpret_cast<uintptr_t>(pData) + dataSize - sizeof(pg_param_register_payloads_params);
    pg_param_register_payloads_params* params =
        reinterpret_cast<pg_param_register_payloads_params*>(paramAddr);
    *client = params->client;
    MEMCPY_S(params->sPayloads, sizeof(params->sPayloads),
             params->cPayloads, sizeof(params->cPayloads));  // save size to sPayloads
    *payloadCount = params->payloadCount;
    getPayloadData(pData, dataSize, *payloadCount, params->payloadOffsets, params->sPayloads);
    *cPayloads = params->cPayloads;
    *sPayloads = params->sPayloads;
    return true;
}

bool IPCIntelPGParam::getPayloadOffsets(const void* pData, int dataSize, int payloadCount,
                                        const ia_binary_data* payloads, int32_t* payloadOffsets) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!payloads, false, "@%s, nullptr payloads", __func__);
    CheckError(!payloadOffsets, false, "@%s, nullptr payloadOffsets", __func__);

    uintptr_t startAddr = reinterpret_cast<uintptr_t>(pData);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size <= 0) continue;

        int32_t offset = reinterpret_cast<uintptr_t>(payloads[i].data) - startAddr;
        CheckError((offset > (dataSize - payloads[i].size) || offset < 0), false,
                   "@%s, error offset %d", __func__, offset);
        payloadOffsets[i] = offset;
    }
    return 0;
}

bool IPCIntelPGParam::getPayloadData(void* pData, int dataSize, int payloadCount,
                                     const int32_t* payloadOffsets, ia_binary_data* payloads) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(!payloads, false, "@%s, nullptr payloads", __func__);
    CheckError(!payloadOffsets, false, "@%s, nullptr payloadOffsets", __func__);

    unsigned char* startAddr = reinterpret_cast<unsigned char*>(pData);
    for (int i = 0; i < payloadCount; i++) {
        if (payloads[i].size <= 0) continue;

        CheckError(payloadOffsets[i] > dataSize - payloads[i].size, false, "@%s, error offset %d",
                   __func__, payloadOffsets[i]);
        payloads[i].data = startAddr + payloadOffsets[i];
    }
    return true;
}

bool IPCIntelPGParam::clientFlattenEncode(void* pData, int dataSize, uintptr_t client,
                                          unsigned int ipuParamSize, int32_t ipuParamHandle,
                                          int32_t payloadCount, const ia_binary_data* payloads) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_encode_params), false, "@%s, buffer is small", __func__);
    CheckError(!payloads, false, "@%s, nullptr payloads", __func__);
    CheckError(payloadCount > IPU_MAX_TERMINAL_COUNT, false, "@%s, wrong payloadCount", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_encode_params), false, "@%s, small dataSize", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);
    CheckError(!ipuParameters, false, "@%s, nullptr output", __func__);
    CheckError(!palDataAddr, false, "%s, palDataAddr is nullptr", __func__);
    CheckError(!payloadCount, false, "%s, payloadCount is nullptr", __func__);
    CheckError(!payloads, false, "%s, payloads is nullptr", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_decode_params), false, "@%s, size is small", __func__);
    CheckError(!payloads, false, "@%s, nullptr payloads", __func__);
    CheckError(payloadCount > IPU_MAX_TERMINAL_COUNT, false, "@%s, wrong payloadCount", __func__);

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
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_decode_params), false, "@%s, size is small", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);
    CheckError(!payloadCount, false, "%s, payloadCount is nullptr", __func__);
    CheckError(!payloads, false, "%s, payloads is nullptr", __func__);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    *client = params->client;
    *payloadCount = params->payloadCount;
    *payloads = params->payloads;
    return true;
}

bool IPCIntelPGParam::serverFlattenDecode(void* pData, int dataSize,
                                          const ia_binary_data& statistics) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_decode_params), false, "@%s, size is small", __func__);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    params->clientStatsSize = statistics.size;
    return true;
}

bool IPCIntelPGParam::clientUnflattenDecode(void* pData, int dataSize, ia_binary_data* statistics) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_decode_params), false, "@%s, size is small", __func__);
    CheckError(!statistics, false, "@%s, statistics is nullptr", __func__);

    pg_param_decode_params* params = static_cast<pg_param_decode_params*>(pData);
    statistics->size = params->clientStatsSize;
    return true;
}

bool IPCIntelPGParam::clientFlattenDeinit(void* pData, int dataSize, uintptr_t client) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_deinit_params), false, "@%s, buffer is small", __func__);

    pg_param_deinit_params* params = static_cast<pg_param_deinit_params*>(pData);
    params->client = client;

    return true;
}

bool IPCIntelPGParam::serverUnflattenDeinit(const void* pData, int dataSize, uintptr_t* client) {
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < sizeof(pg_param_deinit_params), false, "@%s, size is small", __func__);
    CheckError(!client, false, "@%s, nullptr client", __func__);

    const pg_param_deinit_params* params = static_cast<const pg_param_deinit_params*>(pData);
    *client = params->client;

    return true;
}

}  // namespace icamera
