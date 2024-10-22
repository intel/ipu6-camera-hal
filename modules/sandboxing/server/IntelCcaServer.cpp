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

#define LOG_TAG IntelCcaServer

#include "modules/sandboxing/server/IntelCcaServer.h"

#include <algorithm>
#include <memory>
#include <string>

#include "CameraLog.h"
#include "iutils/Utils.h"
#include "modules/sandboxing/IPCGraphConfig.h"

namespace icamera {
IntelCcaServer::IntelCcaServer(int cameraId, TuningMode mode)
        : mCameraId(cameraId),
          mTuningMode(mode),
          mCca(nullptr) {
    LOG1("<id%d>@%s, mode:%d", cameraId, __func__, mode);

    mCca = IntelCca::getInstance(cameraId, mode);
    CheckAndLogError(!mCca, VOID_VALUE, "%s, IntelCca::getInstance fails, cameraId(%d), mode(%d)",
                     __func__, mCameraId, mTuningMode);
}

IntelCcaServer::~IntelCcaServer() {
    LOG1("@%s", __func__);

    IntelCca::releaseInstance(mCameraId, mTuningMode);
}

status_t IntelCcaServer::init(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_init_data* params = static_cast<intel_cca_init_data*>(pData);
    LOG1("@%s, params->bitmap:0x%x", __func__, params->inParams.bitmap);

    ia_err ret = mCca->init(params->inParams);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAEC(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aec_data* params = static_cast<intel_cca_run_aec_data*>(pData);

    if (params->hasStats) {
        ia_err ret = mCca->setStatsParams(params->inStatsParams);
        CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails to set stats: %d", __func__,
                         ret);
    }

    ia_err ret = mCca->runAEC(params->frameId, params->inParams, &params->results);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIQ(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aiq_data* params = static_cast<intel_cca_run_aiq_data*>(pData);

    ia_err ret = mCca->runAIQ(params->frameId, params->inParams, params->results);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    if (params->mknResultsHandle >= 0) {
        ret = mCca->getMKN(params->type, params->mknResults);
        CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails to get MKN: %d", __func__,
                         ret);
    }

    return OK;
}

status_t IntelCcaServer::runLTM(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_ltm_data* params = static_cast<intel_cca_run_ltm_data*>(pData);

    ia_err ret = mCca->runLTM(params->frameId, params->inParams);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateZoom(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_update_zoom_data* params = static_cast<intel_cca_update_zoom_data*>(pData);

    ia_err ret = mCca->updateZoom(params->streamId, params->inParams);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runDVS(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_dvs_data* params = static_cast<intel_cca_run_dvs_data*>(pData);

    ia_err ret = mCca->runDVS(params->streamId, params->frameId);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::runAIC(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();

    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_run_aic_data* params = static_cast<intel_cca_run_aic_data*>(pData);

    bool retVal = unflattenProgramGroup(&params->inParams->program_group);
    CheckAndLogError(retVal != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    ia_err ret = mCca->runAIC(params->frameId, params->inParams, &params->palOutData);
    CheckAndLogError(ret != ia_err_none && ret != ia_err_not_run, ret, "@%s, fails: %d", __func__,
                     ret);

    return ret;
}

status_t IntelCcaServer::getCMC(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_cmc_data* params = static_cast<intel_cca_get_cmc_data*>(pData);

    ia_err ret = mCca->getCMC(&params->results, params->cpf.size ? &params->cpf : nullptr);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::getAiqd(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_aiqd_data* params = static_cast<intel_cca_get_aiqd_data*>(pData);

    ia_err ret = mCca->getAiqd(&params->results);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::updateTuning(void* pData, int dataSize) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_update_tuning_data* params = static_cast<intel_cca_update_tuning_data*>(pData);

    ia_err ret = mCca->updateTuning(params->lardTags, params->lardParams, params->nvmParams,
                                    params->streamId);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

status_t IntelCcaServer::deinit(void* pData, int dataSize) {
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    mCca->deinit();
    LOG1("@%s", __func__);

    return OK;
}

status_t IntelCcaServer::decodeStats(void* pData, void* statsAddr) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(statsAddr == nullptr, UNKNOWN_ERROR, "@%s, statsAddr is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_decode_stats_data* params = static_cast<intel_cca_decode_stats_data*>(pData);

    if (params->statsBuffer.size > 0) {
        params->statsBuffer.data = statsAddr;
    }

    ia_err ret = mCca->decodeStats(reinterpret_cast<uint64_t>(params->statsBuffer.data),
                                   params->statsBuffer.size, params->bitmap, &params->results,
                                   &params->outStats);
    CheckAndLogError(ret != ia_err_none, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    return OK;
}

bool IntelCcaServer::unflattenProgramGroup(cca::cca_program_group* result) {
    CheckAndLogError(!result, false, "@%s, result is nullptr", __func__);
    CheckAndLogError(result->base.kernel_count > MAX_STREAM_KERNEL_COUNT, false,
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
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(mCca == nullptr, UNKNOWN_ERROR, "@%s, mCca is nullptr", __func__);

    intel_cca_get_pal_data_size* params = static_cast<intel_cca_get_pal_data_size*>(pData);
    bool ret = unflattenProgramGroup(&params->pg);
    CheckAndLogError(ret != true, UNKNOWN_ERROR, "@%s, unflattenProgramGroup fails", __func__);

    uint32_t size = mCca->getPalDataSize(params->pg);
    CheckAndLogError(size == 0, UNKNOWN_ERROR, "@%s, fails: %d", __func__, ret);

    params->returnSize = size;

    return OK;
}
} /* namespace icamera */
