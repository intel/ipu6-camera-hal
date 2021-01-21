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

#define LOG_TAG "CIPR_BUFFER"

#include "modules/ia_cipr/include/Buffer.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "modules/ia_cipr/include/Context.h"

using icamera::CAMERA_DEBUG_LOG_DBG;
using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_VERBOSE;
using icamera::CAMERA_DEBUG_LOG_WARNING;

namespace icamera {
namespace CIPR {
Result Buffer::allocateCommon() {
    if (mMemoryDesc.flags & MemoryFlag::DeviceMask) {
        CheckError(mContext == nullptr, Result::InvaildArg,
                   "%s: device memory flags specified, migrate first!", __func__);
    }

    CheckError(mMemoryDesc.cpuPtr && !(mMemoryDesc.flags & MemoryFlag::CopyFromUser),
               Result::InvaildArg, "%s: already allocated", __func__);

    void* cpuPtr = CIPR::mallocMemory(mMemoryDesc.size);
    CheckError(!cpuPtr, Result::NoMemory, "@%s, cpuPtr is nullptr", __func__);

    if (mMemoryDesc.flags & MemoryFlag::CopyFromUser) {
        if (mMemoryDesc.cpuPtr) {
            memoryCopy(cpuPtr, mMemoryDesc.size, mMemoryDesc.cpuPtr, mMemoryDesc.size);
        } else if (mMemoryDesc.flags & MemoryFlag::MemoryHandle) {
            LOGE("%s: copy from handle to host only memory not implemented", __func__);
            if (cpuPtr) CIPR::freeMemory(cpuPtr);
            return Result::GeneralError;
        }
        mMemoryDesc.flags &= ~MemoryFlag::CopyFromUser;
    }

    mMemoryDesc.cpuPtr = cpuPtr;
    mMemoryDesc.flags &= ~MemoryFlag::AllocateCpuPtr;
    mMemoryDesc.flags |= MemoryFlag::CpuPtr | MemoryFlag::Allocated;

    return Result::OK;
}

void Buffer::destroy() {
    if ((mMemoryDesc.flags & MemoryFlag::Allocated) && (mMemoryDesc.flags & MemoryFlag::CpuPtr) &&
        mMemoryDesc.cpuPtr) {
        CIPR::freeMemory(mMemoryDesc.cpuPtr);
    }
}

Result Buffer::getMemoryCommon(MemoryDesc* out) {
    if (mMemoryDesc.flags & MemoryFlag::AllocateCpuPtr) {
        CheckError(mMemoryDesc.cpuPtr, Result::InternalError,
                   "%s: ALLOCATE_CPU_PTR not handled but pointer exists!", __func__);

        Result ret = allocate();
        CheckError(ret != Result::OK, ret, "@%s allocate failed.", __func__);
    }

    *out = mMemoryDesc;
    return Result::OK;
}

Result Buffer::validateBuffer(const MemoryDesc* memory) {
    bool valid = true;
    bool haveMemory = false;
    const MemoryDesc* mem = (memory) ? memory : &mMemoryDesc;

    CheckError(!mInitialized, Result::InternalError, "Buffer::validateBuffer mInitialized error");

    haveMemory = mem->flags & MemoryFlag::Allocated || mem->flags & MemoryFlag::MemoryFromUser;

    // cppcheck-suppress clarifyCondition
    if ((!(mem->flags & MemoryFlag::CpuPtr)) ^ (mem->cpuPtr == nullptr)) {
        valid &= false;
    }

    // cppcheck-suppress clarifyCondition
    if ((!(mem->flags & MemoryFlag::MemoryHandle)) ^ (mem->handle == 0)) {
        valid &= false;
    }

    if ((mem->flags & MemoryFlag::CpuOnly) && (mem->flags & MemoryFlag::HardwareOnly)) {
        valid &= false;
    }

    if (!haveMemory &&
        ((mem->flags & MemoryFlag::MemoryHandle) || (mem->flags & MemoryFlag::CpuPtr))) {
        valid &= false;
    }

    if (haveMemory && (mem->flags & MemoryFlag::AllocateCpuPtr)) {
        valid &= false;
    }

    if (haveMemory && (mem->flags & MemoryFlag::CopyFromUser)) {
        valid &= false;
    }

    if (haveMemory &&
        !((mem->flags & MemoryFlag::MemoryHandle) || (mem->flags & MemoryFlag::CpuPtr) ||
          (mem->flags & MemoryFlag::HardwareOnly))) {
        valid &= false;
    }

    return (valid) ? Result::OK : Result::GeneralError;
}

Result Buffer::allocate() {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);

    Result ret = mContext ? mContext->allocate(&mMemoryDesc) : allocateCommon();
    CheckError(ret != Result::OK, ret, "%s: allocation failed", __func__);

    ret = validateBuffer(nullptr);
    CheckError(ret != Result::OK, ret, "%s: bitfield validation after allocate failed", __func__);

    return Result::OK;
}

Result Buffer::createWithUserMemory(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory) {
    if (size < userMemory->size) {
        LOG2(
            "%s: requested bytes to allocate less than provided user memory in argument, "
            "truncating",
            __func__);
    }

    CheckError(userMemory->size < size, Result::InternalError,
               "@%s: requested bytes to allocate more than provided user memory in argument",
               __func__);

    if (userMemory->flags & MemoryFlag::CpuPtr) {
        if (createWithUserMemoryWithCpuPtr(flags, userMemory) != Result::OK) {
            return Result::InternalError;
        }
    } else if (userMemory->flags & MemoryFlag::MemoryHandle) {
        if (createWithUserMemoryCommon(flags, userMemory) != Result::OK) {
            return Result::InternalError;
        }
    }

    CheckError(userMemory->anchor, Result::InternalError,
               "@%s: inheriting object from another not supported!", __func__);

    CheckError(mMemoryDesc.cpuPtr == nullptr && mMemoryDesc.handle == 0, Result::InternalError,
               "@%s: Invalid user memory given as argument!", __func__);

    return Result::OK;
}

Result Buffer::createWithUserMemoryWithCpuPtr(MemoryFlag flags, const MemoryDesc* userMemory) {
    CheckError(!userMemory->cpuPtr, Result::InternalError,
               "@%s: user did not provide pointer with "
               "IA_CIPR_MEMORY_CPU_PTR",
               __func__);

    CheckError(userMemory->flags & MemoryFlag::MemoryHandle, Result::InternalError,
               "@%s: IA_CIPR_MEMORY_CPU_PTR conflict with "
               "IA_CIPR_MEMORY_HANDLE",
               __func__);

    CheckError(flags & MemoryFlag::AllocateCpuPtr && !(flags & MemoryFlag::CopyFromUser),
               Result::InternalError,
               "@%s: IA_CIPR_MEMORY_CPU_PTR conflict with "
               "IA_CIPR_MEMORY_ALLOCATE_CPU_PTR",
               __func__);

    mMemoryDesc.cpuPtr = userMemory->cpuPtr;
    mMemoryDesc.flags |= MemoryFlag::CpuPtr | MemoryFlag::MemoryFromUser;

    return Result::OK;
}

Result Buffer::createWithUserMemoryCommon(MemoryFlag flags, const MemoryDesc* userMemory) {
    CheckError(!userMemory->handle, Result::InternalError,
               "@%s: user did not provide handle with "
               "MemoryFlag::MemoryHandle",
               __func__);

    CheckError(userMemory->flags & MemoryFlag::CpuPtr, Result::InternalError,
               "@%s: MemoryFlag::MemoryHandle conflict with "
               "MemoryFlag::cpuPtr",
               __func__);

    CheckError(flags & MemoryFlag::CopyFromUser, Result::InternalError,
               "@%s: MemoryFlag::CopyFromUser conflict! ", __func__);

    CheckError(flags & MemoryFlag::AllocateCpuPtr && !(flags & MemoryFlag::MemoryHandle),
               Result::InternalError,
               "@%s: MemoryFlag::MemoryHandle conflict with "
               "MemoryFlag::AllocateCpuPtr",
               __func__);

    mMemoryDesc.handle = userMemory->handle;
    mMemoryDesc.flags |= MemoryFlag::MemoryHandle | MemoryFlag::MemoryFromUser;

    return Result::OK;
}

Buffer::Buffer(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory) {
    Result ret;
    mInitialized = false;
    mContext = nullptr;

    if (size == 0) {
        return;
    }

    memset(&mMemoryDesc, 0, sizeof(MemoryDesc));
    mMemoryDesc.anchor = this;
    mMemoryDesc.size = size;

    if (userMemory) {
        ret = createWithUserMemory(size, flags, userMemory);
        CheckError(ret != Result::OK, VOID_VALUE, "@s: Buffer::Buffer Error", __func__);
        mMemoryDesc.flags |= userMemory->flags;
    }

    mMemoryDesc.flags |= flags;

    mInitialized = true;
    ret = validateBuffer(nullptr);
    if (ret != Result::OK) {
        LOG2("Buffer::Buffer validateBuffer Error");
        mInitialized = false;
    }
}

Buffer::Buffer(Buffer* parent, uint32_t offset, uint32_t size) {
    mInitialized = false;
    mContext = nullptr;

    if (!parent || size == 0) {
        return;
    }

    CheckError(parent->mMemoryDesc.size < offset + size, VOID_VALUE,
               "%s: parent buffer size %d not enough for region requested (offset %d, size %d)",
               __func__, parent->mMemoryDesc.size, offset, size);

    CheckError(parent->isRegion(), VOID_VALUE, "%s: Nested regions are illecal", __func__);

    memset(&mMemoryDesc, 0, sizeof(MemoryDesc));
    mMemoryDesc.size = size;
    this->mOffset = offset;
    mMemoryDesc.anchor = parent;
    mMemoryDesc.flags = parent->mMemoryDesc.flags;

    parent->mRegions.insert(this);

    mInitialized = true;
}

Buffer::~Buffer() {
    if (!mInitialized) {
        return;
    }

    mInitialized = false;

    if (isRegion()) {
        if (!mMemoryDesc.anchor) {
            LOG2("%s: parent already released", __func__);
        } else {
            mMemoryDesc.anchor->mRegions.erase(this);
        }
    }

    if (!mRegions.empty()) {
        LOG2("%s: parent buffer destroy leaves stale regions", __func__);

        for (auto& x : mRegions) {
            x->mMemoryDesc.anchor = nullptr;
            delete x;
        }
    }

    if (mContext) {
        mContext->destroy(&mMemoryDesc);
    }

    destroy();
}

bool Buffer::isRegion() const {
    return mMemoryDesc.anchor != this;
}

Result Buffer::getMemory(MemoryDesc* out) {
    Buffer* region = nullptr;
    Buffer* buffer = this;
    Result ret;

    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!out, Result::InvaildArg, "@%s, out is nullptr", __func__);

    if (isRegion()) {
        region = this;
        buffer = mMemoryDesc.anchor;
    }

    if (buffer->mContext) {
        ret = mContext->getMemory(&buffer->mMemoryDesc, out);
    } else {
        ret = buffer->getMemoryCommon(out);
    }
    CheckError(ret != Result::OK, ret, "@%s: getMemory() failed", __func__);

    ret = buffer->validateBuffer(out);
    CheckError(ret != Result::OK, ret, "@%s: bitfield validation failed", __func__);

    if (region) {
        CheckError(region->mOffset + region->mMemoryDesc.size > out->size, Result::InternalError,
                   "@%s: memory region doesn't fit in parent store!", __func__);

        if (out->cpuPtr) {
            out->cpuPtr = reinterpret_cast<uint8_t*>(out->cpuPtr) + region->mOffset;
        }

        out->size = region->mMemoryDesc.size;
        out->anchor = region;
    }

    return Result::OK;
}

Result Buffer::getMemoryCpuPtr(void** ptr) {
    CIPR::MemoryDesc memory;

    CheckError(!ptr, Result::InvaildArg, "@%s: ptr == null", __func__);

    auto ret = getMemory(&memory);
    CheckError(ret != Result::OK, ret, "@%s: getMemory failed.", __func__);

    *ptr = memory.cpuPtr;

    return Result::OK;
}

Result Buffer::getMemorySize(int* size) {
    CheckError(!size, Result::InvaildArg, "@%s: size == 0", __func__);
    *size = mMemoryDesc.size;

    return Result::OK;
}

Buffer* Buffer::getParent() {
    if (!isRegion()) {
        LOGE("%s: not a child object", __func__);
        return nullptr;
    }

    if (!mMemoryDesc.anchor) {
        return nullptr;
    }
    return mMemoryDesc.anchor;
}

Result Buffer::attatchDevice(Context* ctx) {
    CheckError(!mInitialized, Result::InternalError, "@%s, mInitialized is false", __func__);
    CheckError(!ctx, Result::InvaildArg, "@%s, ctx is nullptr", __func__);
    CheckError(mMemoryDesc.anchor != this, Result::InvaildArg,
               "%s: buffer regions cannot be independently migrated", __func__);

    mContext = ctx;
    return mContext->migrate(&mMemoryDesc);
}
}  // namespace CIPR
}  // namespace icamera
