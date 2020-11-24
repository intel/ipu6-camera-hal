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

#define LOG_TAG "IntelAlgoClient"

#include "modules/sandboxing/client/IntelAlgoClient.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <vector>

#include "Parameters.h"
#include "PlatformData.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

IntelAlgoClient* IntelAlgoClient::sInstance = nullptr;
Mutex IntelAlgoClient::sLock;

IntelAlgoClient* IntelAlgoClient::getInstance() {
    AutoMutex lock(sLock);

    if (!sInstance) {
        sInstance = new IntelAlgoClient;
    }

    return sInstance;
}

void IntelAlgoClient::releaseInstance() {
    AutoMutex lock(sLock);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

IntelAlgoClient::IntelAlgoClient()
        : mErrCb(nullptr),
          mGpuBridge(nullptr),
          mIPCStatus(true),
          mMojoManager(nullptr),
          mInitialized(false) {
    LOGIPC("@%s", __func__);
}

IntelAlgoClient::~IntelAlgoClient() {
    LOGIPC("@%s", __func__);
}

int IntelAlgoClient::initialize() {
    LOGIPC("@%s, mMojoManager: %p", __func__, mMojoManager);
    CheckError(!mMojoManager, UNKNOWN_ERROR, "@%s, mMojoManager is nullptr", __func__);

    mCallback = base::Bind(&IntelAlgoClient::callbackHandler, base::Unretained(this));
    IntelAlgoClient::return_callback = returnCallback;

    mNotifyCallback = base::Bind(&IntelAlgoClient::notifyHandler, base::Unretained(this));
    IntelAlgoClient::notify = notifyCallback;

    mBridge = cros::CameraAlgorithmBridge::CreateInstance(cros::CameraAlgorithmBackend::kVendorCpu,
                                                          mMojoManager);
    CheckError(!mBridge, UNKNOWN_ERROR, "@%s, mBridge is nullptr", __func__);
    CheckError(mBridge->Initialize(this) != 0, UNKNOWN_ERROR, "@%s, mBridge init fails", __func__);

    if (PlatformData::isUsingGpuAlgo()) {
        LOGIPC("@%s GPU algo enabled", __func__);
        mGpuBridge = cros::CameraAlgorithmBridge::CreateInstance(
            cros::CameraAlgorithmBackend::kVendorGpu, mMojoManager);
        CheckError(!mGpuBridge, UNKNOWN_ERROR, "@%s, mGpuBridge is nullptr", __func__);
        CheckError(mGpuBridge->Initialize(this) != 0, UNKNOWN_ERROR, "@%s, mGpuBridge init fails",
                   __func__);
    }

    for (int i = 0; i < IPC_GROUP_NUM; i++) {
        if (static_cast<IPC_GROUP>(i) != IPC_GROUP_GPU) {
            mRunner[i] =
                std::unique_ptr<Runner>(new Runner(static_cast<IPC_GROUP>(i), mBridge.get()));
        } else if (mGpuBridge) {
            mRunner[i] = std::unique_ptr<Runner>(new Runner(IPC_GROUP_GPU, mGpuBridge.get()));
        }
    }

    mIPCStatus = true;
    mInitialized = true;

    return OK;
}

bool IntelAlgoClient::isIPCFine() {
    std::lock_guard<std::mutex> l(mIPCStatusMutex);
    LOGIPC("@%s, mIPCStatus:%d", __func__, mIPCStatus);

    return mIPCStatus;
}

void IntelAlgoClient::registerErrorCallback(const camera_callback_ops_t* errCb) {
    LOGIPC("@%s, errCb:%p", __func__, errCb);

    std::lock_guard<std::mutex> l(mIPCStatusMutex);
    mErrCb = errCb;

    if (!mIPCStatus && mErrCb) {
        camera_msg_data_t data = {CAMERA_IPC_ERROR, {0}};
        mErrCb->notify(mErrCb, data);
    }
}

int IntelAlgoClient::allocateShmMem(const std::string& name, int size, int* fd, void** addr) {
    LOGIPC("@%s, name:%s, size:%d", __func__, name.c_str(), size);

    *fd = -1;
    *addr = nullptr;
    int shmFd = -1;
    void* shmAddr = nullptr;

    shmFd = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    CheckError((shmFd == -1), UNKNOWN_ERROR, "@%s, call shm_open fail", __func__);

    do {
        int ret = fcntl(shmFd, F_GETFD);
        if (ret == -1) {
            LOGE("@%s, call fcntl fail, error %s", __func__, strerror(errno));
            break;
        }

        ret = ftruncate(shmFd, size);
        if (ret == -1) {
            LOGE("@%s, call ftruncate fail, error %s", __func__, strerror(errno));
            break;
        }

        struct stat sb;
        ret = fstat(shmFd, &sb);
        if (ret == -1) {
            LOGE("@%s, call fstat fail, error %s", __func__, strerror(errno));
            break;
        }

        shmAddr = mmap(0, sb.st_size, PROT_WRITE, MAP_SHARED, shmFd, 0);
        if (!shmAddr) {
            LOGE("@%s, call mmap fail, error %s", __func__, strerror(errno));
            break;
        }

        *fd = shmFd;
        *addr = shmAddr;

        return OK;
    } while (0);

    close(shmFd);
    return UNKNOWN_ERROR;
}

void IntelAlgoClient::releaseShmMem(const std::string& name, int size, int fd, void* addr) {
    LOGIPC("@%s, name:%s, size:%d, fd:%d, addr:%p", __func__, name.c_str(), size, fd, addr);

    munmap(addr, size);
    close(fd);
    shm_unlink(name.c_str());
}

int IntelAlgoClient::requestSync(IPC_CMD cmd, int32_t bufferHandle) {
    LOGIPC("@%s, cmd:%d:%s, bufferHandle:%d, mInitialized:%d", __func__, cmd,
           IntelAlgoIpcCmdToString(cmd), bufferHandle, mInitialized);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false", __func__);
    CheckError(!isIPCFine(), UNKNOWN_ERROR, "@%s, IPC error happens", __func__);

    IPC_GROUP group = IntelAlgoIpcCmdToGroup(cmd);

    return mRunner[group]->requestSync(cmd, bufferHandle);
}

int IntelAlgoClient::requestSync(IPC_CMD cmd) {
    return requestSync(cmd, -1);
}

int32_t IntelAlgoClient::registerBuffer(int bufferFd, void* addr, ShmMemUsage usage) {
    LOGIPC("@%s, bufferFd: %d, mInitialized: %d, addr: %p, usage: %d", __func__, bufferFd,
           mInitialized, addr, usage);
    CheckError(!mInitialized, -1, "@%s, mInitialized is false", __func__);
    CheckError(usage >= MAX_ALGO_SHM, -1, "@%s, usage: %d isn't supported", __func__, usage);
    CheckError(!isIPCFine(), -1, "@%s, IPC error happens", __func__);

    int32_t handle = -1;
    if (usage == CPU_ALGO_SHM) {
        handle = mBridge->RegisterBuffer(bufferFd);
    } else if (mGpuBridge) {
        handle = mGpuBridge->RegisterBuffer(bufferFd);
    }
    if (handle >= 0) {
        std::lock_guard<std::mutex> l(mShmMapMutex);
        mShmMap[usage][addr] = handle;
    }

    return handle;
}

void IntelAlgoClient::deregisterBuffer(int32_t bufferHandle, ShmMemUsage usage) {
    LOGIPC("@%s, bufferHandle: %d, mInitialized: %d, usage: %d", __func__, bufferHandle,
           mInitialized, usage);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(usage >= MAX_ALGO_SHM, VOID_VALUE, "@%s, usage: %d isn't supported", __func__,
               usage);
    CheckError(!isIPCFine(), VOID_VALUE, "@%s, IPC error happens", __func__);

    {
        std::lock_guard<std::mutex> l(mShmMapMutex);
        for (auto& item : mShmMap[usage]) {
            if (item.second == bufferHandle) {
                mShmMap[usage].erase(item.first);
                break;
            }
        }
    }

    std::vector<int32_t> handles({bufferHandle});
    if (usage == CPU_ALGO_SHM) {
        mBridge->DeregisterBuffers(handles);
    } else if (mGpuBridge) {
        mGpuBridge->DeregisterBuffers(handles);
    }
}

int32_t IntelAlgoClient::registerGbmBuffer(int bufferFd, ShmMemUsage usage) {
    LOGIPC("@%s, bufferFd:%d, mInitialized:%d", __func__, bufferFd, mInitialized);
    CheckError(!mInitialized, -1, "@%s, mInitialized is false", __func__);
    CheckError(!isIPCFine(), -1, "@%s, IPC error happens", __func__);

    if (usage == CPU_ALGO_SHM) {
        return mBridge->RegisterBuffer(bufferFd);
    } else if (mGpuBridge) {
        return mGpuBridge->RegisterBuffer(bufferFd);
    }
    return OK;
}

void IntelAlgoClient::deregisterGbmBuffer(int32_t bufferHandle, ShmMemUsage usage) {
    LOGIPC("@%s, bufferHandle:%d, mInitialized:%d", __func__, bufferHandle, mInitialized);
    CheckError(!mInitialized, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(!isIPCFine(), VOID_VALUE, "@%s, IPC error happens", __func__);

    std::vector<int32_t> handles({bufferHandle});
    if (usage == CPU_ALGO_SHM) {
        mBridge->DeregisterBuffers(handles);
    } else if (mGpuBridge) {
        mGpuBridge->DeregisterBuffers(handles);
    }
}

int32_t IntelAlgoClient::getBufferHandle(void* addr, ShmMemUsage usage) {
    CheckError(!mInitialized, -1, "@%s, mInitialized is false", __func__);
    CheckError(usage >= MAX_ALGO_SHM, -1, "@%s, usage: %d isn't supported", __func__, usage);
    if (!addr) return -1;

    LOGIPC("%s, the buffer addr: %p, usage: %d", __func__, addr, usage);
    std::lock_guard<std::mutex> l(mShmMapMutex);
    CheckError(mShmMap[usage].find(addr) == mShmMap[usage].end(), -1,
               "%s, Invalid client addr: %p, usage: %d", __func__, addr, usage);

    return mShmMap[usage][addr];
}

void IntelAlgoClient::callbackHandler(uint32_t req_id, uint32_t status, int32_t buffer_handle) {
    LOGIPC("@%s, req_id:%d, status:%d, buffer_handle:%d", __func__, req_id, status, buffer_handle);

    IPC_GROUP group = IntelAlgoIpcCmdToGroup(static_cast<IPC_CMD>(req_id));
    mRunner[group]->callbackHandler(status, buffer_handle);
}

void IntelAlgoClient::notifyHandler(uint32_t msg) {
    LOGIPC("@%s, msg:%d", __func__, msg);

    if (msg != CAMERA_ALGORITHM_MSG_IPC_ERROR) {
        LOGE("@%s, receive msg:%d, not CAMERA_ALGORITHM_MSG_IPC_ERROR", __func__, msg);
        return;
    }

    std::lock_guard<std::mutex> l(mIPCStatusMutex);
    mIPCStatus = false;

    if (mErrCb) {
        camera_msg_data_t data = {CAMERA_IPC_ERROR, {0}};
        mErrCb->notify(mErrCb, data);
    } else {
        LOGE("@%s, mErrCb is nullptr, no device error is sent out", __func__);
    }
    LOGE("@%s, receive CAMERA_ALGORITHM_MSG_IPC_ERROR", __func__);
}

void IntelAlgoClient::returnCallback(const camera_algorithm_callback_ops_t* callback_ops,
                                     uint32_t req_id, uint32_t status, int32_t buffer_handle) {
    LOGIPC("@%s", __func__);
    CheckError(!callback_ops, VOID_VALUE, "@%s, callback_ops is nullptr", __func__);

    auto s = const_cast<IntelAlgoClient*>(static_cast<const IntelAlgoClient*>(callback_ops));
    s->callbackHandler(req_id, status, buffer_handle);
}

void IntelAlgoClient::notifyCallback(const struct camera_algorithm_callback_ops* callback_ops,
                                     camera_algorithm_error_msg_code_t msg) {
    LOGIPC("@%s", __func__);
    CheckError(!callback_ops, VOID_VALUE, "@%s, callback_ops is nullptr", __func__);

    auto s = const_cast<IntelAlgoClient*>(static_cast<const IntelAlgoClient*>(callback_ops));
    s->notifyHandler((uint32_t)msg);
}

IntelAlgoClient::Runner::Runner(IPC_GROUP group, cros::CameraAlgorithmBridge* bridge)
        : mGroup(group),
          mBridge(bridge),
          mIsCallbacked(false),
          mCbResult(true),
          mInitialized(false) {
    LOGIPC("@%s, group:%d", __func__, mGroup);

    pthread_condattr_t attr;
    int ret = pthread_condattr_init(&attr);
    if (ret != 0) {
        LOGE("@%s, call pthread_condattr_init fails, ret:%d", __func__, ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (ret != 0) {
        LOGE("@%s, call pthread_condattr_setclock fails, ret:%d", __func__, ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    ret = pthread_cond_init(&mCbCond, &attr);
    if (ret != 0) {
        LOGE("@%s, call pthread_cond_init fails, ret:%d", __func__, ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    pthread_condattr_destroy(&attr);

    ret = pthread_mutex_init(&mCbLock, nullptr);
    CheckError(ret != 0, VOID_VALUE, "@%s, call pthread_mutex_init fails, ret:%d", __func__, ret);

    mInitialized = true;
}

IntelAlgoClient::Runner::~Runner() {
    LOGIPC("@%s, group:%d", __func__, mGroup);

    int ret = pthread_cond_destroy(&mCbCond);
    if (ret != 0) {
        LOGE("@%s, call pthread_cond_destroy fails, ret:%d", __func__, ret);
    }

    ret = pthread_mutex_destroy(&mCbLock);
    if (ret != 0) {
        LOGE("@%s, call pthread_mutex_destroy fails, ret:%d", __func__, ret);
    }
}

int IntelAlgoClient::Runner::requestSync(IPC_CMD cmd, int32_t bufferHandle) {
    LOGIPC("@%s, cmd:%d:%s, group:%d, bufferHandle:%d, mInitialized:%d", __func__, cmd,
           IntelAlgoIpcCmdToString(cmd), mGroup, bufferHandle, mInitialized);
    CheckError(!mInitialized, UNKNOWN_ERROR, "@%s, mInitialized is false, cmd:%d:%s", __func__, cmd,
               IntelAlgoIpcCmdToString(cmd));

    std::lock_guard<std::mutex> lck(mMutex);

    std::vector<uint8_t> reqHeader(IPC_REQUEST_HEADER_USED_NUM);
    reqHeader[0] = IPC_MATCHING_KEY;

    // cmd is for request id, no duplicate command will be issued at any given time.
    mBridge->Request(cmd, reqHeader, bufferHandle);
    int ret = waitCallback();
    CheckError((ret != OK), UNKNOWN_ERROR, "@%s, waitCallback fails, cmd:%d:%s", __func__, cmd,
               IntelAlgoIpcCmdToString(cmd));

    LOGIPC("@%s, cmd:%d:%s, group:%d, mCbResult:%d, done!", __func__, cmd,
           IntelAlgoIpcCmdToString(cmd), mGroup, mCbResult);

    // check callback result
    CheckError((mCbResult != true), UNKNOWN_ERROR, "@%s, callback fails, cmd:%d:%s", __func__, cmd,
               IntelAlgoIpcCmdToString(cmd));

    return OK;
}

void IntelAlgoClient::Runner::callbackHandler(uint32_t status, int32_t buffer_handle) {
    LOGIPC("@%s, group:%d, status:%d, buffer_handle:%d", __func__, mGroup, status, buffer_handle);
    if (status != 0) {
        LOGE("@%s, group:%d, status:%d, buffer_handle:%d", __func__, mGroup, status, buffer_handle);
    }
    mCbResult = status != 0 ? false : true;

    pthread_mutex_lock(&mCbLock);
    mIsCallbacked = true;
    int ret = pthread_cond_signal(&mCbCond);
    pthread_mutex_unlock(&mCbLock);

    CheckError(ret != 0, VOID_VALUE, "@%s, group:%d, call pthread_cond_signal fails, ret:%d",
               __func__, mGroup, ret);
}

int IntelAlgoClient::Runner::waitCallback() {
    LOGIPC("@%s, group:%d", __func__, mGroup);

    nsecs_t startTime = CameraUtils::systemTime();

    pthread_mutex_lock(&mCbLock);
    if (!mIsCallbacked) {
        int ret = 0;
        struct timespec ts = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += 5;  // 5s timeout

        while (!mIsCallbacked && !ret) {
            ret = pthread_cond_timedwait(&mCbCond, &mCbLock, &ts);
        }
        if (ret != 0) {
            LOGE("@%s, group:%d, call pthread_cond_timedwait fail, ret:%d, it takes %" PRId64 "ms",
                 __func__, mGroup, ret, (CameraUtils::systemTime() - startTime) / 1000000);
            pthread_mutex_unlock(&mCbLock);
            return UNKNOWN_ERROR;
        }
    }
    mIsCallbacked = false;
    pthread_mutex_unlock(&mCbLock);

    LOGIPC("@%s: group:%d, it takes %" PRId64 "ms", __func__, mGroup,
           (CameraUtils::systemTime() - startTime) / 1000000);

    return OK;
}

} /* namespace icamera */
