/*
 * Copyright (C) 2020-2023 Intel Corporation.
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
            status = mTNR.getTnrBufferSize(requestInfo);
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

        // LEVEL0_ICBM_S
        case IPC_ICBM_INIT:
            (void) requestSize;
            status = mICBMServer.setup(reinterpret_cast<ICBMInitInfo*>(addr));
            break;
        case IPC_ICBM_RUN_FRAME: {
            status = UNKNOWN_ERROR;
            ICBMReqInfo* runInfo = reinterpret_cast<ICBMReqInfo*>(addr);
            ShmInfo inBuffer = {};
            if (runInfo->inHandle < 0) break;
            ShmInfo outBuffer = {};
            if (runInfo->inHandle < 0) break;

            status = getIntelAlgoServer()->getShmInfo(runInfo->inHandle, &inBuffer);
            if (status != OK) {
                LOGE("%s, the buffer handle for ICBM inBuffer data is invalid", __func__);
                break;
            }

            status = getIntelAlgoServer()->getShmInfo(runInfo->outHandle, &outBuffer);
            if (status != OK) {
                LOGE("%s, the buffer handle for ICBM outBuffer data is invalid", __func__);
                break;
            }
            if (runInfo->paramHandle >= 0) {
                ShmInfo paramBuffer = {};
                status = getIntelAlgoServer()->getShmInfo(runInfo->paramHandle, &paramBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for parameter is invalid", __func__);
                    break;
                }
                runInfo->paramAddr = paramBuffer.addr;
            }
            runInfo->inII.bufAddr = inBuffer.addr;
            runInfo->outII.bufAddr = outBuffer.addr;

            status = mICBMServer.processFrame(*runInfo);

            runInfo->inII.bufAddr = nullptr;
            runInfo->outII.bufAddr = nullptr;
            runInfo->paramAddr = nullptr;
            break;
        }
        case IPC_ICBM_DEINIT: {
            ICBMReqInfo* shutInfo = static_cast<ICBMReqInfo*>(addr);
            status = mICBMServer.shutdown(*shutInfo);
            break;
        }
        // LEVEL0_ICBM_E
        default:
            LOGE("@%s, req_id:%d is not defined", __func__, req_id);
            status = UNKNOWN_ERROR;
            break;
    }
    LOG1("@%s, req_id:%d:%s, status:%d", __func__, req_id,
         IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), status);

    (void)requestSize;
    (void)addr;
    getIntelAlgoServer()->returnCallback(req_id, status, buffer_handle);
}
} /* namespace icamera */
