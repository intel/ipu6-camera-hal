/*
 * Copyright (C) 2020 Intel Corporation.
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

#define LOG_TAG "IntelCPUAlgoServer"

#include "modules/sandboxing/server/IntelCPUAlgoServer.h"

#include <base/logging.h>
#include <ia_log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "iutils/Utils.h"

namespace icamera {

IntelCPUAlgoServer::IntelCPUAlgoServer(IntelAlgoServer* server) : RequestHandler(server) {
    LOGIPC("@%s", __func__);
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
    LOGIPC("@%s, req_id:%d:%s, requestSize:%zu, addr:%p, buffer_handle:%d", __func__, req_id,
           IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), requestSize, addr, buffer_handle);

    switch (req_id) {
        case IPC_LARD_INIT:
            status = mLard.init(addr, requestSize);
            break;
        case IPC_LARD_GET_TAG_LIST:
            status = mLard.getTagList(addr, requestSize);
            break;
        case IPC_LARD_RUN:
            status = mLard.run(addr, requestSize);
            break;
        case IPC_LARD_DEINIT:
            status = mLard.deinit(addr, requestSize);
            break;
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
            status = mFaceDetection.deinit();
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
        case IPC_CMC_INIT:
            status = mCmc.init(addr, requestSize);
            break;
        case IPC_CMC_DEINIT:
            status = mCmc.deinit(addr, requestSize);
            break;
        case IPC_MKN_INIT:
            status = mMkn.init(addr, requestSize);
            break;
        case IPC_MKN_ENABLE:
            status = mMkn.enable(addr, requestSize);
            break;
        case IPC_MKN_PREPARE:
            status = mMkn.prepare(addr, requestSize);
            break;
        case IPC_MKN_DEINIT:
            status = mMkn.deinit(addr, requestSize);
            break;
        case IPC_LTM_INIT:
            status = mLtm.init(addr, requestSize);
            break;
        case IPC_LTM_RUN:
            status = mLtm.run(addr, requestSize);
            break;
        case IPC_LTM_DEINIT:
            status = mLtm.deinit(addr, requestSize);
            break;
        case IPC_AIQ_INIT:
            status = mAiq.init(addr, requestSize);
            break;
        case IPC_AIQ_AE_RUN:
            status = mAiq.aeRun(addr, requestSize);
            break;
        case IPC_AIQ_AF_RUN:
            status = mAiq.afRun(addr, requestSize);
            break;
        case IPC_AIQ_AWB_RUN:
            status = mAiq.awbRun(addr, requestSize);
            break;
        case IPC_AIQ_GBCE_RUN:
            status = mAiq.gbceRun(addr, requestSize);
            break;
        case IPC_AIQ_PA_RUN_V1:
            status = mAiq.paRunV1(addr, requestSize);
            break;
        case IPC_AIQ_SA_RUN_V2:
            status = mAiq.saRunV2(addr, requestSize);
            break;
        case IPC_AIQ_STATISTICS_SET_V4:
            status = mAiq.statisticsSetV4(addr, requestSize);
            break;
        case IPC_AIQ_GET_AIQD_DATA:
            status = mAiq.getAiqdData(addr, requestSize);
            break;
        case IPC_AIQ_DEINIT:
            status = mAiq.deinit(addr, requestSize);
            break;
        case IPC_AIQ_GET_VERSION:
            status = mAiq.getVersion(addr, requestSize);
            break;
        case IPC_DVS_INIT:
            status = mDvs.init(addr, requestSize);
            break;
        case IPC_DVS_CONFIG:
            status = mDvs.config(addr, requestSize);
            break;
        case IPC_DVS_SET_NONE_BLANK_RATION:
            status = mDvs.setNonBlankRatio(addr, requestSize);
            break;
        case IPC_DVS_SET_DIGITAL_ZOOM_MODE:
            status = mDvs.setDigitalZoomMode(addr, requestSize);
            break;
        case IPC_DVS_SET_DIGITAL_ZOOM_REGION:
            status = mDvs.setDigitalZoomRegion(addr, requestSize);
            break;
        case IPC_DVS_SET_DIGITAL_ZOOM_COORDINATE:
            status = mDvs.setDigitalZoomCoordinate(addr, requestSize);
            break;
        case IPC_DVS_SET_DIGITAL_ZOOM_MAGNITUDE:
            status = mDvs.setDigitalZoomMagnitude(addr, requestSize);
            break;
        case IPC_DVS_FREE_MORPH_TABLE:
            status = mDvs.freeMorphTable(addr, requestSize);
            break;
        case IPC_DVS_ALLOCATE_MORPH_TABLE:
            status = mDvs.allocateMorphTalbe(addr, requestSize);
            break;
        case IPC_DVS_GET_MORPH_TABLE:
            status = mDvs.getMorphTalbe(addr, requestSize);
            break;
        case IPC_DVS_SET_STATISTICS:
            status = mDvs.setStatistics(addr, requestSize);
            break;
        case IPC_DVS_EXECUTE:
            status = mDvs.execute(addr, requestSize);
            break;
        case IPC_DVS_GET_IMAGE_TRANSFORMATION:
            status = mDvs.getImageTransformation(addr, requestSize);
            break;
        case IPC_DVS_DEINIT:
            status = mDvs.deinit(addr, requestSize);
            break;
        case IPC_ISP_ADAPTOR_INIT:
            status = mIspAdaptor.init(addr, requestSize);
            break;
        case IPC_ISP_ADAPTOR_DEINIT:
            status = mIspAdaptor.deInit(addr, requestSize);
            break;
        case IPC_ISP_GET_PAL_SIZE:
            status = mIspAdaptor.getPalDataSize(addr, requestSize);
            break;
        case IPC_ISP_CONVERT_STATS: {
            ConvertStatsParam* params = static_cast<ConvertStatsParam*>(addr);
            ShmInfo statsDataInfo = {};
            status = getIntelAlgoServer()->getShmInfo(params->statsHandle, &statsDataInfo);
            if (status == OK) {
                status = mIspAdaptor.queryAndConvertStats(addr, requestSize, statsDataInfo.addr);
            } else {
                LOGE("%s, the buffer handle for stats data is invalid", __func__);
            }
            break;
        }
        case IPC_ISP_RUN_PAL: {
            RunPalParam* palParams = static_cast<RunPalParam*>(addr);
            ShmInfo palDataInfo;
            status = getIntelAlgoServer()->getShmInfo(palParams->palDataHandle, &palDataInfo);
            if (status != OK) {
                LOGE("%s, the buffer handle for pal data is invalid", __func__);
                break;
            }
            LOGIPC("@%s, pal data info: fd:%d, size:%zu, addr: %p", __func__, palDataInfo.fd,
                   palDataInfo.size, palDataInfo.addr);

            status = mIspAdaptor.runPal(addr, requestSize, palDataInfo.addr);
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

    LOGIPC("@%s, req_id:%d:%s, status:%d", __func__, req_id,
           IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), status);
    getIntelAlgoServer()->returnCallback(req_id, status, buffer_handle);
}
}  // namespace icamera
