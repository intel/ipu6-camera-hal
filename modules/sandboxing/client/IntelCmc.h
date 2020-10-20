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

#include <ia_cmc_parser.h>

#include <vector>

#include "IntelAlgoCommon.h"
#include "modules/sandboxing/IPCIntelCmc.h"

namespace icamera {
class IntelCmc {
 public:
    IntelCmc();
    virtual ~IntelCmc();

    // the nvmData must be nullptr currently
    bool init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData);

    ia_cmc_t* getCmc() const;
    uintptr_t getCmcHandle() const;

    void deinit();

 private:
    IPCIntelCmc mIpc;
    IntelAlgoCommon mCommon;

    bool mInitialized;

    ShmMemInfo mMemInit;
    ShmMemInfo mMemDeinit;

    std::vector<ShmMem> mMems;

    ia_cmc_t* mHandle;  // it points to the SHM

    // the pointer is in another process.
    // because the ia_aiq_init() needs the cmc pointer,
    // so keep the pinter in the IntelCmc and pass it aiq when init aiq.
    // at the same time, it will be used when deinit the cmc.
    uintptr_t mCmcRemoteHandle;
};
} /* namespace icamera */
