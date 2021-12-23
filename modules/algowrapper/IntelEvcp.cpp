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

#define LOG_TAG IntelEvcp

#include "modules/algowrapper/IntelEvcp.h"

#include <unistd.h>

#include <memory>

#include "igfxcmrt/cm_rt.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelEvcp::IntelEvcp() : mWidth(0), mHeight(0) {}

int IntelEvcp::init(int width, int height) {
    mWidth = width;
    mHeight = height;
    mEvcpDLL = std::unique_ptr<UltraManEvcp>(new UltraManEvcp(width, height));

    return mEvcpDLL->init() ? OK : UNKNOWN_ERROR;
}

bool IntelEvcp::runEvcpFrame(void* inBufAddr, int size) {
    auto flushCacheBlock = [](void* start, size_t sz) {
        const int CACHE_LINE_SIZE_FOR_ADL = 64;
        const int CACHE_LINE_MASK_FOR_ADL = CACHE_LINE_SIZE_FOR_ADL - 1;

        char* p = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(start) &
                                          ~CACHE_LINE_MASK_FOR_ADL);
        char* end = reinterpret_cast<char*>(start) + sz;

        asm volatile("mfence" :::"memory");
        while (p < end) {
            asm volatile("clflush (%0)" ::"r"(p));
            p += CACHE_LINE_SIZE_FOR_ADL;
        }
        asm volatile("mfence" :::"memory");
        return true;
    };

    return runEvcpFrameNoncopy(inBufAddr) && flushCacheBlock(inBufAddr, size);
}

bool IntelEvcp::runEvcpFrameNoncopy(void* inBufAddr) {
    CmSurface2DUP* inSurface = createCMSurface(reinterpret_cast<unsigned char*>(inBufAddr));

    bool r = mEvcpDLL->processFrame(inSurface);

    return destroyCMSurface(inSurface) && r;
}

CmSurface2DUP* IntelEvcp::createCMSurface(void* bufAddr) {
    CmSurface2DUP* cmSurface = nullptr;

    mEvcpDLL->createCMSurface2DUP(mWidth, mHeight, CM_SURFACE_FORMAT_NV12, bufAddr, cmSurface);

    return cmSurface;
}

bool IntelEvcp::destroyCMSurface(CmSurface2DUP* surface) {
    return mEvcpDLL->destroyCMSurface2DUP(surface);
}

bool IntelEvcp::updateEvcpParam(const EvcpParam* evcpParam) {
    return mEvcpDLL->updateEvcpInfo(evcpParam);
}

void IntelEvcp::getEvcpParam(EvcpParam* evcpParam) const {
    evcpParam->evcpEccEnabled = mEvcpDLL->isEccEnabled();
    evcpParam->evcpBdEnabled = mEvcpDLL->isDimmerEnabled();
    evcpParam->evcpEnableTurbo = mEvcpDLL->isTurboOn();
    evcpParam->evcpPowerSave = mEvcpDLL->isLowPowerMode();
    evcpParam->evcpBGConcealment = mEvcpDLL->isBGConcelment();
    evcpParam->evcpBGReplacement = mEvcpDLL->isBGReplacement();
    evcpParam->evcpFaceBea = mEvcpDLL->isFaceBeaEnable();
    evcpParam->evcpFaceFra = mEvcpDLL->isFaceFraEnable();
}

}  // namespace icamera
