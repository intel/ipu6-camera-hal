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

#define LOG_TAG OnePunchIC2

#include "src/icbm/OnePunchIC2.h"

#include <string>

#include "iutils/CameraLog.h"
#include "Utils.h"

namespace icamera {

IntelOPIC2::IntelOPIC2() {
    mCtxSession = reinterpret_cast<uintptr_t>(this);
}

#define UF_FEATURE_NAME "user_framing"

int IntelOPIC2::setup(void* initParams) {
    (void)initParams;
    LOGI("$%s, Going to setup up IC2...", __func__);

    int ver[3];
    ::iaic_query_version(&ver[0], &ver[1], &ver[2]);
    LOGI("$%s, IC Version %d.%d.%d", __func__, ver[0], ver[1], ver[2]);

    size_t featureLen;
    std::string featureStr;
    ::iaic_query_features(nullptr, &featureLen);
    featureStr.resize(featureLen);
    ::iaic_query_features(featureStr.data(), &featureLen);
    LOGI("$%s, IC supported features: %s", __func__, featureStr.c_str());

    iaic_options option{};
    option.profiling = false;
    option.blocked_init = false;
    ::iaic_create_session(mCtxSession, UF_FEATURE_NAME, option);

    return OK;
}

void IntelOPIC2::shutdown() {
    LOGI("$%s, Going to shutdown IC2...", __func__);

    std::lock_guard<std::mutex> lock(mCritObject);
    ::iaic_close_session(mCtxSession, UF_FEATURE_NAME);
}

int IntelOPIC2::processFrame(const ImageInfo& iii, const ImageInfo& iio,
                             const ICBMReqInfo& reqInfo) {
    iaic_memory inMem, outMem;

    inMem = createMemoryDesc(iii);
    outMem = createMemoryDesc(iio);

    std::string featureStr = buildFeatureStr(reqInfo);
    outMem.feature_name = featureStr.c_str();
    outMem.port_name = "drain:drain";
    inMem.p = iii.bufAddr;
    outMem.p = iio.bufAddr;

    std::lock_guard<std::mutex> lock(mCritObject);
    bool res = ::iaic_execute(mCtxSession, inMem, outMem);
    if (::iaic_get_status(mCtxSession, featureStr.c_str()) == iaic_session_status::opened) {
        ::iaic_get_data(mCtxSession, outMem);
    }

    CheckAndLogError(res != true, UNKNOWN_ERROR, "$%s, IC2 Internal Error on processing frame",
                     __func__);

    return OK;
}

iaic_memory IntelOPIC2::createMemoryDesc(const ImageInfo& ii) {
    iaic_memory mem = {};

    mem.has_gfx = false;
    mem.size[0] = ii.size;
    mem.size[1] = ii.width;
    mem.size[2] = ii.height;
    mem.size[3] = ii.stride;

    return mem;
}

/*
    Due to libintelic2.so's limitations, UF is the only feature supported now.
*/
std::string IntelOPIC2::buildFeatureStr(const ICBMReqInfo& req) {
    if (req.usrFrmEnabled == UF_MODE_ON) {
        return std::string(UF_FEATURE_NAME);
    } else {
        return std::string();
    }
}

}  // namespace icamera
