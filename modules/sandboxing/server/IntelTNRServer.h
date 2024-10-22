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

#pragma once

#include <memory>
#include <unordered_map>

#include "iutils/Errors.h"
#include "modules/algowrapper/IntelTNR7US.h"

namespace icamera {

class IntelTNRServer {
 public:
    IntelTNRServer();
    virtual ~IntelTNRServer();

    int init(void* pData, int dataSize);
    int deInit(TnrRequestInfo* requestInfo);
    int prepareSurface(void* pData, int dataSize, TnrRequestInfo* requestInfo);
    int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                    uint32_t outBufSize, void* tnrParam, TnrRequestInfo* requestInfo);
    int asyncParamUpdate(TnrRequestInfo* requestInfo);
    int getTnrBufferSize(TnrRequestInfo* requestInfo);

 private:
    // the tnr instance of the id
    std::unordered_map<int, TnrType> mTnrSlotMap;
    std::unordered_map<int, std::unique_ptr<IntelTNR7US>> mIntelTNRMap;
    std::unordered_map<int, std::unique_ptr<std::mutex>> mLockMap;
    // transfer cameraId and type to index of the mTnrSlotMap and mIntelTNRMap
    int getIndex(int cameraId, TnrType type) { return (cameraId << TNR_INSTANCE_MAX) + type; }
};
} /* namespace icamera */
