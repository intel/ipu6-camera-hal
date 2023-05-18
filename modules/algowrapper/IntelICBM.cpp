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

#define LOG_TAG IntelICBM

#include "modules/algowrapper/IntelICBM.h"

#include "Errors.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

int IntelICBM::setup(ICBMInitInfo* initParam) {
    mIntelOPIC2 = IntelOPIC2::getInstance();

    return mIntelOPIC2->setup(initParam);
}

int IntelICBM::shutdown(const ICBMReqInfo& request) {
    CheckAndLogError(mIntelOPIC2 == nullptr, UNKNOWN_ERROR, "@%s, no active ICBM session",
                     __func__);
    int ret = mIntelOPIC2->shutdown(request);
    // ret is the active session count, only release the object when no active session
    if (ret == 0) {
        IntelOPIC2::releaseInstance();
        mIntelOPIC2 = nullptr;
    }
    return ret;
}

int IntelICBM::processFrame(const ICBMReqInfo& reqInfo) {
    CheckAndLogError(mIntelOPIC2 == nullptr, UNKNOWN_ERROR, "@%s, no active ICBM session",
                     __func__);
    if (reqInfo.reqType == icamera::ICBMReqType::USER_FRAMING)
        return mIntelOPIC2->processFrame(reqInfo);
    if (reqInfo.reqType == icamera::ICBMReqType::LEVEL0_TNR)
        return mIntelOPIC2->runTnrFrame(reqInfo);
    return 0;
}

}  // namespace icamera
