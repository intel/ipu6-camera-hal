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

#include <vector>

#include "modules/algowrapper/IntelCca.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    LOG2("@%s, cameraId: %d, tuningMode: %d", __func__, cameraId, mode);

    AutoMutex lock(sLock);
    for (auto &it : sCcaInstance) {
        if (cameraId == it.cameraId) {
            if (it.ccaHandle.find(mode) == it.ccaHandle.end())
                it.ccaHandle[mode] = new IntelCca();

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
    LOG2("@%s, cameraId: %d, tuningMode: %d", __func__, cameraId, mode);

    AutoMutex lock(sLock);
    for (auto &it : sCcaInstance) {
        if (cameraId == it.cameraId && it.ccaHandle.find(mode) != it.ccaHandle.end()) {
            IntelCca *cca = it.ccaHandle[mode];
            it.ccaHandle.erase(mode);
            delete cca;
            cca = nullptr;
        }
    }
}

IntelCca::IntelCca() {
    LOG2("@%s", __func__);

    mIntelCCA = nullptr;
}

IntelCca::~IntelCca() {
    LOG2("@%s", __func__);
    releaseIntelCCA();
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
    LOG2("@%s, bitmap:%d", __func__, initParams.bitmap);

    ia_err ret = getIntelCCA()->init(initParams);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    LOG2("@%s", __func__);

    ia_err ret = getIntelCCA()->setStatsParams(params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results) {
    LOG2("@%s", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ia_err ret = getIntelCCA()->runAEC(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results) {
    LOG2("@%s", __func__);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ia_err ret = getIntelCCA()->runAIQ(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runLTM(uint64_t frameId, const cca::cca_ltm_input_params& params) {
    LOG2("@%s", __func__);

    ia_err ret = getIntelCCA()->runLTM(frameId, params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateZoom(const cca::cca_dvs_zoom& params) {
    LOG2("@%s", __func__);

    ia_err ret = getIntelCCA()->updateZoom(params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runDVS(uint64_t frameId) {
    LOG2("@%s", __func__);

    ia_err ret = getIntelCCA()->runDVS(frameId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAIC(uint64_t frameId, const cca::cca_pal_input_params* params,
                        ia_binary_data* pal) {
    LOG2("@%s", __func__);
    CheckError(!params, ia_err_argument, "@%s, params is nullptr", __func__);
    CheckError(!pal, ia_err_argument, "@%s, pal is nullptr", __func__);

    ia_err ret = getIntelCCA()->runAIC(frameId, *params, pal);

    // if PAL doesn't run, set output size to 0
    if (ret == ia_err_not_run) pal->size = 0;

    LOG2("@%s, ret:%d, pal result size: %d", __func__, ret, pal->size);

    return ret;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc) {
    LOG2("@%s", __func__);
    CheckError(!cmc, ia_err_argument, "@%s, cmc is nullptr", __func__);

    ia_err ret = getIntelCCA()->getCMC(*cmc);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    LOG2("@%s", __func__);
    CheckError(!mkn, ia_err_argument, "@%s, mkn is nullptr", __func__);

    ia_err ret = getIntelCCA()->getMKN(type, *mkn);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    LOG2("@%s", __func__);
    CheckError(!aiqd, ia_err_argument, "@%s, aiqd is nullptr", __func__);

    ia_err ret = getIntelCCA()->getAiqd(*aiqd);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams) {
    LOG2("@%s", __func__);

    ia_err ret = getIntelCCA()->updateTuning(lardTags, lardParams);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

void IntelCca::deinit() {
    LOG2("@%s", __func__);

    getIntelCCA()->deinit();
    releaseIntelCCA();
}

void IntelCca::getVersion(std::string* version) {
    LOG2("@%s", __func__);
    CheckError(!version, VOID_VALUE, "@%s, version is nullptr", __func__);

    *version = std::string(getIntelCCA()->getVersion());
}

ia_err IntelCca::decodeStats(uint64_t statsPointer, uint32_t statsSize,
                             ia_isp_bxt_statistics_query_results_t* results) {
    LOG2("@%s, statsPointer: 0x%lu, statsSize:%d", __func__, statsPointer, statsSize);
    CheckError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    ia_err ret = getIntelCCA()->decodeStats(statsPointer, statsSize, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

uint32_t IntelCca::getPalDataSize(const cca::cca_program_group& programGroup) {
    uint32_t size = getIntelCCA()->getPalSize(programGroup);
    LOG2("@%s, pal data size: %zu", __func__, size);

    return size;
}

void* IntelCca::allocatePalBuffer(int streamId, int index, int palDataSize) {
    LOG2("@%s index: %d, streamId: %d, size: %d", __func__, index, streamId, palDataSize);

    return calloc(1, palDataSize);
}

void IntelCca::freePalBuffer(void* addr) {
    LOG2("@%s addr: %p", __func__, addr);
    free(addr);
}

} /* namespace icamera */
