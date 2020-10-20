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

#define LOG_TAG "IntelAlgoServer"

#include "modules/sandboxing/server/IntelAlgoServer.h"

#include <base/logging.h>
#include <ia_log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>
#include <string>

#include "iutils/Utils.h"
#ifndef GPU_ALGO_SERVER
#include "modules/sandboxing/server/IntelCPUAlgoServer.h"
#else
#include "modules/sandboxing/server/IntelGPUAlgoServer.h"
#endif

namespace icamera {

IntelAlgoServer* IntelAlgoServer::mInstance = nullptr;

void IntelAlgoServer::init() {
    LOGIPC("@%s", __func__);

    if (mInstance == nullptr) {
        mInstance = new IntelAlgoServer;
    }
}

void IntelAlgoServer::deInit() {
    LOGIPC("@%s", __func__);

    delete mInstance;
    mInstance = nullptr;
}

IntelAlgoServer::IntelAlgoServer() : mCallback(nullptr) {
    LOGIPC("@%s", __func__);

    ia_env env = {&Log::ccaPrintDebug, &Log::ccaPrintError, &Log::ccaPrintInfo};
    ia_log_init(&env);

    for (int i = 0; i < kThreadNum; i++) {
        std::string name = IntelAlgoServerThreadName(i);
        mThreads[i] = std::unique_ptr<base::Thread>(new base::Thread(name));
        mThreads[i]->Start();
    }
#ifndef GPU_ALGO_SERVER
    mRequestHandler = std::unique_ptr<RequestHandler>(new IntelCPUAlgoServer(this));
#else
    mRequestHandler = std::unique_ptr<RequestHandler>(new IntelGPUAlgoServer(this));
#endif

    for (int32_t i = 1; i <= HANDLE_INDEX_MAX_VALUE; i++) {
        mHandlesQueue.push(i);
    }
}

IntelAlgoServer::~IntelAlgoServer() {
    LOGIPC("@%s", __func__);
    ia_log_deinit();
}

int32_t IntelAlgoServer::initialize(const camera_algorithm_callback_ops_t* callback_ops) {
    LOGIPC("@%s, callback_ops:%p", __func__, callback_ops);

    CheckError((!callback_ops), -EINVAL, "@%s, the callback_ops is nullptr", __func__);

    mCallback = callback_ops;

    return 0;
}

int32_t IntelAlgoServer::registerBuffer(int buffer_fd) {
    LOGIPC("@%s, buffer_fd:%d", __func__, buffer_fd);

    std::lock_guard<std::mutex> l(mRegisterBufMutex);
    CheckError((mHandles.find(buffer_fd) != mHandles.end()), -EINVAL,
               "@%s, Buffer already registered", __func__);
    CheckError(mHandlesQueue.empty(), -EBADFD, "@%s, Failed to get buffer handle index", __func__);

    struct stat sb;
    int ret = fstat(buffer_fd, &sb);
    CheckError((ret == -1), -EBADFD, "@%s, Failed to get buffer status", __func__);

    void* addr = mmap(0, sb.st_size, PROT_WRITE, MAP_SHARED, buffer_fd, 0);
    CheckError((!addr), -EBADFD, "@%s, Failed to map buffer", __func__);

    int32_t handle = mHandlesQueue.front();
    mHandlesQueue.pop();
    mHandles[buffer_fd] = handle;

    mShmInfoMap[handle].fd = buffer_fd;
    mShmInfoMap[handle].addr = addr;
    mShmInfoMap[handle].size = sb.st_size;

    return handle;
}

int IntelAlgoServer::parseReqHeader(const uint8_t req_header[], uint32_t size) {
    LOGIPC("@%s, size:%d", __func__, size);

    CheckError(size < IPC_REQUEST_HEADER_USED_NUM || req_header[0] != IPC_MATCHING_KEY, -1,
               "@%s, fails, req_header[0]:%d, size:%d", __func__, req_header[0], size);

    return 0;
}

void IntelAlgoServer::returnCallback(uint32_t req_id, status_t status, int32_t buffer_handle) {
    LOGIPC("@%s, req_id:%d:%s, status:%d", __func__, req_id,
           IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)), status);
    (*mCallback->return_callback)(mCallback, req_id, (status == OK ? 0 : 1), buffer_handle);
}

status_t IntelAlgoServer::getShmInfo(const int32_t buffer_handle, ShmInfo* memInfo) {
    CheckError(!memInfo, UNKNOWN_ERROR, "%s, memInfo is nullptr", __func__);
    if (buffer_handle == -1) return OK;

    CheckError(mShmInfoMap.find(buffer_handle) == mShmInfoMap.end(), UNKNOWN_ERROR,
               "%s, Invalid buffer handle", __func__);
    *memInfo = mShmInfoMap[buffer_handle];

    LOGIPC("@%s, fd:%d, size:%zu, addr: %p", __func__, memInfo->fd, memInfo->size, memInfo->addr);

    return OK;
}

void IntelAlgoServer::handleRequest(const MsgReq& msg) {
    LOGIPC("@%s", __func__);
    CheckError(!mRequestHandler, VOID_VALUE, "@%s, handler is null", __func__);
    mRequestHandler->handleRequest(msg);
}

void IntelAlgoServer::request(uint32_t req_id, const uint8_t req_header[], uint32_t size,
                              int32_t buffer_handle) {
    LOGIPC("@%s, size:%d, buffer_handle:%d", __func__, size, buffer_handle);
    LOGIPC("@%s, req_id:%d:%s", __func__, req_id,
           IntelAlgoIpcCmdToString(static_cast<IPC_CMD>(req_id)));

    IPC_GROUP group = IntelAlgoIpcCmdToGroup(static_cast<IPC_CMD>(req_id));
    LOGIPC("@%s, group:%d", __func__, group);

    int ret = parseReqHeader(req_header, size);
    if (ret != 0) {
        returnCallback(req_id, UNKNOWN_ERROR, buffer_handle);
        return;
    }

    MsgReq msg = {req_id, buffer_handle};

#ifndef GPU_ALGO_SERVER
    int threadId = group;
#else
    // GPU server thread id start from IPC_GROUP_GPU
    int threadId = group - IPC_GROUP_GPU;
#endif
    if (threadId >= 0 && threadId < kThreadNum) {
        if (mThreads[threadId] && mThreads[threadId]->task_runner()) {
            mThreads[threadId]->task_runner()->PostTask(FROM_HERE,
                base::Bind(&IntelAlgoServer::handleRequest, base::Unretained(this), msg));
        }
    }
}

void IntelAlgoServer::deregisterBuffers(const int32_t buffer_handles[], uint32_t size) {
    LOGIPC("@%s, size:%d", __func__, size);

    std::lock_guard<std::mutex> l(mRegisterBufMutex);
    for (uint32_t i = 0; i < size; i++) {
        int32_t handle = buffer_handles[i];
        if (mShmInfoMap.find(handle) == mShmInfoMap.end()) {
            continue;
        }

        mHandles.erase(mShmInfoMap[handle].fd);

        munmap(mShmInfoMap[handle].addr, mShmInfoMap[handle].size);
        close(mShmInfoMap[handle].fd);
        mShmInfoMap.erase(handle);
        mHandlesQueue.push(handle);
    }
}

static int32_t initialize(const camera_algorithm_callback_ops_t* callback_ops) {
    LOGIPC("@%s, callback_ops:%p", __func__, callback_ops);
    return IntelAlgoServer::getInstance()->initialize(callback_ops);
}

static int32_t registerBuffer(int32_t buffer_fd) {
    LOGIPC("@%s, buffer_fd:%d", __func__, buffer_fd);
    return IntelAlgoServer::getInstance()->registerBuffer(buffer_fd);
}

static void request(uint32_t req_id, const uint8_t req_header[], uint32_t size,
                    int32_t buffer_handle) {
    LOGIPC("@%s, size:%d, buffer_handle:%d", __func__, size, buffer_handle);
    IntelAlgoServer::getInstance()->request(req_id, req_header, size, buffer_handle);
}

static void deregisterBuffers(const int32_t buffer_handles[], uint32_t size) {
    LOGIPC("@%s, size:%d", __func__, size);
    return IntelAlgoServer::getInstance()->deregisterBuffers(buffer_handles, size);
}

extern "C" {
camera_algorithm_ops_t CAMERA_ALGORITHM_MODULE_INFO_SYM
    __attribute__((__visibility__("default"))) = {.initialize = initialize,
                                                  .register_buffer = registerBuffer,
                                                  .request = request,
                                                  .deregister_buffers = deregisterBuffers};
}

__attribute__((constructor)) void initIntelAlgoServer() {
    icamera::Log::setDebugLevel();
    IntelAlgoServer::init();
}

__attribute__((destructor)) void deinitIntelAlgoServer() {
    IntelAlgoServer::deInit();
}

} /* namespace icamera */
