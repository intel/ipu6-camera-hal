/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2015-2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_CAMERASRC_H__
#define __GST_CAMERASRC_H__
#include <sys/types.h>
#include <map>
#include <gst/gst.h>
#include "Parameters.h"
#include <linux/videodev2.h>
#include "gstcampushsrc.h"
#include <queue>
#include <vector>
#include <set>

G_BEGIN_DECLS

using namespace std;

#define DEFAULT_FRAME_WIDTH 1920
#define DEFAULT_FRAME_HEIGHT 1080
#define DEFAULT_FPS_N 30
#define DEFAULT_FPS_D 1

/* Default value of int type properties */
#define DEFAULT_PROP_BUFFERCOUNT 6
#define MAX_PROP_BUFFERCOUNT 10
#define MIN_PROP_BUFFERCOUNT 2
#define DEFAULT_PROP_WDR_LEVEL 100
#define DEFAULT_PROP_RUN_3A_CADENCE 1
#define DEFAULT_PROP_EXPOSURE_TIME 0
#define DEFAULT_PROP_GAIN 0.0
#define DEFAULT_PROP_PRINT_FPS false
#define DEFAULT_PROP_PRINT_FIELD false
#define DEFAULT_PROP_INPUT_WIDTH 0
#define DEFAULT_PROP_INPUT_HEIGHT 0
#define MIN_PROP_INPUT_WIDTH 0
#define MIN_PROP_INPUT_HEIGHT 0
#define MAX_PROP_INPUT_WIDTH 1920
#define MAX_PROP_INPUT_HEIGHT 1080
#define MAX_ISP_SETTINGS_SIZE 1024 * 1024
#define DEFAULT_PROP_CROP false
#define DEFAULT_PROP_CROP_X 0
#define DEFAULT_PROP_CROP_Y 0
#define MIN_PROP_CROP_X 0
#define MIN_PROP_CROP_Y 0
#define MAX_PROP_CROP_X 1920
#define MAX_PROP_CROP_Y 1080

/* Default value of enum type property 'io-mode':userptr */
#define DEFAULT_PROP_IO_MODE GST_CAMERASRC_IO_MODE_USERPTR
/* Default value of enum type property 'interlace-mode':any */
#define DEFAULT_PROP_INTERLACE_MODE GST_CAMERASRC_INTERLACE_FIELD_ANY
/* Default value of enum type property 'deinterlace-method':none */
#define DEFAULT_DEINTERLACE_METHOD GST_CAMERASRC_DEINTERLACE_METHOD_NONE
/* Default value of enum type property 'iris-mode':auto */
#define DEFAULT_PROP_IRIS_MODE GST_CAMERASRC_IRIS_MODE_AUTO
/* Default value of enum type property 'blc-area-mode':off */
#define DEFAULT_PROP_BLC_AREA_MODE GST_CAMERASRC_BLC_AREA_MODE_OFF
/* Default value of enum type property 'awb-mode':auto */
#define DEFAULT_PROP_AWB_MODE GST_CAMERASRC_AWB_MODE_AUTO
/* Default value of enum type property 'scene-mode':auto */
#define DEFAULT_PROP_SCENE_MODE GST_CAMERASRC_SCENE_MODE_AUTO
/* Default value of enum type property 'sensor-resolution':1080p */
#define DEFAULT_PROP_SENSOR_RESOLUTION GST_CAMERASRC_SENSOR_RESOLUTION_1080P
/* Default value of enum type property 'ae-mode':auto */
#define DEFAULT_PROP_AE_MODE GST_CAMERASRC_AE_MODE_AUTO
/* Default value of enum type property 'weight-grid-mode':auto */
#define DEFAULT_PROP_WEIGHT_GRID_MODE GST_CAMERASRC_WEIGHT_GRID_MODE_AUTO
/* Default value of enum type property 'ae-converge-speed':normal */
#define DEFAULT_PROP_CONVERGE_SPEED GST_CAMERASRC_CONVERGE_SPEED_NORMAL
#define DEFAULT_PROP_CONVERGE_SPEED_MODE GST_CAMERASRC_CONVERGE_SPEED_MODE_AIQ
/* Default value of enum type property 'flip-mode':none */
#define DEFAULT_PROP_FLIP_MODE GST_CAMERASRC_FLIP_MODE_NONE
/* Default value of enum type property 'antibanding':auto */
#define DEFAULT_PROP_ANTIBANDING_MODE GST_CAMERASRC_ANTIBANDING_MODE_AUTO
/* Default value of enum type property 'exp-priority':auto */
#define DEFAULT_PROP_EXPOSURE_PRIORITY GST_CAMERASRC_EXPOSURE_PRIORITY_AUTO
/* Default value of enum type property 'color-range':full */
#define DEFAULT_PROP_COLOR_RANGE_MODE GST_CAMERASRC_COLOR_RANGE_MODE_FULL
/* Default value of enum type property 'video-stabilization':off */
#define DEFAULT_PROP_VIDEO_STABILIZATION_MODE GST_CAMERASRC_VIDEO_STABILIZATION_MODE_OFF
/* Default value of enum type property 'buffer-flag': read */
#define DEFAULT_PROP_BUFFER_USAGE GST_CAMERASRC_BUFFER_USAGE_NONE
/* Default value of enum type property 'fisheye-dewarping':off */
#define DEFAULT_PROP_FISHEYE_DEWARPING_MODE GST_CAMERASRC_FISHEYE_DEWARPING_MODE_OFF

/* Default value of string type properties */
#define DEFAULT_PROP_WP NULL
#define DEFAULT_PROP_AE_REGION NULL
#define DEFAULT_PROP_EXPOSURE_TIME_RANGE NULL
#define DEFAULT_PROP_GAIN_RANGE NULL
#define DEFAULT_PROP_CCT_RANGE NULL
#define DEFAULT_PROP_COLOR_TRANSFORM NULL
#define DEFAULT_PROP_CUSTOM_AIC_PARAMETER NULL
#define DEFAULT_PROP_INPUT_FORMAT NULL
#define DEFAULT_PROP_ISP_CONTROL NULL
#define DEFAULT_PROP_LTM_TUNING_DATA NULL

typedef enum
{
  GST_CAMERASRC_DEINTERLACE_METHOD_NONE = 0,
  GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_BOB = 1,
  GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE = 2,
  GST_CAMERASRC_DEINTERLACE_METHOD_HARDWARE_WEAVE = 3,
} GstCamerasrcDeinterlaceMethod;

typedef enum
{
  GST_CAMERASRC_IO_MODE_USERPTR = 0,
  GST_CAMERASRC_IO_MODE_MMAP = 1,
  GST_CAMERASRC_IO_MODE_DMA_EXPORT = 2,
  GST_CAMERASRC_IO_MODE_DMA_IMPORT = 3,
} GstCamerasrcIoMode;

typedef enum
{
  GST_CAMERASRC_INTERLACE_FIELD_ANY = V4L2_FIELD_ANY,
  GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE = V4L2_FIELD_ALTERNATE,
} GstCamerasrcInterlaceField;

typedef enum
{
  GST_CAMERASRC_IRIS_MODE_AUTO = 0,
  GST_CAMERASRC_IRIS_MODE_MANUAL = 1,
  GST_CAMERASRC_IRIS_MODE_CUSTOMIZED = 2,
} GstCamerasrcIrisMode;

typedef enum
{
  GST_CAMERASRC_BLC_AREA_MODE_OFF = 0,
  GST_CAMERASRC_BLC_AREA_MODE_ON = 1,
} GstCamerasrcBlcAreaMode;

typedef enum
{
  GST_CAMERASRC_AWB_MODE_AUTO = 0,
  GST_CAMERASRC_AWB_MODE_INCANDESCENT = 1,
  GST_CAMERASRC_AWB_MODE_FLUORESCENT = 2,
  GST_CAMERASRC_AWB_MODE_DAYLIGHT = 3,
  GST_CAMERASRC_AWB_MODE_FULLY_OVERCAST = 4,
  GST_CAMERASRC_AWB_MODE_PARTLY_OVERCAST = 5,
  GST_CAMERASRC_AWB_MODE_SUNSET = 6,
  GST_CAMERASRC_AWB_MODE_VIDEO_CONFERENCING = 7,
  GST_CAMERASRC_AWB_MODE_CCT_RANGE = 8,
  GST_CAMERASRC_AWB_MODE_WHITE_POINT = 9,
  GST_CAMERASRC_AWB_MODE_MANUAL_GAIN = 10,
  GST_CAMERASRC_AWB_MODE_COLOR_TRANSFORM = 11,
} GstCamerasrcAwbMode;

typedef enum
{
  GST_CAMERASRC_NR_MODE_OFF = 0,
  GST_CAMERASRC_NR_MODE_AUTO = 1,
  GST_CAMERASRC_NR_MODE_NORMAL = 2,
  GST_CAMERASRC_NR_MODE_EXPERT = 3,
} GstCamerasrcNrMode;

typedef enum
{
  GST_CAMERASRC_SCENE_MODE_AUTO = 0,
  GST_CAMERASRC_SCENE_MODE_HDR = 1,
  GST_CAMERASRC_SCENE_MODE_ULL = 2,
  GST_CAMERASRC_SCENE_MODE_HLC = 3,
  GST_CAMERASRC_SCENE_MODE_NORMAL = 4,
  GST_CAMERASRC_SCENE_MODE_CUSTOM_AIC = 5,
  GST_CAMERASRC_SCENE_MODE_VIDEO_LL = 6,
  GST_CAMERASRC_SCENE_MODE_STILL_CAPTURE = 7,
  GST_CAMERASRC_SCENE_MODE_HDR2 = 8,
} GstCamerasrcSceneMode;

typedef enum
{
  GST_CAMERASRC_SENSOR_RESOLUTION_1080P = 0,
  GST_CAMERASRC_SENSOR_RESOLUTION_720P = 1,
  GST_CAMERASRC_SENSOR_RESOLUTION_4K = 2,
} GstCanerasrcSensorResolution;

typedef enum
{
  GST_CAMERASRC_AE_MODE_AUTO = 0,
  GST_CAMERASRC_AE_MODE_MANUAL = 1,
} GstCamerasrcAeMode;

typedef enum
{
  GST_CAMERASRC_WEIGHT_GRID_MODE_AUTO = 0,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_1 = 1,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_2 = 2,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_3 = 3,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_4 = 4,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_5 = 5,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_6 = 6,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_7 = 7,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_8 = 8,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_9 = 9,
  GST_CAMERASRC_CUSTOM_WEIGHT_GRID_10 = 10,
} GstCamerasrcWeightGridMode;

typedef enum
{
  GST_CAMERASRC_CONVERGE_SPEED_NORMAL = 0,
  GST_CAMERASRC_CONVERGE_SPEED_MID = 1,
  GST_CAMERASRC_CONVERGE_SPEED_LOW = 2,
} GstCamerasrcConvergeSpeed;

typedef enum
{
  GST_CAMERASRC_CONVERGE_SPEED_MODE_AIQ = 0,
  GST_CAMERASRC_CONVERGE_SPEED_MODE_HAL = 1,
} GstCamerasrcConvergeSpeedMode;

typedef enum
{
  GST_CAMERASRC_FLIP_MODE_NONE = 0,
  GST_CAMERASRC_FLIP_MODE_VFLIP = 1,
  GST_CAMERASRC_FLIP_MODE_HFLIP = 2,
  GST_CAMERASRC_FLIP_MODE_VHFLIP = 3,
} GstCamerasrcFlipMode;

typedef enum
{
  GST_CAMERASRC_ANTIBANDING_MODE_AUTO = 0,
  GST_CAMERASRC_ANTIBANDING_MODE_50HZ = 1,
  GST_CAMERASRC_ANTIBANDING_MODE_60HZ = 2,
  GST_CAMERASRC_ANTIBANDING_MODE_OFF = 3,
} GstCamerasrcAntibandingMode;

typedef enum
{
  GST_CAMERASRC_EXPOSURE_PRIORITY_AUTO = 0,
  GST_CAMERASRC_EXPOSURE_PRIORITY_SHUTTER = 1,
  GST_CAMERASRC_EXPOSURE_PRIORITY_ISO = 2,
  GST_CAMERASRC_EXPOSURE_PRIORITY_APERTURE = 3,
} GstCamerasrcExposurePriority;

typedef enum
{
  GST_CAMERASRC_COLOR_RANGE_MODE_FULL = 0,
  GST_CAMERASRC_COLOR_RANGE_MODE_REDUCED = 1,
} GstCamerasrcColorRangeMode;

typedef enum
{
  GST_CAMERASRC_BUFFER_USAGE_NONE = 0,
  GST_CAMERASRC_BUFFER_USAGE_READ = 1,
  GST_CAMERASRC_BUFFER_USAGE_WRITE = 2,
  GST_CAMERASRC_BUFFER_USAGE_DMA_EXPORT = 3,
} GstCamerasrcBufferUsage;

typedef enum
{
  GST_CAMERASRC_VIDEO_STABILIZATION_MODE_OFF = 0,
  GST_CAMERASRC_VIDEO_STABILIZATION_MODE_ON = 1,
} GstCamerasrcVideoStabilizationMode;

typedef enum
{
  GST_CAMERASRC_FISHEYE_DEWARPING_MODE_OFF = 0,
  GST_CAMERASRC_FISHEYE_DEWARPING_MODE_REARVIEW = 1,
  GST_CAMERASRC_FISHEYE_DEWARPING_MODE_HITCHVIEW = 2,
} GstCamerasrcFisheydDewarpingMode;

typedef enum
{
  GST_CAMERASRC_STATUS_DEFAULT = 0,
  GST_CAMERASRC_STATUS_RUNNING = 1,
  GST_CAMERASRC_STATUS_STOP = 2,
} GstCamerasrcRunningStat;

#define GST_TYPE_CAMERASRC \
  (gst_camerasrc_get_type())
#define GST_CAMERASRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CAMERASRC,Gstcamerasrc))
#define GST_CAMERASRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CAMERASRC,GstcamerasrcClass))
#define GST_IS_CAMERASRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CAMERASRC))
#define GST_IS_CAMERASRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CAMERASRC))
#define GST_CAMERASRC_CAST(obj) \
  ((Gstcamerasrc *)(obj))

#define GST_CAMSRC_GET_LOCK(src) \
  (&GST_CAMERASRC_CAST(src)->lock)
#define GST_CAMSRC_LOCK(src) \
  g_mutex_lock(GST_CAMSRC_GET_LOCK(src))
#define GST_CAMSRC_UNLOCK(src) \
  g_mutex_unlock(GST_CAMSRC_GET_LOCK(src))
#define GST_CAMSRC_GET_COND(src) \
  (&GST_CAMERASRC_CAST(src)->cond)
#define GST_CAMSRC_WAIT(src) \
  g_cond_wait(GST_CAMSRC_GET_COND(src), GST_CAMSRC_GET_LOCK(src))
#define GST_CAMSRC_BROADCAST(src) \
  g_cond_broadcast(GST_CAMSRC_GET_COND(src))

typedef struct _Gstcamerasrc Gstcamerasrc;
typedef struct _GstcamerasrcClass GstcamerasrcClass;
typedef struct _GstFpsDebug GstFpsDebug;
typedef struct _Gst3AManualControl Gst3AManualControl;
typedef struct _GstStreamInfo GstStreamInfo;

typedef struct
{
  unsigned int uuid;
  unsigned int size;
} isp_control_header;

/* Used to update fps*/
struct _GstFpsDebug
{
  struct timeval dqbuf_start_tm_count,dqbuf_tm_start,qbuf_tm_end;
  double buf_count;
  double last_buf_count;
  double max_fps;
  double min_fps;
  double av_fps;
  double sum_time;
  double tm_interval;
  bool init_max_min_fps;
};

struct _Gst3AManualControl
{
  /* Image Adjustment*/
  guint sharpness;
  guint brightness;
  guint contrast;
  guint hue;
  guint saturation;
  /* Exposure Settings*/
  int iris_mode;
  guint iris_level;
  int exposure_time;
  guint exposure_ev;
  int exposure_priority;
  gfloat gain;
  int ae_mode;
  int weight_grid_mode;
  char ae_region[128];
  int converge_speed;
  int converge_speed_mode;
  gchar *exp_time_range;
  gchar *gain_range;
  /* Backlight Settings*/
  int blc_area_mode;
  guint wdr_level;
  /* White Balance*/
  int awb_mode;
  gchar *cct_range;
  gchar *wp;
  char color_transform[64];
  guint awb_shift_r;
  guint awb_shift_g;
  guint awb_shift_b;
  guint awb_gain_r;
  guint awb_gain_g;
  guint awb_gain_b;
  /* Video Adjustment*/
  int scene_mode;
  int sensor_resolution;

  /* Custom Aic Parameter */
  gchar *custom_aic_param;

  int antibanding_mode;

  int color_range_mode;

  /* Flags to monitor if property is set.
   * We only consider monitoring 'exposure-time', 'gain',
   * and 'scene-mode' because they have dependency
   * to each other */
  gboolean manual_set_exposure_time;
  gboolean manual_set_gain;
  gboolean manual_set_scene_mode;
};

using namespace icamera;

/* Describe info of each stream when constructing bufferpool */
struct _GstStreamInfo
{
  GstBufferPool *pool;

  /* This is used for down stream plugin buffer pool, in
   * dma-import mode, icamerasrc will get the down stream
   * buffer pool to allocate buffers */
  GstBufferPool *downstream_pool;

  /* Weave buffers are used only when deinterlace_method='sw_weave'
    * top stores odd lines, bottom stores even lines*/
  camera_buffer_t *top;
  camera_buffer_t *bottom;
  /* previous sequence*/
  int previous_sequence;

  /* Buffer config */
  guint bpl;
  GstVideoInfo info;
  const char *fmt_name;
  camera_info_t cam_info;

  /* stream config flag */
  gboolean stream_config_done;

  /* buffers queue */
  queue<camera_buffer_t*> *buffer_queue;

  /* Calculate Gstbuffer timestamp*/
  GstClockTime time_end;
  GstClockTime time_start;
  GstClockTime gstbuf_timestamp;

  /* Fps of stream */
  GstFpsDebug fps_debug;

  /* status of pad */
  bool activated;

  /* stream usage */
  int stream_usage;
};

struct _Gstcamerasrc
{
  GstCamPushSrc element;

  /* Stream config */
  map<string, int> stream_map;
  stream_config_t stream_list;
  GstStreamInfo streams[GST_CAMERASRC_MAX_STREAM_NUM];
  stream_t s[GST_CAMERASRC_MAX_STREAM_NUM];
  camera_buffer_t *buffer_list[GST_CAMERASRC_MAX_STREAM_NUM];
  GHashTable *pad_indexes;

  /* Pipeline config */
  int number_of_cameras;
  int number_of_activepads;
  gboolean first_frame;
  gboolean camera_open;
  gboolean camera_init;
  Parameters *param;
  set <unsigned int> *isp_control_tags;
  GstCamerasrcRunningStat running;

  /* Used with GST_CAMSRC_LOCK and GST_CAMSRC_WAIT etc. */
  GMutex lock;
  GCond cond;

  /* Protection for bufferpool activation */
  gboolean start_config;
  gboolean start_streams;
  int stream_start_count;

  /* Used for buffer queue action */
  GMutex qbuf_mutex;

  /* non-3A properties */
  int device_id;
  int interlace_field;
  int deinterlace_method;
  int io_mode;
  int flip_mode;
  int run_3a_cadence;
  int video_stabilization_mode;
  int fisheye_dewarping_mode;
  int buffer_usage;
  int src_stream_usage;
  guint number_of_buffers;
  guint num_vc;
  gboolean print_fps;
  gboolean print_field;
  const char *input_fmt;
  stream_t input_config;

  /* 3A properties */
  Gst3AManualControl man_ctl;

  /* log print level */
  int debugLevel;

  camera_crop_region_t crop_region;
};

struct _GstcamerasrcClass
{
  GstCamPushSrcClass parent_class;
};

GType gst_camerasrc_get_type (void);

G_END_DECLS

#endif /* __GST_CAMERASRC_H__ */
