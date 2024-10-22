/*
 * Copyright (C) 2021 Intel Corporation.
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

#define LOG_TAG UltraManEvcp

#include "src/evcp/UltraManEvcp.h"

#include <stdlib.h>
#include <unistd.h>

#include <string>

#include "iutils/CameraLog.h"
#include "iutils/Thread.h"
#include "iutils/Utils.h"

#define EVCP_SUCCESS 0
#define LIBEVCP_NAME "libevcp.so"

namespace icamera {

UltraManEvcp::UltraManEvcp(int width, int height)
        : mCurrWidth(width),
          mCurrHeight(height),
          mFrameCount(0),
          mEccEnabled(false),
          mDimmerEnabled(false),
          mLowPowerMode(false),
          mMdfTurbo(false),
          mEvcpBGConcelment(false),
          mEvcpBGReplacement(false),
          mFaceBea(false),
          mFaceFra(false),
          mCtx(nullptr),
          mInit(false),
          mHandleEvcpLib(nullptr) {}

bool UltraManEvcp::init() {
    setenv("NoJitter", "1", 1);
    setenv("EccTaskOverwhelm", "0 1,0 1", 1);

    memset(&mApi, 0, sizeof(mApi));

    mDevice = getSystemCMDevice();
    CheckAndLogError(mDevice == nullptr, false, "$%s:CMDevice is nullptr", __func__);

    loadEvcpLibrary();
    CheckAndLogError(mApi.EvcpCreate == nullptr, false, "$%s:Cannot call Evcp ctor", __func__);

    mCtx = mApi.EvcpCreate(mCurrWidth, mCurrHeight, mDevice, 1);
    CheckAndLogError(mCtx == nullptr, false, "$%s:Evcp context is nullptr", __func__);

    mInit = true;
    return true;
}

UltraManEvcp::~UltraManEvcp() {
    if (!mInit) return;

    mInit = false;
    if (mCtx && mApi.EvcpDestroy) {
        LOG1("@%s EVCP Shutdown ", __func__);
        mApi.EvcpDestroy(mCtx);
    }
    ::DestroyCmDevice(mDevice);
    CameraUtils::dlcloseLibrary(mHandleEvcpLib);
    mHandleEvcpLib = nullptr;
}

bool UltraManEvcp::checkEVCPBackgroundURL(const std::string& name) const {
    if (name.empty()) {
        LOGW("%s, FileName empty ", __func__);
        return true;
    }
    if (access(name.c_str(), R_OK) != 0) {
        LOGE("%s, Cannot read file: [%s]", __func__, name.c_str());
        return false;
    }

    std::size_t pos;
    pos = name.find_last_of("_");
    if (pos == std::string::npos) {
        LOGW("%s, Malform filename: %s", __func__, name.c_str());
        return true;
    }

    std::size_t extPos;
    extPos = name.find(".nv12");
    if (pos == std::string::npos) {
        LOGW("%s, Malform filename: %s", __func__, name.c_str());
        return true;
    }

    if (extPos - pos <= 3) {
        LOGW("%s, Malform filename: %s", __func__, name.c_str());
        return true;
    }

    std::string sizeInfo = name.substr(pos + 1, extPos - pos - 1);

    std::size_t xPos = sizeInfo.find('x');
    int width = std::atol(std::string(sizeInfo.begin(), sizeInfo.begin() + xPos).c_str());
    int height = std::atol(sizeInfo.data() + xPos + 1);

    if ((width != mCurrWidth) || (height != mCurrHeight)) {
        LOGW("%s Size mismatch, width = %d, height = %, mCurrHeight = %d, mCurrHeight = %d",
             __func__, width, height, mCurrWidth, mCurrHeight);
    }

    return true;
}

bool UltraManEvcp::isEvcpEnable() const {
    return isEccEnabled() || isBGConcelment() || isFaceBeaEnable() || isFaceFraEnable();
}

bool UltraManEvcp::processFrame(CmSurface2DUP* pSample) {
    if (!isEvcpEnable()) {
        return true;
    }

    if (!mApi.EvcpProcessFrame || !mCtx) {
        LOGE("%s, Evcp function error.", __func__);
        return false;
    }

    bool lockStatus = mCritMutexEccObject.try_lock();
    CheckAndLogError(lockStatus == false, false, "%s return as lock is occupied by others",
                     __func__);

    if (mApi.EvcpProcessFrame(&mCtx, pSample, pSample) == EVCP_SUCCESS) {
        mFrameCount++;
        mCritMutexEccObject.unlock();
        return true;
    }

    // Not a fatal error
    LOGW("%s EvcpProcessFrame BAD", __func__);
    mCritMutexEccObject.unlock();
    return true;
}

bool UltraManEvcp::updateEvcpInfo(const EvcpParam* param) {
    if (!param->evcpBGConcealment && param->evcpBGReplacement) {
        LOGW("%s EvcpBGConcealment and EvcpBGReplacement conflict", __func__);
        return false;
    }

    if (strlen(param->evcpBGFileName) &&
        !checkEVCPBackgroundURL(std::string(param->evcpBGFileName))) {
        LOGE("%s checkEVCPBackgroundURL FAILED", __func__);
        return false;
    }

    bool res = flushStatus(param);
    if (!res) LOGE("%s flushStatus FAILED", __func__);

    LOG1("@%s ECC [%d], DIM [%d], LP [%d], TURBO [%d] CON [%d] REPL [%d], FB[%d], FF[%d]", __func__,
         mEccEnabled, mDimmerEnabled, mLowPowerMode, mMdfTurbo, mEvcpBGConcelment,
         mEvcpBGReplacement, mFaceBea, mFaceFra);

    return true;
}

bool UltraManEvcp::loadEvcpLibrary() {
    mHandleEvcpLib = CameraUtils::dlopenLibrary(LIBEVCP_NAME, RTLD_NOW);
    CheckAndLogError(!mHandleEvcpLib, false, "mHandleEvcpLib is nullptr");
#ifndef TIGA
#define TIGA(member, fn_name)                                                                  \
    do {                                                                                       \
        mApi.member =                                                                          \
            (EVCP_API_HANDLE::pFn##member)CameraUtils::dlsymLibrary(mHandleEvcpLib, #fn_name); \
        if (mApi.member == nullptr) {                                                          \
            LOG1("@%s: LOADING " #fn_name "failed for member", __func__);                      \
        }                                                                                      \
        LOG1("@%s: " #fn_name "= %x", __func__, mApi.member);                                  \
    } while (0)
#endif
    TIGA(EvcpCreate, evcp_create_context);
    TIGA(EvcpDestroy, evcp_destroy_context);
    TIGA(EvcpToggleEcc, evcp_toggle_ecc);
    TIGA(EvcpSetMdfTurbo, evcp_set_mdf_turbo);
    TIGA(EvcpSetVerbose, evcp_set_verbose);
    TIGA(EvcpSetPowerSavingMode, evcp_set_power_saving_mode);
    TIGA(EvcpProcessFrame, evcp_process_frame3);
    TIGA(EvcpProcessFrame2D, evcp_process_frame2);
    TIGA(EvcpSetBackGroundURL, evcp_set_background_url);
    TIGA(EvcpToggleBackgroundConcelment, evcp_toggle_background_concealment);
    TIGA(EvcpToggleBackgroundReplacement, evcp_toggle_background_replacement);
    TIGA(EvcpSetFaceBea, evcp_toggle_face_beautify);
    TIGA(EvcpSetFaceFra, evcp_toggle_face_centering);
#undef TIGA

    return true;
}

bool UltraManEvcp::setEvcpEnable(bool v) {
    bool res = false;

    if (mApi.EvcpToggleEcc) {
        LOG1("%s mEccEnabled will setting to %d", __func__, (int)v);
        if (mApi.EvcpToggleEcc(mCtx, static_cast<int>(v)) == EVCP_SUCCESS) {
            mEccEnabled = v;
            res = true;
        }
    }
    return res;
}

bool UltraManEvcp::setEvcpPowerSavingMode(bool v) {
    bool res = false;

    if (mApi.EvcpSetPowerSavingMode) {
        LOG1("%s mEvcpSetPowerSavingMode setting to %d", __func__, (int)v);
        if (mApi.EvcpSetPowerSavingMode(mCtx, static_cast<int>(v)) == EVCP_SUCCESS) {
            mLowPowerMode = v;
            res = true;
        }
    }
    return res;
}

bool UltraManEvcp::setEvcpSetMdfTurbo(bool v) {
    bool res = false;

    if (mApi.EvcpSetMdfTurbo) {
        LOG1("%s mMdfTurbo setting to %d", __func__, (int)v);
        if (mApi.EvcpSetMdfTurbo(mCtx, static_cast<int>(v)) == EVCP_SUCCESS) {
            mMdfTurbo = v;
            res = true;
        }
    }
    return res;
}

bool UltraManEvcp::setEvcpFaceBea(bool v) {
    bool res = false;

    if (mApi.EvcpSetFaceBea) {
        LOG1("%s EvcpSetFaceBea setting to %d", __func__, (int)v);
        if (mApi.EvcpSetFaceBea(mCtx, static_cast<int>(v)) == EVCP_SUCCESS) {
            mFaceBea = v;
            res = true;
        }
    }
    return res;
}

bool UltraManEvcp::setEvcpFaceFra(bool v) {
    bool res = false;

    if (mApi.EvcpSetFaceFra) {
        LOG1("%s FF will setting to  %d", __func__, (int)v);
        if (mApi.EvcpSetFaceFra(mCtx, static_cast<int>(v)) == EVCP_SUCCESS) {
            mFaceFra = v;
            res = true;
        }
    }
    return res;
}

bool UltraManEvcp::setEvcpConcelment(bool con, bool replace, const char* filePath) {
    bool res = true;

    if (mApi.EvcpToggleBackgroundConcelment) {
        if (mApi.EvcpToggleBackgroundConcelment(mCtx, static_cast<int>(con)) == EVCP_SUCCESS) {
            LOG1("%s setEvcpConcelment OK", __func__);
            mEvcpBGConcelment = con;
            if (!mEvcpBGConcelment) mEvcpBGReplacement = false;
        } else {
            LOGE("%s setEvcpConcelment FAILED", __func__);
            res = false;
        }
    }

    if (res && mApi.EvcpToggleBackgroundReplacement) {
        res = setEvcpReplacement(replace, filePath);
        if (res == false) {
            LOGE("%s setEvcpReplacement FAILED", __func__);
        }
    }

    return res;
}

bool UltraManEvcp::setEvcpReplacement(bool replace, const char* filePath) {
    bool res = true;

    if (filePath) {
        if (mApi.EvcpSetBackGroundURL(mCtx, filePath) == EVCP_SUCCESS) {
            LOG1("%s EvcpSetBackGroundURL OK", __func__);
            mEvcpBackgroundURL = std::string(filePath);
        } else {
            LOGE("%s EvcpSetBackGroundURL FAILED", __func__);
            res = false;
        }
    } else {
        replace = false;
    }

    if (res) {
        if (mApi.EvcpToggleBackgroundReplacement(mCtx, static_cast<int>(replace)) == EVCP_SUCCESS) {
            LOG1("%s EvcpToggleBackgroundReplacement OK", __func__);
            mEvcpBGReplacement = replace;
        } else {
            LOGE("%s EvcpToggleBackgroundReplacement FAILED", __func__);
            res = false;
        }
    }

    return res;
}

bool UltraManEvcp::flushStatus(const EvcpParam* param) {
    AutoMutex lock(mCritMutexEccObject);

    if (mEccEnabled != param->evcpEccEnabled) {
        bool res = setEvcpEnable(param->evcpEccEnabled);
        CheckAndLogError(res != true, false, "%s: SetEvcpEnable fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    if (mLowPowerMode != param->evcpPowerSave) {
        bool res = setEvcpPowerSavingMode(param->evcpPowerSave);
        CheckAndLogError(res != true, false, "%s: SetEvcpPowerSavingMode fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    if (mMdfTurbo != param->evcpEnableTurbo) {
        bool res = setEvcpSetMdfTurbo(param->evcpEnableTurbo);
        CheckAndLogError(res != true, false, "%s: SetEvcpSetMdfTurbo fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    if (mEvcpBGConcelment != param->evcpBGConcealment ||
        (mEvcpBGConcelment && param->evcpBGReplacement != mEvcpBGReplacement) ||
        ((mEvcpBGConcelment && param->evcpBGReplacement == mEvcpBGReplacement &&
          mEvcpBackgroundURL != param->evcpBGFileName))) {
        bool res = setEvcpConcelment(param->evcpBGConcealment, param->evcpBGReplacement,
                                     param->evcpBGFileName);
        CheckAndLogError(res != true, false, "%s: SetEvcpConcelment fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    if (mFaceBea != param->evcpFaceBea) {
        bool res = setEvcpFaceBea(param->evcpFaceBea);
        CheckAndLogError(res != true, false, "%s: SetFaceBea fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    if (mFaceFra != param->evcpFaceFra) {
        bool res = setEvcpFaceFra(param->evcpFaceFra);
        CheckAndLogError(res != true, false, "%s: SetFaceFra fails, ret %d", __func__,
                         static_cast<bool>(res));
    }

    return true;
}

CmDevice* UltraManEvcp::getSystemCMDevice() {
    unsigned int version = 0;

    if (CreateCmDevice(mDevice, version) != CM_SUCCESS) {
        LOGE("%s CreateCmDevice Failed", __func__);
        return nullptr;
    }

    return mDevice;
}

bool UltraManEvcp::createCMSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                                       void* sysMem, CmSurface2DUP*& surface) {
    if (mDevice->CreateSurface2DUP(width, height, format, sysMem, surface) != CM_SUCCESS) {
        LOGE("%s CreateSurface2DUP FAILED", __func__);
        return false;
    }

    return true;
}

bool UltraManEvcp::destroyCMSurface2DUP(CmSurface2DUP*& surface) {
    if (mDevice->DestroySurface2DUP(surface) != CM_SUCCESS) {
        LOGE("%s destroyCMSurface2DUP FAILED", __func__);
        return false;
    }

    return true;
}

}  // namespace icamera
