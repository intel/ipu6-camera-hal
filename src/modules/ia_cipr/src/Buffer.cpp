/*
 * Copyright (C) 2020-2021 Intel Corporation.
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

#define LOG_TAG CIPR_BUFFER

#include "modules/ia_cipr/include/Buffer.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "modules/ia_cipr/include/Context.h"

using icamera::CAMERA_DEBUG_LOG_ERR;
using icamera::CAMERA_DEBUG_LOG_INFO;
using icamera::CAMERA_DEBUG_LOG_WARNING;

namespace icamera {
namespace CIPR {
Result Buffer::allocateCommon() {
    if (mMemoryDesc.flags & MemoryFlag::DeviceMask) {
        CheckAndLogError(mContext == nullptr, Result::InvaildArg, "mContext is nullptr");
    }

    CheckAndLogError(mMemoryDesc.cpuPtr && !(mMemoryDesc.flags & MemoryFlag::CopyFromUser),
                     Result::InvaildArg, "Buffer has already allocated");

    void* cpuPtr = CIPR::mallocMemory(mMemoryDesc.size);
    CheckAndLogError(!cpuPtr, Result::NoMemory, "The cpuPtr is nullptr");

    if (mMemoryDesc.flags & MemoryFlag::CopyFromUser) {
        if (mMemoryDesc.cpuPtr) {
            MEMCPY_S(cpuPtr, mMemoryDesc.size, mMemoryDesc.cpuPtr, mMemoryDesc.size);
        } else if (mMemoryDesc.flags & MemoryFlag::MemoryHandle) {
            LOGE("Copying from handle to host only was not implemented");
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
        CheckAndLogError(mMemoryDesc.cpuPtr, Result::InternalError,
                         "Flag is AllocateCpuPtr but cpuPtr isn't nullptr");

        Result ret = allocate();
        CheckAndLogError(ret != Result::OK, ret, "Failed to allocate buffer");
    }

    *out = mMemoryDesc;
    return Result::OK;
}

Result Buffer::validateBuffer(const MemoryDesc* memory) {
    CheckAndLogError(!mInitialized, Result::InternalError,
                     "mInitialized is false in validateBuffer");

    const MemoryDesc* mem = (memory) ? memory : &mMemoryDesc;

    bool valid = true;
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

    bool haveMemory = mem->flags & MemoryFlag::Allocated || mem->flags & MemoryFlag::MemoryFromUser;
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
    CheckAndLogError(!mInitialized, Result::InternalError, "mInitialized is false in allocate");

    Result ret = mContext ? mContext->allocate(&mMemoryDesc) : allocateCommon();
    CheckAndLogError(ret != Result::OK, ret, "Failed to allocate buffer");

    ret = validateBuffer(nullptr);
    CheckAndLogError(ret != Result::OK, ret, "Failed to validate bits field");

    return Result::OK;
}

Result Buffer::createWithUserMemory(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory) {
    CheckAndLogError(userMemory->size < size, Result::InternalError,
                     "Requested bytes to allocate is more than provided user memory");

    if (size < userMemory->size) {
        LOG2("Requested bytes is less than provided user memory");
    }

    if (userMemory->flags & MemoryFlag::CpuPtr) {
        if (createWithUserMemoryWithCpuPtr(flags, userMemory) != Result::OK) {
            return Result::InternalError;
        }
    } else if (userMemory->flags & MemoryFlag::MemoryHandle) {
        if (createWithUserMemoryCommon(flags, userMemory) != Result::OK) {
            return Result::InternalError;
        }
    }

    CheckAndLogError(userMemory->anchor, Result::InternalError,
                     "Inheriting object from another is not supported!");

    CheckAndLogError(mMemoryDesc.cpuPtr == nullptr && mMemoryDesc.handle == 0,
                     Result::InternalError, "Invalid user memory given as argument!");

    return Result::OK;
}

Result Buffer::createWithUserMemoryWithCpuPtr(MemoryFlag flags, const MemoryDesc* userMemory) {
    CheckAndLogError(!userMemory->cpuPtr, Result::InternalError, "cpuPtr is nullptr in userMemory");

    CheckAndLogError(userMemory->flags & MemoryFlag::MemoryHandle, Result::InternalError,
                     "MemoryHandle is set for user memory");

    CheckAndLogError(flags & MemoryFlag::AllocateCpuPtr && !(flags & MemoryFlag::CopyFromUser),
                     Result::InternalError, "AllocateCpuPtr is set but CopyFromUser isn't set");

    mMemoryDesc.cpuPtr = userMemory->cpuPtr;
    mMemoryDesc.flags |= MemoryFlag::CpuPtr | MemoryFlag::MemoryFromUser;

    return Result::OK;
}

Result Buffer::createWithUserMemoryCommon(MemoryFlag flags, const MemoryDesc* userMemory) {
    CheckAndLogError(!userMemory->handle, Result::InternalError, "handle is nullptr in userMemory");

    CheckAndLogError(userMemory->flags & MemoryFlag::CpuPtr, Result::InternalError,
                     "MemoryHandle conflicts with cpuPtr");

    CheckAndLogError(flags & MemoryFlag::CopyFromUser, Result::InternalError,
                     "MemoryFlag::CopyFromUser is set");

    CheckAndLogError(flags & MemoryFlag::AllocateCpuPtr && !(flags & MemoryFlag::MemoryHandle),
                     Result::InternalError, "MemoryHandle conflicts with AllocateCpuPtr");

    mMemoryDesc.handle = userMemory->handle;
    mMemoryDesc.flags |= MemoryFlag::MemoryHandle | MemoryFlag::MemoryFromUser;

    return Result::OK;
}

Buffer::Buffer(uint32_t size, MemoryFlag flags, const MemoryDesc* userMemory) {
    mInitialized = false;
    mContext = nullptr;

    if (size == 0) {
        return;
    }

    memset(&mMemoryDesc, 0, sizeof(MemoryDesc));
    mMemoryDesc.anchor = this;
    mMemoryDesc.size = size;

    Result ret = Result::OK;
    if (userMemory) {
        ret = createWithUserMemory(size, flags, userMemory);
        CheckAndLogError(ret != Result::OK, VOID_VALUE, "Failed to create user memory");
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

    CheckAndLogError(parent->mMemoryDesc.size < offset + size, VOID_VALUE,
                     "Parent buffer size %d not enough for region requested (offset %d, size %d)",
                     parent->mMemoryDesc.size, offset, size);

    CheckAndLogError(parent->isRegion(), VOID_VALUE, "Nested regions are illegal");

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

    CheckAndLogError(!mInitialized, Result::InternalError, "mInitialized is false in getMemory");
    CheckAndLogError(!out, Result::InvaildArg, "The out is nullptr");

    if (isRegion()) {
        region = this;
        buffer = mMemoryDesc.anchor;
    }

    Result ret = Result::OK;
    if (buffer->mContext) {
        ret = mContext->getMemory(&buffer->mMemoryDesc, out);
    } else {
        ret = buffer->getMemoryCommon(out);
    }
    CheckAndLogError(ret != Result::OK, ret, "Failed to get memory");

    ret = buffer->validateBuffer(out);
    CheckAndLogError(ret != Result::OK, ret, "Failed to validate bits field");

    if (region) {
        CheckAndLogError(region->mOffset + region->mMemoryDesc.size > out->size,
                         Result::InternalError, "memory region doesn't fit in parent store!");

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

    CheckAndLogError(!ptr, Result::InvaildArg, "ptr is nullptr");

    auto ret = getMemory(&memory);
    CheckAndLogError(ret != Result::OK, ret, "Failed to get memory for cpu buffer");

    *ptr = memory.cpuPtr;

    return Result::OK;
}

Result Buffer::getMemorySize(int* size) {
    CheckAndLogError(!size, Result::InvaildArg, "size is 0");
    *size = mMemoryDesc.size;

    return Result::OK;
}

Buffer* Buffer::getParent() {
    CheckAndLogError(!isRegion(), nullptr, "It isn't a child object");

    if (!mMemoryDesc.anchor) {
        return nullptr;
    }
    return mMemoryDesc.anchor;
}

Result Buffer::attatchDevice(Context* ctx) {
    CheckAndLogError(!mInitialized, Result::InternalError, "mInitialized is false in attatch dev");
    CheckAndLogError(!ctx, Result::InvaildArg, "ctx is nullptr");
    CheckAndLogError(mMemoryDesc.anchor != this, Result::InvaildArg,
                     "The buffer regions cannot be independently migrated");

    mContext = ctx;
    return mContext->migrate(&mMemoryDesc);
}
}  // namespace CIPR
}  // namespace icamera
