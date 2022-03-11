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

#include <dlfcn.h>

#include <mutex>
#include <string>

#include "EvcpCommon.h"
#include "iutils/Utils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Werror"
// HANDLE is redefined in cm_rt.h, avoid the redefinition build error
#define HANDLE cancel_fw_pre_define
#include "igfxcmrt/cm_rt.h"

#pragma clang diagnostic pop

struct evcp_context;

struct EVCP_API_HANDLE {
#define _DEF_FUNC(ret, name, ...)          \
    typedef ret (*pFn##name)(__VA_ARGS__); \
    pFn##name name

    _DEF_FUNC(evcp_context*, EvcpCreate, int w, int h, void* hdl, int bit);
    _DEF_FUNC(void, EvcpDestroy, evcp_context* ctx);
    _DEF_FUNC(int, EvcpToggleEcc, evcp_context* ctx, int enable);
    _DEF_FUNC(int, EvcpSetMdfTurbo, evcp_context* ctx, bool enable);
    _DEF_FUNC(int, EvcpSetVerbose, evcp_context* ctx, int verbose);
    _DEF_FUNC(int, EvcpSetPowerSavingMode, evcp_context* ctx, bool enable);
    _DEF_FUNC(int, EvcpProcessFrame, evcp_context** ctx, CmSurface2DUP* src, CmSurface2DUP* dst);
    _DEF_FUNC(int, EvcpProcessFrame2D, evcp_context** ctx, CmSurface2D* src, CmSurface2D* dst);
    _DEF_FUNC(int, EvcpSetBackGroundURL, evcp_context* ctx, const char* path);
    _DEF_FUNC(int, EvcpToggleBackgroundConcelment, evcp_context* ctx, int enable);
    _DEF_FUNC(int, EvcpToggleBackgroundReplacement, evcp_context* ctx, int enable);
    _DEF_FUNC(int, EvcpSetFaceBea, evcp_context* ctx, int enable);
    _DEF_FUNC(int, EvcpSetFaceFra, evcp_context* ctx, int enable);
#undef _DEF_FUNC
};

namespace icamera {

class UltraManEvcp {
 public:
    UltraManEvcp(int width, int height);
    bool init();

    virtual ~UltraManEvcp();
    bool updateEvcpInfo(const EvcpParam* param);
    bool processFrame(CmSurface2DUP* pSample);

    bool isEccEnabled() const { return mEccEnabled; }
    bool isDimmerEnabled() const { return mDimmerEnabled; }
    bool isLowPowerMode() const { return mLowPowerMode; }
    bool isTurboOn() const { return mMdfTurbo; }
    bool isBGConcelment() const { return mEvcpBGConcelment; }
    bool isBGReplacement() const { return mEvcpBGReplacement; }
    bool isFaceBeaEnable() const { return mFaceBea; }
    bool isFaceFraEnable() const { return mFaceFra; }
    std::string getBGFilePath() const { return mEvcpBackgroundURL; }

    bool createCMSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                             void* sysMem, CmSurface2DUP*& surface);

    bool destroyCMSurface2DUP(CmSurface2DUP*& surface);

 private:
    bool flushStatus(const EvcpParam* param);
    bool checkEVCPBackgroundURL(const std::string& name) const;

    CmDevice* getSystemCMDevice();
    bool loadEvcpLibrary();

    bool setEvcpEnable(bool v);
    bool setEvcpPowerSavingMode(bool v);
    bool setEvcpSetMdfTurbo(bool v);
    bool setEvcpConcelment(bool con, bool replace, const char* filePath);
    bool setEvcpReplacement(bool replace, const char* filePath);
    bool isEvcpEnable() const;
    std::string mEvcpBackgroundURL;

    bool setEvcpFaceBea(bool v);
    bool setEvcpFaceFra(bool v);

    uint32_t mCurrWidth;
    uint32_t mCurrHeight;
    uint64_t mFrameCount;
    bool mEccEnabled;
    bool mDimmerEnabled;
    bool mLowPowerMode;
    bool mMdfTurbo;

    bool mEvcpBGConcelment;
    bool mEvcpBGReplacement;

    bool mFaceBea;
    bool mFaceFra;

    std::mutex mCritMutexEccObject;
    EVCP_API_HANDLE mApi;

    evcp_context* mCtx;
    CmDevice* mDevice;
    bool mInit;
    void* mHandleEvcpLib;

    DISALLOW_COPY_AND_ASSIGN(UltraManEvcp);
};
}  // namespace icamera
