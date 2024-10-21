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

#define LOG_TAG V4l2_subdevice_cc

#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

using namespace icamera::Log;
using namespace icamera;

#include <sys/ioctl.h>
#include <v4l2_device.h>

namespace cros {

V4L2Subdevice::V4L2Subdevice(const std::string& name)
        : V4L2Device(name),
          state_(SubdevState::CLOSED) {
    LOG1("@%s", __func__);
}

V4L2Subdevice::~V4L2Subdevice() {
    LOG1("@%s", __func__);
    if (state_ == SubdevState::CLOSED) return;
    Close();
}

int V4L2Subdevice::Open(int flags) {
    LOG1("@%s", __func__);

    int status = V4L2Device::Open(flags);
    if (status == 0) state_ = SubdevState::OPEN;
    return status;
}

int V4L2Subdevice::Close() {
    LOG1("@%s", __func__);

    int status = V4L2Device::Close();
    state_ = (status == 0) ? SubdevState::CLOSED : SubdevState::ERROR;
    return status;
}

int V4L2Subdevice::SetFormat(const struct v4l2_subdev_format& format) {
    LOG1("@%s", __func__);

    if ((state_ != SubdevState::OPEN) && (state_ != SubdevState::CONFIGURED)) {
        LOGE("%s: Invaild Device status: %d", __func__, state_);
        return -EINVAL;
    }

    int ret = ::ioctl(fd_, VIDIOC_SUBDEV_S_FMT, &format);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_SUBDEV_S_FMT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return -EINVAL;
    }

    state_ = SubdevState::CONFIGURED;
    return 0;
}

int V4L2Subdevice::GetFormat(struct v4l2_subdev_format* format) {
    LOG1("@%s", __func__);

    if ((state_ != SubdevState::OPEN) && (state_ != SubdevState::CONFIGURED)) {
        LOGE("%s: Invaild Device status: %d", __func__, state_);
        return -EINVAL;
    }

    int ret = ::ioctl(fd_, VIDIOC_SUBDEV_G_FMT, format);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_SUBDEV_G_FMT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return -EINVAL;
    }

    return 0;
}

int V4L2Subdevice::GetPadFormat(int pad_index, int* width, int* height, int* code) {
    LOG1("@%s", __func__);

    if (!width || !height || !code) {
        LOGE("%s: Device node %s some of parameters is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }
    struct v4l2_subdev_format format = {};

    format.pad = pad_index;
    format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    int ret = GetFormat(&format);
    if (ret == 0) {
        *width = format.format.width;
        *height = format.format.height;
        *code = format.format.code;
    }
    return ret;
}

int V4L2Subdevice::SetSelection(const struct v4l2_subdev_selection& selection) {
    LOG1("@%s", __func__);

    if ((state_ != SubdevState::OPEN) && (state_ != SubdevState::CONFIGURED)) {
        LOGE("%s: Invaild Device status: %d", __func__, state_);
        return -EINVAL;
    }

    int ret = ::ioctl(fd_, VIDIOC_SUBDEV_S_SELECTION, &selection);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_SUBDEV_S_SELECTION error: %s", __func__,
             name_.c_str(), strerror(errno));
    }
    return ret;
}

int V4L2Subdevice::SetRouting(v4l2_subdev_route* routes, uint32_t numRoutes) {
    LOG1("@%s", __func__);

    if (!routes) {
        LOGE("%s: Device node %s routes is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }

    v4l2_subdev_routing r = {routes, numRoutes};

    int ret = ::ioctl(fd_, VIDIOC_SUBDEV_S_ROUTING, &r);
    if (ret < 0) {
        LOG1("%s: Device node %s IOCTL VIDIOC_SUBDEV_S_ROUTING error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return ret;
}

int V4L2Subdevice::GetRouting(v4l2_subdev_route* routes, uint32_t* numRoutes) {
    LOG1("@%s", __func__);

    if (!routes || !numRoutes) {
        LOGE("%s: Device node %s routes or numRoutes is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }

    v4l2_subdev_routing r = {routes, *numRoutes};

    int ret = ::ioctl(fd_, VIDIOC_SUBDEV_G_ROUTING, &r);
    if (ret < 0) {
        LOG1("%s: Device node %s IOCTL VIDIOC_SUBDEV_G_ROUTING error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    *numRoutes = r.num_routes;

    return ret;
}

}  // namespace cros
