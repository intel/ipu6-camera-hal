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

#define LOG_TAG EvcpManager

#include "src/evcp/EvcpManager.h"

#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>

#include "PlatformData.h"
#include "core/CameraBuffer.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
std::unordered_map<int, EvcpManager*> EvcpManager::sInstances;
std::unordered_map<int, EvcpParam> EvcpManager::mLatestParam;
Mutex EvcpManager::sLock;
Mutex EvcpManager::sParamLock;

EvcpManager* EvcpManager::getInstance(int cameraId) {
    AutoMutex lock(sLock);

    if (sInstances.find(cameraId) == sInstances.end()) {
        return nullptr;
    }

    return sInstances[cameraId];
}

bool EvcpManager::createInstance(int cameraId, int width, int height) {
    AutoMutex lock(sLock);
    if (sInstances.find(cameraId) == sInstances.end()) {
        EvcpManager* thr = new EvcpManager(cameraId, width, height, nullptr);
        bool r = thr->init();
        if (!r) {
            delete thr;
            return false;
        }
        sInstances[cameraId] = thr;
    }

    return true;
}

void EvcpManager::destoryInstance(int cameraId) {
    AutoMutex lock(sLock);

    if (sInstances.find(cameraId) != sInstances.end()) {
        delete sInstances[cameraId];
        sInstances.erase(cameraId);
    }
}

EvcpManager::EvcpManager(int cameraId, int width, int height, EvcpParam* evcpParam)
        : mCameraId(cameraId),
          mWidth(width),
          mHeight(height) {}

bool EvcpManager::init() {
    mEvcp = std::unique_ptr<IntelEvcp>(new IntelEvcp());
    int ret = mEvcp->init(mWidth, mHeight);
    CheckAndLogError(ret != OK, false, "$%s: mEvcp init fails, ret %d", __func__, ret);

    return mEvcp->updateEvcpParam(&mLatestParam[mCameraId]);
}

bool EvcpManager::checkingStatus() {
    AutoMutex lock(sParamLock);

    if (!mLatestParam[mCameraId].evcpEccEnabled && !mLatestParam[mCameraId].evcpBGConcealment &&
        !mLatestParam[mCameraId].evcpFaceBea && !mLatestParam[mCameraId].evcpFaceFra) {
        return false;
    }

    return true;
}

bool EvcpManager::saveNV12Img(int fd, int width, int height, char* filename) {
    std::string fn;

    struct stat sb;
    if (::fstat(fd, &sb)) return false;

    int filesize = sb.st_size;

    void* p = ::mmap(nullptr, filesize, PROT_READ, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED) return false;

    fn = "/run/camera/Babun_";
    fn += std::to_string(width);
    fn += "x";
    fn += std::to_string(height);
    fn += ".NV12";

    std::fstream ofs;
    ofs.open(fn, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        ::munmap(p, filesize);
        return false;
    }

    ofs.write(reinterpret_cast<char*>(p), filesize);

    ::munmap(p, filesize);
    ::strncpy(filename, fn.c_str(), FNLEN);

    return true;
}

EvcpParam EvcpManager::getParamFromExp(const icamera::Parameters* param) {
    EvcpParam earam = {};

    uint8_t eccEnable, bcMode, ffMode;
    int w, h, bgFD;

    param->getEvcpEccMode(&eccEnable);
    param->getEvcpBCMode(&bcMode);
    param->getEvcpBRParameters(&w, &h, &bgFD);
    param->getEvcpFFMode(&ffMode);

    earam.evcpEccEnabled = eccEnable;
    if (bcMode == INTEL_VENDOR_CAMERA_IC_BC_MODE_BLUR) {
        earam.evcpBGConcealment = true;
    } else if (bcMode == INTEL_VENDOR_CAMERA_IC_BC_MODE_REPLACEMENT) {
        earam.evcpBGConcealment = true;
        earam.evcpBGReplacement = true;

        if (bgFD > 0) {
            earam.evcpBGBufferHandle = bgFD;
            if (saveNV12Img(bgFD, w, h, earam.evcpBGFileName) == false) {
                earam.evcpBGConcealment = false;
                earam.evcpBGReplacement = false;
                earam.evcpBGBufferHandle = -1;
                earam.evcpBGFileName[0] = '\0';
            }
        }
    }
    earam.evcpFaceFra = ffMode == INTEL_VENDOR_CAMERA_IC_FF_MODE_AUTO;

    return earam;
}

void EvcpManager::prepare4Param(icamera::Parameters* param) {
    EvcpParam curParam = getEvcpParam();
    EvcpParam expParam = getParamFromExp(param);

    if (!evcpParamCmp(&curParam, &expParam)) updateEvcpParam(expParam);
}

void EvcpManager::runEvcp(const camera_buffer_t& buffer, icamera::Parameters* param) {
    prepare4Param(param);
    if (checkingStatus() == false) return;

    runEvcpL(buffer);
}

void EvcpManager::runEvcpL(const camera_buffer_t& buffer) {
    int size = buffer.s.size;

    nsecs_t startTime = CameraUtils::systemTime();

#ifdef ENABLE_SANDBOXING
    bool ret = mEvcp->runEvcpFrame(buffer.dmafd, size);
#else
    void* pBuf = (buffer.s.memType == V4L2_MEMORY_DMABUF) ?
                     CameraBuffer::mapDmaBufferAddr(buffer.dmafd, size) :
                     buffer.addr;

    bool ret = mEvcp->runEvcpFrame(pBuf, size);

    if (buffer.s.memType == V4L2_MEMORY_DMABUF) {
        CameraBuffer::unmapDmaBufferAddr(pBuf, size);
    }
#endif

    if (ret == false) {
        LOGW("@%s Evcp run frame fails", __func__);
    }

    LOG2("@%s: ret:%s, Evcp takes %ums", __func__, ret ? "true" : "false",
         (unsigned)((CameraUtils::systemTime() - startTime) / 1000000));
}

bool EvcpManager::updateEvcpParam(EvcpParam evcpParam) {
    if (mEvcp->updateEvcpParam(&evcpParam)) {
        AutoMutex lock(sParamLock);
        mLatestParam[mCameraId] = evcpParam;

        return true;
    }

    return false;
}

EvcpParam EvcpManager::getEvcpParam() const {
    AutoMutex lock(sParamLock);

    return mLatestParam[mCameraId];
}

}  // namespace icamera
