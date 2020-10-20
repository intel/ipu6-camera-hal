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

#define LOG_TAG "IntelPGParamS"

#include "modules/sandboxing/server/IntelPGParamServer.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

IntelPGParamServer::IntelPGParamServer() {}

IntelPGParamServer::~IntelPGParamServer() {}

int IntelPGParamServer::init(void* pData, int dataSize) {
    int pgId = 0;
    uintptr_t client = 0;
    ia_p2p_platform_t platform = IA_P2P_PLATFORM_IPU6;
    PgConfiguration pgConfig;

    bool ret = mIpc.serverUnflattenInit(pData, dataSize, &pgId, &client, &platform, &pgConfig);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenInit fails", __func__);

    PGParamPackage package;
    package.pgId = pgId;
    package.mPayloadCount = 0;
    CLEAR(package.mPayloads);
    package.mPGBuffer = nullptr;
    mPGParamPackages[client] = package;
    mPGParamPackages[client].mPGParamAdapt = std::shared_ptr<IntelPGParam>(new IntelPGParam(pgId));
    int result = mPGParamPackages[client].mPGParamAdapt->init(platform, pgConfig);
    CheckError(result != OK, result, "@%s, init fails", __func__);

    return OK;
}

int IntelPGParamServer::prepare(void* pData, int dataSize, void* palDataAddr) {
    uintptr_t client = 0;
    ia_binary_data ipuParameters = {nullptr, 0};
    ia_css_rbm_t* rbm = nullptr;
    ia_css_kernel_bitmap_t* bitmap = nullptr;
    uint32_t* maxStatsSize = nullptr;
    bool ret = mIpc.serverUnflattenPrepare(pData, dataSize, &client, palDataAddr, &ipuParameters,
                                           &rbm, &bitmap, &maxStatsSize);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenPrepare fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);

    int result =
        mPGParamPackages[client].mPGParamAdapt->prepare(&ipuParameters, rbm, bitmap, maxStatsSize);
    CheckError(result != OK, result, "@%s, prepare fails", __func__);

    return OK;
}

int IntelPGParamServer::allocatePGBuffer(void* pData, int dataSize) {
    uintptr_t client = 0;
    int pgSize = 0;
    bool ret = mIpc.serverUnflattenAllocatePGBuffer(pData, dataSize, &client, &pgSize);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenAllocatePGBuffer fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);

    // Get server data pointer of PGBuffer
    void* pgBuffer = nullptr;
    ret = mIpc.assignPGBuffer(pData, dataSize, pgSize, &pgBuffer);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, assignPGBuffer fails", __func__);

    mPGParamPackages[client].mPGBuffer = reinterpret_cast<ia_css_process_group_t*>(pgBuffer);
    return OK;
}

int IntelPGParamServer::getFragmentDescriptors(void* pData, int dataSize) {
    uintptr_t client = 0;
    int descCount = 0;
    ia_p2p_fragment_desc* descs = nullptr;
    bool ret = mIpc.serverUnflattenGetFragDescs(pData, dataSize, &client, &descCount, &descs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenGetFragDescs fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);

    int count = mPGParamPackages[client].mPGParamAdapt->getFragmentDescriptors(descCount, descs);
    CheckError(count <= 0, count, "@%s, getFragmentDescriptors fails", __func__);

    ret = mIpc.serverFlattenGetFragDescs(pData, dataSize, count);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenGetFragDescs fails", __func__);

    return OK;
}

int IntelPGParamServer::setPGAndPrepareProgram(void* pData, int dataSize) {
    uintptr_t client = 0;
    bool ret = mIpc.serverUnflattenPrepareProgram(pData, dataSize, &client);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenPrepareProgram fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);
    PGParamPackage& package = mPGParamPackages[client];

    int result = package.mPGParamAdapt->setPGAndPrepareProgram(package.mPGBuffer);
    CheckError(result != OK, result, "@%s, setPGAndPrepareProgram fails", __func__);

    // Get payload size here
    package.mPayloadCount =
        package.mPGParamAdapt->getPayloadSizes(ARRAY_SIZE(package.mPayloads), package.mPayloads);
    CheckError(!package.mPayloadCount, UNKNOWN_ERROR, "@%s, getPayloadSizes fails", __func__);

    ret =
        mIpc.serverFlattenPrepareProgram(pData, dataSize, package.mPayloadCount, package.mPayloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenPrepareProgram fails", __func__);

    return OK;
}

int IntelPGParamServer::registerPayloads(void* pData, int dataSize) {
    uintptr_t client = 0;
    int payloadCount = 0;
    ia_binary_data* cPayloads = nullptr;
    ia_binary_data* sPayloads = nullptr;
    bool ret = mIpc.serverUnflattenRegisterPayloads(pData, dataSize, &client, &payloadCount,
                                                    &cPayloads, &sPayloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenRegisterPayloads fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);
    PGParamPackage& package = mPGParamPackages[client];

    // Save <client addr, server addr>
    for (int i = 0; i < payloadCount; i++) {
        if (cPayloads[i].size > 0) {
            package.mAllocatedPayloads[cPayloads[i].data] = sPayloads[i];
        }
    }

    return OK;
}

int IntelPGParamServer::updatePALAndEncode(void* pData, int dataSize, void* palDataAddr) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelPGParamServer", "updatePALAndEncode");
    uintptr_t client = 0;
    ia_binary_data ipuParameters = {nullptr, 0};
    ia_binary_data* payloads = nullptr;
    int32_t payloadCount = 0;

    bool ret = mIpc.serverUnflattenEncode(pData, dataSize, &client, palDataAddr, &ipuParameters,
                                          &payloadCount, &payloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenEncode fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);
    PGParamPackage& package = mPGParamPackages[client];
    CheckError(payloadCount != package.mPayloadCount, UNKNOWN_ERROR, "@%s, wrong payloadCount",
               __func__);

    int result = findPayloads(package.mPayloadCount, payloads, &package.mAllocatedPayloads,
                              package.mPayloads);
    CheckError(result != OK, result, "@%s, findPayloads fails", __func__);

    result = package.mPGParamAdapt->updatePALAndEncode(&ipuParameters,
                                                       package.mPayloadCount,
                                                       package.mPayloads);
    CheckError(result != OK, result, "@%s, updatePALAndEncode fails", __func__);

    return OK;
}

int IntelPGParamServer::decode(void* pData, int dataSize, void* statsAddr) {
    PERF_CAMERA_ATRACE();
    TRACE_LOG_PROCESS("IntelPGParamServer", "decode");
    uintptr_t client = 0;
    ia_binary_data statistics = {statsAddr, 0};
    ia_binary_data* payloads = nullptr;
    int32_t payloadCount = 0;

    bool ret = mIpc.serverUnflattenDecode(pData, dataSize, &client, &payloadCount, &payloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverUnflattenDecode fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), UNKNOWN_ERROR,
               "%s, the pg doesn't exist in the table", __func__);
    PGParamPackage& package = mPGParamPackages[client];
    CheckError(payloadCount != package.mPayloadCount, UNKNOWN_ERROR, "@%s, wrong payloadCount",
               __func__);

    int result = findPayloads(package.mPayloadCount, payloads, &package.mAllocatedPayloads,
                              package.mPayloads);
    CheckError(result != OK, result, "@%s, findPayloads fails", __func__);

    result = package.mPGParamAdapt->decode(package.mPayloadCount, package.mPayloads, &statistics);
    CheckError(result != OK, result, "@%s, decode fails", __func__);

    ret = mIpc.serverFlattenDecode(pData, dataSize, statistics);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, serverFlattenDecode fails", __func__);

    return OK;
}

void IntelPGParamServer::deinit(void* pData, int dataSize) {
    uintptr_t client = 0;

    bool ret = mIpc.serverUnflattenDeinit(pData, dataSize, &client);
    CheckError(ret == false, VOID_VALUE, "@%s, serverUnflattenDeinit fails", __func__);

    CheckError((mPGParamPackages.find(client) == mPGParamPackages.end()), VOID_VALUE,
               "%s, the pg doesn't exist in the table", __func__);

    mPGParamPackages[client].mPGParamAdapt->deinit();
    mPGParamPackages.erase(client);
}

int IntelPGParamServer::findPayloads(int32_t payloadCount, ia_binary_data* clientPayloads,
                                     std::unordered_map<void*, ia_binary_data>* allocated,
                                     ia_binary_data* serverPayloads) {
    CheckError(!clientPayloads, BAD_VALUE, "@%s, payloads is nullptr", __func__);
    CLEAR(*serverPayloads);
    for (size_t i = 0; i < payloadCount; i++) {
        if (!clientPayloads[i].size) continue;

        CheckError(allocated->find(clientPayloads[i].data) == allocated->end(), UNKNOWN_ERROR,
                   "%s: can't find client payload %p for term %d", __func__,
                   clientPayloads[i].data, i);
        ia_binary_data& alloc = allocated->at(clientPayloads[i].data);
        CheckError(alloc.size != clientPayloads[i].size, UNKNOWN_ERROR,
                   "%s: payload size mismatch for term %d", __func__, i);
        serverPayloads[i] = alloc;
    }
    return OK;
}

}  // namespace icamera
