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

#pragma once

#include <vector>

#include "Buffer.h"
#include "Context.h"
#include "Types.h"
#include "iutils/Utils.h"

namespace icamera {
namespace CIPR {
class Command {
 public:
    explicit Command(const PSysCommandConfig& cfg);
    ~Command();

    Result getConfig(PSysCommandConfig* cfg);
    Result setConfig(const PSysCommandConfig& cfg);
    Result enqueue(Context* ctx);

 private:
    Result grokBuffers(const PSysCommandConfig& cfg);
    Result updatePG(const PSysCommandConfig& cfg);
    Result updateKernel(const PSysCommandConfig& cfg, const MemoryDesc& memory);
    Result getLegacyPGMem(const PSysCommandConfig& cfg, MemoryDesc* memory);

 private:
    PSysCommand* mCmd = nullptr;
    bool mInitialized = false;

    DISALLOW_COPY_AND_ASSIGN(Command);
};
}  // namespace CIPR
}  // namespace icamera
