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

#define LOG_TAG "IntelCcaServer"

#include "modules/sandboxing/server/IntelCcaServer.h"

#include <algorithm>
#include <memory>
#include <string>

#include "CameraLog.h"
#include "iutils/Utils.h"
#include "modules/sandboxing/IPCGraphConfig.h"

namespace icamera {
IntelCcaServer::IntelCcaServer() {
    LOGIPC("@%s", __func__);

    mCca = std::unique_ptr<IntelCca>(new IntelCca());
}

IntelCcaServer::~IntelCcaServer() {
    LOGIPC("@%s", __func__);
}

status_t IntelCcaServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    cca::cca_init_params* params = static_cast<cca::cca_init_params*>(pData);
    LOG2("@%s, params->bitmap:%d", __func__, params->bitmap);

    ia_err ret = mCca->init(*params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->init fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::setStats(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    cca::cca_stats_params* params = static_cast<cca::cca_stats_params*>(pData);

    ia_err ret = mCca->setStatsParams(*params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->init fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAEC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_run_aec_data* params = static_cast<intel_cca_run_aec_data*>(pData);

    ia_err ret = mCca->runAEC(params->frameId, params->inParams, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->runAEC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIQ(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_run_aiq_data* params = static_cast<intel_cca_run_aiq_data*>(pData);

    ia_err ret = mCca->runAIQ(params->frameId, params->inParams, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->runAIQ fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runLTM(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_run_ltm_data* params = static_cast<intel_cca_run_ltm_data*>(pData);

    ia_err ret = mCca->runLTM(params->frameId, params->inParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->runLTM fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateZoom(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    cca::cca_dvs_zoom* params = static_cast<cca::cca_dvs_zoom*>(pData);

    ia_err ret = mCca->updateZoom(*params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->updateZoom fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runDVS(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    uint64_t* params = static_cast<uint64_t*>(pData);

    ia_err ret = mCca->runDVS(*params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->runDVS fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_run_aic_data* params = static_cast<intel_cca_run_aic_data*>(pData);

    bool retVal = unflattenProgramGroup(&params->inParams->program_group);
    CheckError(retVal != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    ia_err ret = mCca->runAIC(params->frameId, params->inParams, &params->palOutData);
    CheckError(ret != ia_err_none && ret != ia_err_not_run, ret,
               "@%s, mCca->runAIC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

status_t IntelCcaServer::getCMC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    cca::cca_cmc* params = static_cast<cca::cca_cmc*>(pData);

    ia_err ret = mCca->getCMC(params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->getCMC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::getMKN(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_mkn_data* params = static_cast<intel_cca_mkn_data*>(pData);

    ia_err ret = mCca->getMKN(params->type, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->getMKN fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::getAiqd(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    cca::cca_aiqd* params = static_cast<cca::cca_aiqd*>(pData);

    ia_err ret = mCca->getAiqd(params);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->getAiqd fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateTuning(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_update_tuning_data* params = static_cast<intel_cca_update_tuning_data*>(pData);

    ia_err ret = mCca->updateTuning(params->lardTags, params->lardParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->updateTuning fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::deinit() {
    LOGIPC("@%s", __func__);

    mCca->deinit();

    return OK;
}

status_t IntelCcaServer::getVersion(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_version_data* params = static_cast<intel_cca_version_data*>(pData);

    std::string version;
    mCca->getVersion(&version);
    snprintf(params->data, sizeof(params->data), "%s", version.c_str());
    params->size = std::min(version.size(), sizeof(params->data));
    LOGIPC("@%s, aiq version:%s, size:%d", __func__, version.c_str(), params->size);

    return OK;
}

status_t IntelCcaServer::decodeStats(void* pData, int dataSize, void* statsAddr) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(statsAddr == nullptr, UNKNOWN_ERROR, "@%s, statsAddr is nullptr", __func__);

    intel_cca_decode_stats_data* params = static_cast<intel_cca_decode_stats_data*>(pData);

    if (params->statsBuffer.size > 0) {
        params->statsBuffer.data = statsAddr;
    }

    ia_err ret = mCca->decodeStats(reinterpret_cast<uint64_t>(params->statsBuffer.data),
                                   params->statsBuffer.size, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, mCca->decodeStats fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

bool IntelCcaServer::unflattenProgramGroup(cca::cca_program_group* result) {
    CheckError(!result, false, "@%s, result is nullptr", __func__);
    CheckError(result->base.kernel_count > MAX_STREAM_KERNEL_COUNT, false,
               "%s, the buffer of kernel array is too small", __func__);

    result->base.run_kernels = result->run_kernels;
    for (unsigned j = 0; j < result->base.kernel_count; ++j) {
        if (result->run_kernels[j].resolution_info) {
            result->run_kernels[j].resolution_info = &result->resolution_info[j];
        }

        if (result->run_kernels[j].resolution_history) {
            result->run_kernels[j].resolution_history = &result->resolution_history[j];
        }
    }
    if (result->base.pipe) {
        result->base.pipe = result->pipe;
    }

    return true;
}

status_t IntelCcaServer::getPalDataSize(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);

    intel_cca_get_pal_data_size* params = static_cast<intel_cca_get_pal_data_size*>(pData);
    bool ret = unflattenProgramGroup(&params->pg);
    CheckError(ret != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    uint32_t size = mCca->getPalDataSize(params->pg);
    CheckError(size == 0, UNKNOWN_ERROR, "@%s, mCca->getPalDataSize fails", __func__);

    params->returnSize = size;
    LOG2("@%s, size:%d", __func__, size);

    return OK;
}
} /* namespace icamera */
