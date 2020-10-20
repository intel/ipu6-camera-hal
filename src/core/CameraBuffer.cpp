/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#define LOG_TAG "CameraBuffer"

#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#include "PlatformData.h"
#include "CameraBuffer.h"

namespace icamera {
CameraBuffer::CameraBuffer(int cameraId, int usage, int memory, uint32_t size, int index, int format) :
    mNumPlanes(1),
    mAllocatedMemory(false),
    mU(nullptr),
    mBufferUsage(usage),
    mSettingSequence(-1)

{
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    int num_plane = 1;

    LOG1("%s: construct CameraBuffer with cameraId:%d, usage:%d, memory:%d, size:%d, format:%d, index:%d",
         __func__, cameraId, usage, memory, size, format, index);

    mU = new camera_buffer_t;
    CLEAR(*mU);
    mU->flags = BUFFER_FLAG_INTERNAL;
    mU->sequence = -1;

    switch (usage) {
        case BUFFER_USAGE_PSYS_INPUT:
            //follow through
        case BUFFER_USAGE_PSYS_INTERNAL:
        case BUFFER_USAGE_GENERAL:
            if (PlatformData::isIsysEnabled(cameraId)
                && PlatformData::isCSIFrontEndCapture(cameraId)) {
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
                num_plane = CameraUtils::getNumOfPlanes(format);
            } else {
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            }
            break;
        case BUFFER_USAGE_PSYS_STATS:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            break;
        case BUFFER_USAGE_MIPI_CAPTURE:
        case BUFFER_USAGE_METADATA:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            num_plane = CameraUtils::getNumOfPlanes(format);
            break;
        default:
            LOGE("Not supported Usage");
    }

    CLEAR(mMmapAddrs);
    CLEAR(mDmaFd);

    initBuffer(memory, type, size, index, num_plane);
}

CameraBuffer::~CameraBuffer()
{
    LOG1("Free CameraBuffer");

    freeMemory();

    if (mU->flags & BUFFER_FLAG_INTERNAL) {
        delete mU;
    }
}

void CameraBuffer::initBuffer(int memType, v4l2_buf_type bufType, uint32_t size, int idx, int num_plane)
{
    mV.SetMemory(memType);
    mV.SetIndex(idx);

    mV.SetType(bufType);

    if (!V4L2_TYPE_IS_MULTIPLANAR(bufType)) {
        mV.SetOffset(0, 0);
        mV.SetLength(size, 0);
        LOGE("SINGLE PLANE!");
    } else {
        mV.SetLength(num_plane, 0);
        mNumPlanes = num_plane;
        for (int i = 0; i < mNumPlanes; ++i) {
            mV.SetLength(size, i);
        }
    }

    mV.SetFlags(mV.Flags() | V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN);
}

//Helper function to construct a Internal CameraBuffer
std::shared_ptr<CameraBuffer> CameraBuffer::create(int cameraId, int usage, int memory, unsigned int size, int index,
                                              int srcFmt, int srcWidth, int srcHeight)
{
    std::shared_ptr<CameraBuffer> camBuffer = std::make_shared<CameraBuffer>(cameraId, usage, memory, size, index, srcFmt);

    CheckError(!camBuffer, nullptr, "@%s: fail to alloc CameraBuffer", __func__);

    camBuffer->setUserBufferInfo(srcFmt, srcWidth, srcHeight);

    int ret = camBuffer->allocateMemory();

    CheckError(ret != OK, nullptr, "Allocate memory failed ret %d", ret);

    return camBuffer;
}

//Internal frame Buffer
void CameraBuffer::setUserBufferInfo(int format, int width, int height)
{
    LOG1("%s: format:%d, width:%d, height:%d", __func__, format, width, height);
    mU->s.width = width;
    mU->s.height = height;
    mU->s.format = format;
    if (format != -1) {
        mU->s.stride = CameraUtils::getStride(format, width);
    }
}

void CameraBuffer::setUserBufferInfo(int format, int width, int height, void *usrPtr)
{
    setUserBufferInfo(format, width, height);
    setAddr(usrPtr, 0);
}

//Called when a buffer is from the application
void CameraBuffer::setUserBufferInfo(camera_buffer_t *ubuffer)
{
    CheckError(ubuffer == nullptr, VOID_VALUE, "%s: ubuffer is nullptr", __func__);

    if (mU->flags & BUFFER_FLAG_INTERNAL) delete mU;
    mU = ubuffer;

    LOG1("%s: ubuffer->s.MemType: %d, addr: %p, fd: %d", __func__, ubuffer->s.memType,
         ubuffer->addr, ubuffer->dmafd);
    //update the v4l2 buffer memory with user infro
    switch (ubuffer->s.memType) {
        case V4L2_MEMORY_USERPTR:
            setAddr(ubuffer->addr, 0);
            break;
        case V4L2_MEMORY_DMABUF:
            setFd(ubuffer->dmafd, 0);
            break;
        case V4L2_MEMORY_MMAP:
            /* do nothing */
            break;
        default:
            LOGE("iomode %d is not supported yet.", mV.Memory());
            break;
    }

    if (mU->s.streamType == CAMERA_STREAM_INPUT || ubuffer->sequence >= 0) {
        mV.SetSequence(ubuffer->sequence);
        LOG2("%s, input buffer sequence %lld", __func__, ubuffer->sequence);
    }
}

void CameraBuffer::updateV4l2Buffer(const v4l2_buffer_t& v4l2buf)
{
    mV.SetField(v4l2buf.field);
    mV.SetTimestamp(v4l2buf.timestamp);
    mV.SetSequence(v4l2buf.sequence);
    mV.SetRequestFd(v4l2buf.request_fd);
}

/*export mmap buffer as dma_buf fd stored in mV and mU*/
int CameraBuffer::exportMmapDmabuf(V4L2VideoNode *vDevice)
{
    std::vector<int> fds;

    int ret = vDevice->ExportFrame(mV.Index(), &fds);

    CheckError(ret != OK, -1, "exportMmapDmabuf failed, ret %d", ret);

    for (size_t i = 0; i < fds.size(); ++i) {
        setFd(fds[i], i);
    }

    if (mU->flags & BUFFER_FLAG_DMA_EXPORT) {
        mU->dmafd = getFd(0);
    }

    return OK;
}

int CameraBuffer::allocateMemory(V4L2VideoNode* vDevice)
{
    int ret = BAD_VALUE;
    LOG1("%s", __func__);
    switch(mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            ret = allocateUserPtr();
            mAllocatedMemory = true;
            mU->addr = getAddr();
            break;
        case V4L2_MEMORY_MMAP:
            exportMmapDmabuf(vDevice);
            ret = allocateMmap(vDevice);
            mU->addr = getAddr();
            mAllocatedMemory = true;
            break;
        default:
            LOGE("memory type %d is incorrect for allocateMemory.", mV.Memory());
            return BAD_VALUE;
    }

    return ret;
}

int CameraBuffer::allocateUserPtr()
{
    void* buffer = nullptr;
    for (int i = 0; i < mNumPlanes; ++i) {
        int ret = posix_memalign(&buffer, getpagesize(), mV.Length(i));
        CheckError(ret != 0, -1, "%s, posix_memalign fails, ret:%d", __func__, ret);
        mV.SetUserptr(reinterpret_cast<uintptr_t>(buffer), i);
        mMmapAddrs[i] = buffer;
    }
    return OK;
}

void CameraBuffer::freeUserPtr()
{
    for (int i = 0; i < mNumPlanes; ++i) {
        void* ptr = reinterpret_cast<void*>(mV.Userptr(i));
        mMmapAddrs[i] = nullptr;
        ::free(ptr);
        mV.SetUserptr(reinterpret_cast<uintptr_t>(nullptr), i);
    }
}

int CameraBuffer::allocateMmap(V4L2VideoNode* dev)
{
    std::vector<void*> addrs;
    int ret = dev->MapMemory(mV.Index(), PROT_READ | PROT_WRITE,
                            MAP_SHARED, &addrs);

    CheckError(ret != OK, -1, "allocateMmap failed, ret %d", ret);

    for (unsigned int i = 0; i < addrs.size(); ++i) {
        if (addrs[i] == MAP_FAILED) {
            mMmapAddrs[i] = nullptr;
            continue ;
        }
        mMmapAddrs[i] = addrs[i];
    }

    return OK;
}

void* CameraBuffer::getAddr(int plane)
{
    CheckError(plane < 0 || plane >= mNumPlanes, nullptr, "Wrong plane number %d", plane);

    switch (mV.Memory()) {
        case V4L2_MEMORY_MMAP:
            return mMmapAddrs[plane];
        case V4L2_MEMORY_USERPTR:
            return reinterpret_cast<void*>(mV.Userptr(plane));
        default:
            LOGE("%s: Not supported memory type %u", __func__, mV.Memory());
    }
    return nullptr;
}

void CameraBuffer::setAddr(void *addr, int plane)
{
    CheckError(plane < 0 || plane >= mNumPlanes, VOID_VALUE,
               "Wrong plane number %d", plane);

    switch (mV.Memory()) {
        case V4L2_MEMORY_MMAP:
            mMmapAddrs[plane] = addr;
            return;
        case V4L2_MEMORY_USERPTR:
            mV.SetUserptr(reinterpret_cast<uintptr_t>(addr), plane);
            mMmapAddrs[plane] = addr;
            return;
        default:
            LOGE("%s: Not supported memory type %u", __func__, mV.Memory());
    }
}

void CameraBuffer::freeMmap()
{
    int ret = OK;

    for (int i = 0; i < mNumPlanes; i++) {
        if (getFd(i) != -1) {
            ::close(getFd(i));
            setFd(-1, i);
        }
        if (mMmapAddrs[i]) {
            ret = ::munmap(mMmapAddrs[i], mV.Length(i));
            CheckError(ret != 0, VOID_VALUE, "failed to munmap buffer %d", i);
            mMmapAddrs[i] = nullptr;
        }
    }
}

void* CameraBuffer::mapDmaBufferAddr(int fd, unsigned int bufferSize)
{
    if(fd < 0 || !bufferSize) {
        LOGE("%s, fd:0x%x, bufferSize:%u", __func__, fd, bufferSize);
        return nullptr;
    }
    return ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

void CameraBuffer::unmapDmaBufferAddr(void* addr, unsigned int bufferSize)
{
    if(addr == nullptr || !bufferSize) {
        LOGE("%s, addr:%p, bufferSize:%u", __func__, addr, bufferSize);
        return;
    }
    munmap(addr, bufferSize);
}

void CameraBuffer::freeMemory()
{
    if (!mAllocatedMemory) {
        LOG2("@%s Memory(in %p) is not allocated by CameraBuffer class. Don't free it.", __func__, this);
        return ;
    }

    switch(mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            freeUserPtr();
            break;
        case V4L2_MEMORY_MMAP:
            freeMmap();
            break;
        default:
            LOGE("Free camera buffer failed, due to memory %d type is not implemented yet.", mV.Memory());
    }
}

void CameraBuffer::updateUserBuffer(void)
{
    mU->timestamp = TIMEVAL2NSECS(getTimestamp());
    mU->s.field = getField();

    // Use valid setting sequence to align shutter/parameter with buffer
    mU->sequence = (mSettingSequence < 0) ? getSequence() : mSettingSequence;
}

void CameraBuffer::updateFlags(void)
{
    int flag = V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN;
    bool set = true;

    //clear the flags if the buffers is accessed by the SW
    if ((mU->flags & BUFFER_FLAG_SW_READ) || (mU->flags & BUFFER_FLAG_SW_WRITE)) {
        set = false;
    }

    mV.SetFlags(set ? (mV.Flags() | flag): (mV.Flags() & (~flag)));
}

bool CameraBuffer::isFlagsSet(int flag)
{
    return ((mU->flags & flag) ? true : false);
}

void CameraBuffer::setFd(int val, int plane)
{
    if (mV.Memory() == V4L2_MEMORY_MMAP) {
        mDmaFd[plane] = val;
    } else {
        mV.SetFd(val, plane);
    }
}

int CameraBuffer::getFd(int plane)
{
    if (mV.Memory() == V4L2_MEMORY_MMAP) {
        return mDmaFd[plane];
    }

    return mV.Fd(plane);
}

}//namespace icamera
