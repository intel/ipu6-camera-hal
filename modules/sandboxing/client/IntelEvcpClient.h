/*
 * Copyright (C) 2021 Intel Corporation
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

#include "BufferQueue.h"
#include "CameraBuffer.h"
#include "Parameters.h"
#include "PlatformData.h"
#include "src/evcp/EvcpCommon.h"

namespace icamera {

class IntelEvcp {
 public:
    IntelEvcp() {}
    ~IntelEvcp();
    int init(int width, int height, EvcpParam* param = nullptr);

    bool runEvcpFrame(int dmafd, int dataSize);
    bool updateEvcpParam(EvcpParam* evcpParam);
    void getEvcpParam(EvcpParam* evcpParam);

 private:
    bool initResolution(int width, int height);
    bool initParamBuffer();
    bool initRunInfoBuffer();

    IntelAlgoCommon mCommon;
    ShmMemInfo mParamMems;
    ShmMemInfo mEvcpRunInfoMem;

    DISALLOW_COPY_AND_ASSIGN(IntelEvcp);
};
}  // namespace icamera
