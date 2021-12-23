/*
 * Copyright (C) 2020-2021 Intel Corporation.
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

#define LOG_TAG IntelCPUAlgoServer

#include "modules/sandboxing/server/IntelCPUAlgoServer.h"

#include <ia_log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "iutils/Utils.h"

namespace icamera {

// Common check before the function call
#define FUNCTION_PREPARED_RETURN                                           \
    uint16_t key = getKey(p->cameraId, p->tuningMode);                     \
    if (mCcas.find(key) == mCcas.end()) {                                  \
        LOGE("@%s, req_id:%d, it doesn't find the cca", __func__, req_id); \
        status = UNKNOWN_ERROR;                                            \
        break;                                                             \
    }

IntelCPUAlgoServer::~IntelCPUAlgoServer() {
    for (auto& it : mCcas) {
        delete it.second;
    }
}

void IntelCPUAlgoServer::handleRequest(const MsgReq& msg) {
    uint32_t req_id = msg.req_id;
    int32_t buffer_handle = msg.buffer_handle;

    ShmInfo info = {};
    status_t status = getIntelAlgoServer()->getShmInfo(buffer_handle, &info);
    if (status != OK) {
        LOGE("@%s, Invalid buffer handle", __func__);
        getIntelAlgoServer()->returnCallback(req_id, UNKNOWN_ERROR, buffer_handle);
        return;
    }

    size_t requestSize = info.size;
    void* addr = info.addr;

    switch (req_id) {
        case IPC_FD_INIT:
            status = mFaceDetection.init(addr, requestSize);
            break;
        case IPC_FD_RUN: {
            FaceDetectionRunParams* palParams = static_cast<FaceDetectionRunParams*>(addr);
            void* addrImage = nullptr;
            if (palParams->bufferHandle >= 0) {
                ShmInfo imageDataInfo;
                status = getIntelAlgoServer()->getShmInfo(palParams->bufferHandle, &imageDataInfo);
                if (status != OK) {
                    LOGE("%s, the buffer handle for image data is invalid", __func__);
                    break;
                }
                addrImage = imageDataInfo.addr;
            }
            status = mFaceDetection.run(addr, requestSize, addrImage);
            break;
        }
        case IPC_FD_DEINIT:
            status = mFaceDetection.deinit(addr, requestSize);
            break;
        case IPC_GRAPH_ADD_KEY:
            mGraph.addCustomKeyMap();
            break;
        case IPC_GRAPH_PARSE:
            mGraph.parse(addr, requestSize);
            break;
        case IPC_GRAPH_RELEASE_NODES:
            mGraph.releaseGraphNodes();
            break;
        case IPC_GRAPH_QUERY_GRAPH_SETTINGS:
            status = mGraph.queryGraphSettings(addr, requestSize);
            break;
        case IPC_GRAPH_CONFIG_STREAMS:
            mGraph.configStreams(addr, requestSize);
            break;
        case IPC_GRAPH_GET_CONFIG_DATA:
            mGraph.getGraphConfigData(addr, requestSize);
            break;
        case IPC_GRAPH_GET_CONNECTION:
            mGraph.pipelineGetConnections(addr, requestSize);
            break;
        case IPC_GRAPH_GET_PG_ID:
            mGraph.getPgIdForKernel(addr, requestSize);
            break;
        case IPC_CCA_CONSTRUCT: {
            intel_cca_struct_data* p = static_cast<intel_cca_struct_data*>(addr);
            uint16_t key = getKey(p->cameraId, p->tuningMode);
            if (mCcas.find(key) != mCcas.end()) {
                delete mCcas[key];
                mCcas.erase(key);
            }

            mCcas[key] = new IntelCcaServer(p->cameraId, p->tuningMode);

            break;
        }
        case IPC_CCA_DESTRUCT: {
            intel_cca_struct_data* p = static_cast<intel_cca_struct_data*>(addr);
            uint16_t key = getKey(p->cameraId, p->tuningMode);
            if (mCcas.find(key) == mCcas.end()) {
                LOGE("@%s, req_id:%d, it doesn't find the cca", __func__, req_id);
                status = UNKNOWN_ERROR;
                break;
            }

            delete mCcas[key];
            mCcas.erase(key);

            break;
        }
        case IPC_CCA_INIT: {
            intel_cca_init_data* p = static_cast<intel_cca_init_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->init(addr, requestSize);
            break;
        }
        case IPC_CCA_SET_STATS: {
            intel_cca_set_stats_data* p = static_cast<intel_cca_set_stats_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->setStats(addr, requestSize);
            break;
        }
        case IPC_CCA_RUN_AEC: {
            intel_cca_run_aec_data* p = static_cast<intel_cca_run_aec_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->runAEC(addr, requestSize);
            break;
        }
        case IPC_CCA_RUN_AIQ: {
            intel_cca_run_aiq_data* p = static_cast<intel_cca_run_aiq_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->runAIQ(addr, requestSize);
            break;
        }
        case IPC_CCA_RUN_LTM: {
            intel_cca_run_ltm_data* p = static_cast<intel_cca_run_ltm_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->runLTM(addr, requestSize);
            break;
        }
        case IPC_CCA_UPDATE_ZOOM: {
            intel_cca_update_zoom_data* p = static_cast<intel_cca_update_zoom_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->updateZoom(addr, requestSize);
            break;
        }
        case IPC_CCA_RUN_DVS: {
            intel_cca_run_dvs_data* p = static_cast<intel_cca_run_dvs_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->runDVS(addr, requestSize);
            break;
        }
        case IPC_CCA_RUN_AIC: {
            status = UNKNOWN_ERROR;
            intel_cca_run_aic_data* p = static_cast<intel_cca_run_aic_data*>(addr);
            FUNCTION_PREPARED_RETURN

            if (p->palDataHandle >= 0) {
                ShmInfo inParamsInfo = {};
                status = getIntelAlgoServer()->getShmInfo(p->inParamsHandle, &inParamsInfo);
                if (status != OK) {
                    LOGE("%s, the buffer handle for inParamsHandle is invalid", __func__);
                    break;
                }
                p->inParams = static_cast<cca::cca_pal_input_params*>(inParamsInfo.addr);

                ShmInfo palDataInfo = {};
                status = getIntelAlgoServer()->getShmInfo(p->palDataHandle, &palDataInfo);
                if (status != OK) {
                    LOGE("%s, the buffer handle for palDataHandle is invalid", __func__);
                    break;
                }
                p->palOutData.data = palDataInfo.addr;

                status = mCcas[key]->runAIC(addr, requestSize);
            }
            break;
        }
        case IPC_CCA_GET_CMC: {
            intel_cca_get_cmc_data* p = static_cast<intel_cca_get_cmc_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->getCMC(addr, requestSize);
            break;
        }
        case IPC_CCA_GET_MKN: {
            intel_cca_mkn_data* p = static_cast<intel_cca_mkn_data*>(addr);
            FUNCTION_PREPARED_RETURN

            if (p->resultsHandle >= 0) {
                ShmInfo paramsInfo = {};
                status = getIntelAlgoServer()->getShmInfo(p->resultsHandle, &paramsInfo);
                if (status != OK) {
                    LOGE("%s, the buffer handle for resultsHandle is invalid", __func__);
                    break;
                }
                p->results = static_cast<cca::cca_mkn*>(paramsInfo.addr);
            }
            status = mCcas[key]->getMKN(addr, requestSize);
            break;
        }
        case IPC_CCA_GET_AIQD: {
            intel_cca_get_aiqd_data* p = static_cast<intel_cca_get_aiqd_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->getAiqd(addr, requestSize);
            break;
        }
        case IPC_CCA_UPDATE_TUNING: {
            intel_cca_update_tuning_data* p = static_cast<intel_cca_update_tuning_data*>(addr);
            FUNCTION_PREPARED_RETURN

            mCcas[key]->updateTuning(addr, requestSize);
            break;
        }
        case IPC_CCA_DEINIT: {
            intel_cca_deinit_data* p = static_cast<intel_cca_deinit_data*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->deinit(addr, requestSize);
            break;
        }
        case IPC_CCA_DECODE_STATS: {
            intel_cca_decode_stats_data* p = static_cast<intel_cca_decode_stats_data*>(addr);
            FUNCTION_PREPARED_RETURN

            ShmInfo info = {};
            status = getIntelAlgoServer()->getShmInfo(p->statsHandle, &info);
            if (status != OK) {
                LOGE("%s, the buffer handle for stats data is invalid", __func__);
                break;
            }
            status = mCcas[key]->decodeStats(addr, requestSize, info.addr);
            break;
        }
        case IPC_CCA_GET_PAL_SIZE: {
            intel_cca_get_pal_data_size* p = static_cast<intel_cca_get_pal_data_size*>(addr);
            FUNCTION_PREPARED_RETURN

            status = mCcas[key]->getPalDataSize(addr, requestSize);
            break;
        }
        case IPC_PG_PARAM_INIT:
            status = mPGParam.init(addr, requestSize);
            break;
        case IPC_PG_PARAM_PREPARE: {
            pg_param_prepare_params* prepareParams = static_cast<pg_param_prepare_params*>(addr);
            ShmInfo palDataInfo = {};
            status = getIntelAlgoServer()->getShmInfo(prepareParams->ipuParamHandle, &palDataInfo);
            if (status != OK) {
                LOGE("%s, the buffer handle for pal data is invalid", __func__);
                break;
            }
            status = mPGParam.prepare(addr, requestSize, palDataInfo.addr);
            break;
        }
        case IPC_PG_PARAM_ALLOCATE_PG:
            status = mPGParam.allocatePGBuffer(addr, requestSize);
            break;
        case IPC_PG_PARAM_GET_FRAG_DESCS:
            status = mPGParam.getFragmentDescriptors(addr, requestSize);
            break;
        case IPC_PG_PARAM_PREPARE_PROGRAM:
            status = mPGParam.setPGAndPrepareProgram(addr, requestSize);
            break;
        case IPC_PG_PARAM_REGISTER_PAYLOADS:
            status = mPGParam.registerPayloads(addr, requestSize);
            break;
        case IPC_PG_PARAM_ENCODE: {
            pg_param_encode_params* encodeParams = static_cast<pg_param_encode_params*>(addr);
            ShmInfo palDataInfo = {};
            status = getIntelAlgoServer()->getShmInfo(encodeParams->ipuParamHandle, &palDataInfo);
            if (status != OK) {
                LOGE("%s, the buffer handle for pal data is invalid", __func__);
                break;
            }
            status = mPGParam.updatePALAndEncode(addr, requestSize, palDataInfo.addr);
            break;
        }
        case IPC_PG_PARAM_DECODE: {
            pg_param_decode_params* decodeParams = static_cast<pg_param_decode_params*>(addr);
            ShmInfo decodeInfo = {};
            status = getIntelAlgoServer()->getShmInfo(decodeParams->clientStatsHandle, &decodeInfo);
            if (status == OK) {
                status = mPGParam.decode(addr, requestSize, decodeInfo.addr);
            } else {
                LOGE("%s, the buffer handle for stats data is invalid", __func__);
            }
            break;
        }
        case IPC_PG_PARAM_DEINIT:
            mPGParam.deinit(addr, requestSize);
            break;
        default:
            LOGE("@%s, req_id:%d is not defined", __func__, req_id);
            status = UNKNOWN_ERROR;
            break;
    }

    LOG2("@%s, req_id:%d:%s, status:%d", __func__, req_id,
         IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), status);
    getIntelAlgoServer()->returnCallback(req_id, status, buffer_handle);
}

uint16_t IntelCPUAlgoServer::getKey(int cameraId, TuningMode mode) {
    return ((cameraId & 0xFF) << 8) + (mode & 0xFF);
}
}  // namespace icamera
