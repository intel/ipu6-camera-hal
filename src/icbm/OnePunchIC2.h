/*
 * Copyright (C) 2023 Intel Corporation
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

#include <string>
#include <memory>
#include <mutex>

#include "src/icbm/ICBMTypes.h"
#include "src/icbm/IntelICBM.h"
#include "src/icbm/OPIC2Api.h"

namespace icamera {

class IntelOPIC2 : public IIntelICBM {
 public:
    IntelOPIC2();
    ~IntelOPIC2() {}

    int setup(void* initParam);
    void shutdown();

    int processFrame(const ImageInfo& iii, const ImageInfo& iio, const ICBMReqInfo& reqInfo);

 private:
    iaic_session mCtxSession;
    std::mutex mCritObject;

    static iaic_memory createMemoryDesc(const ImageInfo& param);
    static std::string buildFeatureStr(const ICBMReqInfo& req);
};

}  // namespace icamera
