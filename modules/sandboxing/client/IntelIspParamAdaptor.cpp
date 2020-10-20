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

#define LOG_TAG "IntelIspParamAdaptor"

#include "modules/sandboxing/client/IntelIspParamAdaptor.h"

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelIspParamAdaptor::IntelIspParamAdaptor() : mInitialized(false) {
    LOGIPC("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/adaptorInit" + std::to_string(personal) + SHM_NAME;
    std::string deInitName = "/adaptorDeinit" + std::to_string(personal) + SHM_NAME;
    std::string getSize = "/adaptorGetSize" + std::to_string(personal) + SHM_NAME;
    std::string convertStats = "/adaptorConvertStats" + std::to_string(personal) + SHM_NAME;
    std::string runPal = "/adaptorRunPal" + std::to_string(personal) + SHM_NAME;

    mMems = {{initName.c_str(), sizeof(IspBxtInitParam), &mMemInit, false},
             {deInitName.c_str(), sizeof(IspBxtDeInitParam), &mMemDeinit, false},
             {getSize.c_str(), sizeof(PalDataSizeParam), &mMemGetSize, false},
             {convertStats.c_str(), sizeof(ConvertStatsParam), &mMemStats, false},
             {runPal.c_str(), sizeof(RunPalParam), &mMemRunPal, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelIspParamAdaptor::~IntelIspParamAdaptor() {
    LOGIPC("@%s", __func__);
    mPalDataMems.clear();
    mCommon.releaseAllShmMems(mMems);
}

ia_isp_bxt* IntelIspParamAdaptor::init(const ia_binary_data* ispData, const ia_cmc_t* iaCmc,
                                       unsigned int maxStatsWidth, unsigned int maxStatsHeight,
                                       unsigned int maxNumStatsIn, ia_mkn* iaMkn) {
    CheckError(!ispData || !iaCmc, nullptr, "%s, No CPF or CMC data", __func__);
    LOGIPC("%s, ispData size: %d, pointer: %p, max width: %u, max height: %u", __func__,
           ispData->size, ispData->data, maxStatsWidth, maxStatsHeight);
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    bool ret = mIpc.clientFlattenInit(mMemInit.mAddr, mMemInit.mSize, ispData, iaCmc, maxStatsWidth,
                                      maxStatsHeight, maxNumStatsIn, iaMkn);
    CheckError(ret == false, nullptr, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_ISP_ADAPTOR_INIT, mMemInit.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    IspBxtInitParam* params = static_cast<IspBxtInitParam*>(mMemInit.mAddr);
    return reinterpret_cast<ia_isp_bxt*>(params->ispRemoteHandle);
}

void IntelIspParamAdaptor::deInit(const ia_isp_bxt* ispBxtHandle) {
    LOGIPC("@%s", __func__);
    CheckError(!ispBxtHandle, VOID_VALUE, "%s, ispBxtHandle is nullptr", __func__);
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    bool ret = mIpc.clientFlattenDeInit(mMemDeinit.mAddr, mMemDeinit.mSize, ispBxtHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, clientFlattenDeInit fails", __func__);

    ret = mCommon.requestSync(IPC_ISP_ADAPTOR_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}

int IntelIspParamAdaptor::getPalDataSize(const ia_isp_bxt_program_group* programGroup) {
    LOGIPC("@%s", __func__);
    CheckError(mInitialized == false, -1, "@%s, mInitialized is false", __func__);
    CheckError(!programGroup, -1, "%s, programGroup is nullptr", __func__);

    bool ret = mIpc.clientFlattenGetPalSize(mMemGetSize.mAddr, mMemGetSize.mSize, programGroup);
    CheckError(ret == false, -1, "@%s, clientFlattenGetPalSize fails", __func__);

    ret = mCommon.requestSync(IPC_ISP_GET_PAL_SIZE, mMemGetSize.mHandle);
    CheckError(ret == false, -1, "@%s, requestSync fails", __func__);

    PalDataSizeParam* params = static_cast<PalDataSizeParam*>(mMemGetSize.mAddr);
    LOGIPC("@%s, the pal data size is: %d", __func__, params->palDataSize);

    return params->palDataSize;
}

status_t IntelIspParamAdaptor::queryAndConvertStats(const ia_isp_bxt* ispBxtHandle,
                                                    const ConvertInputParam* inputParams,
                                                    ConvertResult* result) {
    LOGIPC("@%s", __func__);
    CheckError(!ispBxtHandle, UNKNOWN_ERROR, "%s, ispBxtHandle is nullptr", __func__);
    CheckError(!inputParams || !result, UNKNOWN_ERROR, "%s, inputParams or result nullptr",
               __func__);
    CheckError(!inputParams->dvsReso || !inputParams->aeResults, UNKNOWN_ERROR,
               "%s, inputParams or result nullptr", __func__);
    CheckError((!inputParams->statsBuffer || !inputParams->statsBuffer->data ||
                inputParams->statsBuffer->size <= 0),
               UNKNOWN_ERROR, "%s, Wrong statistics buffer", __func__);
    CheckError(!result->queryResults, UNKNOWN_ERROR, "%s, queryResults is nullptr", __func__);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    int32_t statsHandle = mCommon.getShmMemHandle(inputParams->statsBuffer->data);
    bool ret = mIpc.clientFlattenConvertStats(mMemStats.mAddr, mMemStats.mSize, ispBxtHandle,
                                              inputParams, statsHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenConvertStats fails", __func__);

    ret = mCommon.requestSync(IPC_ISP_CONVERT_STATS, mMemStats.mHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenConvertStats(mMemStats.mAddr, mMemStats.mSize, result);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientUnflattenConvertStats fails", __func__);

    return OK;
}

void* IntelIspParamAdaptor::allocatePalBuffer(int streamId, int index, int palDataSize) {
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string palDataName = "/palData" + std::to_string(streamId) + std::to_string(index) +
                              std::to_string(personal) + SHM_NAME;

    ShmMemInfo memInfo;
    CLEAR(memInfo);
    bool success = mCommon.allocShmMem(palDataName, palDataSize, &memInfo);
    CheckError(success == false, nullptr, "%s, failed to allocate share memory for pal", __func__);
    LOGIPC("%s, the buffer handle: %d, address: %p", __func__, memInfo.mHandle, memInfo.mAddr);

    mPalDataMems.push_back(memInfo);

    return memInfo.mAddr;
}

void IntelIspParamAdaptor::freePalBuffer(void* addr) {
    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    for (auto& mem : mPalDataMems) {
        if (mem.mAddr == addr) {
            mCommon.freeShmMem(mem);
        }
    }
}

status_t IntelIspParamAdaptor::runPal(ia_isp_bxt* ispBxtHandle,
                                      const ia_isp_bxt_input_params_v2* inputParams,
                                      ia_binary_data* outputData) {
    LOGIPC("@%s", __func__);
    CheckError((!ispBxtHandle || !inputParams), UNKNOWN_ERROR,
               "%s, ispBxtHandle or inputParams is nullptr", __func__);
    CheckError((!outputData || !outputData->data || outputData->size <= 0), UNKNOWN_ERROR,
               "%s, Wrong pal data buffer", __func__);
    CheckError(mInitialized == false, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);

    int32_t palDataHandle = mCommon.getShmMemHandle(outputData->data);
    CheckError(!palDataHandle, UNKNOWN_ERROR, "%s, the pal buffer(%p) doesn't exist in SHM list",
               __func__, outputData->data);
    LOGIPC("%s, pal buffer address: %p, buffer handle: %d", __func__, outputData->data,
           palDataHandle);

    bool ret = mIpc.clientFlattenRunPal(mMemRunPal.mAddr, mMemRunPal.mSize, ispBxtHandle,
                                        inputParams, outputData, palDataHandle);
    CheckError(ret == false, UNKNOWN_ERROR, "@%s, clientFlattenRunPal fails", __func__);

    ret = mCommon.requestSync(IPC_ISP_RUN_PAL, mMemRunPal.mHandle);
    CheckError(ret == false, -1, "@%s, requestSync fails", __func__);

    RunPalParam* params = static_cast<RunPalParam*>(mMemRunPal.mAddr);
    outputData->size = params->palOutput.size;
    LOGIPC("%s, the buffer handle is: %d size: %d ptr: %p after running pal", __func__,
           params->palDataHandle, outputData->size, outputData->data);

    return OK;
}

}  // namespace icamera
