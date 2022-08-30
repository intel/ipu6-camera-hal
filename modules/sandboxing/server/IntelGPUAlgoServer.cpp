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

#define LOG_TAG IntelGPUAlgoServer

#include "modules/sandboxing/server/IntelGPUAlgoServer.h"

#include <ia_log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "iutils/Utils.h"

namespace icamera {

void IntelGPUAlgoServer::handleRequest(const MsgReq& msg) {
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
#ifdef TNR7_CM
        case IPC_GPU_TNR_INIT:
            status = mTNR.init(addr, requestSize);
            break;
        case IPC_GPU_TNR_GET_SURFACE_INFO: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            status = mTNR.getSurfaceInfo(requestInfo);
            break;
        }
        case IPC_GPU_TNR_PREPARE_SURFACE: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            ShmInfo surfaceBuffer = {};
            if (requestInfo->surfaceHandle >= 0) {
                status =
                    getIntelAlgoServer()->getShmInfo(requestInfo->surfaceHandle, &surfaceBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for surfaceBuffer data is invalid", __func__);
                    break;
                }
            }
            status = mTNR.prepareSurface(surfaceBuffer.addr, surfaceBuffer.size, requestInfo);
            break;
        }
        case IPC_GPU_TNR_RUN_FRAME:
        case IPC_GPU_TNR_THREAD2_RUN_FRAME: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            ShmInfo inBuffer = {};
            ShmInfo outBuffer = {};
            ShmInfo paramBuffer = {};
            if (requestInfo->inHandle >= 0) {
                status = getIntelAlgoServer()->getShmInfo(requestInfo->inHandle, &inBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for inBuffer data is invalid", __func__);
                    break;
                }
            }
            if (requestInfo->outHandle >= 0) {
                status = getIntelAlgoServer()->getShmInfo(requestInfo->outHandle, &outBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for outBuffer data is invalid", __func__);
                    break;
                }
            }
            if (requestInfo->paramHandle >= 0) {
                status = getIntelAlgoServer()->getShmInfo(requestInfo->paramHandle, &paramBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for parameter is invalid", __func__);
                    break;
                }
            }

            status = mTNR.runTnrFrame(inBuffer.addr, outBuffer.addr, inBuffer.size, outBuffer.size,
                                      paramBuffer.addr, requestInfo);
            break;
        }
        case IPC_GPU_TNR_PARAM_UPDATE:
        case IPC_GPU_TNR_THREAD2_PARAM_UPDATE: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            status = mTNR.asyncParamUpdate(requestInfo);
            break;
        }
        case IPC_GPU_TNR_DEINIT: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            status = mTNR.deInit(requestInfo);
            break;
        }
#endif
            // ENABLE_EVCP_S
        case IPC_EVCP_INIT:
            status = mEvcp.init(addr, requestSize);
            break;
        case IPC_EVCP_UPDCONF:
            status = mEvcp.updateEvcpParam(reinterpret_cast<EvcpParam*>(addr));
            break;
        case IPC_EVCP_GETCONF:
            mEvcp.getEvcpParam(reinterpret_cast<EvcpParam*>(addr));
            status = OK;
            break;
        case IPC_EVCP_RUN_FRAME: {
            status = UNKNOWN_ERROR;
            EvcpRunInfo* runInfo = static_cast<EvcpRunInfo*>(addr);
            ShmInfo inBuffer = {};
            if (runInfo->inHandle < 0) break;

            status = getIntelAlgoServer()->getShmInfo(runInfo->inHandle, &inBuffer);
            if (status != OK) {
                LOGE("%s, the buffer handle for EVCP inBuffer data is invalid", __func__);
                break;
            }

            status = mEvcp.runEvcpFrame(inBuffer.addr, inBuffer.size);
            break;
        }
        case IPC_EVCP_DEINIT:
            status = mEvcp.deInit();
            break;
            // ENABLE_EVCP_E
        default:
            LOGE("@%s, req_id:%d is not defined", __func__, req_id);
            status = UNKNOWN_ERROR;
            break;
    }
    LOG1("@%s, req_id:%d:%s, status:%d", __func__, req_id,
         IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), status);

    getIntelAlgoServer()->returnCallback(req_id, status, buffer_handle);
}
} /* namespace icamera */
