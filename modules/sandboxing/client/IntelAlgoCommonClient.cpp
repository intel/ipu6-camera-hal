/*
 * Copyright (C) 2019-2021 Intel Corporation.
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

#define LOG_TAG IntelAlgoCommonClient

#include "modules/sandboxing/client/IntelAlgoCommonClient.h"

#include <string>
#include <vector>

#include "CameraLog.h"
#include "PlatformData.h"

namespace icamera {
IntelAlgoCommon::IntelAlgoCommon() {
    mClient = IntelAlgoClient::getInstance();
    LOG1("@%s, Construct, mClient:%p", __func__, mClient);
}

IntelAlgoCommon::~IntelAlgoCommon() {
    LOG1("@%s Destroy", __func__);
}

bool IntelAlgoCommon::allocShmMem(const std::string& name, int size, ShmMemInfo* shm,
                                  ShmMemUsage usage) {
    CheckAndLogError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    shm->mName = name;
    shm->mSize = size;
    int ret = mClient->allocateShmMem(shm->mName, shm->mSize, &shm->mFd, &shm->mAddr);
    CheckAndLogError((ret != OK), false, "@%s, call allocateShmMem fail", __func__);

    shm->mHandle = mClient->registerBuffer(shm->mFd, shm->mAddr, usage);
    if (shm->mHandle < 0) {
        LOGE("@%s, call mBridge->RegisterBuffer fail", __func__);
        mClient->releaseShmMem(shm->mName, shm->mSize, shm->mFd, shm->mAddr);
        return false;
    }

    return true;
}

int32_t IntelAlgoCommon::registerGbmBuffer(int bufferFd, ShmMemUsage usage) {
    CheckAndLogError(mClient == nullptr, -1, "@%s, mClient is nullptr", __func__);

    return mClient->registerGbmBuffer(bufferFd, usage);
}

void IntelAlgoCommon::deregisterGbmBuffer(int32_t bufferHandle, ShmMemUsage usage) {
    CheckAndLogError(mClient == nullptr, VOID_VALUE, "@%s, mClient is nullptr", __func__);

    mClient->deregisterGbmBuffer(bufferHandle, usage);
}

bool IntelAlgoCommon::requestSync(IPC_CMD cmd, int32_t handle) {
    CheckAndLogError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    return mClient->requestSync(cmd, handle) == OK ? true : false;
}

bool IntelAlgoCommon::requestSync(IPC_CMD cmd) {
    CheckAndLogError(mClient == nullptr, false, "@%s, mClient is nullptr", __func__);

    return mClient->requestSync(cmd) == OK ? true : false;
}

ia_err IntelAlgoCommon::requestSyncCca(IPC_CMD cmd, int32_t handle) {
    CheckAndLogError(mClient == nullptr, ia_err_argument, "@%s, mClient is nullptr", __func__);

    return (ia_err)(mClient->requestSync(cmd, handle));
}

ia_err IntelAlgoCommon::requestSyncCca(IPC_CMD cmd) {
    CheckAndLogError(mClient == nullptr, ia_err_argument, "@%s, mClient is nullptr", __func__);

    return (ia_err)(mClient->requestSync(cmd));
}

void IntelAlgoCommon::freeShmMem(const ShmMemInfo& shm, ShmMemUsage usage) {
    CheckAndLogError(mClient == nullptr, VOID_VALUE, "@%s, mClient is nullptr", __func__);
    if (shm.mHandle < 0 || shm.mFd < 0) {
        LOGE("@%s, mHandle:%d, mFd:%d, one of them < 0", __func__, shm.mHandle, shm.mFd);
        return;
    }

    mClient->deregisterBuffer(shm.mHandle, usage);
    mClient->releaseShmMem(shm.mName, shm.mSize, shm.mFd, shm.mAddr);
}

bool IntelAlgoCommon::allocateAllShmMems(std::vector<ShmMem>* mems) {
    for (auto& it : *mems) {
        ShmMemInfo* mem = it.mem;
        mem->mName = it.name;
        mem->mSize = it.size;
        bool ret = allocShmMem(mem->mName, mem->mSize, mem);
        CheckAndLogError(!ret, false, "@%s, allocShmMem fails, name:%s, size:%d", __func__,
                         mem->mName.c_str(), mem->mSize);
        it.allocated = true;
    }

    return true;
}

void IntelAlgoCommon::releaseAllShmMems(const std::vector<ShmMem>& mems) {
    for (auto& it : mems) {
        if (it.allocated) {
            freeShmMem(*it.mem);
        }
    }
}

int32_t IntelAlgoCommon::getShmMemHandle(void* addr, ShmMemUsage usage) {
    CheckAndLogError(mClient == nullptr, -1, "@%s, mClient is nullptr", __func__);
    return mClient->getBufferHandle(addr, usage);
}

} /* namespace icamera */
