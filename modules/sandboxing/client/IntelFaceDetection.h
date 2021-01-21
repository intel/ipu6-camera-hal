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

#include <vector>

#include "FaceBase.h"
#include "IntelAlgoCommon.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "modules/sandboxing/IPCIntelFD.h"

namespace icamera {
class IntelFaceDetection {
 public:
    IntelFaceDetection();
    virtual ~IntelFaceDetection();

    status_t init(FaceDetectionInitParams* initData, int dataSize);
    status_t deinit(FaceDetectionDeinitParams* deinitParams, int dataSize);
    status_t run(FaceDetectionRunParams* runParams, int dataSize, int dmafd = -1);
    FaceDetectionRunParams* prepareRunBuffer(unsigned int index);

 private:
    IPCIntelFD mIpc;
    IntelAlgoCommon mCommon;

    bool mInitialized;

    ShmMemInfo mMemInit;
    ShmMemInfo mMemDeinit;
    ShmMemInfo mMemRunBufs[MAX_STORE_FACE_DATA_BUF_NUM];
    std::vector<ShmMem> mMems;
};
} /* namespace icamera */
