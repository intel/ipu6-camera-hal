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

#define LOG_TAG "IntelGPUAlgoServer"

#include "modules/sandboxing/server/IntelGPUAlgoServer.h"

#include <base/logging.h>
#include <ia_log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "iutils/Utils.h"

namespace icamera {

IntelGPUAlgoServer::IntelGPUAlgoServer(IntelAlgoServer* server) : RequestHandler(server) {
    LOGIPC("@%s", __func__);
}

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
    LOGIPC("@%s, req_id:%d:%s, requestSize:%zu, buffer_handle:%d addr:%p", __func__, req_id,
           IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), requestSize, buffer_handle, addr);
    switch (req_id) {
#ifdef TNR7_CM
        case IPC_GPU_TNR_INIT:
            status = mTNR.init(addr, requestSize);
            break;
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
            status = mTNR.prepareSurface(surfaceBuffer.addr, surfaceBuffer.size,
                                         requestInfo->cameraId, requestInfo->type);
            break;
        }
        case IPC_GPU_TNR_RUN_FRAME: {
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
                    LOGE("%s, the buffer handle for inBuffer data is invalid", __func__);
                    break;
                }
            }
            if (requestInfo->paramHandle >= 0) {
                status = getIntelAlgoServer()->getShmInfo(requestInfo->paramHandle, &paramBuffer);
                if (status != OK) {
                    LOGE("%s, the buffer handle for inBuffer data is invalid", __func__);
                    break;
                }
            }

            status = mTNR.runTnrFrame(inBuffer.addr, outBuffer.addr, inBuffer.size, outBuffer.size,
                                      paramBuffer.addr, requestInfo->cameraId, requestInfo->type);
            break;
        }
        case IPC_GPU_TNR_PARAM_UPDATE: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            status =
                mTNR.asyncParamUpdate(requestInfo->cameraId, requestInfo->gain, requestInfo->type);
            break;
        }
        case IPC_GPU_TNR_DEINIT: {
            TnrRequestInfo* requestInfo = static_cast<TnrRequestInfo*>(addr);
            status = mTNR.deInit(requestInfo->cameraId, requestInfo->type);
            break;
        }
#endif
        default:
            LOGE("@%s, req_id:%d is not defined", __func__, req_id);
            status = UNKNOWN_ERROR;
            break;
    }

    getIntelAlgoServer()->returnCallback(req_id, status, buffer_handle);
}
} /* namespace icamera */
