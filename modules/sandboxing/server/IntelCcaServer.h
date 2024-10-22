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

#include "memory"
#include <unordered_map>

#include "iutils/Errors.h"
#include "modules/algowrapper/IntelCca.h"
#include "modules/sandboxing/IPCIntelCca.h"

namespace icamera {
class IntelCcaServer {
 public:
    IntelCcaServer(int cameraId, TuningMode mode);
    virtual ~IntelCcaServer();

    status_t init(void* pData, int dataSize);
    status_t runAEC(void* pData, int dataSize);
    status_t runAIQ(void* pData, int dataSize);
    status_t runLTM(void* pData, int dataSize);
    status_t updateZoom(void* pData, int dataSize);
    status_t runDVS(void* pData, int dataSize);
    status_t runAIC(void* pData, int dataSize);
    status_t getCMC(void* pData, int dataSize);
    status_t getAiqd(void* pData, int dataSize);
    status_t updateTuning(void* pData, int dataSize);
    status_t deinit(void* pData, int dataSize);
    status_t decodeStats(void* pData, void* statsAddr);
    status_t getPalDataSize(void* pData, int dataSize);

 private:
    bool unflattenProgramGroup(cca::cca_program_group* result);

 private:
    int mCameraId;
    TuningMode mTuningMode;

    IntelCca* mCca;
};
} /* namespace icamera */
