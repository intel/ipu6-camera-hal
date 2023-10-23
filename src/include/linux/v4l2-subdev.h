/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __LINUX_V4L2_SUBDEV_H
#define __LINUX_V4L2_SUBDEV_H
#include <linux/ioctl.h>
#include <linux/types.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#include <linux/v4l2-common.h>
#include <linux/v4l2-mediabus.h>
enum v4l2_subdev_format_whence {
  V4L2_SUBDEV_FORMAT_TRY = 0,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  V4L2_SUBDEV_FORMAT_ACTIVE = 1,
};
struct v4l2_subdev_format {
  __u32 which;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 pad;
  struct v4l2_mbus_framefmt format;
   __u32 stream;
  __u32 reserved[7];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct v4l2_subdev_crop {
  __u32 which;
  __u32 pad;
  struct v4l2_rect rect;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 reserved[8];
};
struct v4l2_subdev_mbus_code_enum {
  __u32 pad;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 index;
  __u32 code;
  __u32 which;
  __u32 reserved[8];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct v4l2_subdev_frame_size_enum {
  __u32 index;
  __u32 pad;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 code;
  __u32 min_width;
  __u32 max_width;
  __u32 min_height;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 max_height;
  __u32 which;
  __u32 reserved[8];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct v4l2_subdev_frame_interval {
  __u32 pad;
  struct v4l2_fract interval;
  __u32 reserved[9];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct v4l2_subdev_frame_interval_enum {
  __u32 index;
  __u32 pad;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 code;
  __u32 width;
  __u32 height;
  struct v4l2_fract interval;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 which;
  __u32 reserved[8];
};
struct v4l2_subdev_selection {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  __u32 which;
  __u32 pad;
  __u32 target;
  __u32 flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
  struct v4l2_rect r;
  __u32 reserved[8];
};
#define v4l2_subdev_edid v4l2_edid
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_G_FMT _IOWR('V', 4, struct v4l2_subdev_format)
#define VIDIOC_SUBDEV_S_FMT _IOWR('V', 5, struct v4l2_subdev_format)
#define VIDIOC_SUBDEV_G_FRAME_INTERVAL _IOWR('V', 21, struct v4l2_subdev_frame_interval)
#define VIDIOC_SUBDEV_S_FRAME_INTERVAL _IOWR('V', 22, struct v4l2_subdev_frame_interval)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_ENUM_MBUS_CODE _IOWR('V', 2, struct v4l2_subdev_mbus_code_enum)
#define VIDIOC_SUBDEV_ENUM_FRAME_SIZE _IOWR('V', 74, struct v4l2_subdev_frame_size_enum)
#define VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL _IOWR('V', 75, struct v4l2_subdev_frame_interval_enum)
#define VIDIOC_SUBDEV_G_CROP _IOWR('V', 59, struct v4l2_subdev_crop)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_S_CROP _IOWR('V', 60, struct v4l2_subdev_crop)
#define VIDIOC_SUBDEV_G_SELECTION _IOWR('V', 61, struct v4l2_subdev_selection)
#define VIDIOC_SUBDEV_S_SELECTION _IOWR('V', 62, struct v4l2_subdev_selection)
#define VIDIOC_SUBDEV_G_STD _IOR('V', 23, v4l2_std_id)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_S_STD _IOW('V', 24, v4l2_std_id)
#define VIDIOC_SUBDEV_ENUMSTD _IOWR('V', 25, struct v4l2_standard)
#define VIDIOC_SUBDEV_G_EDID _IOWR('V', 40, struct v4l2_edid)
#define VIDIOC_SUBDEV_S_EDID _IOWR('V', 41, struct v4l2_edid)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_QUERYSTD _IOR('V', 63, v4l2_std_id)
#define VIDIOC_SUBDEV_S_DV_TIMINGS _IOWR('V', 87, struct v4l2_dv_timings)
#define VIDIOC_SUBDEV_G_DV_TIMINGS _IOWR('V', 88, struct v4l2_dv_timings)
#define VIDIOC_SUBDEV_ENUM_DV_TIMINGS _IOWR('V', 98, struct v4l2_enum_dv_timings)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define VIDIOC_SUBDEV_QUERY_DV_TIMINGS _IOR('V', 99, struct v4l2_dv_timings)
#define VIDIOC_SUBDEV_DV_TIMINGS_CAP _IOWR('V', 100, struct v4l2_dv_timings_cap)
#define VIDIOC_SUBDEV_S_ROUTING _IOWR('V', 39, struct v4l2_subdev_routing)
#define VIDIOC_SUBDEV_G_ROUTING _IOWR('V', 38, struct v4l2_subdev_routing)
#define V4L2_SUBDEV_ROUTE_FL_ACTIVE (1 << 0)
#define V4L2_SUBDEV_ROUTE_FL_IMMUTABLE (1 << 1)
#define V4L2_SUBDEV_ROUTE_FL_SOURCE (1 << 2)
/**
 * struct v4l2_subdev_route - A signal route inside a subdev
 * @sink_pad: the sink pad
 * @sink_stream: the sink stream
 * @source_pad: the source pad
 * @source_stream: the source stream
 * @flags: route flags:
 *
 * V4L2_SUBDEV_ROUTE_FL_ACTIVE: Is the stream in use or not? An
 * active stream will start when streaming is enabled on a video
 * node. Set by the user.
 *
 * V4L2_SUBDEV_ROUTE_FL_SOURCE: Is the sub-device the source of a
 * stream? In this case the sink information is unused (and
 * zero). Set by the driver.
 *
 * V4L2_SUBDEV_ROUTE_FL_IMMUTABLE: Is the stream immutable, i.e.
 * can it be activated and inactivated? Set by the driver.
 */
struct v4l2_subdev_route {
 __u32 sink_pad;
 __u32 sink_stream;
 __u32 source_pad;
 __u32 source_stream;
 __u32 flags;
 __u32 reserved[5];
};

/**
 * struct v4l2_subdev_routing - Routing information
 * @routes: the routes array
 * @num_routes: the total number of routes in the routes array
 */
struct v4l2_subdev_routing {
 struct v4l2_subdev_route *routes;
 __u32 num_routes;
 __u32 reserved[5];
};

#endif
