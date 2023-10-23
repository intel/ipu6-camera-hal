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
#include <vector>
#include <unordered_map>

#include "iutils/CameraLog.h"
#include "Utils.h"
#include "TNRCommon.h"

namespace icamera {

IntelOPIC2* IntelOPIC2::sInstance = nullptr;
std::mutex IntelOPIC2::sLock;

void UserFramingBuilder::linkToMemoryChain(MemoryChainDescription& memoryChain) {
    memoryChain.linkIn("user_framing", "source:source", "drain:drain");
}

void BackgroundBlurBuilder::linkToMemoryChain(MemoryChainDescription& memoryChain) {
    memoryChain.linkIn("background_concealment", "in:source", "out:drain");
}

static const std::unordered_map<ICBMFeatureType, const char*> gFeatureStrMapping = {
    {ICBMFeatureType::USER_FRAMING, "user_framing"},
    {ICBMFeatureType::BC_MODE_BB, "background_concealment"},
    {ICBMFeatureType::LEVEL0_TNR, "tnr7us_l0"},
};

IntelOPIC2* IntelOPIC2::getInstance() {
    std::lock_guard<std::mutex> lock(sLock);
    if (!sInstance) {
        sInstance = new IntelOPIC2();
    }

    return sInstance;
}

void IntelOPIC2::releaseInstance() {
    std::lock_guard<std::mutex> lock(sLock);
    if (sInstance) delete sInstance;
    sInstance = nullptr;
}

IntelOPIC2::IntelOPIC2() {
    mLockMap.clear();
    mSessionMap.clear();
    mFeatureMap.clear();
}

IntelOPIC2::~IntelOPIC2() {
    mLockMap.clear();
    mSessionMap.clear();
    mFeatureMap.clear();
}

int IntelOPIC2::setup(ICBMInitInfo* initParams, std::shared_ptr<IC2ApiHandle> handle) {
    CheckAndLogError(!handle, NAME_NOT_FOUND, "%s", __func__);
    mIC2Api = handle;

    int ver[3];
    mIC2Api->query_version(&ver[0], &ver[1], &ver[2]);
    LOG1("@%s, IC2 Version %d.%d.%d", __func__, ver[0], ver[1], ver[2]);

    size_t featureLen;
    std::string supportedFeatures;
    mIC2Api->query_features(nullptr, &featureLen);
    supportedFeatures.resize(featureLen);
    mIC2Api->query_features(supportedFeatures.data(), &featureLen);
    LOG1("@%s, IC supported features: %s", __func__, supportedFeatures.c_str());

    LOG1("<%d>@%s type %d", initParams->cameraId, __func__, initParams->sessionType);
    int key = getIndexKey(initParams->cameraId, initParams->sessionType);

    CheckWarning((mSessionMap.find(key) != mSessionMap.end()), OK,
                 "<id%d> @%s, request type: %d is already exist", initParams->cameraId, __func__,
                 initParams->sessionType);

    for (int feature = USER_FRAMING; feature < REQUEST_MAX; feature <<= 1) {
        if (!(initParams->sessionType & feature)) continue;
        const char* featureStr = gFeatureStrMapping.at(static_cast<ICBMFeatureType>(feature));
        if (strstr(supportedFeatures.c_str(), featureStr) == nullptr) {
            LOG1("<%d>@%s type %d not supported", initParams->cameraId, __func__, feature);
            return OK;
        }
    }

    mLockMap[key] = std::unique_ptr<std::mutex>(new std::mutex);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);

    mFeatureMap[key] = std::vector<const char*>();
    // we use the key value as the unique session id
    mSessionMap[key] = static_cast<iaic_session>(key);
    if (initParams->sessionType & ICBMFeatureType::USER_FRAMING) {
        iaic_options option{};
        option.profiling = false;
        option.blocked_init = false;
        option.external_device = nullptr;
        const char* featureStr = gFeatureStrMapping.at(ICBMFeatureType::USER_FRAMING);
        mIC2Api->create_session(mSessionMap[key], featureStr, option);
        mFeatureMap[key].push_back(featureStr);
    }

    if (initParams->sessionType & ICBMFeatureType::BC_MODE_BB) {
        iaic_options option{};
        option.profiling = false;
        option.blocked_init = false;
        option.external_device = nullptr;
        const char* featureStr = gFeatureStrMapping.at(ICBMFeatureType::BC_MODE_BB);
        mIC2Api->create_session(mSessionMap[key], featureStr, option);
        mFeatureMap[key].push_back(featureStr);
    }
    if (initParams->sessionType & ICBMFeatureType::LEVEL0_TNR) {
        iaic_options option{};
        option.profiling = true;
        option.blocked_init = true;
        option.external_device = nullptr;
        const char* featureStr = gFeatureStrMapping.at(ICBMFeatureType::LEVEL0_TNR);
        mIC2Api->create_session(mSessionMap[key], featureStr, option);
        mFeatureMap[key].push_back(featureStr);
    }

    return OK;
}

int IntelOPIC2::shutdown(const ICBMReqInfo& reqInfo) {
    LOG1("<%d>@%s type %d", reqInfo.cameraId, __func__, reqInfo.sessionType);
    int key = getIndexKey(reqInfo.cameraId, reqInfo.sessionType);

    CheckAndLogError((mSessionMap.find(key) == mSessionMap.end()), NAME_NOT_FOUND,
                     "<id%d> @%s, request type: %d is not exist", reqInfo.cameraId, __func__,
                     reqInfo.sessionType);
    int ret = -1;
    {
        std::unique_lock<std::mutex> lock(*mLockMap[key]);
        for (auto& feature : mFeatureMap[key]) {
            mIC2Api->close_session(mSessionMap[key], feature);
        }
        mSessionMap.erase(key);
        ret = mSessionMap.size();
    }
    mLockMap.erase(key);
    return ret;
}

int IntelOPIC2::processFrame(const ICBMReqInfo& reqInfo) {
    int key = getIndexKey(reqInfo.cameraId, reqInfo.sessionType);

    CheckAndLogError((mSessionMap.find(key) == mSessionMap.end()), BAD_VALUE,
                     "<id%d> @%s, request type: %d is not exist", reqInfo.cameraId, __func__,
                     reqInfo.sessionType);
    auto mcd = createMemoryChain(reqInfo);
    auto mem = mcd.getIOPort();
    if (mem.first == nullptr) return OK;

    std::unique_lock<std::mutex> lock(*mLockMap[key]);
    bool res = mIC2Api->execute(mSessionMap[key], *mem.first, *mem.second);
    mIC2Api->get_data(mSessionMap[key], *mem.second);
    CheckAndLogError(res != true, UNKNOWN_ERROR, "%s, IC2 Internal Error on processing frame",
                     __func__);

    return OK;
}

int IntelOPIC2::runTnrFrame(const ICBMReqInfo& reqInfo) {
    LOG2("%s, ", __func__);
    int key = getIndexKey(reqInfo.cameraId, reqInfo.sessionType);

    CheckAndLogError((mSessionMap.find(key) == mSessionMap.end()), BAD_VALUE,
                     "<id%d> @%s, request type: %d is not exist", reqInfo.cameraId, __func__,
                     reqInfo.sessionType);

    const char* featureName = gFeatureStrMapping.at(ICBMFeatureType::LEVEL0_TNR);
    iaic_memory inMem, outMem;

    inMem.gfx = iaic_gfx_none;
    inMem.size[0] = reqInfo.inII.size;
    inMem.size[1] = reqInfo.inII.width;
    inMem.size[2] = reqInfo.inII.height;
    inMem.size[3] = reqInfo.inII.stride;
    inMem.media_type = iaic_nv12;
    inMem.p = reqInfo.inII.bufAddr;
    inMem.feature_name = featureName;
    inMem.port_name = "in:source";
    inMem.next = nullptr;

    outMem = inMem;
    outMem.size[0] = reqInfo.outII.size;
    outMem.size[1] = reqInfo.outII.width;
    outMem.size[2] = reqInfo.outII.height;
    outMem.size[3] = reqInfo.outII.stride;
    outMem.port_name = "out:drain";
    outMem.p = reqInfo.outII.bufAddr;

    Tnr7Param* tnrParam = static_cast<Tnr7Param*>(reqInfo.paramAddr);
    LOG2("%s,  is first %f", __func__, tnrParam->bc.is_first_frame);
    std::unique_lock<std::mutex> lock(*mLockMap[key]);
    setData(mSessionMap[key], &tnrParam->bc.is_first_frame, sizeof(tnrParam->bc.is_first_frame),
            featureName, "tnr7us/pal:is_first_frame");
    setData(mSessionMap[key], &tnrParam->bc.do_update, sizeof(tnrParam->bc.do_update), featureName,
            "tnr7us/pal:do_update");
    setData(mSessionMap[key], &tnrParam->bc.tune_sensitivity, sizeof(tnrParam->bc.tune_sensitivity),
            featureName, "tnr7us/pal:tune_sensitivity");
    setData(mSessionMap[key], &tnrParam->bc.coeffs, sizeof(tnrParam->bc.coeffs), featureName,
            "tnr7us/pal:coeffs");
    setData(mSessionMap[key], &tnrParam->bc.global_protection,
            sizeof(tnrParam->bc.global_protection), featureName, "tnr7us/pal:global_protection");
    setData(mSessionMap[key], &tnrParam->bc.global_protection_inv_num_pixels,
            sizeof(tnrParam->bc.global_protection_inv_num_pixels), featureName,
            "tnr7us/pal:global_protection_inv_num_pixels");
    setData(mSessionMap[key], &tnrParam->bc.global_protection_sensitivity_lut_values,
            sizeof(tnrParam->bc.global_protection_sensitivity_lut_values), featureName,
            "tnr7us/pal:global_protection_sensitivity_lut_values");
    setData(mSessionMap[key], &tnrParam->bc.global_protection_sensitivity_lut_slopes,
            sizeof(tnrParam->bc.global_protection_sensitivity_lut_slopes), featureName,
            "tnr7us/pal:global_protection_sensitivity_lut_slopes");
    // tnr7 imTnrSession, ms params
    setData(mSessionMap[key], &tnrParam->ims.update_limit, sizeof(tnrParam->ims.update_limit),
            featureName, "tnr7us/pal:update_limit");
    setData(mSessionMap[key], &tnrParam->ims.update_coeff, sizeof(tnrParam->ims.update_coeff),
            featureName, "tnr7us/pal:update_coeff");
    setData(mSessionMap[key], &tnrParam->ims.d_ml, sizeof(tnrParam->ims.d_ml), featureName,
            "tnr7us/pal:d_ml");
    setData(mSessionMap[key], &tnrParam->ims.d_slopes, sizeof(tnrParam->ims.d_slopes), featureName,
            "tnr7us/pal:d_slopes");
    setData(mSessionMap[key], &tnrParam->ims.d_top, sizeof(tnrParam->ims.d_top), featureName,
            "tnr7us/pal:d_top");
    setData(mSessionMap[key], &tnrParam->ims.outofbounds, sizeof(tnrParam->ims.outofbounds),
            featureName, "tnr7us/pal:outofbounds");
    setData(mSessionMap[key], &tnrParam->ims.radial_start, sizeof(tnrParam->ims.radial_start),
            featureName, "tnr7us/pal:radial_start");
    setData(mSessionMap[key], &tnrParam->ims.radial_coeff, sizeof(tnrParam->ims.radial_coeff),
            featureName, "tnr7us/pal:radial_coeff");
    setData(mSessionMap[key], &tnrParam->ims.frame_center_x, sizeof(tnrParam->ims.frame_center_x),
            featureName, "tnr7us/pal:frame_center_x");
    setData(mSessionMap[key], &tnrParam->ims.frame_center_y, sizeof(tnrParam->ims.frame_center_y),
            featureName, "tnr7us/pal:frame_center_y");
    setData(mSessionMap[key], &tnrParam->ims.r_coeff, sizeof(tnrParam->ims.r_coeff), featureName,
            "tnr7us/pal:r_coeff");
    // tnr7 bmTnrSession, lend params
    setData(mSessionMap[key], &tnrParam->blend.max_recursive_similarity,
            sizeof(tnrParam->blend.max_recursive_similarity), featureName,
            "tnr7us/pal:max_recursive_similarity");

    if (mIC2Api->execute(mSessionMap[key], inMem, outMem)) {
        mIC2Api->get_data(mSessionMap[key], outMem);
        return OK;
    }

    return UNKNOWN_ERROR;
}

void IntelOPIC2::setData(iaic_session uid, void* p, size_t size, const char* featureName,
                         const char* portName) {
    iaic_memory setting{};
    setting.has_gfx = false;
    setting.feature_name = featureName;

    setting.port_name = portName;
    setting.p = p;
    setting.size[0] = size;
    mIC2Api->set_data(uid, setting);
}

MemoryChainDescription IntelOPIC2::createMemoryChain(const ICBMReqInfo& reqInfo) {
    MemoryChainDescription mCD(reqInfo.inII, reqInfo.outII);

    if (reqInfo.reqType & ICBMFeatureType::USER_FRAMING) {
        UserFramingBuilder().linkToMemoryChain(mCD);
    }

    if (reqInfo.reqType & ICBMFeatureType::BC_MODE_BB) {
        BackgroundBlurBuilder().linkToMemoryChain(mCD);
    }
    if (reqInfo.reqType & ICBMFeatureType::LEVEL0_TNR) {
        mCD.linkIn(gFeatureStrMapping.at(ICBMFeatureType::LEVEL0_TNR), "in:source", "out:drain");
    }

    return mCD;
}

}  // namespace icamera
