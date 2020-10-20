/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#include "modules/algowrapper/IntelAiq.h"
#include "modules/sandboxing/IPCCommon.h"
#include "modules/sandboxing/IPCIntelAiq.h"

namespace icamera {
class IntelAiqServer {
 public:
    IntelAiqServer();
    virtual ~IntelAiqServer();

    status_t init(void* pData, int dataSize);
    status_t aeRun(void* pData, int dataSize);
    status_t afRun(void* pData, int dataSize);
    status_t awbRun(void* pData, int dataSize);
    status_t gbceRun(void* pData, int dataSize);
    status_t paRunV1(void* pData, int dataSize);
    status_t saRunV2(void* pData, int dataSize);
    status_t statisticsSetV4(void* pData, int dataSize);
    status_t getAiqdData(void* pData, int dataSize);
    status_t deinit(void* pData, int dataSize);
    status_t getVersion(void* pData, int dataSize);

 private:
    IPCIntelAiq mIpc;
    std::unordered_map<ia_aiq*, std::unique_ptr<IntelAiq>> mIntelAiqs;
};

} /* namespace icamera */
