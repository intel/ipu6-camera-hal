/*
 * Copyright (C) 2020 Intel Corporation.
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

#define LOG_TAG "CIPR_CONTEXT"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using icamera::CAMERA_DEBUG_LOG_DBG;
using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_VERBOSE;
using icamera::CAMERA_DEBUG_LOG_WARNING;

#include "modules/ia_cipr/include/Context.h"
#include "modules/ia_cipr/include/ipu-psys.h"

const char* DRIVER_NAME = "/dev/ipu-psys0";

namespace icamera {
namespace CIPR {
Result Context::allocate(MemoryDesc* mem) {
    CheckError(!mem, Result::InvaildArg, "@%s, mem is nullptr", __func__);
    CheckError(mem->cpuPtr, Result::InvaildArg, "%s: already has an address!", __func__);

    mem->cpuPtr = CIPR::mallocAlignedMemory(mem->size, CIPR::getPageSize());
    CheckError(!mem->cpuPtr, Result::NoMemory, "@%s, mem is out", __func__);

    if (!(mem->flags & MemoryFlag::Uninitialized)) {
        memset(mem->cpuPtr, 0, mem->size);
    }

    mem->flags &= ~MemoryFlag::AllocateCpuPtr;
    mem->flags |= MemoryFlag::CpuPtr | MemoryFlag::Allocated;

    return Result::OK;
}

Result Context::migrate(MemoryDesc* mem) {
    CheckError(!mem, Result::InvaildArg, "@%s, mem is nullptr", __func__);

    if (mem->flags & MemoryFlag::AllocateCpuPtr) {
        Result ret = allocate(mem);
        CheckError(ret != Result::OK, ret, "@%s: allocate failed.", __func__);
    }

    if (mem->flags & MemoryFlag::PSysAPI) {
        return Result::OK;
    }

    return registerBuffer(mem);
}

Result Context::getMemory(MemoryDesc* mem, MemoryDesc* out) {
    CheckError(!mem, Result::InvaildArg, "@%s, mem is nullptr", __func__);
    CheckError(!out, Result::InvaildArg, "@%s, out is nullptr", __func__);

    if (!(mInitFlag == Context::Flags::DEBUG) && (mem->flags & MemoryFlag::HardwareOnly)) {
        icamera::LOG2("%s: host cannot access HW only memory!", __func__);
        return Result::GeneralError;
    }

    if ((mem->flags & MemoryFlag::MemoryHandle) && !(mem->flags & MemoryFlag::CpuPtr)) {
        icamera::LOG2("%s: fallback host address mapping not implemented!", __func__);
        return Result::GeneralError;
    }

    *out = *mem;
    return Result::OK;
}

Result Context::destroy(MemoryDesc* mem) {
    return unregisterBuffer(mem);
}

Context::Context() {
    mInitialized = false;
    mFd = open(DRIVER_NAME, 0, O_RDWR | O_NONBLOCK);
    int errnoCopy = errno;

    CheckError(mFd < 0, VOID_VALUE, "%s: Failed to open PSYS device! open returned error: %s",
               __func__, strerror(errnoCopy));

    mInitFlag = Context::Flags::NONE;
    mInitialized = true;
}

Context::~Context() {
    if (!mInitialized) {
        return;
    }

    mInitialized = false;

    int rv = ::close(mFd);
    int errnoCopy = errno;

    CheckError(rv < 0, VOID_VALUE, "Close returned error: %s", strerror(errnoCopy));
}

Result Context::getCapabilities(PSYSCapability* cap) {
    struct ipu_psys_capability psys_capability = {};

    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!cap, Result::InvaildArg, "@%s, cap is nullptr", __func__);

    Result ret = doIoctl(static_cast<int>(IPU_IOC_QUERYCAP), &psys_capability);
    CheckError(ret != Result::OK, ret, "%s: failed to retrieve capabilities", __func__);

    cap->version = psys_capability.version;
    CIPR::memoryCopy(cap->driver, sizeof(cap->driver), psys_capability.driver,
                     sizeof(psys_capability.driver));

    CIPR::memoryCopy(cap->devModel, sizeof(cap->devModel), psys_capability.dev_model,
                     sizeof(psys_capability.dev_model));

    cap->programGroupCount = psys_capability.pg_count;

    return Result::OK;
}

Result Context::getManifest(uint32_t index, uint32_t* mainfestSize, void* manifest) {
    struct ipu_psys_manifest pg_manifest = {};

    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!mainfestSize, Result::InvaildArg, "@%s, mainfestSize is nullptr", __func__);

    pg_manifest.index = index;
    pg_manifest.size = 0;
    pg_manifest.manifest = manifest;

    Result ret = doIoctl(static_cast<int>(IPU_IOC_GET_MANIFEST), &pg_manifest);
    CheckError(ret != Result::OK, ret, "%s: Failed to retrieve manifest with index %d", __func__,
               index);

    *mainfestSize = pg_manifest.size;
    return Result::OK;
}

Result Context::doIoctl(int request, void* ptr) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);

#ifdef ANDROID
    int res = ::ioctl(mFd, request, ptr);
#else
    int res = ::ioctl(mFd, (unsigned int)request, ptr);
#endif
    int errnoCopy = errno;

    if (res < 0) {
        icamera::LOG2("Ioctl returned error: %s", strerror(errnoCopy));
        switch (errnoCopy) {
            case ENOMEM:
                return Result::NoMemory;
            case EINVAL:
                return Result::InvaildArg;
            case EFAULT:
                return Result::InvaildArg;
            case ENOENT:
                return Result::NoEntry;
            default:
                return Result::GeneralError;
        }
    }
    return Result::OK;
}

Result Context::registerBuffer(MemoryDesc* mem) {
    Result res;

    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!mem, Result::InvaildArg, "@%s, mem is nullptr", __func__);
    CheckError(mem->flags & MemoryFlag::Migrated, Result::InvaildArg,
               "Buffer already migrated with device.");

    struct ipu_psys_buffer* ioc_buffer =
        reinterpret_cast<ipu_psys_buffer*>(callocMemory(1, sizeof(*ioc_buffer)));
    CheckError(!ioc_buffer, Result::NoMemory, "Could not create psys buffer");

    ioc_buffer->len = mem->size;
    if (mem->flags & MemoryFlag::CpuPtr) {
        ioc_buffer->base.userptr = mem->cpuPtr;
        ioc_buffer->flags |= IPU_BUFFER_FLAG_USERPTR;
        res = doIoctl(static_cast<int>(IPU_IOC_GETBUF), ioc_buffer);

        if (res != Result::OK) {
            CIPR::freeMemory(ioc_buffer);
            return res;
        }

        if (!(ioc_buffer->flags & IPU_BUFFER_FLAG_DMA_HANDLE)) {
            icamera::LOG2("CIPR: IOC_GETBUF succeed but did not return dma handle");
            CIPR::freeMemory(ioc_buffer);
            return Result::InternalError;
        } else if (ioc_buffer->flags & IPU_BUFFER_FLAG_USERPTR) {
            icamera::LOG2("CIPR: IOC_GETBUF succeed but did not consume the userptr flag");
            CIPR::freeMemory(ioc_buffer);
            return Result::InternalError;
        }
    } else if (mem->flags & MemoryFlag::MemoryHandle) {
        ioc_buffer->base.fd = static_cast<int>(mem->handle);
        ioc_buffer->flags |= IPU_BUFFER_FLAG_DMA_HANDLE;
    }

    if (mem->flags & MemoryFlag::NoFlush) {
        ioc_buffer->flags |= IPU_BUFFER_FLAG_NO_FLUSH;
    }

    res = doIoctl(static_cast<int>(IPU_IOC_MAPBUF),
                  reinterpret_cast<void*>((intptr_t)ioc_buffer->base.fd));

    if (res != Result::OK) {
        CIPR::freeMemory(ioc_buffer);
        return res;
    }

    mem->sysBuff = ioc_buffer;
    mem->flags |= MemoryFlag::Migrated;

    icamera::LOG2("%s: registered %p -> fd %d size:%u offset:%u bytes_used:%u", __func__,
                  mem->cpuPtr, ioc_buffer->base.fd, ioc_buffer->len, ioc_buffer->data_offset,
                  ioc_buffer->bytes_used);

    return res;
}

Result Context::unregisterBuffer(MemoryDesc* mem) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized == false", __func__);
    CheckError(!mem, Result::InvaildArg, "@%s, mem is nullptr", __func__);

    if (mem->sysBuff == nullptr) {
        return Result::OK;
    }

    struct ipu_psys_buffer* ioc_buffer = mem->sysBuff;
    CheckError(!(ioc_buffer->flags & IPU_BUFFER_FLAG_DMA_HANDLE), Result::GeneralError,
               "Wrong flag!", __func__);

    Result res = doIoctl(static_cast<int>(IPU_IOC_UNMAPBUF),
                         reinterpret_cast<void*>((intptr_t)ioc_buffer->base.fd));
    if (res != Result::OK) {
        icamera::LOG2("%s: cannot unmap buffer fd %d, possibly already unmapped", __func__,
                      ioc_buffer->base.fd);
    }

    if (mem->flags & MemoryFlag::CpuPtr) {
        res = psysClose(ioc_buffer->base.fd);
        CheckError(res != Result::OK, res, "@%s: pSysClose failed", __func__);
    }

    CIPR::freeMemory(ioc_buffer);
    mem->sysBuff = nullptr;

    return res;
}

Result Context::psysClose(int fd) {
    int res = close(fd);
    int error = errno;

    if (res < 0) {
        icamera::LOG2("Close returned error: %s", strerror(error));
        switch (error) {
            case EBADF:
                return Result::InvaildArg;
            case EIO:
            case EINTR:
            default:
                return Result::GeneralError;
        }
    }

    return Result::OK;
}

ContextPoller Context::getPoller(int events, int timeout) {
    ContextPoller poller;
    poller.mFd = mFd;
    poller.mEvents = events;
    poller.mTimeout = timeout;

    return poller;
}

int ContextPoller::poll() {
    struct pollfd fds;

    fds.fd = mFd;
    fds.events = mEvents;

    return ::poll(&fds, 1, mTimeout);
}
}  // namespace CIPR
}  // namespace icamera
