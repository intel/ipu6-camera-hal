/*
 * Copyright (C) 2022-2023 Intel Corporation
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

#define LOG_TAG IntelICBMServer

#include "modules/sandboxing/server/IntelICBMServer.h"

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

int IntelICBMServer::setup(ICBMInitInfo* initParam) {
    mIntelICBM = std::unique_ptr<IntelICBM>(new IntelICBM());

    return mIntelICBM->setup(initParam);
}

int IntelICBMServer::shutdown(const ICBMReqInfo& reqInfo) {
    int ret = mIntelICBM->shutdown(reqInfo);

    return ret >= 0 ? OK : ret;
}

int IntelICBMServer::processFrame(const ICBMReqInfo& reqInfo) {
    return mIntelICBM->processFrame(reqInfo);
}

}  // namespace icamera
