/*
 * Copyright (C) 2013-2021 Intel Corporation
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

// Copyright 2018 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define LOG_TAG V4l2_device_cc

#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <v4l2_device.h>

#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
using namespace icamera::Log;
using namespace icamera;

namespace cros {
V4L2Device::V4L2Device(const std::string& name) : name_(name), fd_(-1) {}

V4L2Device::~V4L2Device() {
    LOG1("@%s %s", __func__, name_.c_str());
    if (IsOpened()) {
        Close();
    }
}

int V4L2Device::Open(int flags) {
    LOG1("@%s", __func__);

    if (IsOpened()) {
        return 0;
    }

    struct stat st = {};
    if (stat(name_.c_str(), &st) == -1) {
        LOGE("%s: Failed to stat device node %s %s", __func__, name_.c_str(), strerror(errno));
        return -ENODEV;
    }
    if (!S_ISCHR(st.st_mode)) {
        LOGE("%s: Device node is not chardev %s %s", __func__, name_.c_str(), strerror(errno));
        return -ENODEV;
    }

    fd_ = ::open(name_.c_str(), flags);
    if (fd_ < 0) {
        LOGE("%s: Failed to open device node %s %s", __func__, name_.c_str(), strerror(errno));
        return -errno;
    }
    return 0;
}

int V4L2Device::Close() {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }

    int ret = ::close(fd_);
    if (ret < 0) {
        LOGE("%s: Cannot close device node %s %s", __func__, name_.c_str(), strerror(errno));
        return ret;
    }

    fd_ = -1;
    return 0;
}

int V4L2Device::SubscribeEvent(int event) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    struct v4l2_event_subscription sub = {};
    sub.type = event;
    int ret = ::ioctl(fd_, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_SUBSCRIBE_EVENT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Device::SubscribeEvent(int event, int id) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    struct v4l2_event_subscription sub = {};
    sub.type = event;
    sub.id = id;
    int ret = ::ioctl(fd_, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_SUBSCRIBE_EVENT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Device::UnsubscribeEvent(int event) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    struct v4l2_event_subscription sub = {};
    sub.type = event;

    int ret = ::ioctl(fd_, VIDIOC_UNSUBSCRIBE_EVENT, &sub);

    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_UNSUBSCRIBE_EVENT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Device::UnsubscribeEvent(int event, int id) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    struct v4l2_event_subscription sub = {};
    sub.type = event;
    sub.id = id;

    int ret = ::ioctl(fd_, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_UNSUBSCRIBE_EVENT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Device::DequeueEvent(struct v4l2_event* event) {
    LOG1("@%s", __func__);

    if (!event) {
        LOGE("%s: Device node %s event is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    int ret = ::ioctl(fd_, VIDIOC_DQEVENT, event);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_DQEVENT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Device::SetControl(struct v4l2_control* control) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }
    if (!control) {
        LOGE("%s: Device node %s control is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }
    return ::ioctl(fd_, VIDIOC_S_CTRL, control);
}

int V4L2Device::SetControl(struct v4l2_ext_control* ext_control) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }
    if (!ext_control) {
        LOGE("%s: Device node %s ext_control is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }
    struct v4l2_ext_controls controls = {};
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_control->id);
    controls.count = 1;
    controls.controls = ext_control;
    return ::ioctl(fd_, VIDIOC_S_EXT_CTRLS, &controls);
}

int V4L2Device::SetControl(int id, int32_t value) {
    LOG1("@%s", __func__);

    int ret = 0;

    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    ext_control.value = value;
    ret = SetControl(&ext_control);
    if (ret != 0) {
        LOGE("%s: Device node %s SetControl(int, int) error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2Device::SetControl(int id, int64_t value) {
    LOG1("@%s", __func__);

    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    ext_control.value64 = value;
    int ret = SetControl(&ext_control);
    if (ret != 0) {
        LOGE("%s: Device node %s SetControl(int, int64_t) error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2Device::SetControl(int id, const std::string& value) {
    LOG1("@%s", __func__);

    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    ext_control.string = const_cast<char*>(value.c_str());
    int ret = SetControl(&ext_control);
    if (ret != 0) {
        LOGE("%s: Device node %s SetControl(int, std::string) error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2Device::GetControl(struct v4l2_ext_control* ext_control) {
    LOG1("@%s", __func__);

    if (!IsOpened()) {
        LOGE("%s: Device node %s is not opened! %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }
    struct v4l2_ext_controls controls = {};
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_control->id);
    controls.count = 1;
    controls.controls = ext_control;

    int ret = ::ioctl(fd_, VIDIOC_G_EXT_CTRLS, &controls);
    if (ret != 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_G_EXT_CTRLS error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }
    return 0;
}

int V4L2Device::GetControl(int id, int32_t* value) {
    LOG1("@%s", __func__);

    if (!value) {
        LOGE("%s: Device node %s value is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }
    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    int ret = GetControl(&ext_control);
    if (ret == 0) {
        *value = ext_control.value;
    }
    return ret;
}

int V4L2Device::GetControl(int id, int64_t* value) {
    LOG1("@%s", __func__);

    if (!value) {
        LOGE("%s: Device node %s value is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }
    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    int ret = GetControl(&ext_control);
    if (ret == 0) {
        *value = ext_control.value64;
    }
    return ret;
}

int V4L2Device::GetControl(int id, std::string* value) {
    LOG1("@%s", __func__);

    if (!value) {
        LOGE("%s: Device node %s value is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }
    struct v4l2_ext_control ext_control = {};
    ext_control.id = id;
    int ret = GetControl(&ext_control);
    if (ret == 0) {
        *value = ext_control.string;
    }
    return ret;
}

int V4L2Device::QueryMenu(v4l2_querymenu* menu) {
    LOG1("@%s", __func__);

    if (!menu) {
        LOGE("%s: Device node %s menu is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }

    if (fd_ == -1) {
        LOGE("%s: Device node %s fd is -1: %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }

    int ret = ::ioctl(fd_, VIDIOC_QUERYMENU, menu);
    if (ret != 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_QUERYMENU error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2Device::QueryControl(v4l2_queryctrl* control) {
    LOG1("@%s", __func__);

    if (!control) {
        LOGE("%s: Device node %s control is nullptr", __func__, name_.c_str());
        return -EINVAL;
    }

    if (fd_ == -1) {
        LOGE("%s: Device node %s fd is -1: %s", __func__, name_.c_str(), strerror(errno));
        return -EINVAL;
    }

    int ret = ::ioctl(fd_, VIDIOC_QUERYCTRL, control);
    if (ret != 0) {
        LOGW("%s: Device node %s IOCTL VIDIOC_QUERYCTRL error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2Device::Poll(int timeout) {
    LOG1("@%s", __func__);

    struct pollfd pfd = {0};
    int ret(0);

    if (fd_ == -1) {
        LOGE("%s: Device node %s fd is -1: %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    pfd.fd = fd_;
    pfd.events = POLLPRI | POLLIN | POLLERR;

    ret = ::poll(&pfd, 1, timeout);

    if (ret < 0) {
        LOGE("%s: Device node %s poll error: %s", __func__, name_.c_str(), strerror(errno));
        return ret;
    }

    if (pfd.revents & POLLERR) {
        LOGE("%s: Device node %s poll POLLERR rcvd: %s", __func__, name_.c_str(), strerror(errno));
        return -1;
    }

    return ret;
}

V4L2DevicePoller::V4L2DevicePoller(const std::vector<V4L2Device*>& devices, int flush_fd)
        : devices_(devices),
          flush_fd_(flush_fd),
          poll_fds_(devices_.size() + ((flush_fd == -1) ? 0 : 1)) {
    LOG1("@%s", __func__);

    for (size_t i = 0; i < devices_.size(); i++) {
        if (!devices_[i]) {
            LOGE("%s: device_ index error.", __func__);
            poll_fds_.resize(0);
            return;
        }
        poll_fds_[i].fd = devices_[i]->fd_;
        poll_fds_[i].revents = 0;
    }
    if (flush_fd_ != -1) {
        poll_fds_.back().fd = flush_fd_;
        poll_fds_.back().events = POLLIN | POLLPRI;
    }
}

int V4L2DevicePoller::Poll(int timeout_ms, int events, std::vector<V4L2Device*>* ready_devices) {
    LOG1("@%s", __func__);

    if (poll_fds_.empty()) {
        return -EINVAL;
    }
    for (size_t i = 0; i < devices_.size(); i++) {
        poll_fds_[i].events = events;
    }
    int ret = ::poll(poll_fds_.data(), poll_fds_.size(), timeout_ms);
    if (ret <= 0) {
        for (size_t i = 0; i < devices_.size(); i++) {
            LOGE("%s: Device node fd %d poll timeout.", __func__, devices_[i]->fd_);
        }
        return ret;
    }

    // check first the flush
    if (flush_fd_ != -1 && (poll_fds_.back().revents & (POLLIN | POLLPRI))) {
        LOG1("%s: Device node fd %d poll return from flush.", __func__, poll_fds_.back().fd);
        return ret;
    }

    bool is_pollerr = false;
    for (size_t i = 0; i < devices_.size(); i++) {
        if (poll_fds_[i].revents & POLLERR) {
            LOGE("%s: Device node fd %d poll POLLERR rcvd.", __func__, poll_fds_[i].fd);
            is_pollerr = true;
        }
    }
    if (is_pollerr) {
        return -1;
    }

    if (ready_devices != nullptr) {
        // check other active devices.
        for (size_t i = 0; i < devices_.size(); i++) {
            // return nodes that have data available
            if (poll_fds_[i].revents & events) {
                ready_devices->push_back(devices_[i]);
            }
        }
    }
    return ret;
}
}  // namespace cros
