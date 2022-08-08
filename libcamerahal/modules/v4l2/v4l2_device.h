/*
 * Copyright (C) 2013-2020 Intel Corporation
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

#ifndef CAMERA_INCLUDE_CROS_CAMERA_V4L2_DEVICE_H_
#define CAMERA_INCLUDE_CROS_CAMERA_V4L2_DEVICE_H_

#include <errno.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <poll.h>

#include <atomic>
#include <string>
#include <vector>

#define CROS_CAMERA_EXPORT __attribute__((visibility("default")))

namespace cros {

/*
 * Wrapper for v4l2_buffer to provide compatible
 * interfaces for multi-plane buffers.
 */
class CROS_CAMERA_EXPORT V4L2Buffer {
 public:
    V4L2Buffer();
    explicit V4L2Buffer(const V4L2Buffer& buf);
    uint32_t Index() const { return v4l2_buf_.index; }
    void SetIndex(uint32_t index) { v4l2_buf_.index = index; }
    uint32_t Type() const { return v4l2_buf_.type; }
    void SetType(uint32_t type);
    uint32_t Flags() const { return v4l2_buf_.flags; }
    void SetFlags(uint32_t flags) { v4l2_buf_.flags = flags; }
    uint32_t Field() const { return v4l2_buf_.field; }
    void SetField(uint32_t field) { v4l2_buf_.field = field; }
    struct timeval Timestamp() const {
        return v4l2_buf_.timestamp;
    }
    void SetTimestamp(struct timeval timestamp) { v4l2_buf_.timestamp = timestamp; }
    struct v4l2_timecode Timecode() const {
        return v4l2_buf_.timecode;
    }
    void SetTimecode(struct v4l2_timecode timecode) { v4l2_buf_.timecode = timecode; }
    uint32_t Sequence() const { return v4l2_buf_.sequence; }
    void SetSequence(uint32_t sequence) { v4l2_buf_.sequence = sequence; }
    uint32_t Memory() const { return v4l2_buf_.memory; }
    void SetMemory(uint32_t memory) { v4l2_buf_.memory = memory; }
    uint32_t Offset(uint32_t plane) const;
    void SetOffset(uint32_t offset, uint32_t plane);
    uintptr_t Userptr(uint32_t plane) const;
    void SetUserptr(uintptr_t userptr, uint32_t plane);
    int RequestFd() const;
    int SetRequestFd(int fd);
    int ResetRequestFd();
    int Fd(uint32_t plane) const;
    void SetFd(int fd, uint32_t plane);
    uint32_t BytesUsed(uint32_t plane) const;
    void SetBytesUsed(uint32_t bytesused, uint32_t plane);
    uint32_t Length(uint32_t plane) const;
    void SetLength(uint32_t length, uint32_t plane);
    const struct v4l2_buffer* Get() const { return &v4l2_buf_; }
    V4L2Buffer& operator=(const V4L2Buffer& buf);

 private:
    struct v4l2_buffer v4l2_buf_;
    std::vector<struct v4l2_plane> planes_;  // For multi-planar buffers.
};

/*
 * Wrapper for v4l2_format to provide compatible
 * interfaces for multi-plane buffers.
 */
class CROS_CAMERA_EXPORT V4L2Format {
 public:
    V4L2Format();
    explicit V4L2Format(const struct v4l2_format& fmt);
    explicit V4L2Format(const V4L2Format& fmt) = default;
    V4L2Format& operator=(const V4L2Format& fmt) = default;
    uint32_t Type() const { return type_; }
    void SetType(uint32_t type);
    uint32_t Width() const;
    void SetWidth(uint32_t width);
    uint32_t Height() const;
    void SetHeight(uint32_t height);
    uint32_t PixelFormat() const;
    void SetPixelFormat(uint32_t format);
    uint32_t Field() const;
    void SetField(uint32_t field);
    uint32_t BytesPerLine(uint32_t plane) const;
    void SetBytesPerLine(uint32_t bytesperline, uint32_t plane);
    uint32_t SizeImage(uint32_t plane) const;
    void SetSizeImage(uint32_t size, uint32_t plane);
    uint32_t ColorSpace() const;
    void SetColorSpace(uint32_t profile);
    uint32_t Quantization() const;
    void SetQuantization(uint32_t quantization);
    struct v4l2_format* Get();

 private:
    uint32_t type_;
    uint32_t width_;
    uint32_t height_;
    uint32_t pixel_fmt_;
    uint32_t field_;
    uint32_t color_space_;
    uint32_t quantization_;
    std::vector<uint32_t> plane_bytes_per_line_;
    std::vector<uint32_t> plane_size_image_;
    struct v4l2_format v4l2_fmt_;
};

/**
 * A class encapsulating simple V4L2 device operations.
 *
 * Base class that contains common V4L2 operations used by video nodes and
 * subdevices. It provides a slightly higher interface than IOCTLS to access
 * the devices. It also stores:
 * - Name
 * - File descriptor
 */
class CROS_CAMERA_EXPORT V4L2Device {
 public:
    friend class V4L2DevicePoller;

    explicit V4L2Device(const std::string& name);

    virtual ~V4L2Device();

    // This method opens the V4L2 device.
    //
    // Args:
    //    |flags|: open flags, e.g. O_RDWR, O_NONBLOCK.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    virtual int Open(int flags);

    // This method closes the V4L2 device.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    virtual int Close();

    // These methods sets the control of V4L2 device.
    //
    // Args:
    //    |id|: control identifier.
    //    |value|: new value.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetControl(int id, int32_t value);
    int SetControl(int id, int64_t value);
    int SetControl(int id, const std::string& value);
    int SetControl(struct v4l2_control* control);

    // These methods gets the control of V4L2 device.
    //
    // Args:
    //    |id|: control identifier.
    //    |value|: current value.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int GetControl(int id, int32_t* value);
    int GetControl(int id, int64_t* value);
    int GetControl(int id, std::string* value);

    // This method enumerates the menu control items of V4L2 device.
    //
    // Args:
    //    |menu|: menu control.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int QueryMenu(v4l2_querymenu* menu);

    // This method enumerates the control of V4L2 device.
    //
    // Args:
    //    |control|: control.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int QueryControl(v4l2_queryctrl* control);

    // This method subscribes event of V4L2 device.
    //
    // Args:
    //    |event|: event type.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SubscribeEvent(int event);

    int SubscribeEvent(int event, int id);

    // This method unsubscribes event of V4L2 device.
    //
    // Args:
    //    |event|: event type.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int UnsubscribeEvent(int event);

    int UnsubscribeEvent(int event, int id);

    // This method dequeues event of V4L2 device.
    //
    // Args:
    //    |event|: V4L2 event.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int DequeueEvent(struct v4l2_event* event);

    // This method checks whether V4L2 device is opened.
    //
    // Returns:
    //    True if it is opened.
    bool IsOpened() { return fd_ != -1; }

    int Poll(int timeout);

    // This method gets the name of V4L2 device.
    //
    // Returns:
    //    Device name.
    const std::string Name() { return name_; }

 private:
    int SetControl(struct v4l2_ext_control* ext_control);

    int GetControl(struct v4l2_ext_control* ext_control);

 protected:
    std::string name_; /*!< path to device in file system, ex: /dev/video0 */

    int fd_; /*!< file descriptor obtained when device is open */
};

class CROS_CAMERA_EXPORT V4L2DevicePoller {
 public:
    // |flush_fd|: file descriptor of the pipe device that will be used to return
    // from Poll() in case of flush request, i.e., to abort poll before timeout
    explicit V4L2DevicePoller(const std::vector<V4L2Device*>& devices, int flush_fd);

    virtual ~V4L2DevicePoller() {}

    // This method polls the V4L2 device.
    //
    // Args:
    //    |timeout_ms|: the number of milliseconds that Poll() should block
    //      waiting for devices to become ready.
    //    |events|: a bit mask specifying the events the client is
    //      interested in.
    //    |ready_devices|: devices that become ready
    //
    // Returns:
    //    On success, a positive number is returned; this is the number of
    //    devices which have the specified events occurred. A value of 0
    //    indicates that the call timed out and no file descriptors were ready.
    //    On error, -1 is returned.
    int Poll(int timeout_ms, int events, std::vector<V4L2Device*>* ready_devices);

 private:
    std::vector<V4L2Device*> devices_;

    int flush_fd_;

    std::vector<struct pollfd> poll_fds_;
};

/**
 * A class encapsulating simple V4L2 video device node operations.
 *
 * This class extends V4L2Device and adds extra internal state
 * and more convenience methods to manage an associated buffer pool
 * with the device.
 * This class introduces new methods specific to control video device nodes.
 */
class CROS_CAMERA_EXPORT V4L2VideoNode final : public V4L2Device {
 public:
    explicit V4L2VideoNode(const std::string& name);

    ~V4L2VideoNode();

    // This method opens the video device.
    //
    // Args:
    //    |flags|: open flags, e.g. O_RDWR, O_NONBLOCK.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Open(int flags) final;

    // This method closes the video device.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Close() final;

    // This method gets the data format of video device.
    //
    // Args:
    //    |format|: V4L2 format returned.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int GetFormat(V4L2Format* format);

    // This method configures the data format of video device.
    //
    // Args:
    //    |format|: V4L2 format.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetFormat(const V4L2Format& format);

    // This method configures the selection rectangles of video device.
    //
    // Args:
    //    |selection|: V4L2 selection rectangle.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetSelection(const struct v4l2_selection& selection);

    // This method get the memory type of video device.
    //
    // Returns:
    //    V4L2 memory type.
    enum v4l2_memory GetMemoryType();

    // This method get the buffer type of video device.
    //
    // Returns:
    //    V4L2 buffer type.
    enum v4l2_buf_type GetBufferType();

    // This method maps video device buffers into memory.
    //
    // Args:
    //    |index|: number of the buffer.
    //    |prot|: desired memory protection of the mapping.
    //    |flags|: mapping flags.
    //    |mapped|: mapped addresses.
    //
    // Returns:
    //    A pointer to the mapped area; MAP_FAILED on failure.
    int MapMemory(unsigned int index, int prot, int flags, std::vector<void*>* mapped);

    // This method sets up buffers of the video device. If USERPTR memory type is
    // specified, the caller may need to fill in the userptr field of returned
    // buffers.
    //
    // Args:
    //    |num_buffers|: number of buffers to set up
    //    |is_cached|: whether the buffers are cached or not
    //    |memory_type_|: memory type of buffers
    //    |buffers|: buffers returned
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetupBuffers(size_t num_buffers, bool is_cached, enum v4l2_memory memory_type,
                     std::vector<V4L2Buffer>* buffers);

    // This method stops streaming of the video device.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Stop(bool releaseBuffers = true);

    // This method starts streaming of the video device.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Start();

    // This method grabs a buffer from the video device's outgoing queue.
    //
    // Args:
    //    |buf|: V4L2 buffer.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int GrabFrame(V4L2Buffer* buf);

    // This method enqueues a buffer in the video device's outgoing queue.
    //
    // Args:
    //    |buf|: V4L2 buffer.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int PutFrame(V4L2Buffer* buf);

    // This method exports a buffer as DMABUF file descriptors.
    //
    // Args:
    //    |index|: number of the buffer.
    //    |fds|: exported file descriptors.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int ExportFrame(unsigned int index, std::vector<int>* fds);

    // This method queries the capabilities of the video device.
    //
    // Args:
    //    |cap|: V4L2 capabilities
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int QueryCap(struct v4l2_capability* cap);

 private:
    int Qbuf(V4L2Buffer* buf);

    int Dqbuf(V4L2Buffer* buf);

    int QueryBuffer(int index, enum v4l2_memory memory_type, V4L2Buffer* buf);

    int RequestBuffers(size_t num_buffers, enum v4l2_memory memory_type);

    int StopLocked(bool releaseBuffers = true);

    void DestroyBufferPool();

    void PrintBufferInfo(const std::string& func, const V4L2Buffer& buf);

    enum class VideoNodeState {
        CLOSED = 0, /*!< kernel device closed */
        OPEN,       /*!< device node opened */
        CONFIGURED, /*!< device format set, IOC_S_FMT */
        PREPARED,   /*!< device has requested buffers (set_buffer_pool)*/
        STARTED,    /*!< stream started, IOC_STREAMON */
        ERROR       /*!< undefined state */
    };

    // Lock to protect |state_|
    // base::Lock state_lock_;

    VideoNodeState state_;

    // Device capture configuration
    V4L2Format format_;

    bool is_buffer_cached_;

    enum v4l2_buf_type buffer_type_;

    enum v4l2_memory memory_type_;
};

/**
 * A class encapsulating simple V4L2 sub device node operations.
 *
 * Sub-devices are control points to the new V4L2 media controller
 * architecture.
 */
class CROS_CAMERA_EXPORT V4L2Subdevice final : public V4L2Device {
 public:
    explicit V4L2Subdevice(const std::string& name);

    ~V4L2Subdevice();

    // This method opens the sub-device.
    //
    // Args:
    //    |flags|: open flags, e.g. O_RDWR, O_NONBLOCK.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Open(int flags) final;

    // This method closes the sub-device.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int Close() final;

    // This method configures format of the sub-device.
    //
    // Args:
    //    |format|: V4L2 sub-device format.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetFormat(const struct v4l2_subdev_format& format);

    // This method configures the selection rectangles of sub-device pad.
    //
    // Args:
    //    |selection|: V4L2 sub-device selection rectangle.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int SetSelection(const struct v4l2_subdev_selection& selection);

    // This method gets the format of sub-device pad.
    //
    // Args:
    //    |pad_index|: pad number.
    //    |width|: image width.
    //    |height|: image height.
    //    |code|: format code.
    //
    // Returns:
    //    0 on success; corresponding error code on failure.
    int GetPadFormat(int pad_index, int* width, int* height, int* code);

    int SetRouting(v4l2_subdev_route* routes, uint32_t numRoutes);
    int GetRouting(v4l2_subdev_route* routes, uint32_t* numRoutes);

 private:
    int GetFormat(struct v4l2_subdev_format* format);

 private:
    enum class SubdevState {
        CLOSED = 0, /*!< kernel device closed */
        OPEN,       /*!< device node opened */
        CONFIGURED, /*!< device format set, IOC_S_FMT */
        STARTED,    /*!< device has been started */
        ERROR       /*!< undefined state */
    };

    // Lock to protect |state_|
    // base::Lock state_lock_;

    SubdevState state_;
};

}  // namespace cros
#endif  // CAMERA_INCLUDE_CROS_CAMERA_V4L2_DEVICE_H_
