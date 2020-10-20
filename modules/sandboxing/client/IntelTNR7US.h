/*
 * Copyright (C) 2020 Intel Corporation
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
#include "TNRCommon.h"

namespace icamera {

class IntelTNR7US {
 public:
    explicit IntelTNR7US(int cameraId);
    ~IntelTNR7US();
    int init(int width, int height, TnrType type);
    int runTnrFrame(const void* inBufAddr, void* outBufAddr, uint32_t inBufSize,
                    uint32_t outBufSize, Tnr7Param* tnrParam);
    Tnr7Param* allocTnr7ParamBuf();
    void* allocCamBuf(uint32_t bufSize, int id);
    void freeAllBufs();
    int asyncParamUpdate(int gain);

 private:
    int mCameraId;
    TnrType mTnrType;
    IntelAlgoCommon mCommon;
    TnrRequestInfo* mTnrRequestInfo;
    std::vector<ShmMemInfo> mCamBufMems;
    ShmMemInfo mParamMems;
    ShmMemInfo mTnrRequestInfoMem;
    DISALLOW_COPY_AND_ASSIGN(IntelTNR7US);
};
}  // namespace icamera
