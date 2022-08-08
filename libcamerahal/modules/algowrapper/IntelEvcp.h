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

#include <unordered_map>
#include <memory>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#pragma clang diagnostic ignored "-Wunused-private-field"
// HANDLE is redefined in cm_rt.h, avoid the redefinition build error
#define HANDLE cancel_fw_pre_define
#include "igfxcmrt/cm_rt.h"
#pragma clang diagnostic pop

#include "PlatformData.h"
#include "evcp/UltraManEvcp.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "src/evcp/EvcpCommon.h"

namespace icamera {

class IntelEvcp {
 public:
    IntelEvcp();
    ~IntelEvcp() {}
    int init(int width, int height);

    bool runEvcpFrame(void* inBufAddr, int size);
    bool updateEvcpParam(const EvcpParam* evcpParam);
    void getEvcpParam(EvcpParam* evcpParam) const;  //  caller to ensure not nullptr

 private:
    bool runEvcpFrameNoncopy(void* inBufAddr);

    CmSurface2DUP* createCMSurface(void* bufAddr);
    bool destroyCMSurface(CmSurface2DUP* surface);

 private:
    int mWidth;
    int mHeight;

    std::unique_ptr<UltraManEvcp> mEvcpDLL;

    DISALLOW_COPY_AND_ASSIGN(IntelEvcp);
};

}  // namespace icamera
