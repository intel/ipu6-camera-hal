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

#include <base/bind.h>
#include <base/threading/thread.h>

#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "CameraLog.h"
#include "cros-camera/camera_algorithm.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "modules/sandboxing/IPCCommon.h"

namespace icamera {

#define HANDLE_INDEX_MAX_VALUE 1024
struct MsgReq {
    uint32_t req_id;
    int32_t buffer_handle;
};

typedef struct {
    int32_t fd;
    void* addr;
    size_t size;
} ShmInfo;

class IntelAlgoServer;
class RequestHandler {
 public:
    explicit RequestHandler(IntelAlgoServer* server) { mIntelAlgoServer = server; }
    virtual ~RequestHandler() {}
    virtual void handleRequest(const MsgReq& msg) = 0;
    IntelAlgoServer* getIntelAlgoServer() { return mIntelAlgoServer; }

 private:
    IntelAlgoServer* mIntelAlgoServer;
};

class IntelAlgoServer {
 public:
    static void init();
    static void deInit();

    static IntelAlgoServer* getInstance() { return mInstance; }

    int32_t initialize(const camera_algorithm_callback_ops_t* callback_ops);
    int32_t registerBuffer(int buffer_fd);
    void request(uint32_t req_id, const uint8_t req_header[], uint32_t size, int32_t buffer_handle);
    void deregisterBuffers(const int32_t buffer_handles[], uint32_t size);

    void handleRequest(const MsgReq& msg);
    status_t getShmInfo(const int32_t buffer_handle, ShmInfo* memInfo);
    void returnCallback(uint32_t req_id, status_t status, int32_t buffer_handle);

 private:
    IntelAlgoServer();
    ~IntelAlgoServer();
    int parseReqHeader(const uint8_t req_header[], uint32_t size);

 private:
    static IntelAlgoServer* mInstance;
#ifndef GPU_ALGO_SERVER
    static const int kThreadNum = IPC_CPU_GROUP_NUM;
#else
    static const int kThreadNum = IPC_GPU_GROUP_NUM;
#endif
    std::unique_ptr<base::Thread> mThreads[kThreadNum];
    std::unique_ptr<RequestHandler> mRequestHandler;

    const camera_algorithm_callback_ops_t* mCallback;

    // key: shared memory fd from client
    // value: handle that returns from RegisterBuffer()
    std::unordered_map<int32_t, int32_t> mHandles;

    // key: handle that returns from RegisterBuffer()
    // value: shared memory fd and mapped address
    std::unordered_map<int32_t, ShmInfo> mShmInfoMap;
    std::queue<int32_t> mHandlesQueue;
    std::mutex mRegisterBufMutex;

    DISALLOW_COPY_AND_ASSIGN(IntelAlgoServer);
};
} /* namespace icamera */
