/*
 * Copyright (C) 2020-2023 Intel Corporation
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

#define LOG_TAG IntelCca

#include <vector>

#include "modules/algowrapper/IntelCca.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    LOG2("<id%d>@%s, tuningMode:%d, cca instance size:%zu", cameraId, __func__, mode,
         sCcaInstance.size());

    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId) {
            if (it.ccaHandle.find(mode) == it.ccaHandle.end()) {
                it.ccaHandle[mode] = new IntelCca(cameraId, mode);
            }
            return it.ccaHandle[mode];
        }
    }

#ifndef ENABLE_SANDBOXING
    if (sCcaInstance.empty()) {
        ia_env env = {&Log::ccaPrintInfo, &Log::ccaPrintError, &Log::ccaPrintInfo};
        ia_log_init(&env);
    }
#endif

    IntelCca::CCAHandle handle = {};
    handle.cameraId = cameraId;
    handle.ccaHandle[mode] = new IntelCca(cameraId, mode);
    sCcaInstance.push_back(handle);

    return handle.ccaHandle[mode];
}

void IntelCca::releaseInstance(int cameraId, TuningMode mode) {
    LOG2("<id%d>@%s, tuningMode:%d", cameraId, __func__, mode);

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
    LOG2("@%s, cca instance size:%zu", __func__, sCcaInstance.size());
    for (auto& it : sCcaInstance) {
        for (auto& oneCcaHandle : it.ccaHandle) {
            IntelCca* intelCca = oneCcaHandle.second;
            delete intelCca;
        }
        it.ccaHandle.clear();
    }
}

IntelCca::IntelCca(int cameraId, TuningMode mode) : mCameraId(cameraId), mTuningMode(mode) {
    mIntelCCA = nullptr;
}

IntelCca::~IntelCca() {
    releaseIntelCCA();
    freeStatsDataMem();
    mMemStatsInfoMap.clear();
}

cca::IntelCCA* IntelCca::getIntelCCA() {
    if (mIntelCCA == nullptr) {
        mIntelCCA = new cca::IntelCCA();
    }
    return mIntelCCA;
}

void IntelCca::releaseIntelCCA() {
    delete mIntelCCA;
    mIntelCCA = nullptr;
}
ia_err IntelCca::init(const cca::cca_init_params& initParams) {
    ia_err ret = getIntelCCA()->init(initParams);
    LOG2("@%s, bitmap:0x%x, ret:%d, version:%s", __func__, initParams.bitmap, ret,
         getIntelCCA()->getVersion());

    return ret;
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    ia_err ret = getIntelCCA()->setStatsParams(params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results, bool lowPower) {
    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ia_err ret = getIntelCCA()->runAEC(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results, camera_makernote_mode_t mode) {
    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ia_err ret = getIntelCCA()->runAIQ(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params) {
    ia_err ret = getIntelCCA()->runLTM(frameId, params);
    LOG2("@%s, frameId: %lu, ret:%d", __func__, frameId, ret);

    return ret;
}

ia_err IntelCca::reconfigDvs(const cca::cca_dvs_init_param& dvsInitParam,
                             const cca::cca_gdc_configurations& gdcConfigs) {
    ia_err ret = getIntelCCA()->reconfigDvs(dvsInitParam, gdcConfigs);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateZoom(uint32_t streamId, const cca::cca_dvs_zoom& params) {
    ia_err ret = getIntelCCA()->updateZoom(streamId, params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runDVS(uint32_t streamId, uint64_t frameId) {
    ia_err ret = getIntelCCA()->runDVS(streamId, frameId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAIC(uint64_t frameId, const cca::cca_pal_input_params* params,
                        ia_binary_data* pal) {
    CheckAndLogError(!params, ia_err_argument, "@%s, params is nullptr", __func__);
    CheckAndLogError(!pal, ia_err_argument, "@%s, pal is nullptr", __func__);

    // Currently the aicId is same as stream_id
    ia_err ret = getIntelCCA()->runAIC(frameId, *params, pal, params->stream_id);

    // if PAL doesn't run, set output size to 0
    if (ret == ia_err_not_run) pal->size = 0;

    LOG2("@%s, ret:%d, pal result size: %d", __func__, ret, pal->size);

    return ret;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc, const cca::cca_cpf* cpf) {
    CheckAndLogError(!cmc, ia_err_argument, "@%s, cmc is nullptr", __func__);

    ia_err ret = getIntelCCA()->getCMC(*cmc, cpf);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    CheckAndLogError(!mkn, ia_err_argument, "@%s, mkn is nullptr", __func__);

    ia_err ret = getIntelCCA()->getMKN(type, *mkn);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    CheckAndLogError(!aiqd, ia_err_argument, "@%s, aiqd is nullptr", __func__);

    ia_err ret = getIntelCCA()->getAiqd(*aiqd);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                              const cca::cca_nvm& nvm, int32_t streamId) {
    ia_err ret = getIntelCCA()->updateTuning(lardTags, lardParams, nvm, streamId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

// PRIVACY_MODE_S
ia_err IntelCca::getBrightestIndex(uint32_t* outMaxBin) {
    ia_err ret = getIntelCCA()->getBrightestIndex(outMaxBin);
    LOG2("@%s, Brightest Index: %u", __func__, *outMaxBin);

    return ret;
}
// PRIVACY_MODE_E

bool IntelCca::allocStatsDataMem(unsigned int size) {
    LOG2("<id%d>@%s, tuningMode:%d, size:%d", mCameraId, __func__, mTuningMode, size);

    freeStatsDataMem();

    AutoMutex l(mMemStatsMLock);
    for (int i = 0; i < kMaxQueueSize; i++) {
        void* p = malloc(size);
        CheckAndLogError(!p, false, "failed to malloc stats buffer");
        StatsBufInfo info = {size, p, 0};

        int64_t index = i * (-1) - 1;  // default index list: -1, -2, -3, ...
        mMemStatsInfoMap[index] = info;
    }

    return true;
}

void IntelCca::freeStatsDataMem() {
    LOG2("<id%d>@%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    AutoMutex l(mMemStatsMLock);
    for (auto it = mMemStatsInfoMap.begin(); it != mMemStatsInfoMap.end(); ++it) {
        free(it->second.ptr);
    }

    mMemStatsInfoMap.clear();
}

void* IntelCca::getStatsDataBuffer() {
    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.empty()) return nullptr;

    void* p = mMemStatsInfoMap.begin()->second.ptr;
    LOG2("<id%d>@%s, stats buffer addr: %p", mCameraId, __func__, p);
    return p;
}

void IntelCca::decodeHwStatsDone(int64_t sequence, unsigned int byteUsed) {
    LOG2("<id%d>@%s, tuningMode:%d, sequence:%ld, byteUsed:%d", mCameraId, __func__, mTuningMode,
         sequence, byteUsed);

    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.empty() || mMemStatsInfoMap.count(sequence) > 0) return;

    auto it = mMemStatsInfoMap.begin();
    it->second.usedSize = byteUsed;
    mMemStatsInfoMap[sequence] = it->second;

    if (sequence != it->first) mMemStatsInfoMap.erase(it->first);
}

void* IntelCca::fetchHwStatsData(int64_t sequence, unsigned int* byteUsed) {
    LOG2("<id%d>@%s, tuningMode:%d, sequence:%ld", mCameraId, __func__, mTuningMode, sequence);
    CheckAndLogError(!byteUsed, nullptr, "byteUsed is nullptr");

    AutoMutex l(mMemStatsMLock);
    if (mMemStatsInfoMap.find(sequence) != mMemStatsInfoMap.end()) {
        *byteUsed = mMemStatsInfoMap[sequence].usedSize;
        void* p = mMemStatsInfoMap[sequence].ptr;
        LOG2("decode stats address %p", p);
        return p;
    }

    return nullptr;
}

void IntelCca::deinit() {
    getIntelCCA()->deinit();
    releaseIntelCCA();
}

ia_err IntelCca::decodeStats(uint64_t statsPointer, uint32_t statsSize, uint32_t bitmap,
                             ia_isp_bxt_statistics_query_results_t* results,
                             cca::cca_out_stats* outStats) {
    ia_isp_bxt_statistics_query_results_t resultsTmp = {};
    ia_isp_bxt_statistics_query_results_t* query = results ? results : &resultsTmp;

    ia_err ret = getIntelCCA()->decodeStats(statsPointer, statsSize, bitmap, query, outStats);
    LOG2("@%s, statsPointer: 0x%lu, statsSize:%d, bitmap:%x, ret: %d", __func__, statsPointer,
         statsSize, bitmap, ret);

    LOG2("%s, query results: rgbs_grid(%d), af_grid(%d), dvs_stats(%d), paf_grid(%d)", __func__,
         query->rgbs_grid, query->af_grid, query->dvs_stats, query->paf_grid);

    return ret;
}

uint32_t IntelCca::getPalDataSize(const cca::cca_program_group& programGroup) {
    uint32_t size = getIntelCCA()->getPalSize(programGroup);
    LOG2("@%s, pal data size: %u", __func__, size);

    return size;
}

void* IntelCca::allocMem(int streamId, const std::string& name, int index, int size) {
    LOG1("@%s, name:%s, index: %d, streamId: %d, size: %d", __func__, name.c_str(), index, streamId,
         size);

    return calloc(1, size);
}

void IntelCca::freeMem(void* addr) {
    LOG1("@%s addr: %p", __func__, addr);
    free(addr);
}

} /* namespace icamera */
