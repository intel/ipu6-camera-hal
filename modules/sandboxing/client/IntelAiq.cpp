/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "IntelAiq"

#include "modules/sandboxing/client/IntelAiq.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelAiq::IntelAiq() {
    LOG1("@%s", __func__);

    mAiq = reinterpret_cast<uintptr_t>(nullptr);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string deinitName = "/aiqDeinitShm" + std::to_string(personal) + "Shm";
    std::string aeName = "/aiqAeShm" + std::to_string(personal) + "Shm";
    std::string afName = "/aiqAfShm" + std::to_string(personal) + "Shm";
    std::string awbName = "/aiqAwbShm" + std::to_string(personal) + "Shm";
    std::string gbceName = "/aiqGbceShm" + std::to_string(personal) + "Shm";
    std::string aiqdName = "/aiqAiqdShm" + std::to_string(personal) + "Shm";
    std::string paName = "/aiqPaShm" + std::to_string(personal) + "Shm";
    std::string saName = "/aiqSaShm" + std::to_string(personal) + "Shm";
    std::string statName = "/aiqStatShm" + std::to_string(personal) + "Shm";
    std::string versionName = "/aiqVersionShm" + std::to_string(personal) + "Shm";

    mMems = {{deinitName.c_str(), sizeof(aiq_deinit_params), &mMemDeinit, false},
             {aeName.c_str(), sizeof(ae_run_params), &mMemAe, false},
             {afName.c_str(), sizeof(af_run_params), &mMemAf, false},
             {awbName.c_str(), sizeof(awb_run_params), &mMemAwb, false},
             {gbceName.c_str(), sizeof(gbce_run_params), &mMemGbce, false},
             {aiqdName.c_str(), sizeof(ia_binary_data_params), &mMemAiqd, false},
             {paName.c_str(), sizeof(pa_run_v1_params), &mMemPa, false},
             {saName.c_str(), sizeof(sa_run_v2_params), &mMemSa, false},
             {statName.c_str(), sizeof(set_statistics_set_v4_params), &mMemStat, false},
             {versionName.c_str(), sizeof(ia_aiq_version_params), &mMemVersion, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOG1("@%s, done", __func__);
    mInitialized = true;
}

IntelAiq::~IntelAiq() {
    LOG1("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

ia_aiq* IntelAiq::init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData,
                       const ia_binary_data* aiqdData, unsigned int statsMaxWidth,
                       unsigned int statsMaxHeight, unsigned int maxNumStatsIn, ia_cmc_t* cmc,
                       ia_mkn* mkn) {
    LOG1("@%s, aiqbData:%p, nvmData:%p, aiqdData:%p", __func__, aiqbData, nvmData, aiqdData);
    CheckError(!mInitialized, nullptr, "@%s, mInitialized is false", __func__);

    unsigned int aiqbSize = aiqbData ? aiqbData->size : 0;
    unsigned int nvmSize = nvmData ? nvmData->size : 0;
    unsigned int aiqdSize = aiqdData ? aiqdData->size : 0;
    unsigned int size = sizeof(aiq_init_params) + aiqbSize + nvmSize + aiqdSize;
    LOG2("@%s, aiqbSize:%d, nvmSize:%d, aiqdSize:%d", __func__, aiqbSize, nvmSize, aiqdSize);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/aiqInitShm" + std::to_string(personal) + "Shm";
    ShmMemInfo shm;
    shm.mName = initName.c_str();
    shm.mSize = size;
    bool ret = mCommon.allocShmMem(shm.mName, shm.mSize, &shm);
    CheckError(!ret, nullptr, "@%s, allocShmMem fails", __func__);

    ret = mIpc.clientFlattenInit(aiqbData, nvmData, aiqdData, statsMaxWidth, statsMaxHeight,
                                 maxNumStatsIn, reinterpret_cast<uintptr_t>(cmc),
                                 reinterpret_cast<uintptr_t>(mkn), static_cast<uint8_t*>(shm.mAddr),
                                 size);
    if (!ret) {
        mCommon.freeShmMem(shm);
        LOGE("@%s, clientFlattenInit fails", __func__);
        return nullptr;
    }

    ret = mCommon.requestSync(IPC_AIQ_INIT, shm.mHandle);
    if (!ret) {
        mCommon.freeShmMem(shm);
        LOGE("@%s, requestSync fails", __func__);
        return nullptr;
    }

    aiq_init_params* params = static_cast<aiq_init_params*>(shm.mAddr);
    mAiq = params->results;
    LOG2("@%s, success, aiq:%p\n", __func__, reinterpret_cast<ia_aiq*>(mAiq));

    mCommon.freeShmMem(shm);

    return reinterpret_cast<ia_aiq*>(mAiq);
}

ia_err IntelAiq::aeRun(const ia_aiq_ae_input_params* inputParams, ia_aiq_ae_results** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ae_run_params* params = static_cast<ae_run_params*>(mMemAe.mAddr);

    bool ret = mIpc.clientFlattenAe(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenAe fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_AE_RUN, mMemAe.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenAe(params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenAe fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::afRun(const ia_aiq_af_input_params* inputParams, ia_aiq_af_results** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    af_run_params* params = static_cast<af_run_params*>(mMemAf.mAddr);

    bool ret = mIpc.clientFlattenAf(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenAf fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_AF_RUN, mMemAf.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenAf(*params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenAf fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::awbRun(const ia_aiq_awb_input_params* inputParams, ia_aiq_awb_results** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    awb_run_params* params = static_cast<awb_run_params*>(mMemAwb.mAddr);

    bool ret = mIpc.clientFlattenAwb(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenAwb fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_AWB_RUN, mMemAwb.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenAwb(*params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenAwb fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::gbceRun(const ia_aiq_gbce_input_params* inputParams,
                         ia_aiq_gbce_results** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    gbce_run_params* params = static_cast<gbce_run_params*>(mMemGbce.mAddr);

    bool ret = mIpc.clientFlattenGbce(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenGbce fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_GBCE_RUN, mMemGbce.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenGbce(params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenGbce fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::paRunV1(const ia_aiq_pa_input_params* inputParams,
                         ia_aiq_pa_results_v1** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    pa_run_v1_params* params = static_cast<pa_run_v1_params*>(mMemPa.mAddr);

    bool ret = mIpc.clientFlattenPaV1(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenPaV1 fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_PA_RUN_V1, mMemPa.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenPaV1(params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenPaV1 fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::saRunV2(const ia_aiq_sa_input_params_v1* inputParams,
                         ia_aiq_sa_results_v1** results) {
    LOG1("@%s, inputParams:%p, results:%p", __func__, inputParams, results);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    sa_run_v2_params* params = static_cast<sa_run_v2_params*>(mMemSa.mAddr);

    bool ret = mIpc.clientFlattenSaV2(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenSaV2 fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_SA_RUN_V2, mMemSa.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenSaV2(params, results);
    CheckError(!ret, ia_err_general, "@%s, clientUnflattenSaV2 fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::statisticsSetV4(const ia_aiq_statistics_input_params_v4* inputParams) {
    LOG1("@%s, inputParams:%p", __func__, inputParams);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!inputParams, ia_err_argument, "@%s, inputParams is nullptr", __func__);

    set_statistics_set_v4_params* params =
        static_cast<set_statistics_set_v4_params*>(mMemStat.mAddr);

    bool ret = mIpc.clientFlattenStatSetV4(mAiq, *inputParams, params);
    CheckError(!ret, ia_err_general, "@%s, clientFlattenStatSetV4 fails", __func__);

    ret = mCommon.requestSync(IPC_AIQ_STATISTICS_SET_V4, mMemStat.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    return ia_err_none;
}

ia_err IntelAiq::getAiqdData(ia_binary_data* outData) {
    LOG1("@%s, outData:%p", __func__, outData);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, ia_err_general, "@%s, mAiq is nullptr",
               __func__);
    CheckError(!outData, ia_err_argument, "@%s, outData is nullptr", __func__);

    ia_binary_data_params* params = static_cast<ia_binary_data_params*>(mMemAiqd.mAddr);

    params->aiq_handle = mAiq;

    int ret = mCommon.requestSync(IPC_AIQ_GET_AIQD_DATA, mMemAiqd.mHandle);
    CheckError(!ret, ia_err_general, "@%s, requestSync fails", __func__);

    outData->data = params->data;
    outData->size = params->size;

    return ia_err_none;
}

void IntelAiq::deinit() {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, VOID_VALUE, "@%s, mAiq is nullptr",
               __func__);

    aiq_deinit_params* params = static_cast<aiq_deinit_params*>(mMemDeinit.mAddr);
    params->aiq_handle = mAiq;

    int ret = mCommon.requestSync(IPC_AIQ_DEINIT, mMemDeinit.mHandle);
    CheckError(!ret, VOID_VALUE, "@%s, requestSync fails", __func__);

    mAiq = reinterpret_cast<uintptr_t>(nullptr);
}

void IntelAiq::getVersion(std::string* version) {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(reinterpret_cast<ia_aiq*>(mAiq) == nullptr, VOID_VALUE, "@%s, mAiq is nullptr",
               __func__);

    ia_aiq_version_params* params = static_cast<ia_aiq_version_params*>(mMemVersion.mAddr);
    params->aiq_handle = mAiq;

    int ret = mCommon.requestSync(IPC_AIQ_GET_VERSION, mMemVersion.mHandle);
    CheckError(!ret, VOID_VALUE, "@%s, requestSync fails", __func__);

    *version = params->data;
}

} /* namespace icamera */
