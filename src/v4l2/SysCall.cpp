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

#define LOG_TAG "SysCall"

#include "iutils/CameraLog.h"
#include "SysCall.h"

namespace icamera {

static int sCreatedCount = 0;
bool SysCall::sIsInitialized = false;
SysCall *SysCall::sInstance = nullptr;
//Guard for singleton instance creation
Mutex SysCall::sLock;

/*static*/ SysCall*
SysCall::getInstance()
{
    AutoMutex lock(sLock);
    if (!sIsInitialized) {
        //Use real sys call as default
        sInstance = new SysCall();
        sIsInitialized = true;
    }
    return sInstance;
}

void SysCall::updateInstance(SysCall* newSysCall)
{
    LOG1("%s", __func__);
    AutoMutex lock(sLock);
    if (sIsInitialized) {
        sIsInitialized = false;
    }
    sInstance = newSysCall;
    if (newSysCall != nullptr)
        sIsInitialized = true;
}

SysCall::SysCall()
{
    sCreatedCount++;
    LOG1("Syscall was created %d time", sCreatedCount);
}

SysCall::~SysCall()
{
    sCreatedCount--;
    LOG1("Syscall was destructed %d time", sCreatedCount);
}

int SysCall::open(const char *pathname, int flags)
{
    return ::open(pathname, flags);
}

int SysCall::close(int fd)
{
    return ::close(fd);
}

void *SysCall::mmap(void *addr, size_t len, int prot, int flag, int filedes, off_t off)
{
    return ::mmap(addr, len, prot, flag, filedes, off);
}

int SysCall::munmap(void *addr, size_t len)
{
    return ::munmap(addr, len);
}

int SysCall::ioctl(int fd, int request, struct media_device_info *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct media_link_desc *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct media_links_enum *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct media_links_desc *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct media_entity_desc *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_capability *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, v4l2_fmtdesc *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, enum v4l2_buf_type *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_format *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_requestbuffers *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_buffers *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_buffer *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_subdev_format *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_subdev_stream *arg)
{
    return ioctl(fd, request, (void *)arg);
}
int SysCall::ioctl(int fd, int request, struct v4l2_streamon_info *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_ext_controls *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_control *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_queryctrl *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_subdev_selection *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_subdev_routing *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_querymenu *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_event_subscription *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_event *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, struct v4l2_exportbuffer *arg)
{
    return ioctl(fd, request, (void *)arg);
}

int SysCall::ioctl(int fd, int request, void *arg)
{
    int ret = 0;
    do {
        ret = ::ioctl(fd, request, arg);
    } while (-1 == ret && EINTR == errno);

    return ret;
}

int SysCall::poll(struct pollfd *pfd, nfds_t nfds, int timeout)
{
    int ret = 0;
    do {
        ret = ::poll(pfd, nfds, timeout);
    } while (-1 == ret && EINTR == errno);

    return ret;
}

} //namespace icamera
