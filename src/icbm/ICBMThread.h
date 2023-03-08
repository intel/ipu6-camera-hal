/*
 * Copyright (C) 2023 Intel Corporation.
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

#include "ICBMTypes.h"
#include "IntelICBM.h"

#include "core/CameraBuffer.h"
#include "iutils/Utils.h"
#include "Parameters.h"

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelICBMClient.h"
#else
#include "modules/algowrapper/IntelICBM.h"
#endif

namespace icamera {

class ICBMThread {
 public:
    ICBMThread() {}
    ~ICBMThread() {}

    int setup(ICBMInitInfo* initParams);
    void shutdown();

    int processFrame(const camera_buffer_t& inBuffer, const camera_buffer_t& outBuffer,
                            ICBMReqInfo* param);

 private:
    std::unique_ptr<IntelICBM> mIntelICBM;
};
}  // namespace icamera