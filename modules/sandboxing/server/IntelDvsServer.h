/*
 * Copyright (C) 2019 Intel Corporation.
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
#include "modules/algowrapper/IntelDvs.h"
#include "modules/sandboxing/IPCIntelDvs.h"

namespace icamera {
class IntelDvsServer {
 public:
    IntelDvsServer();
    virtual ~IntelDvsServer();

    status_t init(void* pData, int dataSize);
    status_t deinit(void* pData, int dataSize);
    status_t config(void* pData, int dataSize);
    status_t setNonBlankRatio(void* pData, int dataSize);
    status_t setDigitalZoomMode(void* pData, int dataSize);
    status_t setDigitalZoomRegion(void* pData, int dataSize);
    status_t setDigitalZoomCoordinate(void* pData, int dataSize);
    status_t setDigitalZoomMagnitude(void* pData, int dataSize);
    status_t freeMorphTable(void* pData, int dataSize);
    status_t allocateMorphTalbe(void* pData, int dataSize);
    status_t getMorphTalbe(void* pData, int dataSize);
    status_t setStatistics(void* pData, int dataSize);
    status_t execute(void* pData, int dataSize);
    status_t getImageTransformation(void* pData, int dataSize);

 private:
    IntelDvs* getIntelDvs(ia_dvs_state* dvs);

 private:
    std::unordered_map<ia_dvs_state*, std::unique_ptr<IntelDvs>> mIntelDvss;
    IPCIntelDvs mIpc;
};
}  // namespace icamera
