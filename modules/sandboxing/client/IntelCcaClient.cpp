/*
 * Copyright (C) 2020-2022 Intel Corporation
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

#define LOG_TAG IntelCcaClient

#include "modules/sandboxing/client/IntelCcaClient.h"

#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId) {
            if (it.ccaHandle.find(mode) == it.ccaHandle.end()) {
                it.ccaHandle[mode] = new IntelCca(cameraId, mode);
            }

            return it.ccaHandle[mode];
        }
    }

    IntelCca::CCAHandle handle = {};
    handle.cameraId = cameraId;
    handle.ccaHandle[mode] = new IntelCca(cameraId, mode);
    sCcaInstance.push_back(handle);

    return handle.ccaHandle[mode];
}

void IntelCca::releaseInstance(int cameraId, TuningMode mode) {
    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId && it.ccaHandle.find(mode) != it.ccaHandle.end()) {
            IntelCca* cca = it.ccaHandle[mode];
            it.ccaHandle.erase(mode);
            delete cca;
        }
    }
}

void IntelCca::releaseAllInstances() {
    AutoMutex lock(sLock);

    for (auto& it : sCcaInstance) {
        for (auto& oneCcaHandle : it.ccaHandle) {
            IntelCca* intelCca = oneCcaHandle.second;
            delete intelCca;
        }
        it.ccaHandle.clear();
    }
}

IntelCca::IntelCca(int cameraId, TuningMode mode) : mCameraId(cameraId), mTuningMode(mode) {
    LOG1("<id%d> @%s, tuningMode:%d", cameraId, __func__, mode);

    std::string number = std::to_string(cameraId) + std::to_string(mode) +
                         std::to_string(reinterpret_cast<uintptr_t>(this));
    std::string structName = "/ccaStruct" + number + SHM_NAME;
    std::string initName = "/ccaInit" + number + SHM_NAME;
    std::string statsName = "/ccaStats" + number + SHM_NAME;
    std::string aecName = "/ccaAec" + number + SHM_NAME;
    std::string aiqName = "/ccaAiq" + number + SHM_NAME;
    std::string ltmName = "/ccaLtm" + number + SHM_NAME;
    std::string zoomName = "/ccaZoom" + number + SHM_NAME;
    std::string dvsName = "/ccaDvs" + number + SHM_NAME;
    std::string aicName = "/ccaAic" + number + SHM_NAME;
    std::string cmcName = "/ccaCmc" + number + SHM_NAME;
    std::string mknName = "/ccaMkn" + number + SHM_NAME;
    std::string aiqdName = "/ccaAiqd" + number + SHM_NAME;
    std::string tuningName = "/ccaTuning" + number + SHM_NAME;
    std::string deinitName = "/ccaDeinit" + number + SHM_NAME;
    std::string decodeStatsName = "/ccaDecodeStats" + number + SHM_NAME;
    std::string palSizeName = "/ccaPalSize" + number + SHM_NAME;

    mMems = {
        {structName.c_str(), sizeof(intel_cca_struct_data), &mMemStruct, false},
        {initName.c_str(), sizeof(intel_cca_init_data), &mMemInit, false},
        {statsName.c_str(), sizeof(intel_cca_set_stats_data), &mMemStats, false},
        {aecName.c_str(), sizeof(intel_cca_run_aec_data), &mMemAEC, false},
        {aiqName.c_str(), sizeof(intel_cca_run_aiq_data), &mMemAIQ, false},
        {ltmName.c_str(), sizeof(intel_cca_run_ltm_data), &mMemLTM, false},
        {zoomName.c_str(), sizeof(intel_cca_update_zoom_data), &mMemZoom, false},
        {dvsName.c_str(), sizeof(intel_cca_run_dvs_data), &mMemDVS, false},
        {aicName.c_str(), sizeof(intel_cca_run_aic_data), &mMemAIC, false},
        {cmcName.c_str(), sizeof(intel_cca_get_cmc_data), &mMemCMC, false},
        {mknName.c_str(), sizeof(intel_cca_mkn_data), &mMemMKN, false},
        {aiqdName.c_str(), sizeof(intel_cca_get_aiqd_data), &mMemAIQD, false},
        {tuningName.c_str(), sizeof(intel_cca_update_tuning_data), &mMemTuning, false},
        {deinitName.c_str(), sizeof(intel_cca_deinit_data), &mMemDeinit, false},
        {decodeStatsName.c_str(), sizeof(intel_cca_decode_stats_data), &mMemDecodeStats, false},
        {palSizeName.c_str(), sizeof(intel_cca_get_pal_data_size), &mMemPalSize, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    // construct the remote CCA
    intel_cca_struct_data* params = static_cast<intel_cca_struct_data*>(mMemStruct.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    mCommon.requestSyncCca(IPC_CCA_CONSTRUCT, mMemStruct.mHandle);

    LOG1("@%s, Construct done", __func__);
}

IntelCca::~IntelCca() {
    LOG1("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    // destruct the remote CCA
    intel_cca_struct_data* params = static_cast<intel_cca_struct_data*>(mMemStruct.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    mCommon.requestSyncCca(IPC_CCA_DESTRUCT, mMemStruct.mHandle);

    mCommon.releaseAllShmMems(mMems);

    for (auto& it : mMemsOuter) {
        mCommon.freeShmMem(it.second);
    }
    mMemsOuter.clear();

    freeStatsDataMem();
}

ia_err IntelCca::init(const cca::cca_init_params& initParams) {
    LOG1("<id%d> @%s, tuningMode:%d, bitmap:0x%x", mCameraId, __func__, mTuningMode,
         initParams.bitmap);

    intel_cca_init_data* params = static_cast<intel_cca_init_data*>(mMemInit.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->inParams = initParams;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_INIT, mMemInit.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    LOG2("<id%d> @%s, tuningMode:%d, in params size:%zu", mCameraId, __func__, mTuningMode,
         sizeof(cca::cca_stats_params));

    intel_cca_set_stats_data* statsParams = static_cast<intel_cca_set_stats_data*>(mMemStats.mAddr);
    statsParams->cameraId = mCameraId;
    statsParams->tuningMode = mTuningMode;
    statsParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_SET_STATS, mMemStats.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu, results size:%zu", mCameraId,
         frameId, __func__, mTuningMode, sizeof(cca::cca_ae_input_params),
         sizeof(cca::cca_ae_results));

    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    intel_cca_run_aec_data* aecParams = static_cast<intel_cca_run_aec_data*>(mMemAEC.mAddr);
    aecParams->cameraId = mCameraId;
    aecParams->tuningMode = mTuningMode;
    aecParams->frameId = frameId;
    aecParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AEC, mMemAEC.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = aecParams->results;

    return ret;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu, results size:%zu", mCameraId,
         frameId, __func__, mTuningMode, sizeof(cca::cca_aiq_params), sizeof(cca::cca_aiq_results));
    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    intel_cca_run_aiq_data* aiqParams = static_cast<intel_cca_run_aiq_data*>(mMemAIQ.mAddr);
    aiqParams->cameraId = mCameraId;
    aiqParams->tuningMode = mTuningMode;
    aiqParams->frameId = frameId;
    aiqParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AIQ, mMemAIQ.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = aiqParams->results;

    return ret;
}

ia_err IntelCca::runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu", mCameraId, frameId, __func__,
         mTuningMode, sizeof(cca::cca_ltm_input_params));
    intel_cca_run_ltm_data* ltmParams = static_cast<intel_cca_run_ltm_data*>(mMemLTM.mAddr);
    ltmParams->cameraId = mCameraId;
    ltmParams->tuningMode = mTuningMode;
    ltmParams->frameId = frameId;
    ltmParams->inParams = params;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_LTM, mMemLTM.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::updateZoom(uint32_t streamId, const cca::cca_dvs_zoom& params) {
    LOG1("<id%d> @%s, tuningMode:%d, streamId: %u", mCameraId, __func__, mTuningMode, streamId);

    intel_cca_update_zoom_data* zoomParams =
        static_cast<intel_cca_update_zoom_data*>(mMemZoom.mAddr);
    zoomParams->cameraId = mCameraId;
    zoomParams->tuningMode = mTuningMode;
    zoomParams->inParams = params;
    zoomParams->streamId = streamId;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_UPDATE_ZOOM, mMemZoom.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runDVS(uint32_t streamId, uint64_t frameId) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, streamId: %u", mCameraId, frameId, __func__,
         mTuningMode, streamId);

    intel_cca_run_dvs_data* params = static_cast<intel_cca_run_dvs_data*>(mMemDVS.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->frameId = frameId;
    params->streamId = streamId;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_DVS, mMemDVS.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::runAIC(uint64_t frameId, cca::cca_pal_input_params* params, ia_binary_data* pal) {
    CheckAndLogError(!params, ia_err_argument, "@%s, params is nullptr", __func__);
    CheckAndLogError(!pal, ia_err_argument, "@%s, pal is nullptr", __func__);

    intel_cca_run_aic_data* aicParams = static_cast<intel_cca_run_aic_data*>(mMemAIC.mAddr);
    aicParams->cameraId = mCameraId;
    aicParams->tuningMode = mTuningMode;
    aicParams->frameId = frameId;
    aicParams->inParams = params;
    aicParams->inParamsHandle = mCommon.getShmMemHandle(params);
    aicParams->palOutData = *pal;

    int32_t palDataHandle = mCommon.getShmMemHandle(pal->data);
    CheckAndLogError(!palDataHandle, ia_err_general, "%s, pal buf:%p is not in SHM", __func__,
                     pal->data);
    aicParams->palDataHandle = palDataHandle;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_RUN_AIC, mMemAIC.mHandle);
    CheckAndLogError(ret != ia_err_none && ret != ia_err_not_run, ia_err_general,
                     "@%s, requestSyncCca fails", __func__);
    pal->size = aicParams->palOutData.size;
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu pal size:%d", mCameraId, frameId,
         __func__, mTuningMode, sizeof(cca::cca_pal_input_params), pal->size);

    return ret;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc) {
    CheckAndLogError(!cmc, ia_err_argument, "@%s, cmc is nullptr", __func__);

    intel_cca_get_cmc_data* params = static_cast<intel_cca_get_cmc_data*>(mMemCMC.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_CMC, mMemCMC.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *cmc = params->results;

    return ret;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    CheckAndLogError(!mkn, ia_err_argument, "@%s, mkn is nullptr", __func__);

    intel_cca_mkn_data* params = static_cast<intel_cca_mkn_data*>(mMemMKN.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->type = type;
    params->results = mkn;
    params->resultsHandle = mCommon.getShmMemHandle(mkn);

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_MKN, mMemMKN.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    CheckAndLogError(!aiqd, ia_err_argument, "@%s, aiqd is nullptr", __func__);

    intel_cca_get_aiqd_data* params = static_cast<intel_cca_get_aiqd_data*>(mMemAIQD.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_AIQD, mMemAIQD.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *aiqd = params->results;

    return ret;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                              const cca::cca_nvm& nvm, int32_t streamId) {
    LOG2("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    intel_cca_update_tuning_data* params =
        static_cast<intel_cca_update_tuning_data*>(mMemTuning.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->lardTags = lardTags;
    params->lardParams = lardParams;
    params->nvmParams = nvm;
    params->streamId = streamId;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_UPDATE_TUNING, mMemTuning.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    return ret;
}

bool IntelCca::allocStatsDataMem(unsigned int size) {
    LOG1("<id%d> @%s, tuningMode:%d, size:%d", mCameraId, __func__, mTuningMode, size);

    freeStatsDataMem();

    AutoMutex l(mMemStatsMLock);
    for (int i = 0; i < kMaxQueueSize; i++) {
        std::string number = std::to_string(i) + std::to_string(mCameraId) +
                             std::to_string(mTuningMode) +
                             std::to_string(reinterpret_cast<uintptr_t>(this));
        std::string finalName = "statsData" + number + SHM_NAME;
        StatsBufInfo info = {};
        bool ret = mCommon.allocShmMem(finalName, size, &info.shmMem);
        CheckAndLogError(ret == false, false, "mCommon.allocShmMem failed for stats data");
        info.usedSize = size;
        info.bufSize = size;

        LOG1("the buffer handle: %d, address: %p", info.shmMem.mHandle, info.shmMem.mAddr);
        int64_t index = i * (-1) - 1;  // default index list: -1, -2, -3, ...
        mMemStatsInfoMap[index] = info;
    }

    return true;
}

void IntelCca::freeStatsDataMem() {
    LOG1("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    AutoMutex l(mMemStatsMLock);
    for (auto it = mMemStatsInfoMap.begin(); it != mMemStatsInfoMap.end(); ++it) {
        mCommon.freeShmMem(it->second.shmMem);
    }

    mMemStatsInfoMap.clear();
}

void* IntelCca::getStatsDataBuffer() {
    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.empty()) return nullptr;

    ShmMemInfo memInfo = mMemStatsInfoMap.begin()->second.shmMem;

    return memInfo.mAddr;
}

void IntelCca::decodeHwStatsDone(int64_t sequence, unsigned int byteUsed) {
    LOG2("<id%d:seq%ld> @%s, tuningMode:%d, byteUsed:%d", mCameraId, sequence, __func__,
         mTuningMode, byteUsed);

    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.empty()) return;

    auto it = mMemStatsInfoMap.begin();
    it->second.usedSize = byteUsed;
    mMemStatsInfoMap[sequence] = it->second;

    if (sequence != it->first) mMemStatsInfoMap.erase(it->first);
}

void* IntelCca::fetchHwStatsData(int64_t sequence, unsigned int* byteUsed) {
    CheckAndLogError(!byteUsed, nullptr, "byteUsed is nullptr");

    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.find(sequence) != mMemStatsInfoMap.end()) {
        *byteUsed = mMemStatsInfoMap[sequence].usedSize;
        ShmMemInfo memInfo = mMemStatsInfoMap[sequence].shmMem;
        LOG2("<id%d:seq%ld> @%s, tuningMode:%d, memInfo.mAddr %p", mCameraId, sequence, __func__,
             mTuningMode, memInfo.mAddr);
        return memInfo.mAddr;
    }

    return nullptr;
}

void IntelCca::deinit() {
    LOG1("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    intel_cca_deinit_data* params = static_cast<intel_cca_deinit_data*>(mMemDeinit.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_DEINIT, mMemDeinit.mHandle);
    CheckAndLogError(ret != ia_err_none, VOID_VALUE, "@%s, requestSyncCca fails", __func__);
}

ia_err IntelCca::decodeStats(uint64_t statsPointer, uint32_t statsSize, uint32_t bitmap,
                             ia_isp_bxt_statistics_query_results_t* results,
                             cca::cca_out_stats* outStats) {
    LOG2("<id%d> @%s, tuningMode:%d, statsPointer:0x%lx, statsSize:%d, bitmap:0x%x", mCameraId,
         __func__, mTuningMode, statsPointer, statsSize, bitmap);
    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    intel_cca_decode_stats_data* params =
        static_cast<intel_cca_decode_stats_data*>(mMemDecodeStats.mAddr);

    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->statsHandle = mCommon.getShmMemHandle(reinterpret_cast<void*>(statsPointer));
    params->statsBuffer.data = nullptr;
    params->statsBuffer.size = statsSize;
    params->bitmap = bitmap;
    params->outStats.get_rgbs_stats = false;
    if (outStats) {
        params->outStats.get_rgbs_stats = outStats->get_rgbs_stats;
    }

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_DECODE_STATS, mMemDecodeStats.mHandle);
    CheckAndLogError(ret != ia_err_none, ia_err_general, "@%s, requestSyncCca fails", __func__);

    *results = params->results;
    if (outStats && params->outStats.get_rgbs_stats) {
        *outStats = params->outStats;
        outStats->rgbs_grid.blocks_ptr = outStats->rgbs_blocks;
    }

    return ret;
}

uint32_t IntelCca::getPalDataSize(const cca::cca_program_group& programGroup) {
    intel_cca_get_pal_data_size* params =
        static_cast<intel_cca_get_pal_data_size*>(mMemPalSize.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->pg = programGroup;

    ia_err ret = mCommon.requestSyncCca(IPC_CCA_GET_PAL_SIZE, mMemPalSize.mHandle);
    CheckAndLogError(ret != ia_err_none, 0, "@%s, requestSyncCca fails", __func__);
    LOG2("<id%d> @%s, tuningMode:%d, in params size:%zu, returnSize:%d", mCameraId, __func__,
         mTuningMode, sizeof(cca::cca_program_group), params->returnSize);

    return params->returnSize;
}

void* IntelCca::allocMem(int streamId, const std::string& name, int index, int size) {
    std::string number = std::to_string(streamId) + std::to_string(index) +
                         std::to_string(mCameraId) + std::to_string(mTuningMode) +
                         std::to_string(reinterpret_cast<uintptr_t>(this));
    std::string finalName = name + number + SHM_NAME;

    ShmMemInfo memInfo = {};
    bool ret = mCommon.allocShmMem(finalName, size, &memInfo);
    CheckAndLogError(ret == false, nullptr, "%s, mCommon.allocShmMem fails for pal buf", __func__);
    LOG1("<id%d> @%s, tuningMode:%d, name:%s, index:%d, streamId:%d, size:%d, handle: %d,"
         "address: %p",
         mCameraId, __func__, mTuningMode, name.c_str(), index, streamId, size, memInfo.mHandle,
         memInfo.mAddr);
    mMemsOuter[memInfo.mAddr] = memInfo;

    return memInfo.mAddr;
}

void IntelCca::freeMem(void* addr) {
    LOG1("<id%d> @%s, tuningMode:%d, addr: %p", mCameraId, __func__, mTuningMode, addr);

    if (mMemsOuter.find(addr) != mMemsOuter.end()) {
        mCommon.freeShmMem(mMemsOuter[addr]);
        mMemsOuter.erase(addr);
        return;
    }
    LOGW("@%s, there is no addr:%p, in the mMemsOuter", __func__, addr);
}
} /* namespace icamera */
