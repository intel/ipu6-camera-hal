/*
 * Copyright (C) 2020 Intel Corporation
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

#define LOG_TAG "IntelCca"

#include "modules/sandboxing/client/IntelCca.h"

#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    LOG1("@%s, cameraId: %d, tuningMode: %d", __func__, cameraId, mode);

    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId) {
            if (it.ccaHandle.find(mode) == it.ccaHandle.end()) it.ccaHandle[mode] = new IntelCca();

            return it.ccaHandle[mode];
        }
    }

    IntelCca::CCAHandle handle = {};
    handle.cameraId = cameraId;
    handle.ccaHandle[mode] = new IntelCca();
    sCcaInstance.push_back(handle);

    return handle.ccaHandle[mode];
}

void IntelCca::releaseInstance(int cameraId, TuningMode mode) {
    LOG1("@%s, cameraId: %d, tuningMode: %d", __func__, cameraId, mode);

    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId && it.ccaHandle.find(mode) != it.ccaHandle.end()) {
            IntelCca* cca = it.ccaHandle[mode];
            it.ccaHandle.erase(mode);
            delete cca;
            cca = nullptr;
        }
    }
}

IntelCca::IntelCca() {
    LOG1("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/ccaInit" + std::to_string(personal) + "Shm";
    std::string statsName = "/ccaStats" + std::to_string(personal) + "Shm";
    std::string aecName = "/ccaAec" + std::to_string(personal) + "Shm";
    std::string aiqName = "/ccaAiq" + std::to_string(personal) + "Shm";
    std::string ltmName = "/ccaLtm" + std::to_string(personal) + "Shm";
    std::string zoomName = "/ccaZoom" + std::to_string(personal) + "Shm";
    std::string dvsName = "/ccaDvs" + std::to_string(personal) + "Shm";
    std::string aicName = "/ccaAic" + std::to_string(personal) + "Shm";
    std::string cmcName = "/ccaCmc" + std::to_string(personal) + "Shm";
    std::string mknName = "/ccaMkn" + std::to_string(personal) + "Shm";
    std::string aiqdName = "/ccaAiqd" + std::to_string(personal) + "Shm";
    std::string tuningName = "/ccaTuning" + std::to_string(personal) + "Shm";
    std::string versionName = "/ccaVersion" + std::to_string(personal) + "Shm";
    std::string decodeStatsName = "/ccaDecodeStats" + std::to_string(personal) + "Shm";
    std::string palSizeName = "/ccaPalSize" + std::to_string(personal) + "Shm";

    mMems = {
        {initName.c_str(), sizeof(cca::cca_init_params), &mMemInit, false},
        {statsName.c_str(), sizeof(cca::cca_stats_params), &mMemStats, false},
        {aecName.c_str(), sizeof(intel_cca_run_aec_data), &mMemAEC, false},
        {aiqName.c_str(), sizeof(intel_cca_run_aiq_data), &mMemAIQ, false},
        {ltmName.c_str(), sizeof(intel_cca_run_ltm_data), &mMemLTM, false},
        {zoomName.c_str(), sizeof(cca::cca_dvs_zoom), &mMemZoom, false},
        {dvsName.c_str(), sizeof(uint64_t), &mMemDVS, false},
        {aicName.c_str(), sizeof(intel_cca_run_aic_data), &mMemAIC, false},
        {cmcName.c_str(), sizeof(cca::cca_cmc), &mMemCMC, false},
        {mknName.c_str(), sizeof(intel_cca_mkn_data), &mMemMKN, false},
        {aiqdName.c_str(), sizeof(cca::cca_aiqd), &mMemAIQD, false},
        {tuningName.c_str(), sizeof(intel_cca_update_tuning_data), &mMemTuning, false},
        {versionName.c_str(), sizeof(intel_cca_version_data), &mMemVersion, false},
        {decodeStatsName.c_str(), sizeof(intel_cca_decode_stats_data), &mMemDecodeStats, false},
        {palSizeName.c_str(), sizeof(intel_cca_get_pal_data_size), &mMemPalSize, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOG1("@%s, done", __func__);
    mInitialized = true;
}

IntelCca::~IntelCca() {
    LOG1("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

ia_err IntelCca::init(const cca::cca_init_params& initParams) {
    LOG1("@%s, bitmap:%d", __func__, initParams.bitmap);

    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    cca::cca_init_params* params = static_cast<cca::cca_init_params*>(mMemInit.mAddr);
    *params = initParams;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_INIT, mMemInit.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    LOG1("@%s, in params size:%d", __func__, sizeof(cca::cca_stats_params));

    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    cca::cca_stats_params* statsParams = static_cast<cca::cca_stats_params*>(mMemStats.mAddr);
    *statsParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_SET_STATS, mMemStats.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results) {
    LOG1("@%s, in params size:%d, results size:%d", __func__, sizeof(cca::cca_ae_input_params),
         sizeof(cca::cca_ae_results));

    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_run_aec_data* aecParams = static_cast<intel_cca_run_aec_data*>(mMemAEC.mAddr);
    aecParams->frameId = frameId;
    aecParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AEC, mMemAEC.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = aecParams->results;

    return ret;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results) {
    LOG1("@%s, in params size:%d, results size:%d", __func__, sizeof(cca::cca_aiq_params),
         sizeof(results));

    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_run_aiq_data* aiqParams = static_cast<intel_cca_run_aiq_data*>(mMemAIQ.mAddr);
    aiqParams->frameId = frameId;
    aiqParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AIQ, mMemAIQ.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = aiqParams->results;

    return ret;
}

ia_err IntelCca::runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params) {
    LOG1("@%s, in params size:%d", __func__, sizeof(cca::cca_ltm_input_params));
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_run_ltm_data* ltmParams = static_cast<intel_cca_run_ltm_data*>(mMemLTM.mAddr);
    ltmParams->frameId = frameId;
    ltmParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_LTM, mMemLTM.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::updateZoom(const cca::cca_dvs_zoom& params) {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    cca::cca_dvs_zoom* zoomParams = static_cast<cca::cca_dvs_zoom*>(mMemZoom.mAddr);
    *zoomParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_UPDATE_ZOOM, mMemZoom.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runDVS(uint64_t frameId) {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    uint64_t* params = static_cast<uint64_t*>(mMemDVS.mAddr);
    *params = frameId;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_DVS, mMemDVS.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runAIC(uint64_t frameId, cca::cca_pal_input_params* params, ia_binary_data* pal) {
    LOG1("@%s, cca::cca_pal_input_params size:%d", __func__, sizeof(cca::cca_pal_input_params));
    CheckError(!params, ia_err_argument, "@%s, params is nullptr", __func__);
    CheckError(!pal, ia_err_argument, "@%s, pal is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_run_aic_data* aicParams = static_cast<intel_cca_run_aic_data*>(mMemAIC.mAddr);
    aicParams->frameId = frameId;
    aicParams->inParams = params;
    aicParams->inParamsHandle = mCommon.getShmMemHandle(params);
    aicParams->palOutData = *pal;

    int32_t palDataHandle = mCommon.getShmMemHandle(pal->data);
    CheckError(!palDataHandle, ia_err_general, "%s, pal buf:%p is not in SHM", __func__, pal->data);
    LOG2("%s, pal buffer:%p, buffer handle:%d", __func__, pal->data, palDataHandle);
    aicParams->palDataHandle = palDataHandle;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AIC, mMemAIC.mHandle);
    CheckError(ret != ia_err_none && ret != ia_err_not_run, ia_err_general,
               "@%s, requestSyncCca fails", __func__);
    pal->size = aicParams->palOutData.size;
    LOG2("%s, pal->size:%d", __func__, pal->size);

    return ret;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc) {
    LOG1("@%s", __func__);
    CheckError(!cmc, ia_err_argument, "@%s, cmc is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_CMC, mMemCMC.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    cca::cca_cmc* cmcData = static_cast<cca::cca_cmc*>(mMemCMC.mAddr);
    *cmc = *cmcData;

    return ret;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    LOG1("@%s, results size:%d", __func__, sizeof(cca::cca_mkn));
    CheckError(!mkn, ia_err_argument, "@%s, mkn is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_mkn_data* params = static_cast<intel_cca_mkn_data*>(mMemMKN.mAddr);
    params->type = type;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_MKN, mMemMKN.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *mkn = params->results;

    return ret;
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    LOG1("@%s", __func__);
    CheckError(!aiqd, ia_err_argument, "@%s, aiqd is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_AIQD, mMemAIQD.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    cca::cca_aiqd* aiqdData = static_cast<cca::cca_aiqd*>(mMemAIQD.mAddr);
    *aiqd = *aiqdData;

    return ret;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams) {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_update_tuning_data* params =
        static_cast<intel_cca_update_tuning_data*>(mMemTuning.mAddr);
    params->lardTags = lardTags;
    params->lardParams = lardParams;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_UPDATE_TUNING, mMemTuning.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

void IntelCca::deinit() {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_DEINIT);
    CheckError(ret != ia_err_none, VOID_VALUE, "@%s, requestSyncCca fails", __func__);
}

void IntelCca::getVersion(std::string* version) {
    LOG1("@%s", __func__);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);

    intel_cca_version_data* params = static_cast<intel_cca_version_data*>(mMemVersion.mAddr);

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_VERSION, mMemVersion.mHandle);
    CheckError(ret != ia_err_none, VOID_VALUE, "@%s, requestSyncCca fails", __func__);

    *version = params->data;
}

ia_err IntelCca::decodeStats(uint64_t statsPointer, uint32_t statsSize,
                             ia_isp_bxt_statistics_query_results_t* results) {
    LOG1("@%s, statsPointer: 0x%x, statsSize:%d", __func__, statsPointer, statsSize);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);
    CheckError(!mInitialized, ia_err_general, "@%s, mInitialized is false", __func__);

    intel_cca_decode_stats_data* params =
        static_cast<intel_cca_decode_stats_data*>(mMemDecodeStats.mAddr);

    params->statsHandle = mCommon.getShmMemHandle(reinterpret_cast<void*>(statsPointer));
    params->statsBuffer.data = nullptr;
    params->statsBuffer.size = statsSize;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_DECODE_STATS, mMemDecodeStats.mHandle);
    CheckError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = params->results;

    return ret;
}

uint32_t IntelCca::getPalDataSize(const cca::cca_program_group& programGroup) {
    LOG1("@%s, in params size:%d", __func__, sizeof(cca::cca_program_group));
    CheckError(!mInitialized, 0, "@%s, mInitialized is false", __func__);

    intel_cca_get_pal_data_size* params =
        static_cast<intel_cca_get_pal_data_size*>(mMemPalSize.mAddr);
    params->pg = programGroup;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_PAL_SIZE, mMemPalSize.mHandle);
    CheckError(ret != ia_err_none, 0, "@%s, requestSyncCca fails", __func__);

    LOG1("@%s, returnSize:%d", __func__, params->returnSize);
    return params->returnSize;
}

void* IntelCca::allocatePalBuffer(int streamId, int index, int palDataSize) {
    LOG1("@%s index: %d, streamId: %d, size: %d", __func__, index, streamId, palDataSize);
    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string palDataName = "/palData" + std::to_string(streamId) + std::to_string(index) +
                              std::to_string(personal) + SHM_NAME;

    ShmMemInfo memInfo = {};
    bool ret = mCommon.allocShmMem(palDataName, palDataSize, &memInfo);
    CheckError(ret == false, nullptr, "%s, mCommon.allocShmMem fails for pal buf", __func__);
    LOGIPC("%s, the buffer handle: %d, address: %p", __func__, memInfo.mHandle, memInfo.mAddr);

    mMemsPalData.push_back(memInfo);

    return memInfo.mAddr;
}

void IntelCca::freePalBuffer(void* addr) {
    LOG1("@%s addr: %p", __func__, addr);

    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);

    for (auto& mem : mMemsPalData) {
        if (mem.mAddr == addr) {
            mCommon.freeShmMem(mem);
        }
    }
}
} /* namespace icamera */
