/*
 * Copyright (C) 2015-2018 Intel Corporation.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/media.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

#include "iutils/Thread.h"

namespace icamera {

class SysCall {
protected:
    SysCall();
    virtual ~SysCall();

public:
    virtual int open(const char *pathname, int flags);
    virtual int close(int fd);
    virtual void *mmap(void *addr, size_t len, int prot, int flag, int filedes, off_t off);
    virtual int munmap(void *addr, size_t len);

    virtual int ioctl(int fd, int request, struct media_device_info *arg);
    virtual int ioctl(int fd, int request, struct media_link_desc *arg);
    virtual int ioctl(int fd, int request, struct media_links_enum *arg);
    virtual int ioctl(int fd, int request, struct media_links_desc *arg);
    virtual int ioctl(int fd, int request, struct media_entity_desc *arg);
    virtual int ioctl(int fd, int request, struct v4l2_capability *arg);
    virtual int ioctl(int fd, int request, v4l2_fmtdesc *arg);
    virtual int ioctl(int fd, int request, enum v4l2_buf_type *arg);
    virtual int ioctl(int fd, int request, struct v4l2_format *arg);
    virtual int ioctl(int fd, int request, struct v4l2_requestbuffers *arg);
    virtual int ioctl(int fd, int request, struct v4l2_buffers *arg);
    virtual int ioctl(int fd, int request, struct v4l2_buffer *arg);
    virtual int ioctl(int fd, int request, struct v4l2_subdev_format *arg);
    virtual int ioctl(int fd, int request, struct v4l2_subdev_stream *arg);
    virtual int ioctl(int fd, int request, struct v4l2_streamon_info *arg);
    virtual int ioctl(int fd, int request, struct v4l2_ext_controls *arg);
    virtual int ioctl(int fd, int request, struct v4l2_control *arg);
    virtual int ioctl(int fd, int request, struct v4l2_queryctrl *arg);
    virtual int ioctl(int fd, int request, struct v4l2_subdev_selection *arg);
    virtual int ioctl(int fd, int request, struct v4l2_subdev_routing *arg);
    virtual int ioctl(int fd, int request, struct v4l2_querymenu *arg);
    virtual int ioctl(int fd, int request, struct v4l2_event_subscription *arg);
    virtual int ioctl(int fd, int request, struct v4l2_event *arg);
    virtual int ioctl(int fd, int request, struct v4l2_exportbuffer *arg);

    virtual int poll(struct pollfd *pfd, nfds_t nfds, int timeout);

private:
        int ioctl(int fd, int request, void *arg);

public:
    static SysCall* getInstance();
    static void updateInstance(SysCall* newSysCall);
private:
    SysCall&   operator=(const SysCall&);         //Don't call me

    static bool sIsInitialized;
    static SysCall *sInstance;
    static Mutex sLock;
};

} //namespace icamera
