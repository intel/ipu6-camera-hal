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

#define LOG_TAG IntelTNRServer

#include "modules/sandboxing/server/IntelTNRServer.h"

#include <pvl_types.h>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

IntelTNRServer::IntelTNRServer() {
    LOG1("@%s Construct", __func__);
}

IntelTNRServer::~IntelTNRServer() {
    mIntelTNRMap.clear();
    mLockMap.clear();
    LOG1("@%s Destroy", __func__);
}

int IntelTNRServer::init(void* pData, int dataSize) {
    CheckAndLogError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckAndLogError(dataSize < static_cast<int>(sizeof(TnrInitInfo)), UNKNOWN_ERROR,
                     "@%s, buffer size %d is small", __func__, dataSize);
    TnrInitInfo* initInfo = static_cast<TnrInitInfo*>(pData);
    CheckAndLogError(initInfo->type >= TNR_INSTANCE_MAX || initInfo->type < 0, UNKNOWN_ERROR,
                     "@%s, invalid tnr type: %d", __func__, static_cast<int>(initInfo->type));

    CheckAndLogError(mIntelTNRMap.size() == TNR_INSTANCE_MAX, UNKNOWN_ERROR,
                     "@%s, tnr resource is busy", __func__);

    int tnrInstance;
    // choose a free tnr instance
    for (tnrInstance = TNR_INSTANCE0; tnrInstance < TNR_INSTANCE_MAX; tnrInstance++) {
        bool find = false;
        for (auto& ins : mTnrSlotMap) {
            if (ins.second == tnrInstance) {
                find = true;
                break;
            }
        }
        if (!find) break;
    }
    CheckAndLogError(tnrInstance == TNR_INSTANCE_MAX, UNKNOWN_ERROR,
                     "@%s, no avaliable tnr instance", __func__);

    int key = getIndex(initInfo->cameraId, initInfo->type);
    if (mIntelTNRMap.find(key) == mIntelTNRMap.end()) {
        mIntelTNRMap[key] =
            std::unique_ptr<IntelTNR7US>(IntelTNR7US::createIntelTNR(initInfo->cameraId));
    }

    mLockMap[key] = std::unique_ptr<std::mutex>(new std::mutex);
    mTnrSlotMap[key] = static_cast<TnrType>(tnrInstance);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    return mIntelTNRMap[key]->init(initInfo->width, initInfo->height,
                                   static_cast<TnrType>(tnrInstance));
}

int IntelTNRServer::deInit(TnrRequestInfo* requestInfo) {
    CheckAndLogError(requestInfo == nullptr, UNKNOWN_ERROR, "@%s, requestInfo is nullptr",
                     __func__);
    int key = getIndex(requestInfo->cameraId, requestInfo->type);
    CheckAndLogError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, IntelTNR type: %d is invalid", requestInfo->cameraId, __func__,
                     requestInfo->type);
    {
        std::unique_lock<std::mutex> lock(*mLockMap[key]);
        mIntelTNRMap.erase(key);
        mTnrSlotMap.erase(key);
    }
    mLockMap.erase(key);
    return OK;
}

int IntelTNRServer::prepareSurface(void* pData, int dataSize, TnrRequestInfo* requestInfo) {
    CheckAndLogError(pData == nullptr || requestInfo == nullptr, UNKNOWN_ERROR,
                     "@%s, invalid params, pData: %p, requestInfo: %p", __func__, pData,
                     requestInfo);
    int key = getIndex(requestInfo->cameraId, requestInfo->type);
    CheckAndLogError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, IntelTNR type: %d is invalid", requestInfo->cameraId, __func__,
                     requestInfo->type);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    return mIntelTNRMap[key]->prepareSurface(pData, dataSize);
}

int IntelTNRServer::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                                uint32_t outBufSize, void* tnrParam, TnrRequestInfo* requestInfo) {
    CheckAndLogError(requestInfo == nullptr, UNKNOWN_ERROR, "@%s, requestInfo is nullptr",
                     __func__);
    int key = getIndex(requestInfo->cameraId, requestInfo->type);
    CheckAndLogError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, IntelTNR type: %d is invalid", requestInfo->cameraId, __func__,
                     requestInfo->type);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    return mIntelTNRMap[key]->runTnrFrame(inBufAddr, outBufAddr, inBufSize, outBufSize,
                                          static_cast<Tnr7Param*>(tnrParam),
                                          requestInfo->isForceUpdate, requestInfo->outBufFd);
}

int IntelTNRServer::asyncParamUpdate(TnrRequestInfo* requestInfo) {
    CheckAndLogError(requestInfo == nullptr, UNKNOWN_ERROR, "@%s, requestInfo is nullptr",
                     __func__);
    int key = getIndex(requestInfo->cameraId, requestInfo->type);
    CheckAndLogError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, IntelTNR type: %d is invalid", requestInfo->cameraId, __func__,
                     requestInfo->type);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    return mIntelTNRMap[key]->asyncParamUpdate(requestInfo->gain, requestInfo->isForceUpdate);
}

int IntelTNRServer::getTnrBufferSize(TnrRequestInfo* requestInfo) {
    CheckAndLogError(requestInfo == nullptr, UNKNOWN_ERROR, "@%s, requestInfo is nullptr",
                     __func__);
    int key = getIndex(requestInfo->cameraId, requestInfo->type);
    CheckAndLogError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
                     "<id%d> @%s, IntelTNR type: %d is invalid", requestInfo->cameraId, __func__,
                     requestInfo->type);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    return mIntelTNRMap[key]->getTnrBufferSize(requestInfo->width, requestInfo->height,
                                               &requestInfo->surfaceSize);
}

}  // namespace icamera
