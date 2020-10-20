/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#define LOG_TAG "IntelAlgoCommon"

#include "modules/sandboxing/client/IntelAlgoCommon.h"

#include <string>
#include <vector>

#include "CameraLog.h"
#include "PlatformData.h"

namespace icamera {
IntelAlgoCommon::IntelAlgoCommon() {
    LOGIPC("@%s", __func__);

    mClient = IntelAlgoClient::getInstance();
    LOGIPC("@%s, mClient:%p", __func__, mClient);
}

IntelAlgoCommon::~IntelAlgoCommon() {
    LOGIPC("@%s", __func__);
}

bool IntelAlgoCommon::allocShmMem(const std::string& name, int size, ShmMemInfo* shm,
                                  ShmMemUsage usage) {
    LOGIPC("@%s", __func__);
    CheckError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    shm->mName = name;
    shm->mSize = size;
    int ret = mClient->allocateShmMem(shm->mName, shm->mSize, &shm->mFd, &shm->mAddr);
    CheckError((ret != OK), false, "@%s, call allocateShmMem fail", __func__);

    shm->mHandle = mClient->registerBuffer(shm->mFd, shm->mAddr, usage);
    if (shm->mHandle < 0) {
        LOGE("@%s, call mBridge->RegisterBuffer fail", __func__);
        mClient->releaseShmMem(shm->mName, shm->mSize, shm->mFd, shm->mAddr);
        return false;
    }

    return true;
}

int32_t IntelAlgoCommon::registerGbmBuffer(int bufferFd) {
    LOGIPC("@%s, bufferFd:%d", __func__, bufferFd);
    CheckError(mClient == nullptr, -1, "@%s, mClient is nullptr", __func__);

    return mClient->registerGbmBuffer(bufferFd);
}

void IntelAlgoCommon::deregisterGbmBuffer(int32_t bufferHandle) {
    LOGIPC("@%s, bufferHandle:%d", __func__, bufferHandle);
    CheckError(mClient == nullptr, VOID_VALUE, "@%s, mClient is nullptr", __func__);

    mClient->deregisterGbmBuffer(bufferHandle);
}

bool IntelAlgoCommon::requestSync(IPC_CMD cmd, int32_t handle) {
    LOGIPC("@%s", __func__);
    CheckError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    return mClient->requestSync(cmd, handle) == OK ? true : false;
}

bool IntelAlgoCommon::requestSync(IPC_CMD cmd) {
    LOGIPC("@%s", __func__);
    CheckError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    return mClient->requestSync(cmd) == OK ? true : false;
}

void IntelAlgoCommon::freeShmMem(const ShmMemInfo& shm, ShmMemUsage usage) {
    LOGIPC("@%s, mHandle:%d, mFd:%d, mName:%s, mSize:%d, mAddr:%p", __func__, shm.mHandle, shm.mFd,
           shm.mName.c_str(), shm.mSize, shm.mAddr);
    CheckError(mClient == nullptr, VOID_VALUE, "@%s, mClient is nullptr", __func__);
    if (shm.mHandle < 0 || shm.mFd < 0) {
        LOGE("@%s, mHandle:%d, mFd:%d, one of them < 0", __func__, shm.mHandle, shm.mFd);
        return;
    }

    mClient->deregisterBuffer(shm.mHandle, usage);
    mClient->releaseShmMem(shm.mName, shm.mSize, shm.mFd, shm.mAddr);
}

bool IntelAlgoCommon::allocateAllShmMems(std::vector<ShmMem>* mems) {
    LOGIPC("@%s", __func__);

    for (auto& it : *mems) {
        ShmMemInfo* mem = it.mem;
        mem->mName = it.name;
        mem->mSize = it.size;
        bool ret = allocShmMem(mem->mName, mem->mSize, mem);
        CheckError(!ret, false, "@%s, allocShmMem fails, name:%s, size:%d", __func__,
                   mem->mName.c_str(), mem->mSize);
        it.allocated = true;
    }

    return true;
}

void IntelAlgoCommon::releaseAllShmMems(const std::vector<ShmMem>& mems) {
    LOGIPC("@%s", __func__);

    for (auto& it : mems) {
        if (it.allocated) {
            freeShmMem(*it.mem);
        }
    }
}

int32_t IntelAlgoCommon::getShmMemHandle(void* addr, ShmMemUsage usage) {
    CheckError(mClient == nullptr, -1, "@%s, mClient is nullptr", __func__);
    return mClient->getBufferHandle(addr, usage);
}

} /* namespace icamera */
