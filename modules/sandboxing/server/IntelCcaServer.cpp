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
IntelCcaServer::IntelCcaServer(int cameraId, TuningMode mode) :
    mCameraId(cameraId),
    mTuningMode(mode),
    mCca(nullptr) {
    LOGIPC("@%s, cameraId:%d, mode:%d", __func__, cameraId, mode);

    mCca = IntelCca::getInstance(cameraId, mode);
    CheckError(!mCca, VOID_VALUE, "%s, IntelCca::getInstance fails, cameraId(%d), mode(%d)",
               __func__, mCameraId, mTuningMode);
}

IntelCcaServer::~IntelCcaServer() {
    LOGIPC("@%s", __func__);

    IntelCca::releaseInstance(mCameraId, mTuningMode);
}

status_t IntelCcaServer::init(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_init_data* params = static_cast<intel_cca_init_data*>(pData);
    LOG2("@%s, params->bitmap:%d", __func__, params->inParams.bitmap);

    ia_err ret = mCca->init(params->inParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, init fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::setStats(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_set_stats_data* params = static_cast<intel_cca_set_stats_data*>(pData);

    ia_err ret = mCca->setStatsParams(params->inParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, setStatsParams fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAEC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aec_data* params = static_cast<intel_cca_run_aec_data*>(pData);

    ia_err ret = mCca->runAEC(params->frameId, params->inParams, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, runAEC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIQ(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aiq_data* params = static_cast<intel_cca_run_aiq_data*>(pData);

    ia_err ret = mCca->runAIQ(params->frameId, params->inParams, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, runAIQ fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runLTM(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_ltm_data* params = static_cast<intel_cca_run_ltm_data*>(pData);

    ia_err ret = mCca->runLTM(params->frameId, params->inParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, runLTM fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateZoom(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_update_zoom_data* params = static_cast<intel_cca_update_zoom_data*>(pData);

    ia_err ret = mCca->updateZoom(params->inParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, updateZoom fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runDVS(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_dvs_data* params = static_cast<intel_cca_run_dvs_data*>(pData);

    ia_err ret = mCca->runDVS(params->frameId);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, runDVS fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aic_data* params = static_cast<intel_cca_run_aic_data*>(pData);

    bool retVal = unflattenProgramGroup(&params->inParams->program_group);
    CheckError(retVal != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    ia_err ret = mCca->runAIC(params->frameId, params->inParams, &params->palOutData);
    CheckError(ret != ia_err_none && ret != ia_err_not_run, ret, "@%s, runAIC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

status_t IntelCcaServer::getCMC(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_cmc_data* params = static_cast<intel_cca_get_cmc_data*>(pData);

    ia_err ret = mCca->getCMC(&params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, getCMC fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::getMKN(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_mkn_data* params = static_cast<intel_cca_mkn_data*>(pData);

    ia_err ret = mCca->getMKN(params->type, params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, getMKN fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::getAiqd(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_aiqd_data* params = static_cast<intel_cca_get_aiqd_data*>(pData);

    ia_err ret = mCca->getAiqd(&params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, getAiqd fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateTuning(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_update_tuning_data* params = static_cast<intel_cca_update_tuning_data*>(pData);

    ia_err ret = mCca->updateTuning(params->lardTags, params->lardParams);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, updateTuning fails", __func__);
    LOG2("@%s, ret:%d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::deinit(void* pData, int dataSize) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    mCca->deinit();

    return OK;
}

status_t IntelCcaServer::decodeStats(void* pData, int dataSize, void* statsAddr) {
    LOGIPC("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(statsAddr == nullptr, UNKNOWN_ERROR, "@%s, statsAddr is nullptr", __func__);
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_decode_stats_data* params = static_cast<intel_cca_decode_stats_data*>(pData);

    if (params->statsBuffer.size > 0) {
        params->statsBuffer.data = statsAddr;
    }

    ia_err ret = mCca->decodeStats(reinterpret_cast<uint64_t>(params->statsBuffer.data),
                                   params->statsBuffer.size, &params->results);
    CheckError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, decodeStats fails", __func__);
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
    CheckError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_pal_data_size* params = static_cast<intel_cca_get_pal_data_size*>(pData);
    bool ret = unflattenProgramGroup(&params->pg);
    CheckError(ret != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    uint32_t size = mCca->getPalDataSize(params->pg);
    CheckError(size == 0, UNKNOWN_ERROR, "@%s, getPalDataSize fails", __func__);

    params->returnSize = size;
    LOG2("@%s, size:%d", __func__, size);

    return OK;
}
} /* namespace icamera */
