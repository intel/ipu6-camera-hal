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

#define LOG_TAG "IntelPGParamC"

#include "modules/sandboxing/client/IntelPGParam.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelPGParam::IntelPGParam(int pgId)
        : mInitialized(false),
          mPgId(pgId),
          mClient(reinterpret_cast<uintptr_t>(this)),
          mPayloadCount(0),
          mPGBuffer(nullptr) {
    std::string initName = "/pgParamInit" + std::to_string(mClient) + SHM_NAME;
    std::string prepareName = "/pgParamPrepare" + std::to_string(mClient) + SHM_NAME;
    std::string getFragDescsName = "/pgParamGetFragDescs" + std::to_string(mClient) + SHM_NAME;
    std::string prepareProgramName = "/pgParamPrepareProgram" + std::to_string(mClient) + SHM_NAME;
    std::string encodeName = "/pgParamEncode" + std::to_string(mClient) + SHM_NAME;
    std::string decodeName = "/pgParamDecode" + std::to_string(mClient) + SHM_NAME;
    std::string deinitName = "/pgParamDeinit" + std::to_string(mClient) + SHM_NAME;

    mMems = {
        {initName.c_str(), sizeof(pg_param_init_params), &mMemInit, false},
        {prepareName.c_str(), sizeof(pg_param_prepare_params), &mMemPrepare, false},
        {getFragDescsName.c_str(), sizeof(pg_param_get_fragment_desc_params), &mMemGetFragDescs,
         false},
        {prepareProgramName.c_str(), sizeof(pg_param_prepare_program_params), &mMemPrepareProgram,
         false},
        {encodeName.c_str(), sizeof(pg_param_encode_params), &mMemEncode, false},
        {decodeName.c_str(), sizeof(pg_param_decode_params), &mMemDecode, false},
        {deinitName.c_str(), sizeof(pg_param_deinit_params), &mMemDeinit, false},
    };

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    // Allocate when use
    mMemAllocatePGBuffer.mName = "/pgParamAllocPG" + std::to_string(mClient) + SHM_NAME;
    mMemAllocatePGBuffer.mSize = 0;
    mMemStatistics.mName = "/pgParamStats" + std::to_string(mClient) + SHM_NAME;
    mMemStatistics.mSize = 0;
    mMaxStatsSize = 0;

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelPGParam::~IntelPGParam() {
    mCommon.releaseAllShmMems(mMems);
    while (!mMemAllocatePayloads.empty()) {
        mCommon.freeShmMem(mMemAllocatePayloads.back());
        mMemAllocatePayloads.pop_back();
    }
    if (mMemAllocatePGBuffer.mSize) {
        mCommon.freeShmMem(mMemAllocatePGBuffer);
    }
    if (mMemStatistics.mSize) {
        mCommon.freeShmMem(mMemStatistics);
    }
}

int IntelPGParam::init(ia_p2p_platform_t platform, const PgConfiguration& pgConfig) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);

    bool ret =
        mIpc.clientFlattenInit(mMemInit.mAddr, mMemInit.mSize, mPgId, mClient, platform, pgConfig);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_INIT, mMemInit.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

int IntelPGParam::prepare(const ia_binary_data* ipuParameters, const ia_css_rbm_t* rbm,
                          ia_css_kernel_bitmap_t* bitmap, uint32_t* maxStatsSize) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    CheckError(!ipuParameters, INVALID_OPERATION, "@%s, ipuParams error", __func__);

    int32_t palHandle = mCommon.getShmMemHandle(ipuParameters->data);
    bool ret = mIpc.clientFlattenPrepare(mMemPrepare.mAddr, mMemPrepare.mSize, mClient,
                                         ipuParameters->size, palHandle, rbm);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenPrepare fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_PREPARE, mMemPrepare.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenPrepare(mMemPrepare.mAddr, mMemPrepare.mSize, bitmap, &mMaxStatsSize);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenPrepare fails", __func__);

    if (mMemStatistics.mAddr && mMemStatistics.mSize <= mMaxStatsSize) {
        mCommon.freeShmMem(mMemStatistics);
        mMemStatistics.mSize = 0;
        mMemStatistics.mAddr = nullptr;
    }
    if (maxStatsSize) *maxStatsSize = mMaxStatsSize;
    return OK;
}

int IntelPGParam::getFragmentDescriptors(int descCount, ia_p2p_fragment_desc* descs) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);

    bool ret = mIpc.clientFlattenGetFragDescs(mMemGetFragDescs.mAddr, mMemGetFragDescs.mSize,
                                              mClient, descCount);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenGetFragDescs fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_GET_FRAG_DESCS, mMemGetFragDescs.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    int count = 0;
    ret = mIpc.clientUnflattenGetFragDescs(mMemGetFragDescs.mAddr, mMemGetFragDescs.mSize, &count,
                                           descs);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenGetFragDescs fails", __func__);
    return count;
}

void* IntelPGParam::allocatePGBuffer(int pgSize) {
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    mPGBuffer = nullptr;
    int size = mIpc.getTotalPGBufferSize(pgSize);
    if (mMemAllocatePGBuffer.mAddr && mMemAllocatePGBuffer.mSize < size) {
        mCommon.freeShmMem(mMemAllocatePGBuffer);
        mMemAllocatePGBuffer.mSize = 0;
        mMemAllocatePGBuffer.mAddr = nullptr;
    }
    if (!mMemAllocatePGBuffer.mAddr) {
        mMemAllocatePGBuffer.mSize = size;
        bool ret = mCommon.allocShmMem(mMemAllocatePGBuffer.mName, mMemAllocatePGBuffer.mSize,
                                       &mMemAllocatePGBuffer);
        CheckError(ret == false, nullptr, "@%s, allocShmMem fails", __func__);
    }

    void* pgBuffer = nullptr;
    bool ret = mIpc.assignPGBuffer(mMemAllocatePGBuffer.mAddr, mMemAllocatePGBuffer.mSize, pgSize,
                                   &pgBuffer);
    CheckError(ret == false, nullptr, "@%s, assignPGBuffer fails", __func__);

    ret = mIpc.clientFlattenAllocatePGBuffer(mMemAllocatePGBuffer.mAddr, mMemAllocatePGBuffer.mSize,
                                             mClient, pgSize);
    CheckError(ret == false, nullptr, "@%s, clientFlattenAllocatePGBuffer fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_ALLOCATE_PG, mMemAllocatePGBuffer.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    mPGBuffer = reinterpret_cast<ia_css_process_group_t*>(pgBuffer);
    return mPGBuffer;
}

int IntelPGParam::setPGAndPrepareProgram(ia_css_process_group_t* pg) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    CheckError(mPGBuffer != pg, INVALID_OPERATION, "@%s, pg is not recognized", __func__);

    bool ret = mIpc.clientFlattenPrepareProgram(mMemPrepareProgram.mAddr, mMemPrepareProgram.mSize,
                                                mClient);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenPrepareProgram fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_PREPARE_PROGRAM, mMemPrepareProgram.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    // Get size of payloads
    mPayloadCount = ARRAY_SIZE(mPayloads);
    ret = mIpc.clientUnflattenPrepareProgram(mMemPrepareProgram.mAddr, mMemPrepareProgram.mSize,
                                             &mPayloadCount, mPayloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnlattenPrepareProgram fails", __func__);

    return OK;
}

int IntelPGParam::getPayloadSizes(int payloadCount, ia_binary_data* payloads) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    CheckError(payloadCount < mPayloadCount, UNKNOWN_ERROR, "@%s, payloadCount is small", __func__);
    CheckError(!payloads, UNKNOWN_ERROR, "@%s, payloads is nullptr", __func__);

    MEMCPY_S(payloads, sizeof(ia_binary_data) * payloadCount,
             mPayloads, sizeof(mPayloads));
    return mPayloadCount;
}

int IntelPGParam::allocatePayloads(int payloadCount, ia_binary_data* payloads) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    CheckError(payloadCount > IPU_MAX_TERMINAL_COUNT, UNKNOWN_ERROR, "@%s, payloadCount is small",
               __func__);
    CheckError(!payloads, UNKNOWN_ERROR, "@%s, payloads is nullptr", __func__);

    // Allocate memory
    int size = mIpc.getTotalPayloadSize(payloadCount, payloads);
    CheckError(size <= 0, UNKNOWN_ERROR, "@%s, payloads size error", __func__);
    mMemAllocatePayloads.resize(mMemAllocatePayloads.size() + 1);

    ShmMemInfo& info = mMemAllocatePayloads.back();
    info.mName = "/pgParamAllocPayloads" + std::to_string(mClient)
                 + std::to_string(mMemAllocatePayloads.size()) + SHM_NAME;
    info.mSize = size;
    info.mAddr = nullptr;
    bool ret = mCommon.allocShmMem(info.mName, info.mSize, &info);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, allocShmMem fails", __func__);

    // Split memory of payloads in client side
    ret = mIpc.assignPayloads(info.mAddr, info.mSize, payloadCount, payloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, allocatePayloads fails", __func__);

    ret = mIpc.clientFlattenRegisterPayloads(info.mAddr, info.mSize,
                                             mClient, payloadCount, payloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenRegisterPayloads fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_REGISTER_PAYLOADS, info.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

int IntelPGParam::updatePALAndEncode(const ia_binary_data* ipuParameters, int payloadCount,
                                     ia_binary_data* payloads) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    CheckError(!ipuParameters, INVALID_OPERATION, "@%s, ipuParams error", __func__);
    // Check shared memory of payloads
    CheckError(payloadCount != mPayloadCount, BAD_VALUE, "@%s, payloadCount error", __func__);

    int32_t palHandle = mCommon.getShmMemHandle(ipuParameters->data);
    bool ret = mIpc.clientFlattenEncode(mMemEncode.mAddr, mMemEncode.mSize, mClient,
                                        ipuParameters->size, palHandle,
                                        payloadCount, payloads);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenEncode fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_ENCODE, mMemEncode.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    return OK;
}

int IntelPGParam::decode(int payloadCount, ia_binary_data* payloads, ia_binary_data* statistics) {
    CheckError(mInitialized == false, INVALID_OPERATION, "@%s, mInitialized is false", __func__);
    // Check shared memory of payloads
    CheckError(payloadCount != mPayloadCount, BAD_VALUE, "@%s, payloadCount error", __func__);

    // Check share memory of statistics
    CheckError(!statistics, BAD_VALUE, "@%s, statistics nullptr", __func__);
    CheckError(!mMaxStatsSize, BAD_VALUE, "@%s, bad max stats size", __func__);
    bool ret = true;
    int32_t statsHandle = -1;
    // Prepare shared stats memory
    if (!mMemStatistics.mAddr) {
        mMemStatistics.mSize = mMaxStatsSize;
        ret = mCommon.allocShmMem(mMemStatistics.mName, mMemStatistics.mSize, &mMemStatistics);
        CheckError(ret == false, UNKNOWN_ERROR, "@%s, alloc statsData fails", __func__);
    }
    statsHandle = mCommon.getShmMemHandle(mMemStatistics.mAddr);

    ret = mIpc.clientFlattenDecode(mMemDecode.mAddr, mMemDecode.mSize, mClient,
                                   payloadCount, payloads, statsHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenDecode fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_DECODE, mMemDecode.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenDecode(mMemDecode.mAddr, mMemDecode.mSize, statistics);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenDecode fails", __func__);
    statistics->data = mMemStatistics.mAddr;

    return OK;
}

void IntelPGParam::deinit() {
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    bool ret = mIpc.clientFlattenDeinit(mMemDeinit.mAddr, mMemDeinit.mSize, mClient);
    CheckError(ret == false, VOID_VALUE, "@%s, clientFlattenDeinit fails", __func__);

    ret = mCommon.requestSync(IPC_PG_PARAM_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

}  // namespace icamera
