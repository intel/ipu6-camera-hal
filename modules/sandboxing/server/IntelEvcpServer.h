/*
 * Copyright (C) 2021 Intel Corporation.
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

#include "evcp/EvcpCommon.h"
#include "iutils/Errors.h"
#include "modules/algowrapper/IntelEvcp.h"

namespace icamera {

class IntelEvcpServer {
 public:
    IntelEvcpServer() {}
    ~IntelEvcpServer() {}

    int init(void* pData, int dataSize);
    int deInit();

    int runEvcpFrame(void* bufAddr, int size);
    int updateEvcpParam(EvcpParam* param);
    int getEvcpParam(EvcpParam* evcpParam) const;

 private:
    std::unique_ptr<IntelEvcp> mIntelEvcp;
};
} /* namespace icamera */
