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

#pragma once

#include <pthread.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "CameraLog.h"
#include "Parameters.h"
#include "base/bind.h"
#include "base/callback.h"
#include "cros-camera/camera_algorithm_bridge.h"
#include "iutils/Thread.h"
#include "modules/sandboxing/IPCCommon.h"

namespace icamera {

typedef enum {
    CPU_ALGO_SHM,
    GPU_ALGO_SHM,
    MAX_ALGO_SHM,
} ShmMemUsage;

class IntelAlgoClient : public camera_algorithm_callback_ops_t {
 public:
    static IntelAlgoClient* getInstance();
    static void releaseInstance();

    IntelAlgoClient();
    virtual ~IntelAlgoClient();

    void setMojoManager(cros::CameraMojoChannelManager* manager) { mMojoManager = manager; }

    // Connect to the algo processes(cpu and gpu)
    // It must be called after all preparation are ready in camera service
    int initialize();

    bool isIPCFine();

    // when IPC error happens, device error
    // will be sent out via the camera_callback_ops_t which belongs to CameraHal.
    // before the CameraHal be terminated, set nullptr in the function.
    void registerErrorCallback(const camera_callback_ops_t* errCb);

    int allocateShmMem(const std::string& name, int size, int* fd, void** addr);
    void releaseShmMem(const std::string& name, int size, int fd, void* addr);

    int requestSync(IPC_CMD cmd, int32_t bufferHandle);
    int requestSync(IPC_CMD cmd);

    int32_t registerBuffer(int bufferFd, void* addr, ShmMemUsage usage = CPU_ALGO_SHM);
    void deregisterBuffer(int32_t bufferHandle, ShmMemUsage usage = CPU_ALGO_SHM);
    int32_t registerGbmBuffer(int bufferFd, ShmMemUsage usage = CPU_ALGO_SHM);
    void deregisterGbmBuffer(int32_t bufferHandle, ShmMemUsage usage = CPU_ALGO_SHM);
    int32_t getBufferHandle(void* addr, ShmMemUsage usage = CPU_ALGO_SHM);

 private:
    int waitCallback();

    void callbackHandler(uint32_t req_id, uint32_t status, int32_t buffer_handle);
    void notifyHandler(uint32_t msg);

    // when the request is done, the callback will be received.
    static void returnCallback(const camera_algorithm_callback_ops_t* callback_ops, uint32_t req_id,
                               uint32_t status, int32_t buffer_handle);
    // when IPC error happens in the bridge, notifyCallback will be called.
    static void notifyCallback(const struct camera_algorithm_callback_ops* callback_ops,
                               camera_algorithm_error_msg_code_t msg);

 private:
    /*
     * Get access to the IntelAlgoClient singleton.
     */
    static IntelAlgoClient* sInstance;
    static Mutex sLock;  // Guard for singleton creation.

    const camera_callback_ops_t* mErrCb;

    std::unique_ptr<cros::CameraAlgorithmBridge> mBridge;
    std::unique_ptr<cros::CameraAlgorithmBridge> mGpuBridge;

    base::Callback<void(uint32_t, uint32_t, int32_t)> mCallback;
    base::Callback<void(uint32_t)> mNotifyCallback;
    bool mIPCStatus;             // true: no error happens, false: error happens
    std::mutex mIPCStatusMutex;  // the mutex for mIPCStatus

    // <addr in client, buffer handle in server>
    std::unordered_map<void*, int32_t> mShmMap[MAX_ALGO_SHM];
    std::mutex mShmMapMutex;  // the mutex for mShmMap

    cros::CameraMojoChannelManager* mMojoManager;
    bool mInitialized;

 private:
    class Runner {
     public:
        Runner(IPC_GROUP group, cros::CameraAlgorithmBridge* bridge);
        virtual ~Runner();
        int requestSync(IPC_CMD cmd, int32_t bufferHandle);
        void callbackHandler(uint32_t status, int32_t buffer_handle);

     private:
        int waitCallback();

     private:
        IPC_GROUP mGroup;
        cros::CameraAlgorithmBridge* mBridge;
        pthread_mutex_t mCbLock;
        pthread_cond_t mCbCond;
        bool mIsCallbacked;
        bool mCbResult;  // true: success, false: fail

        bool mInitialized;

        std::mutex mMutex;  // the mutex for the public method
    };

    std::unique_ptr<Runner> mRunner[IPC_GROUP_NUM];
};

} /* namespace icamera */
