/*
 * Copyright (C) 2022 Intel Corporation
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

#define LOG_TAG IntelICBM

#include "modules/algowrapper/IntelICBM.h"
#include "src/icbm/ICBMFactory.h"

#include "Errors.h"
#include "iutils/CameraLog.h"

namespace icamera {

Result IntelICBM::setup(ICBMInitInfo* initParam) {
    mIIntelICBM = std::unique_ptr<IIntelICBM>(createIntelICBM());

    return mIIntelICBM->setup(initParam);
}

void IntelICBM::shutdown() {
    mIIntelICBM->shutdown();
}

Result IntelICBM::processFrame(const ImageInfo& iii, const ImageInfo& iio,
                               const ICBMReqInfo& reqInfo) {
    return mIIntelICBM->processFrame(iii, iio, reqInfo);
}

}  // namespace icamera
