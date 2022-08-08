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

#define LOG_TAG V4l2_video_node_cc

#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

using namespace icamera::Log;
using namespace icamera;

#include <fcntl.h>
#include <limits.h>
#include <linux/media.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <v4l2_device.h>

namespace cros {

#define V4L2_TYPE_IS_META(type) \
    ((type) == V4L2_BUF_TYPE_META_OUTPUT || (type) == V4L2_BUF_TYPE_META_CAPTURE)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

V4L2Buffer::V4L2Buffer() : v4l2_buf_{} {
    LOG1("@%s", __func__);

    v4l2_buf_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    planes_.resize(VIDEO_MAX_PLANES);
    v4l2_buf_.m.planes = planes_.data();
    v4l2_buf_.length = planes_.size();
}

V4L2Buffer::V4L2Buffer(const V4L2Buffer& buf) : v4l2_buf_(buf.v4l2_buf_) {
    LOG1("@%s", __func__);

    if (V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type)) {
        planes_ = buf.planes_;
        v4l2_buf_.m.planes = planes_.data();
    }
}

void V4L2Buffer::SetType(uint32_t type) {
    LOG1("@%s", __func__);

    v4l2_buf_.type = type;
}

uint32_t V4L2Buffer::Offset(uint32_t plane) const {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    return mp ? v4l2_buf_.m.planes[plane].m.mem_offset : v4l2_buf_.m.offset;
}

void V4L2Buffer::SetOffset(uint32_t offset, uint32_t plane) {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    if (mp)
        v4l2_buf_.m.planes[plane].m.mem_offset = offset;
    else
        v4l2_buf_.m.offset = offset;
}

uintptr_t V4L2Buffer::Userptr(uint32_t plane) const {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    return mp ? v4l2_buf_.m.planes[plane].m.userptr : v4l2_buf_.m.userptr;
}

void V4L2Buffer::SetUserptr(uintptr_t userptr, uint32_t plane) {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    if (mp)
        v4l2_buf_.m.planes[plane].m.userptr = userptr;
    else
        v4l2_buf_.m.userptr = userptr;
}

int V4L2Buffer::RequestFd() const {
    LOG1("@%s", __func__);

    return (v4l2_buf_.flags & V4L2_BUF_FLAG_REQUEST_FD) ? v4l2_buf_.request_fd : -1;
}

int V4L2Buffer::SetRequestFd(int fd) {
    LOG1("@%s", __func__);

    if (fd <= 0) {
        return -EINVAL;
    }

    v4l2_buf_.flags |= V4L2_BUF_FLAG_REQUEST_FD;
    v4l2_buf_.request_fd = fd;

    return 0;
}

int V4L2Buffer::ResetRequestFd() {
    LOG1("@%s", __func__);

    v4l2_buf_.flags &= ~V4L2_BUF_FLAG_REQUEST_FD;
    v4l2_buf_.request_fd = 0;

    return 0;
}

int V4L2Buffer::Fd(uint32_t plane) const {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    return mp ? v4l2_buf_.m.planes[plane].m.fd : v4l2_buf_.m.fd;
}

void V4L2Buffer::SetFd(int fd, uint32_t plane) {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    if (mp)
        v4l2_buf_.m.planes[plane].m.fd = fd;
    else
        v4l2_buf_.m.fd = fd;
}

uint32_t V4L2Buffer::BytesUsed(uint32_t plane) const {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    return mp ? v4l2_buf_.m.planes[plane].bytesused : v4l2_buf_.bytesused;
}

void V4L2Buffer::SetBytesUsed(uint32_t bytesused, uint32_t plane) {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    if (mp)
        v4l2_buf_.m.planes[plane].bytesused = bytesused;
    else
        v4l2_buf_.bytesused = bytesused;
}

uint32_t V4L2Buffer::Length(uint32_t plane) const {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    return mp ? v4l2_buf_.m.planes[plane].length : v4l2_buf_.length;
}

void V4L2Buffer::SetLength(uint32_t length, uint32_t plane) {
    LOG1("@%s", __func__);

    bool mp = V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type);

    if (mp)
        v4l2_buf_.m.planes[plane].length = length;
    else
        v4l2_buf_.length = length;
}

V4L2Buffer& V4L2Buffer::operator=(const V4L2Buffer& buf) {
    LOG1("@%s", __func__);

    v4l2_buf_ = buf.v4l2_buf_;
    if (V4L2_TYPE_IS_MULTIPLANAR(v4l2_buf_.type)) {
        planes_ = buf.planes_;
        v4l2_buf_.m.planes = planes_.data();
    }
    return *this;
}

V4L2Format::V4L2Format()
        : type_(0),
          width_(0),
          height_(0),
          pixel_fmt_(0),
          field_(V4L2_FIELD_NONE),
          color_space_(0),
          quantization_(0),
          v4l2_fmt_() {}

V4L2Format::V4L2Format(const v4l2_format& fmt) {
    LOG1("@%s", __func__);

    type_ = fmt.type;
    if (V4L2_TYPE_IS_META(fmt.type)) {
        pixel_fmt_ = fmt.fmt.meta.dataformat;
        plane_size_image_.push_back(fmt.fmt.meta.buffersize);
    } else if (V4L2_TYPE_IS_MULTIPLANAR(fmt.type)) {
        width_ = fmt.fmt.pix_mp.width;
        height_ = fmt.fmt.pix_mp.height;
        pixel_fmt_ = fmt.fmt.pix_mp.pixelformat;
        field_ = fmt.fmt.pix_mp.field;
        color_space_ = fmt.fmt.pix_mp.colorspace;
        quantization_ = fmt.fmt.pix_mp.quantization;
        for (uint8_t plane = 0; plane < fmt.fmt.pix_mp.num_planes; plane++) {
            plane_bytes_per_line_.push_back(fmt.fmt.pix_mp.plane_fmt[plane].bytesperline);
            plane_size_image_.push_back(fmt.fmt.pix_mp.plane_fmt[plane].sizeimage);
        }
    } else {
        width_ = fmt.fmt.pix.width;
        height_ = fmt.fmt.pix.height;
        pixel_fmt_ = fmt.fmt.pix.pixelformat;
        field_ = fmt.fmt.pix.field;
        color_space_ = fmt.fmt.pix.colorspace;
        quantization_ = fmt.fmt.pix.quantization;
        plane_bytes_per_line_.push_back(fmt.fmt.pix.bytesperline);
        plane_size_image_.push_back(fmt.fmt.pix.sizeimage);
    }
}

void V4L2Format::SetType(uint32_t type) {
    LOG1("@%s", __func__);

    type_ = type;
}

uint32_t V4L2Format::Width() const {
    LOG1("@%s", __func__);
    return width_;
}

void V4L2Format::SetWidth(uint32_t width) {
    LOG1("@%s", __func__);

    width_ = width;
}

uint32_t V4L2Format::Height() const {
    LOG1("@%s", __func__);

    return height_;
}

void V4L2Format::SetHeight(uint32_t height) {
    LOG1("@%s", __func__);

    height_ = height;
}

uint32_t V4L2Format::PixelFormat() const {
    LOG1("@%s", __func__);

    return pixel_fmt_;
}

void V4L2Format::SetPixelFormat(uint32_t format) {
    LOG1("@%s", __func__);

    pixel_fmt_ = format;
}

uint32_t V4L2Format::Field() const {
    LOG1("@%s", __func__);

    return field_;
}

void V4L2Format::SetField(uint32_t field) {
    LOG1("@%s", __func__);

    field_ = field;
}

uint32_t V4L2Format::BytesPerLine(uint32_t plane) const {
    LOG1("@%s", __func__);

    return plane_bytes_per_line_[plane];
}

void V4L2Format::SetBytesPerLine(uint32_t bytesperline, uint32_t plane) {
    LOG1("@%s", __func__);

    if (plane >= VIDEO_MAX_PLANES) {
        return;
    }
    if (plane >= plane_bytes_per_line_.size()) {
        plane_bytes_per_line_.resize(plane + 1);
    }
    plane_bytes_per_line_[plane] = bytesperline;
}

uint32_t V4L2Format::SizeImage(uint32_t plane) const {
    LOG1("@%s", __func__);
    return plane_size_image_[plane];
}

void V4L2Format::SetSizeImage(uint32_t size, uint32_t plane) {
    LOG1("@%s", __func__);
    if (plane >= VIDEO_MAX_PLANES) {
        return;
    }
    if (plane >= plane_size_image_.size()) {
        plane_size_image_.resize(plane + 1);
    }
    plane_size_image_[plane] = size;
}

uint32_t V4L2Format::ColorSpace() const {
    LOG1("@%s", __func__);
    return color_space_;
}

void V4L2Format::SetColorSpace(uint32_t profile) {
    LOG1("@%s", __func__);

    color_space_ = profile;
}

uint32_t V4L2Format::Quantization() const {
    LOG1("@%s", __func__);

    return quantization_;
}

void V4L2Format::SetQuantization(uint32_t quantization) {
    LOG1("@%s", __func__);

    quantization_ = quantization;
}

v4l2_format* V4L2Format::Get() {
    LOG1("@%s", __func__);

    v4l2_fmt_.type = type_;
    if (V4L2_TYPE_IS_META(v4l2_fmt_.type)) {
        v4l2_fmt_.fmt.meta.dataformat = pixel_fmt_;
        v4l2_fmt_.fmt.meta.buffersize = plane_size_image_[0];
    } else if (V4L2_TYPE_IS_MULTIPLANAR(v4l2_fmt_.type)) {
        // plane_size_image_.size())
        v4l2_fmt_.fmt.pix_mp.width = width_;
        v4l2_fmt_.fmt.pix_mp.height = height_;
        v4l2_fmt_.fmt.pix_mp.pixelformat = pixel_fmt_;
        v4l2_fmt_.fmt.pix_mp.field = field_;
        v4l2_fmt_.fmt.pix_mp.colorspace = color_space_;
        v4l2_fmt_.fmt.pix_mp.quantization = quantization_;
        v4l2_fmt_.fmt.pix_mp.num_planes = plane_bytes_per_line_.size();
        for (size_t plane = 0; plane < plane_bytes_per_line_.size(); plane++) {
            v4l2_fmt_.fmt.pix_mp.plane_fmt[plane].bytesperline = plane_bytes_per_line_[plane];
        }
        for (size_t plane = 0; plane < plane_size_image_.size(); plane++) {
            v4l2_fmt_.fmt.pix_mp.plane_fmt[plane].sizeimage = plane_size_image_[plane];
        }
    } else {
        v4l2_fmt_.fmt.pix.width = width_;
        v4l2_fmt_.fmt.pix.height = height_;
        v4l2_fmt_.fmt.pix.pixelformat = pixel_fmt_;
        v4l2_fmt_.fmt.pix.field = field_;
        v4l2_fmt_.fmt.pix.colorspace = color_space_;
        v4l2_fmt_.fmt.pix.quantization = quantization_;
        v4l2_fmt_.fmt.pix.bytesperline = plane_bytes_per_line_[0];
        v4l2_fmt_.fmt.pix.sizeimage = plane_size_image_[0];
    }
    return &v4l2_fmt_;
}

V4L2VideoNode::V4L2VideoNode(const std::string& name)
        : V4L2Device(name),
          state_(VideoNodeState::CLOSED),
          is_buffer_cached_(false),
          buffer_type_(V4L2_BUF_TYPE_VIDEO_CAPTURE),
          memory_type_(V4L2_MEMORY_USERPTR) {
    LOG1("@%s", __func__);
}

V4L2VideoNode::~V4L2VideoNode() {
    LOG1("@%s", __func__);
    {
        if (state_ == VideoNodeState::CLOSED) {
            return;
        }
    }
    Close();
}

int V4L2VideoNode::Open(int flags) {
    LOG1("@%s", __func__);

    int ret = V4L2Device::Open(flags);
    if (ret != 0) {
        return ret;
    }

    struct v4l2_capability cap = {};
    ret = QueryCap(&cap);
    if (ret != 0) {
        LOGE("%s: QueryCap error.", __func__);
        V4L2Device::Close();
        return ret;
    }
    std::pair<uint32_t, enum v4l2_buf_type> buffer_type_mapper[] = {
        {V4L2_CAP_VIDEO_CAPTURE, V4L2_BUF_TYPE_VIDEO_CAPTURE},
        {V4L2_CAP_VIDEO_CAPTURE_MPLANE, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE},
        {V4L2_CAP_VIDEO_OUTPUT, V4L2_BUF_TYPE_VIDEO_OUTPUT},
        {V4L2_CAP_VIDEO_OUTPUT_MPLANE, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE},
        {V4L2_CAP_META_CAPTURE, V4L2_BUF_TYPE_META_CAPTURE},
        {V4L2_CAP_META_OUTPUT, V4L2_BUF_TYPE_META_OUTPUT}};
    size_t i = 0;
    for (; i < ARRAY_SIZE(buffer_type_mapper); i++) {
        if (cap.capabilities & buffer_type_mapper[i].first) {
            buffer_type_ = buffer_type_mapper[i].second;
            break;
        }
    }
    if (i == ARRAY_SIZE(buffer_type_mapper)) {
        V4L2Device::Close();
        LOGE("%s: ARRAY_SIZE error.", __func__);
        return -EINVAL;
    }

    state_ = VideoNodeState::OPEN;
    return ret;
}

int V4L2VideoNode::Close() {
    LOG1("@%s", __func__);

    if (state_ == VideoNodeState::STARTED || state_ == VideoNodeState::PREPARED) {
        StopLocked();
    }

    int ret = V4L2Device::Close();
    state_ = (ret == 0) ? VideoNodeState::CLOSED : VideoNodeState::ERROR;

    return ret;
}

enum v4l2_memory V4L2VideoNode::GetMemoryType() {
    LOG1("@%s", __func__);

    return memory_type_;
}

enum v4l2_buf_type V4L2VideoNode::GetBufferType() {
    LOG1("@%s", __func__);

    return buffer_type_;
}

int V4L2VideoNode::Stop(bool releaseBuffers) {
    LOG1("@%s", __func__);

    if (state_ != VideoNodeState::STARTED && state_ != VideoNodeState::PREPARED) {
        return -EINVAL;
    }
    return StopLocked(releaseBuffers);
}

int V4L2VideoNode::StopLocked(bool releaseBuffers) {
    LOG1("@%s", __func__);

    if (state_ == VideoNodeState::STARTED) {
        int ret = ::ioctl(fd_, VIDIOC_STREAMOFF, &buffer_type_);
        if (ret < 0) {
            LOGE("%s: Device node %s IOCTL VIDIOC_STREAMOFF error: %s", __func__, name_.c_str(),
                 strerror(errno));
            return ret;
        }
        state_ = VideoNodeState::PREPARED;
    }

    if (!releaseBuffers) {
        return 0;
    }

    if (state_ == VideoNodeState::PREPARED) {
        RequestBuffers(0, memory_type_);
        state_ = VideoNodeState::CONFIGURED;
    }

    return 0;
}

int V4L2VideoNode::Start() {
    LOG1("@%s", __func__);

    if (state_ != VideoNodeState::PREPARED) {
        return -1;
    }

    int ret = ::ioctl(fd_, VIDIOC_STREAMON, &buffer_type_);
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_STREAMON error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    state_ = VideoNodeState::STARTED;

    return 0;
}

int V4L2VideoNode::SetFormat(const V4L2Format& format) {
    LOG1("@%s", __func__);

    if ((state_ != VideoNodeState::OPEN) && (state_ != VideoNodeState::CONFIGURED) &&
        (state_ != VideoNodeState::PREPARED)) {
        return -EINVAL;
    }

    V4L2Format fmt(format);
    fmt.SetType(buffer_type_);

    if (V4L2_TYPE_IS_META(buffer_type_)) {
        fmt.SetSizeImage(0, 0);
    }

    int ret = ::ioctl(fd_, VIDIOC_S_FMT, fmt.Get());
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_S_FMT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    // Update current configuration with the new one
    format_ = fmt;

    state_ = VideoNodeState::CONFIGURED;
    return 0;
}

int V4L2VideoNode::SetSelection(const struct v4l2_selection& selection) {
    LOG1("@%s", __func__);

    if ((state_ != VideoNodeState::OPEN) && (state_ != VideoNodeState::CONFIGURED)) {
        LOGE("%s: State error. %d", __func__, state_);
        return -EINVAL;
    }

    struct v4l2_selection* sel = const_cast<struct v4l2_selection*>(&selection);
    sel->type = buffer_type_;

    int ret = ::ioctl(fd_, VIDIOC_S_SELECTION, sel);

    return ret;
}

int V4L2VideoNode::MapMemory(unsigned int index, int prot, int flags, std::vector<void*>* mapped) {
    LOG1("@%s", __func__);

    if ((state_ != VideoNodeState::OPEN) && (state_ != VideoNodeState::CONFIGURED) &&
        (state_ != VideoNodeState::PREPARED)) {
        LOGE("%s: State error. %d", __func__, state_);
        return -EINVAL;
    }
    if (memory_type_ != V4L2_MEMORY_MMAP) {
        LOGE("%s: memory_type error. %d", __func__, memory_type_);
        return -EINVAL;
    }
    if (!mapped) {
        LOGE("%s: mapper nullptr. ", __func__);
        return -EINVAL;
    }

    V4L2Buffer buffer;
    int ret = QueryBuffer(index, memory_type_, &buffer);
    if (ret < 0) {
        LOGE("%s: QueryBuffer error. %d", __func__, state_);
        state_ = VideoNodeState::ERROR;
        return ret;
    }
    uint32_t num_planes = V4L2_TYPE_IS_MULTIPLANAR(buffer.Type()) ? buffer.Get()->length : 1;
    for (uint32_t i = 0; i < num_planes; i++) {
        void* res = ::mmap(nullptr, buffer.Length(i), prot, flags, fd_, buffer.Offset(i));
        if (res == MAP_FAILED) {
            LOGE("%s: MMAP error. %d", __func__, strerror(errno));
            return -EINVAL;
        }
        mapped->push_back(res);
    }
    return 0;
}

int V4L2VideoNode::GrabFrame(V4L2Buffer* buf) {
    LOG1("@%s", __func__);

    if (state_ != VideoNodeState::STARTED) {
        LOGE("%s: State error. %d", __func__, state_);
        return -EINVAL;
    }
    if (!buf) {
        LOGE("%s: Device node %s buf is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }

    int ret = Dqbuf(buf);
    if (ret < 0) {
        LOGE("%s: DQBuf error. %d", __func__, ret);
        return ret;
    }

    PrintBufferInfo(__FUNCTION__, *buf);
    return buf->Index();
}

int V4L2VideoNode::PutFrame(V4L2Buffer* buf) {
    LOG1("@%s", __func__);

    int ret = Qbuf(buf);
    PrintBufferInfo(__FUNCTION__, *buf);

    return ret;
}

int V4L2VideoNode::ExportFrame(unsigned int index, std::vector<int>* fds) {
    LOG1("@%s", __func__);

    if (memory_type_ != V4L2_MEMORY_MMAP) {
        LOGE("%s: memory_type error. %d", __func__, memory_type_);
        return -EINVAL;
    }
    if (!fds) {
        LOGE("%s: Device node %s fds is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }

    V4L2Buffer buffer;
    int ret = QueryBuffer(index, memory_type_, &buffer);
    if (ret < 0) {
        LOGE("%s: QueryBuffer error. %d", __func__, ret);
        state_ = VideoNodeState::ERROR;
        return ret;
    }
    uint32_t num_planes = V4L2_TYPE_IS_MULTIPLANAR(buffer.Type()) ? buffer.Get()->length : 1;
    struct v4l2_exportbuffer ebuf = {};
    ebuf.type = buffer_type_;
    ebuf.index = index;
    ebuf.flags = O_RDWR;
    for (uint32_t i = 0; i < num_planes; i++) {
        ret = ::ioctl(fd_, VIDIOC_EXPBUF, &ebuf);
        if (ret < 0) {
            LOGE("%s: Device node %s IOCTL VIDIOC_EXPBUF error: %s", __func__, name_.c_str(),
                 strerror(errno));
            return ret;
        } else {
            fds->push_back(ebuf.fd);
        }
    }
    return 0;
}

int V4L2VideoNode::SetupBuffers(size_t num_buffers, bool is_cached, enum v4l2_memory memory_type,
                                std::vector<V4L2Buffer>* buffers) {
    LOG1("@%s", __func__);

    if (num_buffers == 0 || !buffers || !buffers->empty()) {
        LOGE("%s: Device node %s num_buffers or buffer invaild.", __func__, name_.c_str());
        return -EINVAL;
    }

    if ((state_ != VideoNodeState::CONFIGURED)) {
        LOGE("%s: State error. %d", __func__, state_);
        return -EINVAL;
    }

    int ret = RequestBuffers(num_buffers, memory_type);
    if (ret <= 0) {
        LOGE("%s: RequestBuffers error. %d", __func__, ret);
        return -EINVAL;
    }

    for (size_t i = 0; i < num_buffers; i++) {
        V4L2Buffer buffer;
        int ret = QueryBuffer(i, memory_type, &buffer);
        if (ret < 0) {
            LOGE("%s: QueryBuffer error. %d", __func__, ret);
            state_ = VideoNodeState::ERROR;
            return ret;
        }
        buffers->push_back(std::move(buffer));
    }

    is_buffer_cached_ = is_cached;
    memory_type_ = memory_type;
    state_ = VideoNodeState::PREPARED;
    return 0;
}

int V4L2VideoNode::QueryCap(struct v4l2_capability* cap) {
    LOG1("@%s", __func__);

    int ret = ::ioctl(fd_, VIDIOC_QUERYCAP, cap);

    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_QUERYCAP error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    return 0;
}

int V4L2VideoNode::RequestBuffers(size_t num_buffers, enum v4l2_memory memory_type) {
    LOG1("@%s", __func__);

    if (state_ == VideoNodeState::CLOSED) return 0;

    struct v4l2_requestbuffers req_buf = {};
    req_buf.memory = memory_type;
    req_buf.count = num_buffers;
    req_buf.type = buffer_type_;

    int ret = ::ioctl(fd_, VIDIOC_REQBUFS, &req_buf);

    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_REQBUFS error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    memory_type_ = memory_type;
    state_ = VideoNodeState::PREPARED;
    return req_buf.count;
}

void V4L2VideoNode::PrintBufferInfo(const std::string& func, const V4L2Buffer& buf) {
    LOG1("@%s", __func__);

    switch (memory_type_) {
        case V4L2_MEMORY_USERPTR:
            LOG1("%s: idx: %ud, addr: %p", func.c_str(), buf.Index(), buf.Userptr(0));
            break;
        case V4L2_MEMORY_MMAP:
            LOG1("%s: idx: %ud, offset: %p", func.c_str(), buf.Index(), buf.Offset(0));
            break;
        case V4L2_MEMORY_DMABUF:
            LOG1("%s: idx: %ud, fd: %d", func.c_str(), buf.Index(), buf.Fd(0));
            break;
        default:
            break;
    }
}

int V4L2VideoNode::Qbuf(V4L2Buffer* buf) {
    LOG1("@%s", __func__);

    int ret = ::ioctl(fd_, VIDIOC_QBUF, buf->Get());
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_QBUF error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2VideoNode::Dqbuf(V4L2Buffer* buf) {
    LOG1("@%s", __func__);

    buf->SetMemory(memory_type_);
    buf->SetType(buffer_type_);

    int ret = ::ioctl(fd_, VIDIOC_DQBUF, buf->Get());
    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_DQBUF error: %s", __func__, name_.c_str(),
             strerror(errno));
    }
    return ret;
}

int V4L2VideoNode::QueryBuffer(int index, enum v4l2_memory memory_type, V4L2Buffer* buf) {
    LOG1("@%s", __func__);

    buf->SetFlags(0x0);
    buf->SetMemory(memory_type);
    buf->SetType(buffer_type_);
    buf->SetIndex(index);
    int ret = ::ioctl(fd_, VIDIOC_QUERYBUF, buf->Get());

    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_QUERYBUF error: %s", __func__, name_.c_str(),
             strerror(errno));
        return ret;
    }

    LOG1("Device: name: %s, index %ud, type: %ud, bytesused: %ud, flags: 0x%x", name_.c_str(),
         buf->Index(), buf->Type(), buf->BytesUsed(0), buf->Flags());
    if (memory_type == V4L2_MEMORY_MMAP) {
        LOG1("memory MMAP: offset 0x%p", buf->Offset(0));
    } else if (memory_type == V4L2_MEMORY_USERPTR) {
        LOG1("memory USRPTR: %p", buf->Userptr(0));
    }
    LOG1("length: %ud", buf->Length(0));
    return 0;
}

int V4L2VideoNode::GetFormat(V4L2Format* format) {
    LOG1("@%s", __func__);

    if (!format) {
        LOGE("%s: Device node %s format is nullptr: %s", __func__, name_.c_str());
        return -EINVAL;
    }

    if ((state_ != VideoNodeState::OPEN) && (state_ != VideoNodeState::CONFIGURED)) {
        LOGE("%s: State error. %d", __func__, state_);
        return -EINVAL;
    }

    v4l2_format fmt;
    fmt.type = buffer_type_;
    int ret = ::ioctl(fd_, VIDIOC_G_FMT, &fmt);

    if (ret < 0) {
        LOGE("%s: Device node %s IOCTL VIDIOC_G_FMT error: %s", __func__, name_.c_str(),
             strerror(errno));
        return -EINVAL;
    }

    *format = V4L2Format(fmt);

    return 0;
}

}  // namespace cros
