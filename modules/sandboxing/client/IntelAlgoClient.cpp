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

#define LOG_TAG IntelAlgoClient

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
#include "modules/sandboxing/client/IntelCcaClient.h"

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

    icamera::IntelCca::releaseAllInstances();

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

IntelAlgoClient::IntelAlgoClient()
        : mErrCb(nullptr),
          mGpuBridge(nullptr),
          mIPCStatus(true),
          mMojoManagerToken(nullptr),
          mInitialized(false) {
    LOG1("%s, Construct", __func__);
}

IntelAlgoClient::~IntelAlgoClient() {
    LOG1("%s, Destroy", __func__);
}

int IntelAlgoClient::initialize() {
    LOG1("@%s, mMojoManagerToken: %p", __func__, mMojoManagerToken);
    CheckAndLogError(!mMojoManagerToken, UNKNOWN_ERROR, "@%s, mMojoManagerToken is nullptr",
                     __func__);

    mCallback = base::BindRepeating(&IntelAlgoClient::callbackHandler, base::Unretained(this));
    IntelAlgoClient::return_callback = returnCallback;

    mNotifyCallback = base::BindRepeating(&IntelAlgoClient::notifyHandler, base::Unretained(this));
    IntelAlgoClient::notify = notifyCallback;

    mBridge = cros::CameraAlgorithmBridge::CreateInstance(cros::CameraAlgorithmBackend::kVendorCpu,
                                                          mMojoManagerToken);
    CheckAndLogError(!mBridge, UNKNOWN_ERROR, "@%s, mBridge is nullptr", __func__);
    CheckAndLogError(mBridge->Initialize(this) != 0, UNKNOWN_ERROR, "@%s, mBridge init fails",
                     __func__);

    if (PlatformData::isUsingGpuAlgo()) {
        LOG1("GPU algo enabled");
        mGpuBridge = cros::CameraAlgorithmBridge::CreateInstance(
            cros::CameraAlgorithmBackend::kVendorGpu, mMojoManagerToken);
        CheckAndLogError(!mGpuBridge, UNKNOWN_ERROR, "mGpuBridge is nullptr");
        CheckAndLogError(mGpuBridge->Initialize(this) != 0, UNKNOWN_ERROR, "mGpuBridge init fails");
    }

    for (int i = 0; i < IPC_GROUP_NUM; i++) {
        if (static_cast<IPC_GROUP>(i) < IPC_GROUP_GPU) {
            mRunner[i] =
                std::unique_ptr<Runner>(new Runner(static_cast<IPC_GROUP>(i), mBridge.get()));
        } else if (mGpuBridge) {
            mRunner[i] =
                std::unique_ptr<Runner>(new Runner(static_cast<IPC_GROUP>(i), mGpuBridge.get()));
        }
    }

    mIPCStatus = true;
    mInitialized = true;

    return OK;
}

bool IntelAlgoClient::isIPCFine() {
    std::lock_guard<std::mutex> l(mIPCStatusMutex);

    return mIPCStatus;
}

void IntelAlgoClient::registerErrorCallback(const camera_callback_ops_t* errCb) {
    LOG1("@%s, errCb:%p", __func__, errCb);

    std::lock_guard<std::mutex> l(mIPCStatusMutex);
    mErrCb = errCb;

    if (!mIPCStatus && mErrCb) {
        camera_msg_data_t data = {CAMERA_IPC_ERROR, {}};
        mErrCb->notify(mErrCb, data);
    }
}

int IntelAlgoClient::allocateShmMem(const std::string& name, int size, int* fd, void** addr) {
    *fd = -1;
    *addr = nullptr;
    int shmFd = -1;
    void* shmAddr = nullptr;

    shmFd = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    CheckAndLogError((shmFd == -1), UNKNOWN_ERROR, "call shm_open fail");

    do {
        int ret = fcntl(shmFd, F_GETFD);
        if (ret == -1) {
            LOGE("call fcntl fail, error %s", strerror(errno));
            break;
        }

        ret = ftruncate(shmFd, size);
        if (ret == -1) {
            LOGE("call ftruncate fail, error %s", strerror(errno));
            break;
        }

        struct stat sb;
        ret = fstat(shmFd, &sb);
        if (ret == -1) {
            LOGE("call fstat fail, error %s", strerror(errno));
            break;
        }

        shmAddr = mmap(0, sb.st_size, PROT_WRITE, MAP_SHARED, shmFd, 0);
        if (!shmAddr) {
            LOGE("call mmap fail, error %s", strerror(errno));
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
    munmap(addr, size);
    close(fd);
    shm_unlink(name.c_str());
}

int IntelAlgoClient::requestSync(IPC_CMD cmd, int32_t bufferHandle) {
    LOG2("requestSync cmd:%d:%s, bufferHandle:%d, mInitialized:%d", cmd,
         IntelAlgoIpcCmdToString(cmd), bufferHandle, mInitialized);
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, " mInitialized is false");
    CheckAndLogError(!isIPCFine(), UNKNOWN_ERROR, "IPC error happens");

    IPC_GROUP group = IntelAlgoIpcCmdToGroup(cmd);

    return mRunner[group]->requestSync(cmd, bufferHandle);
}

int IntelAlgoClient::requestSync(IPC_CMD cmd) {
    return requestSync(cmd, -1);
}

int32_t IntelAlgoClient::registerBuffer(int bufferFd, void* addr, ShmMemUsage usage) {
    LOG2("%s bufferFd: %d, mInitialized: %d, addr: %p, usage: %d", __func__, bufferFd, mInitialized,
         addr, usage);
    CheckAndLogError(!mInitialized, -1, "mInitialized is false");
    CheckAndLogError(usage >= MAX_ALGO_SHM, -1, "usage: %d isn't supported", usage);
    CheckAndLogError(!isIPCFine(), -1, "IPC error happens");

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
    LOG2("%s, bufferHandle: %d, mInitialized: %d, usage: %d", __func__, bufferHandle, mInitialized,
         usage);
    CheckAndLogError(!mInitialized, VOID_VALUE, "mInitialized is false");
    CheckAndLogError(usage >= MAX_ALGO_SHM, VOID_VALUE, "usage: %d isn't supported", usage);
    CheckAndLogError(!isIPCFine(), VOID_VALUE, "IPC error happens");

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
    LOG2("%s bufferFd:%d, mInitialized:%d, usage:%d", __func__, bufferFd, mInitialized, usage);
    CheckAndLogError(!mInitialized, -1, "mInitialized is false");
    CheckAndLogError(!isIPCFine(), -1, "IPC error happens");
    CheckAndLogError(usage >= MAX_ALGO_SHM, -1, "usage: %d isn't supported", usage);

    if (usage == CPU_ALGO_SHM) {
        return mBridge->RegisterBuffer(bufferFd);
    } else if (mGpuBridge) {
        return mGpuBridge->RegisterBuffer(bufferFd);
    }
    return OK;
}

void IntelAlgoClient::deregisterGbmBuffer(int32_t bufferHandle, ShmMemUsage usage) {
    LOG1("%s bufferHandle:%d, mInitialized:%d, usage:%d", __func__, bufferHandle, mInitialized,
         usage);
    CheckAndLogError(!mInitialized, VOID_VALUE, "mInitialized is false");
    CheckAndLogError(!isIPCFine(), VOID_VALUE, "IPC error happens");
    CheckAndLogError(usage >= MAX_ALGO_SHM, VOID_VALUE, "usage: %d isn't supported", usage);

    std::vector<int32_t> handles({bufferHandle});
    if (usage == CPU_ALGO_SHM) {
        mBridge->DeregisterBuffers(handles);
    } else if (mGpuBridge) {
        mGpuBridge->DeregisterBuffers(handles);
    }
}

int32_t IntelAlgoClient::getBufferHandle(void* addr, ShmMemUsage usage) {
    CheckAndLogError(!mInitialized, -1, "mInitialized is false");
    CheckAndLogError(usage >= MAX_ALGO_SHM, -1, "usage: %d isn't supported", usage);
    if (!addr) return -1;

    LOG2("the buffer addr: %p, usage: %d", addr, usage);
    std::lock_guard<std::mutex> l(mShmMapMutex);
    CheckAndLogError(mShmMap[usage].find(addr) == mShmMap[usage].end(), -1,
                     "%s, Invalid client addr: %p, usage: %d", __func__, addr, usage);

    return mShmMap[usage][addr];
}

void IntelAlgoClient::callbackHandler(uint32_t req_id, uint32_t status, int32_t buffer_handle) {
    IPC_GROUP group = IntelAlgoIpcCmdToGroup(static_cast<IPC_CMD>(req_id));
    mRunner[group]->callbackHandler(status, buffer_handle);
}

void IntelAlgoClient::notifyHandler(uint32_t msg) {
    if (msg != CAMERA_ALGORITHM_MSG_IPC_ERROR) {
        LOGE("receive msg:%d, not CAMERA_ALGORITHM_MSG_IPC_ERROR", msg);
        return;
    }

    std::lock_guard<std::mutex> l(mIPCStatusMutex);
    mIPCStatus = false;

    if (mErrCb) {
        camera_msg_data_t data = {CAMERA_IPC_ERROR, {}};
        mErrCb->notify(mErrCb, data);
    } else {
        LOGE("mErrCb is nullptr, no device error is sent out");
    }
    LOGE("receive CAMERA_ALGORITHM_MSG_IPC_ERROR");
}

void IntelAlgoClient::returnCallback(const camera_algorithm_callback_ops_t* callback_ops,
                                     uint32_t req_id, uint32_t status, int32_t buffer_handle) {
    CheckAndLogError(!callback_ops, VOID_VALUE, "callback_ops is nullptr");

    auto s = const_cast<IntelAlgoClient*>(static_cast<const IntelAlgoClient*>(callback_ops));
    s->callbackHandler(req_id, status, buffer_handle);
}

void IntelAlgoClient::notifyCallback(const struct camera_algorithm_callback_ops* callback_ops,
                                     camera_algorithm_error_msg_code_t msg) {
    CheckAndLogError(!callback_ops, VOID_VALUE, "callback_ops is nullptr");

    auto s = const_cast<IntelAlgoClient*>(static_cast<const IntelAlgoClient*>(callback_ops));
    s->notifyHandler((uint32_t)msg);
}

IntelAlgoClient::Runner::Runner(IPC_GROUP group, cros::CameraAlgorithmBridge* bridge)
        : mGroup(group),
          mBridge(bridge),
          mIsCallbacked(false),
          mCbStatus(OK),
          mInitialized(false) {
    LOG1("Runner Construct group:%d", mGroup);

    pthread_condattr_t attr;
    int ret = pthread_condattr_init(&attr);
    if (ret != 0) {
        LOGE("call pthread_condattr_init fails, ret:%d", ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (ret != 0) {
        LOGE("call pthread_condattr_setclock fails, ret:%d", ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    ret = pthread_cond_init(&mCbCond, &attr);
    if (ret != 0) {
        LOGE("call pthread_cond_init fails, ret:%d", ret);
        pthread_condattr_destroy(&attr);
        return;
    }

    pthread_condattr_destroy(&attr);

    ret = pthread_mutex_init(&mCbLock, nullptr);
    CheckAndLogError(ret != 0, VOID_VALUE, "call pthread_mutex_init fails, ret:%d", ret);

    mInitialized = true;
}

IntelAlgoClient::Runner::~Runner() {
    LOG1("Runner Destroy, group:%d", mGroup);

    int ret = pthread_cond_destroy(&mCbCond);
    if (ret != 0) {
        LOGE("call pthread_cond_destroy fails, ret:%d", ret);
    }

    ret = pthread_mutex_destroy(&mCbLock);
    if (ret != 0) {
        LOGE("call pthread_mutex_destroy fails, ret:%d", ret);
    }
}

int IntelAlgoClient::Runner::requestSync(IPC_CMD cmd, int32_t bufferHandle) {
    CheckAndLogError(!mInitialized, UNKNOWN_ERROR, "mInitialized is false, cmd:%d:%s", cmd,
                     IntelAlgoIpcCmdToString(cmd));

    std::lock_guard<std::mutex> lck(mMutex);

    std::vector<uint8_t> reqHeader(IPC_REQUEST_HEADER_USED_NUM);
    reqHeader[0] = IPC_MATCHING_KEY;

    // cmd is for request id, no duplicate command will be issued at any given time.
    mBridge->Request(cmd, reqHeader, bufferHandle);
    int ret = waitCallback();
    CheckAndLogError((ret != OK), UNKNOWN_ERROR, "waitCallback fails, cmd:%d:%s", cmd,
                     IntelAlgoIpcCmdToString(cmd));

    // check callback result
    CheckAndLogError((mCbStatus != OK && mCbStatus != ia_err_not_run), mCbStatus,
                     "callback fails, cmd:%d:%s, mCbStatus:%d", cmd, IntelAlgoIpcCmdToString(cmd),
                     mCbStatus);

    return mCbStatus;
}

void IntelAlgoClient::Runner::callbackHandler(uint32_t status, int32_t buffer_handle) {
    if (status != 0 && status != ia_err_not_run) {
        LOGE("Runner callbackHandler group:%d, status:%d, buffer_handle:%d", mGroup, status,
             buffer_handle);
    }
    mCbStatus = status;

    pthread_mutex_lock(&mCbLock);
    mIsCallbacked = true;
    int ret = pthread_cond_signal(&mCbCond);
    pthread_mutex_unlock(&mCbLock);

    CheckAndLogError(ret != 0, VOID_VALUE, "group:%d, call pthread_cond_signal fails, ret:%d",
                     mGroup, ret);
}

int IntelAlgoClient::Runner::waitCallback() {
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
            LOGE("%s, group:%d, call pthread_cond_timedwait fail, ret:%d, it takes %" PRId64 " ms",
                 __func__, mGroup, ret, (CameraUtils::systemTime() - startTime) / 1000000);
            pthread_mutex_unlock(&mCbLock);
            return UNKNOWN_ERROR;
        }
    }
    mIsCallbacked = false;
    pthread_mutex_unlock(&mCbLock);

    LOG2("%s, group:%d IPC call takes %" PRId64 " ms", __func__, mGroup,
         (CameraUtils::systemTime() - startTime) / 1000000);

    return OK;
}

} /* namespace icamera */
