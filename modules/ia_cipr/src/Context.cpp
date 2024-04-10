/*
 * Copyright (C) 2020-2023 Intel Corporation.
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

#define LOG_TAG CIPR_CONTEXT

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

using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_INFO;
using icamera::CAMERA_DEBUG_LOG_WARNING;

#include "modules/ia_cipr/include/Context.h"
#include "modules/ia_cipr/include/ipu-psys.h"

const char* DRIVER_NAME = "/dev/ipu-psys0";

namespace icamera {
namespace CIPR {
Result Context::allocate(MemoryDesc* mem) {
    CheckAndLogError(!mem, Result::InvaildArg, "allocate mem is nullptr");
    CheckAndLogError(mem->cpuPtr, Result::InvaildArg, "cpuPtr already has an address");

    mem->cpuPtr = CIPR::mallocAlignedMemory(mem->size, CIPR::getPageSize());
    CheckAndLogError(!mem->cpuPtr, Result::NoMemory, "Failed to malloc memory");

    if (!(mem->flags & MemoryFlag::Uninitialized)) {
        memset(mem->cpuPtr, 0, mem->size);
    }

    mem->flags &= ~MemoryFlag::AllocateCpuPtr;
    mem->flags |= MemoryFlag::CpuPtr | MemoryFlag::Allocated;

    return Result::OK;
}

Result Context::migrate(MemoryDesc* mem) {
    CheckAndLogError(!mem, Result::InvaildArg, "migrate mem is nullptr");

    if (mem->flags & MemoryFlag::AllocateCpuPtr) {
        Result ret = allocate(mem);
        CheckAndLogError(ret != Result::OK, ret, "Failed to allocate memory");
    }

    if (mem->flags & MemoryFlag::PSysAPI) {
        return Result::OK;
    }

    return registerBuffer(mem);
}

Result Context::getMemory(MemoryDesc* mem, MemoryDesc* out) {
    CheckAndLogError(!mem, Result::InvaildArg, "the mem of memory desc is nullptr");
    CheckAndLogError(!out, Result::InvaildArg, "the out of memory desc is nullptr");

    if (mem->flags & MemoryFlag::HardwareOnly) {
        LOG2("%s: host cannot access HW only memory!", __func__);
        return Result::GeneralError;
    }

    if ((mem->flags & MemoryFlag::MemoryHandle) && !(mem->flags & MemoryFlag::CpuPtr)) {
        LOG2("%s: fallback host address mapping not implemented!", __func__);
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
    CheckAndLogError(mFd < 0, VOID_VALUE, "Failed to open PSYS, error: %s", strerror(errno));

    mInitialized = true;
}

Context::~Context() {
    if (!mInitialized) return;

    int rv = ::close(mFd);
    CheckAndLogError(rv < 0, VOID_VALUE, "Close returned error: %s", strerror(errno));
}

Result Context::getCapabilities(PSYSCapability* cap) {
    CheckAndLogError(!cap, Result::InvaildArg, "cap is nullptr");

    struct ipu_psys_capability psys_capability = {};
    Result ret = doIoctl(static_cast<int>(IPU_IOC_QUERYCAP), &psys_capability);
    CheckAndLogError(ret != Result::OK, ret, "failed to retrieve capabilities");

    cap->version = psys_capability.version;
    CheckAndLogError(sizeof(cap->driver) != sizeof(psys_capability.driver), Result::DataError,
                     "the driver array size wasn't matching");
    MEMCPY_S(cap->driver, sizeof(cap->driver), psys_capability.driver,
             sizeof(psys_capability.driver));

    CheckAndLogError(sizeof(cap->devModel) != sizeof(psys_capability.dev_model), Result::DataError,
                     "the dev model array size wasn't matching");
    MEMCPY_S(cap->devModel, sizeof(cap->devModel), psys_capability.dev_model,
             sizeof(psys_capability.dev_model));

    cap->programGroupCount = psys_capability.pg_count;

    return Result::OK;
}

Result Context::getManifest(uint32_t index, uint32_t* mainfestSize, void* manifest) {
    CheckAndLogError(!mainfestSize, Result::InvaildArg, "mainfestSize is nullptr");

    struct ipu_psys_manifest pg_manifest = {};
    pg_manifest.index = index;
    pg_manifest.size = 0;
    pg_manifest.manifest = manifest;

    Result ret = doIoctl(static_cast<int>(IPU_IOC_GET_MANIFEST), &pg_manifest);
    if (ret != Result::OK) return ret;

    *mainfestSize = pg_manifest.size;
    return Result::OK;
}

Result Context::doIoctl(int request, void* ptr) {
#ifdef ANDROID
    int res = ::ioctl(mFd, request, ptr);
#else
    int res = ::ioctl(mFd, (unsigned int)request, ptr);
#endif
    if (res < 0) {
        int errnoCopy = errno;
        // Some are not real errors, so don't print error here
        LOG2("Ioctl returned error: %s", strerror(errnoCopy));
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
    CheckAndLogError(!mem, Result::InvaildArg, "register mem is nullptr");
    CheckAndLogError(mem->flags & MemoryFlag::Migrated, Result::InvaildArg,
                     "Buffer already migrated with device.");

    struct ipu_psys_buffer* ioc_buffer =
        reinterpret_cast<ipu_psys_buffer*>(callocMemory(1, sizeof(*ioc_buffer)));
    CheckAndLogError(!ioc_buffer, Result::NoMemory, "Could not create psys buffer");

    ioc_buffer->len = mem->size;

    Result res = Result::OK;
    if (mem->flags & MemoryFlag::CpuPtr) {
        ioc_buffer->base.userptr = mem->cpuPtr;
        ioc_buffer->flags |= IPU_BUFFER_FLAG_USERPTR;
        res = doIoctl(static_cast<int>(IPU_IOC_GETBUF), ioc_buffer);

        if (res != Result::OK) {
            CIPR::freeMemory(ioc_buffer);
            return res;
        }

        if (!(ioc_buffer->flags & IPU_BUFFER_FLAG_DMA_HANDLE)) {
            LOG2("CIPR: IOC_GETBUF succeed but did not return dma handle");
            CIPR::freeMemory(ioc_buffer);
            return Result::InternalError;
        } else if (ioc_buffer->flags & IPU_BUFFER_FLAG_USERPTR) {
            LOG2("CIPR: IOC_GETBUF succeed but did not consume the userptr flag");
            CIPR::freeMemory(ioc_buffer);
            return Result::InternalError;
        }
    } else if (mem->flags & MemoryFlag::MemoryHandle) {
        ioc_buffer->base.fd = static_cast<int>(mem->handle);
        ioc_buffer->flags |= IPU_BUFFER_FLAG_DMA_HANDLE;
    }

#ifdef IPU_SYSVER_ipu6v3
    ioc_buffer->flags |= IPU_BUFFER_FLAG_NO_FLUSH;
#else
    if (mem->flags & MemoryFlag::NoFlush) {
        ioc_buffer->flags |= IPU_BUFFER_FLAG_NO_FLUSH;
    }
#endif

    res = doIoctl(static_cast<int>(IPU_IOC_MAPBUF),
                  reinterpret_cast<void*>((intptr_t)ioc_buffer->base.fd));

    if (res != Result::OK) {
        CIPR::freeMemory(ioc_buffer);
        return res;
    }

    mem->sysBuff = ioc_buffer;
    mem->flags |= MemoryFlag::Migrated;

    LOG2("registered buffer:%p -> fd:%d len:%lu offset:%u bytes_used:%u, flags:%x", mem->cpuPtr,
         ioc_buffer->base.fd, ioc_buffer->len, ioc_buffer->data_offset, ioc_buffer->bytes_used,
         ioc_buffer->flags);

    return res;
}

Result Context::unregisterBuffer(MemoryDesc* mem) {
    CheckAndLogError(!mem, Result::InvaildArg, "unregister mem is nullptr");

    if (mem->sysBuff == nullptr) {
        return Result::OK;
    }

    struct ipu_psys_buffer* ioc_buffer = mem->sysBuff;
    CheckAndLogError(!(ioc_buffer->flags & IPU_BUFFER_FLAG_DMA_HANDLE), Result::GeneralError,
                     "Wrong flag and not a DMA handle");

    Result res = doIoctl(static_cast<int>(IPU_IOC_UNMAPBUF),
                         reinterpret_cast<void*>((intptr_t)ioc_buffer->base.fd));
    if (res != Result::OK) {
        LOG2("%s: cannot unmap buffer fd %d, possibly already unmapped", __func__,
             ioc_buffer->base.fd);
    }

    if (mem->flags & MemoryFlag::CpuPtr) {
        res = psysClose(ioc_buffer->base.fd);
        CheckAndLogError(res != Result::OK, res, "pSysClose failed");
    }

    CIPR::freeMemory(ioc_buffer);
    mem->sysBuff = nullptr;

    return res;
}

Result Context::psysClose(int fd) {
    int res = close(fd);
    if (res < 0) {
        int errnoCopy = errno;

        LOGE("Failed to close, error %s", strerror(errnoCopy));
        switch (errnoCopy) {
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
