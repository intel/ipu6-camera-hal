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

#define LOG_TAG "IntelTNRServer"

#include "modules/sandboxing/server/IntelTNRServer.h"

#include <pvl_types.h>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelTNRServer::IntelTNRServer() {
    LOG1("@%s", __func__);
}

IntelTNRServer::~IntelTNRServer() {
    mIntelTNRMap.clear();
    LOG1("@%s", __func__);
}

int IntelTNRServer::init(void* pData, int dataSize) {
    LOG1("@%s, pData:%p, dataSize:%d", __func__, pData, dataSize);
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    CheckError(dataSize < static_cast<int>(sizeof(TnrInitInfo)), UNKNOWN_ERROR, "buffer is small");
    TnrInitInfo* initInfo = static_cast<TnrInitInfo*>(pData);
    CheckError(initInfo->type >= TNR_INSTANCE_MAX || initInfo->type < 0, UNKNOWN_ERROR,
               "@%s, invalid tnr type %d", __func__, static_cast<int>(initInfo->type));

    CheckError(mIntelTNRMap.size() == TNR_INSTANCE_MAX, UNKNOWN_ERROR, "@%s, tnr resource is busy",
               __func__);

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
    CheckError(tnrInstance == TNR_INSTANCE_MAX, UNKNOWN_ERROR, "@%s, no avaliable tnr instance",
               __func__);

    int key = getIndex(initInfo->cameraId, initInfo->type);
    if (mIntelTNRMap.find(key) == mIntelTNRMap.end()) {
        mIntelTNRMap[key] = std::unique_ptr<IntelTNR7US>(new IntelTNR7US(initInfo->cameraId));
    }

    mTnrSlotMap[key] = static_cast<TnrType>(tnrInstance);

    return mIntelTNRMap[key]->init(initInfo->width, initInfo->height,
                                   static_cast<TnrType>(tnrInstance));
}

int IntelTNRServer::deInit(int cameraId, TnrType type) {
    LOG1("@%s", __func__);
    int key = getIndex(cameraId, type);
    CheckError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
               "%s, IntelTNR type:%d is nullptr", __func__, type);
    mIntelTNRMap.erase(key);
    mTnrSlotMap.erase(key);
    return OK;
}

int IntelTNRServer::prepareSurface(void* pData, int dataSize, int cameraId, TnrType type) {
    CheckError(pData == nullptr, UNKNOWN_ERROR, "@%s, pData is nullptr", __func__);
    int key = getIndex(cameraId, type);
    CheckError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
               "%s, IntelTNR type:%d is nullptr", __func__, type);
    return mIntelTNRMap[key]->prepareSurface(pData, dataSize);
}

int IntelTNRServer::runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                                uint32_t outBufSize, void* tnrParam, int cameraId, TnrType type) {
    int key = getIndex(cameraId, type);
    CheckError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
               "%s, IntelTNR type:%d is nullptr", __func__, type);
    return mIntelTNRMap[key]->runTnrFrame(inBufAddr, outBufAddr, inBufSize, outBufSize,
                                          static_cast<Tnr7Param*>(tnrParam));
}

int IntelTNRServer::asyncParamUpdate(int cameraId, int gain, TnrType type) {
    int key = getIndex(cameraId, type);
    CheckError((mIntelTNRMap.find(key) == mIntelTNRMap.end()), UNKNOWN_ERROR,
               "%s, IntelTNR type:%d is nullptr", __func__, type);
    return mIntelTNRMap[key]->asyncParamUpdate(gain);
}

}  // namespace icamera
