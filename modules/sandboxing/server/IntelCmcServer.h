/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include "modules/algowrapper/IntelCmc.h"
#include "modules/sandboxing/IPCCommon.h"
#include "modules/sandboxing/IPCIntelCmc.h"

namespace icamera {
class IntelCmcServer {
 public:
    IntelCmcServer();
    virtual ~IntelCmcServer();

    status_t init(void* pData, int dataSize);
    status_t deinit(void* pData, int dataSize);

 private:
    std::unordered_map<ia_cmc_t*, std::unique_ptr<IntelCmc>> mIntelCmcs;
    IPCIntelCmc mIpc;
};

} /* namespace icamera */
