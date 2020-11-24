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

#define LOG_TAG "GstCameraSrc"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <gst/gst.h>

#include "ICamera.h"
#include "ScopedAtrace.h"

#include "gstcamerasrcbufferpool.h"
#include "gstcamerasrc.h"
#include "gstcameraformat.h"
#include "gstcamera3ainterface.h"
#include "gstcameraispinterface.h"
#include "gstcameradewarpinginterface.h"
#ifndef CHROME_SLIM_CAMHAL
#include "gstcamerawfovinterface.h"
#endif
#include "utils.h"

using namespace icamera;

GST_DEBUG_CATEGORY (gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,

  PROP_BUFFERCOUNT,
  PROP_PRINT_FPS,
  PROP_PRINT_FIELD,
  PROP_INTERLACE_MODE,
  PROP_DEINTERLACE_METHOD,
  PROP_DEVICE_ID,
  PROP_IO_MODE,
  PROP_NUM_VC,
  PROP_DEBUG_LEVEL,
  /* Image Adjust-ment*/
  PROP_SHARPNESS,
  PROP_BRIGHTNESS,
  PROP_CONTRAST,
  PROP_HUE,
  PROP_SATURATION,
  /* Exposure Settings*/
  PROP_IRIS_MODE,
  PROP_IRIS_LEVEL,
  PROP_EXPOSURE_TIME,
  PROP_EXPOSURE_EV,
  PROP_EXPOSURE_PRIORITY,
  PROP_GAIN,
  PROP_AE_MODE,
  PROP_WEIGHT_GRID_MODE,
  PROP_AE_REGION,
  PROP_EXPOSURE_TIME_RANGE,
  PROP_GAIN_RANGE,
  PROP_CONVERGE_SPEED,
  PROP_CONVERGE_SPEED_MODE,
  /* Backlight Settings*/
  PROP_BLC_AREA_MODE,
  PROP_WDR_LEVEL,
  /* White Balance*/
  PROP_AWB_MODE,
  PROP_CCT_RANGE,
  PROP_WP,
  PROP_AWB_GAIN_R,
  PROP_AWB_GAIN_G,
  PROP_AWB_GAIN_B,
  PROP_AWB_SHIFT_R,
  PROP_AWB_SHIFT_G,
  PROP_AWB_SHIFT_B,
  PROP_AWB_COLOR_TRANSFORM,
  /* Video Adjustment*/
  PROP_SCENE_MODE,
  PROP_SENSOR_RESOLUTION,
  /* Custom Aic Parameter*/
  PROP_CUSTOM_AIC_PARAMETER,

  PROP_FLIP_MODE,
  PROP_3A_CADENCE,
  PROP_ANTIBANDING_MODE,
  PROP_COLOR_RANGE_MODE,
  PROP_VIDEO_STABILIZATION_MODE,
  PROP_INPUT_FORMAT,
  PROP_BUFFER_USAGE,
  PROP_SRC_STREAM_USAGE,
  PROP_INPUT_WIDTH,
  PROP_INPUT_HEIGHT,
  PROP_ISP_CONTROL,
  PROP_FISHEYE_DEWARPING_MODE,
  PROP_LTM_TUNING_DATA,
  PROP_CROP,
  PROP_CROP_X,
  PROP_CROP_Y,
};

#define gst_camerasrc_parent_class parent_class

static void gst_camerasrc_3a_interface_init (GstCamerasrc3AInterface *iface);
static void gst_camerasrc_isp_interface_init (GstCamerasrcIspInterface *ispIface);
static void gst_camerasrc_dewarping_interface_init (GstCamerasrcDewarpingInterface *dewarpingIface);
#ifndef CHROME_SLIM_CAMHAL
static void gst_camerasrc_wfov_interface_init (GstCamerasrcWFOVInterface *wfovIface);

G_DEFINE_TYPE_WITH_CODE (Gstcamerasrc, gst_camerasrc, GST_TYPE_CAM_PUSH_SRC,
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_3A_IF, gst_camerasrc_3a_interface_init);
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_ISP_IF, gst_camerasrc_isp_interface_init);
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_DEWARPING_IF, gst_camerasrc_dewarping_interface_init);
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_WFOV_IF, gst_camerasrc_wfov_interface_init));
#else
G_DEFINE_TYPE_WITH_CODE (Gstcamerasrc, gst_camerasrc, GST_TYPE_CAM_PUSH_SRC,
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_3A_IF, gst_camerasrc_3a_interface_init);
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_ISP_IF, gst_camerasrc_isp_interface_init);
        G_IMPLEMENT_INTERFACE(GST_TYPE_CAMERASRC_DEWARPING_IF, gst_camerasrc_dewarping_interface_init) );
#endif

static void gst_camerasrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_camerasrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static gboolean gst_camerasrc_set_caps(GstCamBaseSrc *src, GstPad *pad, GstCaps * caps);
static GstCaps* gst_camerasrc_get_caps(GstCamBaseSrc *src, GstCaps * filter);
static gboolean gst_camerasrc_start(GstCamBaseSrc *basesrc);
static gboolean gst_camerasrc_stop(GstCamBaseSrc *basesrc);
static GstStateChangeReturn gst_camerasrc_change_state(GstElement * element,GstStateChange transition);
static GstCaps *gst_camerasrc_fixate (GstCamBaseSrc * basesrc, GstCaps * caps);
static gboolean gst_camerasrc_negotiate(GstCamBaseSrc *basesrc, GstPad *pad);
static gboolean gst_camerasrc_query(GstCamBaseSrc * bsrc, GstQuery * query );
static gboolean gst_camerasrc_decide_allocation(GstCamBaseSrc *bsrc,GstQuery *query, GstPad *pad);
static GstFlowReturn gst_camerasrc_fill(GstCamPushSrc *src, GstPad *pad, GstBuffer *buf);
static void gst_camerasrc_dispose(GObject *object);
static gboolean gst_camerasrc_unlock(GstCamBaseSrc *src);
static gboolean gst_camerasrc_unlock_stop(GstCamBaseSrc *src);
static GstPad *gst_camerasrc_request_new_pad (GstElement * element, GstPadTemplate * templ,
    const gchar * name_templ, const GstCaps * caps);
static void gst_camerasrc_release_pad (GstElement * element, GstPad * pad);


/* ------3A interface declaration------
 * These functions will provide set and get parameters
 * Refer to implementations for details
 */
static camera_image_enhancement_t gst_camerasrc_get_image_enhancement (GstCamerasrc3A *cam3a,
    camera_image_enhancement_t img_enhancement);
static gboolean gst_camerasrc_set_image_enhancement(GstCamerasrc3A *cam3a,
    camera_image_enhancement_t img_enhancement);
static gboolean gst_camerasrc_set_exposure_time (GstCamerasrc3A *cam3a, guint exp_time);
static gboolean gst_camerasrc_set_iris_mode (GstCamerasrc3A *cam3a,
    camera_iris_mode_t irisMode);
static gboolean gst_camerasrc_set_iris_level (GstCamerasrc3A *cam3a, int irisLevel);
static gboolean gst_camerasrc_set_gain (GstCamerasrc3A *cam3a, float gain);
static gboolean gst_camerasrc_set_blc_area_mode (GstCamerasrc3A *cam3a,
    camera_blc_area_mode_t blcAreaMode);
static gboolean gst_camerasrc_set_wdr_level (GstCamerasrc3A *cam3a, uint8_t level);
static gboolean gst_camerasrc_set_awb_mode (GstCamerasrc3A *cam3a,
    camera_awb_mode_t awbMode);
static camera_awb_gains_t gst_camerasrc_get_awb_gain (GstCamerasrc3A *cam3a,
    camera_awb_gains_t& awbGains);
static gboolean gst_camerasrc_set_awb_gain (GstCamerasrc3A *cam3a,
    camera_awb_gains_t awbGains);
static gboolean gst_camerasrc_set_scene_mode (GstCamerasrc3A *cam3a,
    camera_scene_mode_t sceneMode);
static gboolean gst_camerasrc_set_ae_mode (GstCamerasrc3A *cam3a,
    camera_ae_mode_t aeMode);
static gboolean gst_camerasrc_set_weight_grid_mode (GstCamerasrc3A *cam3a,
    camera_weight_grid_mode_t weightGridMode);
static gboolean gst_camerasrc_set_ae_converge_speed (GstCamerasrc3A *cam3a,
    camera_converge_speed_t speed);
static gboolean gst_camerasrc_set_awb_converge_speed (GstCamerasrc3A *cam3a,
    camera_converge_speed_t speed);
static gboolean gst_camerasrc_set_ae_converge_speed_mode (GstCamerasrc3A *cam3a,
    camera_converge_speed_mode_t mode);
static gboolean gst_camerasrc_set_awb_converge_speed_mode (GstCamerasrc3A *cam3a,
    camera_converge_speed_mode_t mode);
static gboolean gst_camerasrc_set_exposure_ev (GstCamerasrc3A *cam3a, int ev);
static gboolean gst_camerasrc_set_exposure_priority (GstCamerasrc3A *cam3a,
    camera_ae_distribution_priority_t priority);
static camera_range_t gst_camerasrc_get_awb_cct_range (GstCamerasrc3A *cam3a,
    camera_range_t& cct);
static gboolean gst_camerasrc_set_awb_cct_range (GstCamerasrc3A *cam3a,
    camera_range_t cct);
static camera_coordinate_t gst_camerasrc_get_white_point (GstCamerasrc3A *cam3a,
    camera_coordinate_t &whitePoint);
static gboolean gst_camerasrc_set_white_point (GstCamerasrc3A *cam3a,
    camera_coordinate_t whitePoint);
static camera_awb_gains_t gst_camerasrc_get_awb_gain_shift (GstCamerasrc3A *cam3a,
    camera_awb_gains_t& awbGainShift);
static gboolean gst_camerasrc_set_awb_gain_shift (GstCamerasrc3A *cam3a,
    camera_awb_gains_t awbGainShift);
static gboolean gst_camerasrc_set_ae_region (GstCamerasrc3A *cam3a,
    camera_window_list_t aeRegions);
static gboolean gst_camerasrc_set_color_transform (GstCamerasrc3A *cam3a,
    camera_color_transform_t colorTransform);
static gboolean gst_camerasrc_set_custom_aic_param (GstCamerasrc3A *cam3a,
    const void* data, unsigned int length);
static gboolean gst_camerasrc_set_antibanding_mode (GstCamerasrc3A *cam3a,
    camera_antibanding_mode_t bandingMode);
static gboolean gst_camerasrc_set_color_range_mode (GstCamerasrc3A *cam3a,
    camera_yuv_color_range_mode_t colorRangeMode);
static gboolean gst_camerasrc_set_exposure_time_range(GstCamerasrc3A *cam3a,
    camera_range_t exposureTimeRange);
static gboolean gst_camerasrc_set_sensitivity_gain_range (GstCamerasrc3A *cam3a,
    camera_range_t sensitivityGainRange);

static gboolean gst_camerasrc_set_isp_control (GstCamerasrcIsp *camIsp, unsigned int tag, void *data);
static gboolean gst_camerasrc_get_isp_control (GstCamerasrcIsp *camIsp, unsigned int tag, void *data);
static gboolean gst_camerasrc_apply_isp_control (GstCamerasrcIsp *camIsp);
static gboolean gst_camerasrc_set_ltm_tuning_data (GstCamerasrcIsp *camIsp, void *data);
static gboolean gst_camerasrc_get_ltm_tuning_data (GstCamerasrcIsp *camIsp, void *data);

static gboolean gst_camerasrc_set_dewarping_mode (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t mode);
static gboolean gst_camerasrc_get_dewarping_mode (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t &mode);

#ifndef CHROME_SLIM_CAMHAL
static gboolean gst_camerasrc_get_wfov_mode (GstCamerasrcWFOV *camWFOV, uint8_t &mode);
static gboolean gst_camerasrc_get_sensor_mount_type (GstCamerasrcWFOV *camWFOV, camera_mount_type_t &mount_type);
static gboolean gst_camerasrc_set_view_projection (GstCamerasrcWFOV *camWFOV, camera_view_projection_t projection);
static gboolean gst_camerasrc_get_view_projection (GstCamerasrcWFOV *camWFOV, camera_view_projection_t &projection);
static gboolean gst_camerasrc_set_view_rotation (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t rotation);
static gboolean gst_camerasrc_get_view_rotation (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &rotation);
static gboolean gst_camerasrc_set_view_fine_adjustments (GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t fine_adjustments);
static gboolean gst_camerasrc_get_view_fine_adjustments (GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t &fine_adjustments);
static gboolean gst_camerasrc_set_camera_rotation (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t camRotation);
static gboolean gst_camerasrc_get_camera_rotation (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &camRotation);
#endif

#if 0
static gboolean gst_camerasrc_sink_event(GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_camerasrc_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
#endif

static GType
gst_camerasrc_interlace_field_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType interlace_field_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_INTERLACE_FIELD_ANY,
        "interlace mode: ANY", "any"},
    {GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE,
        "interlace mode: ALTERNATE", "alternate"},
    {0, NULL, NULL},
  };

  if (!interlace_field_type) {
    interlace_field_type =
        g_enum_register_static ("GstCamerasrcInterlacMode", method_types);
  }
  return interlace_field_type;
}

static GType
gst_camerasrc_deinterlace_method_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType deinterlace_method_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_DEINTERLACE_METHOD_NONE,
        "don't do deinterlace", "none"},
    {GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_BOB,
        "software bob", "sw_bob"},
    {GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE,
        "software weaving", "sw_weaving"},
    {GST_CAMERASRC_DEINTERLACE_METHOD_HARDWARE_WEAVE,
         "hardware weaving", "hw_weaving"},
    {0, NULL, NULL},
  };

  if (!deinterlace_method_type) {
    deinterlace_method_type =
        g_enum_register_static ("GstCamerasrcDeinterlaceMode", method_types);
  }
  return deinterlace_method_type;
}

static GType
gst_camerasrc_io_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType io_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_IO_MODE_USERPTR,
        "UserPtr", "userptr"},
    {GST_CAMERASRC_IO_MODE_MMAP,
        "MMAP", "mmap"},
    {GST_CAMERASRC_IO_MODE_DMA_EXPORT,
        "DMA export", "dma"},
    {GST_CAMERASRC_IO_MODE_DMA_IMPORT,
        "DMA import", "dma_import"},
    {0, NULL, NULL},
  };

  if (!io_mode_type) {
    io_mode_type = g_enum_register_static ("GstCamerasrcIoMode", method_types);
  }
  return io_mode_type;
}

static GType
gst_camerasrc_device_id_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType device_type = 0;

  int count = get_number_of_cameras();
  static GEnumValue *method_types = new GEnumValue[count+1];
  camera_info_t cam_info;
  int id, ret = 0;

  for (id = 0; id < count; id++) {
      ret = get_camera_info(id, cam_info);
      if (ret < 0) {
          g_print("failed to get device name.");
          return FALSE;
      }

      method_types[id].value = id;
      method_types[id].value_name = cam_info.description;
      method_types[id].value_nick = cam_info.name;
  }

  /* the last element of array should be set NULL*/
  method_types[id].value = 0;
  method_types[id].value_name = NULL;
  method_types[id].value_nick = NULL;

  if (!device_type)
    device_type = g_enum_register_static("GstCamerasrcDeviceName", method_types);

  return device_type;
}

static GType
gst_camerasrc_iris_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType iris_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_IRIS_MODE_AUTO,
        "Auto", "auto"},
    {GST_CAMERASRC_IRIS_MODE_MANUAL,
        "Manual", "manual"},
    {GST_CAMERASRC_IRIS_MODE_CUSTOMIZED,
        "Customized", "customized"},
    {0, NULL, NULL},
  };

  if (!iris_mode_type) {
    iris_mode_type = g_enum_register_static ("GstCamerasrcIrisMode", method_types);
   }
  return iris_mode_type;
}

static GType
gst_camerasrc_blc_area_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType blc_area_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_BLC_AREA_MODE_OFF,
          "Off", "off"},
    {GST_CAMERASRC_BLC_AREA_MODE_ON,
          "On", "on"},
     {0, NULL, NULL},
   };

  if (!blc_area_mode_type) {
    blc_area_mode_type = g_enum_register_static ("GstCamerasrcBlcAreaMode", method_types);
  }
  return blc_area_mode_type;
}

static GType
gst_camerasrc_video_stabilization_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType video_stabilization_mode_type = 0;

  static const GEnumValue modes[] = {
    {GST_CAMERASRC_VIDEO_STABILIZATION_MODE_OFF,
          "Off", "off"},
    {GST_CAMERASRC_VIDEO_STABILIZATION_MODE_ON,
          "On", "on"},
    {0, NULL, NULL},
   };

  if (!video_stabilization_mode_type) {
    video_stabilization_mode_type = g_enum_register_static ("GstCamerasrcVideoStabilizationMode", modes);
  }
  return video_stabilization_mode_type;
}

static GType
gst_camerasrc_flip_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType flip_mode_type = 0;

  static const GEnumValue modes[] = {
    {GST_CAMERASRC_FLIP_MODE_NONE,
          "None", "none"},
    {GST_CAMERASRC_FLIP_MODE_VFLIP,
          "Vertical flip", "vflip"},
    {GST_CAMERASRC_FLIP_MODE_HFLIP,
          "Horizontal flip", "hflip"},
    {GST_CAMERASRC_FLIP_MODE_VHFLIP,
          "Vertical and horizontal flip", "vhflip"},
    {0, NULL, NULL},
   };

  if (!flip_mode_type) {
    flip_mode_type = g_enum_register_static ("GstCamerasrcFlipMode", modes);
  }
  return flip_mode_type;
}

static GType
gst_camerasrc_fisheye_dewarping_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType fisheye_dewarping_mode_type = 0;

  static const GEnumValue modes[] = {
    {GST_CAMERASRC_FISHEYE_DEWARPING_MODE_OFF,
          "Off", "off"},
    {GST_CAMERASRC_FISHEYE_DEWARPING_MODE_REARVIEW,
          "Rearview", "rearview"},
    {GST_CAMERASRC_FISHEYE_DEWARPING_MODE_HITCHVIEW,
          "Hitchview", "hitchview"},
    {0, NULL, NULL},
   };

  if (!fisheye_dewarping_mode_type) {
    fisheye_dewarping_mode_type = g_enum_register_static ("GstCamerasrcFisheyeDewarpingMode", modes);
  }
  return fisheye_dewarping_mode_type;
}

static GType
gst_camerasrc_awb_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType awb_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_AWB_MODE_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_AWB_MODE_INCANDESCENT,
          "Incandescent", "incandescent"},
    {GST_CAMERASRC_AWB_MODE_FLUORESCENT,
          "Fluorescent", "fluorescent"},
    {GST_CAMERASRC_AWB_MODE_DAYLIGHT,
          "Daylight", "daylight"},
    {GST_CAMERASRC_AWB_MODE_FULLY_OVERCAST,
          "Fully overcast", "fully_overcast"},
    {GST_CAMERASRC_AWB_MODE_PARTLY_OVERCAST,
          "Partly overcast", "partly_overcast"},
    {GST_CAMERASRC_AWB_MODE_SUNSET,
          "Sunset", "sunset"},
    {GST_CAMERASRC_AWB_MODE_VIDEO_CONFERENCING,
          "Video conferencing", "video_conferencing"},
    {GST_CAMERASRC_AWB_MODE_CCT_RANGE,
          "CCT range", "cct_range"},
    {GST_CAMERASRC_AWB_MODE_WHITE_POINT,
          "White point", "white_point"},
    {GST_CAMERASRC_AWB_MODE_MANUAL_GAIN,
          "Manual gain", "manual_gain"},
    {GST_CAMERASRC_AWB_MODE_COLOR_TRANSFORM,
          "Color Transform", "color_transform"},
    {0, NULL, NULL},
   };

  if (!awb_mode_type) {
    awb_mode_type = g_enum_register_static ("GstCamerasrcAwbMode", method_types);
  }
  return awb_mode_type;
}

static GType
gst_camerasrc_scene_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType scene_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_SCENE_MODE_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_SCENE_MODE_HDR,
          "HDR", "hdr"},
    {GST_CAMERASRC_SCENE_MODE_ULL,
          "ULL", "ull"},
    {GST_CAMERASRC_SCENE_MODE_HLC,
          "HLC", "hlc"},
    {GST_CAMERASRC_SCENE_MODE_NORMAL,
          "NORMAL", "normal"},
    {GST_CAMERASRC_SCENE_MODE_CUSTOM_AIC,
          "CUSTOM_AIC", "custom_aic"},
    {GST_CAMERASRC_SCENE_MODE_VIDEO_LL,
          "VIDEO_LL", "video-ll"},
    {GST_CAMERASRC_SCENE_MODE_STILL_CAPTURE,
          "STILL_CAPTURE", "still_capture"},
    {GST_CAMERASRC_SCENE_MODE_HDR2,
          "HDR2", "hdr2"},
    {0, NULL, NULL},
   };

  if (!scene_mode_type) {
    scene_mode_type = g_enum_register_static ("GstCamerasrcSceneMode", method_types);
  }
  return scene_mode_type;
}

static GType
gst_camerasrc_sensor_resolution_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType sensor_resolution_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_SENSOR_RESOLUTION_1080P,
          "1080P", "1080p"},
    {GST_CAMERASRC_SENSOR_RESOLUTION_720P,
          "720P", "720p"},
    {GST_CAMERASRC_SENSOR_RESOLUTION_4K,
          "4K", "4K"},
     {0, NULL, NULL},
   };

  if (!sensor_resolution_type) {
    sensor_resolution_type = g_enum_register_static ("GstCamerasrcSensorResolution", method_types);
  }
  return sensor_resolution_type;
}

static GType
gst_camerasrc_ae_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType ae_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_AE_MODE_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_AE_MODE_MANUAL,
          "Manual", "manual"},
     {0, NULL, NULL},
   };

  if (!ae_mode_type) {
    ae_mode_type = g_enum_register_static ("GstCamerasrcAeMode", method_types);
  }
  return ae_mode_type;
}

static GType
gst_camerasrc_weight_grid_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType weight_grid_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_WEIGHT_GRID_MODE_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_1,
          "Custom Weight Grid 1", "wg1"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_2,
          "Custom Weight Grid 2", "wg2"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_3,
          "Custom Weight Grid 3", "wg3"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_4,
          "Custom Weight Grid 4", "wg4"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_5,
          "Custom Weight Grid 5", "wg5"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_6,
          "Custom Weight Grid 6", "wg6"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_7,
          "Custom Weight Grid 7", "wg7"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_8,
          "Custom Weight Grid 8", "wg8"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_9,
          "Custom Weight Grid 9", "wg9"},
    {GST_CAMERASRC_CUSTOM_WEIGHT_GRID_10,
          "Custom Weight Grid 10", "wg10"},
     {0, NULL, NULL},
   };

  if (!weight_grid_mode_type) {
    weight_grid_mode_type = g_enum_register_static ("GstCamerasrcWeightGridMode", method_types);
  }
  return weight_grid_mode_type;
}

static GType
gst_camerasrc_converge_speed_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType cvg_speed_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_CONVERGE_SPEED_NORMAL,
          "NORMAL", "normal"},
    {GST_CAMERASRC_CONVERGE_SPEED_MID,
          "MID", "mid"},
    {GST_CAMERASRC_CONVERGE_SPEED_LOW,
          "LOW", "low"},
     {0, NULL, NULL},
   };

  if (!cvg_speed_type) {
    cvg_speed_type = g_enum_register_static ("GstCamerasrcConvergeSpeed", method_types);
  }
  return cvg_speed_type;
}

static GType
gst_camerasrc_converge_speed_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType cvg_speed_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_CONVERGE_SPEED_MODE_AIQ,
          "USE AIQ", "aiq"},
    {GST_CAMERASRC_CONVERGE_SPEED_MODE_HAL,
          "USE HAL", "hal"},
     {0, NULL, NULL},
   };

  if (!cvg_speed_mode_type) {
    cvg_speed_mode_type = g_enum_register_static ("GstCamerasrcConvergeSpeedMode", method_types);
  }
  return cvg_speed_mode_type;
}

static GType
gst_camerasrc_antibanding_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType antibanding_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_ANTIBANDING_MODE_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_ANTIBANDING_MODE_50HZ,
          "50Hz", "50"},
    {GST_CAMERASRC_ANTIBANDING_MODE_60HZ,
          "60HZ", "60"},
    {GST_CAMERASRC_ANTIBANDING_MODE_OFF,
          "Off", "off"},
    {0, NULL, NULL},
   };

  if (!antibanding_mode_type) {
    antibanding_mode_type = g_enum_register_static ("GstCamerasrcAntibandingMode", method_types);
  }
  return antibanding_mode_type;
}

static GType
gst_camerasrc_color_range_mode_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType color_range_mode_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_COLOR_RANGE_MODE_FULL,
          "Full range(0-255) YUV data", "full"},
    {GST_CAMERASRC_COLOR_RANGE_MODE_REDUCED,
          "Reduced range aka. BT.601(16-235) YUV data", "reduced"},
    {0, NULL, NULL},
   };

  if (!color_range_mode_type) {
    color_range_mode_type = g_enum_register_static ("GstCamerasrcColorRangeMode", method_types);
  }
  return color_range_mode_type;
}

static GType
gst_camerasrc_exposure_priority_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType exp_priority_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_EXPOSURE_PRIORITY_AUTO,
          "Auto", "auto"},
    {GST_CAMERASRC_EXPOSURE_PRIORITY_SHUTTER,
          "Shutter", "shutter"},
    {GST_CAMERASRC_EXPOSURE_PRIORITY_ISO,
          "ISO", "iso"},
    {GST_CAMERASRC_EXPOSURE_PRIORITY_APERTURE,
          "Aperture", "aperture"},
    {0, NULL, NULL},
   };

  if (!exp_priority_type) {
    exp_priority_type = g_enum_register_static ("GstCamerasrcExposurePriority", method_types);
  }
  return exp_priority_type;
}

static GType
gst_camerasrc_buffer_usage_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType buffer_usage_type = 0;

  static const GEnumValue method_types[] = {
    {GST_CAMERASRC_BUFFER_USAGE_NONE,
          "0",  "none"},
    {GST_CAMERASRC_BUFFER_USAGE_READ,
          "Read", "read"},
    {GST_CAMERASRC_BUFFER_USAGE_WRITE,
          "Write", "write"},
    {GST_CAMERASRC_BUFFER_USAGE_DMA_EXPORT,
          "DMA Export", "dma_export"},
    {0, NULL, NULL},
   };

  if (!buffer_usage_type) {
    buffer_usage_type = g_enum_register_static ("GstCamerasrcBufferUsage", method_types);
  }
  return buffer_usage_type;
}

static GType
gst_camerasrc_src_stream_usage_get_type(void)
{
  PERF_CAMERA_ATRACE();
  static GType src_stream_usage_type = 0;

  static const GEnumValue method_types[] = {
    {CAMERA_STREAM_PREVIEW,
          "0",  "preview"},
    {CAMERA_STREAM_VIDEO_CAPTURE,
          "1", "video_capture"},
    {CAMERA_STREAM_STILL_CAPTURE,
          "2", "still_capture"},
    {CAMERA_STREAM_APP,
          "3", "app"},
    {0, NULL, NULL},
   };

  if (!src_stream_usage_type) {
    src_stream_usage_type = g_enum_register_static ("GstCamerasrcSrcStreamUsage", method_types);
  }
  return src_stream_usage_type;
}

static void
gst_camerasrc_dispose(GObject *object)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC (object);
  GST_INFO("CameraId=%d.", camerasrc->device_id);
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_camerasrc_finalize (Gstcamerasrc *camerasrc)
{
  PERF_CAMERA_ATRACE();
  GST_INFO("CameraId=%d.", camerasrc->device_id);

  camera_hal_deinit();
  delete camerasrc->param;
  camerasrc->param = NULL;

  delete camerasrc->isp_control_tags;
  camerasrc->isp_control_tags = NULL;

  g_cond_clear(&camerasrc->cond);
  g_mutex_clear(&camerasrc->lock);

  g_mutex_clear(&camerasrc->qbuf_mutex);

  g_hash_table_unref (camerasrc->pad_indexes);

  G_OBJECT_CLASS (parent_class)->finalize ((GObject *) (camerasrc));
}

/* pad template of request source pads */
static GstStaticPadTemplate video_pad_template = GST_STATIC_PAD_TEMPLATE(
  GST_CAM_BASE_VIDEO_PAD_NAMES,
  GST_PAD_SRC,
  GST_PAD_REQUEST,
  gst_camerasrc_get_all_caps());

static GstStaticPadTemplate still_pad_template = GST_STATIC_PAD_TEMPLATE(
  GST_CAM_BASE_STILL_PAD_NAMES,
  GST_PAD_SRC,
  GST_PAD_REQUEST,
  gst_camerasrc_get_all_caps());

static void
gst_camerasrc_class_init (GstcamerasrcClass * klass)
{
  PERF_CAMERA_ATRACE();
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstCamBaseSrcClass *basesrc_class;
  GstCamPushSrcClass * pushsrc_class;

  gobject_class = G_OBJECT_CLASS(klass);
  gstelement_class = GST_ELEMENT_CLASS(klass);
  basesrc_class = GST_CAM_BASE_SRC_CLASS(klass);
  pushsrc_class = GST_CAM_PUSH_SRC_CLASS(klass);

  gobject_class->set_property = gst_camerasrc_set_property;
  gobject_class->get_property = gst_camerasrc_get_property;
  gobject_class->finalize = (GObjectFinalizeFunc) gst_camerasrc_finalize;
  gobject_class->dispose = gst_camerasrc_dispose;

  gstelement_class->change_state = GST_DEBUG_FUNCPTR(gst_camerasrc_change_state);
  gstelement_class->request_new_pad = GST_DEBUG_FUNCPTR(gst_camerasrc_request_new_pad);
  gstelement_class->release_pad = GST_DEBUG_FUNCPTR(gst_camerasrc_release_pad);

  g_object_class_install_property(gobject_class,PROP_BUFFERCOUNT,
      g_param_spec_int("buffer-count","buffer count","The number of buffer to allocate when do the streaming",
        MIN_PROP_BUFFERCOUNT,MAX_PROP_BUFFERCOUNT,DEFAULT_PROP_BUFFERCOUNT,
        (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_PRINT_FPS,
      g_param_spec_boolean("printfps","printfps","Whether print the FPS when do the streaming",
        DEFAULT_PROP_PRINT_FPS,(GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)));

  g_object_class_install_property(gobject_class,PROP_PRINT_FIELD,
      g_param_spec_boolean("printfield","printfield","Whether print the interlaced buffer field",
        DEFAULT_PROP_PRINT_FIELD,(GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_INTERLACE_MODE,
      g_param_spec_enum ("interlace-mode", "interlace-mode", "The interlace method",
        gst_camerasrc_interlace_field_get_type(), DEFAULT_PROP_INTERLACE_MODE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_DEINTERLACE_METHOD,
      g_param_spec_enum ("deinterlace-method", "Deinterlace method", "The deinterlace method that icamerasrc run",
        gst_camerasrc_deinterlace_method_get_type(), DEFAULT_DEINTERLACE_METHOD, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  /* DEFAULT_PROP_DEVICE_ID is defined at configure time, user can configure with custom device ID */
  g_object_class_install_property (gobject_class,PROP_DEVICE_ID,
      g_param_spec_enum("device-name","device-name","The input devices name queried from HAL",
        gst_camerasrc_device_id_get_type(), DEFAULT_PROP_DEVICE_ID, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_NUM_VC,
      g_param_spec_int("num-vc","Number Virtual Channel","Number of enabled Virtual Channel",
        0,8,0, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_DEBUG_LEVEL,
      g_param_spec_int("debug-level","debug-level","Debug log print level(0 ~ 1<20)",
        0,1048576,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_IO_MODE,
      g_param_spec_enum ("io-mode", "IO mode", "The memory types of the frame buffer",
          gst_camerasrc_io_mode_get_type(), DEFAULT_PROP_IO_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_SHARPNESS,
      g_param_spec_int("sharpness","sharpness","sharpness",
        -128,127,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_BRIGHTNESS,
      g_param_spec_int("brightness","brightness","brightness",
        -128,127,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_CONTRAST,
      g_param_spec_int("contrast","contrast","contrast",
        -128,127,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_HUE,
      g_param_spec_int("hue","hue","hue",
        -128,127,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_SATURATION,
      g_param_spec_int("saturation","saturation","saturation",
        -128,127,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_IRIS_MODE,
      g_param_spec_enum ("iris-mode", "IRIS mode", "IRIS mode",
          gst_camerasrc_iris_mode_get_type(), DEFAULT_PROP_IRIS_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_IRIS_LEVEL,
      g_param_spec_int("iris-level","IRIS level","The percentage of opening in IRIS",
        0,100,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_EXPOSURE_TIME,
      g_param_spec_int("exposure-time","Exposure time","Exposure time(only valid in manual AE mode)",
        0,1000000,DEFAULT_PROP_EXPOSURE_TIME,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_GAIN,
      g_param_spec_float("gain","Gain","Implement total gain or maximal gain(only valid in manual AE mode).Unit: db",
        0.0,100.0,DEFAULT_PROP_GAIN,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_BLC_AREA_MODE,
      g_param_spec_enum ("blc-area-mode", "BLC area mode", "BLC area mode",
          gst_camerasrc_blc_area_mode_get_type(), DEFAULT_PROP_BLC_AREA_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_WDR_LEVEL,
      g_param_spec_int("wdr-level","WDR level","WDR level",
        0,200,DEFAULT_PROP_WDR_LEVEL,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_AWB_MODE,
      g_param_spec_enum ("awb-mode", "AWB mode", "White balance mode",
          gst_camerasrc_awb_mode_get_type(), DEFAULT_PROP_AWB_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AWB_GAIN_R,
      g_param_spec_int("awb-gain-r","AWB R-gain","Manual white balance gains",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AWB_GAIN_G,
      g_param_spec_int("awb-gain-g","AWB G-gain","Manual white balance gains",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class,PROP_AWB_GAIN_B,
      g_param_spec_int("awb-gain-b","AWB B-gain","Manual white balance gains",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_AWB_COLOR_TRANSFORM,
      g_param_spec_string("color-transform","AWB color transform","A 3x3 matrix for AWB color transform",
        DEFAULT_PROP_COLOR_TRANSFORM, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_SCENE_MODE,
      g_param_spec_enum ("scene-mode", "Scene mode", "Scene mode",
          gst_camerasrc_scene_mode_get_type(), DEFAULT_PROP_SCENE_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_SENSOR_RESOLUTION,
      g_param_spec_enum ("sensor-resolution", "Sensor resolution", "Sensor resolution",
          gst_camerasrc_sensor_resolution_get_type(), DEFAULT_PROP_SENSOR_RESOLUTION, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_AE_MODE,
      g_param_spec_enum ("ae-mode", "AE mode", "AE mode",
          gst_camerasrc_ae_mode_get_type(), DEFAULT_PROP_AE_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_WEIGHT_GRID_MODE,
      g_param_spec_enum ("weight-grid-mode", "Weight Grid Mode", "Weight Grid Mode",
          gst_camerasrc_weight_grid_mode_get_type(), DEFAULT_PROP_WEIGHT_GRID_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_CONVERGE_SPEED,
      g_param_spec_enum ("converge-speed", "Converge Speed", "Converge Speed",
          gst_camerasrc_converge_speed_get_type(), DEFAULT_PROP_CONVERGE_SPEED, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_CONVERGE_SPEED_MODE,
      g_param_spec_enum ("converge-speed-mode", "Converge Speed Mode", "Converge Speed Mode",
          gst_camerasrc_converge_speed_mode_get_type(), DEFAULT_PROP_CONVERGE_SPEED_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_EXPOSURE_EV,
      g_param_spec_int("ev","Exposure Ev","Exposure Ev",
          -3,3,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_EXPOSURE_PRIORITY,
      g_param_spec_enum ("exp-priority", "Exposure Priority", "Exposure Priority",
          gst_camerasrc_exposure_priority_get_type(), DEFAULT_PROP_EXPOSURE_PRIORITY, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_CCT_RANGE,
      g_param_spec_string("cct-range","CCT range","CCT range(only valid for manual AWB mode)",
        DEFAULT_PROP_CCT_RANGE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_WP,
      g_param_spec_string("wp-point","White point","White point coordinate(only valid for manual AWB mode)",
        DEFAULT_PROP_WP, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AWB_SHIFT_R,
      g_param_spec_int("awb-shift-r","AWB shift-R","AWB shift-R",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AWB_SHIFT_G,
      g_param_spec_int("awb-shift-g","AWB shift-G","AWB shift-G",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AWB_SHIFT_B,
      g_param_spec_int("awb-shift-b","AWB shift-B","AWB shift-B",
        0,255,0,(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_AE_REGION,
      g_param_spec_string("ae-region","AE region","AE region",
        DEFAULT_PROP_AE_REGION, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_EXPOSURE_TIME_RANGE,
      g_param_spec_string("exposure-time-range","AE exposure time range","Exposure time range",
        DEFAULT_PROP_EXPOSURE_TIME_RANGE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_GAIN_RANGE,
      g_param_spec_string("gain-range","AE gain range","AE gain range",
        DEFAULT_PROP_GAIN_RANGE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_CUSTOM_AIC_PARAMETER,
      g_param_spec_string("custom-aic-param","Custom Aic Parameter","Custom Aic Parameter",
        DEFAULT_PROP_CUSTOM_AIC_PARAMETER, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_FLIP_MODE,
      g_param_spec_enum ("flip-mode", "Flip Mode", "Flip Mode",
          gst_camerasrc_flip_mode_get_type(), DEFAULT_PROP_FLIP_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_3A_CADENCE,
      g_param_spec_int("run3a-cadence","Run 3A Cadence","The frame interval to run 3A",
        1, 60, DEFAULT_PROP_RUN_3A_CADENCE, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_ANTIBANDING_MODE,
      g_param_spec_enum ("antibanding-mode", "Antibanding Mode", "Antibanding Mode",
          gst_camerasrc_antibanding_mode_get_type(), DEFAULT_PROP_ANTIBANDING_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_COLOR_RANGE_MODE,
      g_param_spec_enum ("color-range", "ColorRange Mode", "ColorRange Mode",
          gst_camerasrc_color_range_mode_get_type(), DEFAULT_PROP_COLOR_RANGE_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_VIDEO_STABILIZATION_MODE,
      g_param_spec_enum ("video-stabilization-mode", "Video stabilization mode", "Video stabilization mode",
          gst_camerasrc_video_stabilization_mode_get_type(), DEFAULT_PROP_VIDEO_STABILIZATION_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_FISHEYE_DEWARPING_MODE,
      g_param_spec_enum("fisheye-dewarping-mode","fisheye dewarpingmode","The fisheye dewarping mode used in the LDC correction",
          gst_camerasrc_fisheye_dewarping_mode_get_type(), DEFAULT_PROP_FISHEYE_DEWARPING_MODE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_INPUT_FORMAT,
      g_param_spec_string("input-format","Input format","The format used for input system",
        DEFAULT_PROP_INPUT_FORMAT, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_INPUT_WIDTH,
      g_param_spec_int("input-width","Input width","The width used for input system",
        MIN_PROP_INPUT_WIDTH, MAX_PROP_INPUT_WIDTH, DEFAULT_PROP_INPUT_WIDTH, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_INPUT_HEIGHT,
      g_param_spec_int("input-height","Input height","The height used for input system",
        MIN_PROP_INPUT_HEIGHT, MAX_PROP_INPUT_HEIGHT, DEFAULT_PROP_INPUT_HEIGHT, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_BUFFER_USAGE,
      g_param_spec_enum ("buffer-usage", "Buffer flags", "Used to specify buffer properties",
          gst_camerasrc_buffer_usage_get_type(), DEFAULT_PROP_BUFFER_USAGE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_SRC_STREAM_USAGE,
      g_param_spec_enum ("src-stream-usage", "Src stream flags", "Used to specify src stream properties",
          gst_camerasrc_src_stream_usage_get_type(), CAMERA_STREAM_VIDEO_CAPTURE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(gobject_class,PROP_ISP_CONTROL,
      g_param_spec_string("isp-control","isp control","Use a file which contains the detial settings to control isp",
        DEFAULT_PROP_ISP_CONTROL,(GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE)));

  g_object_class_install_property(gobject_class,PROP_LTM_TUNING_DATA,
      g_param_spec_string("ltm-tuning","ltm tuning","Use a file which contains the ltm tuning data",
        DEFAULT_PROP_LTM_TUNING_DATA,(GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE)));

  g_object_class_install_property(gobject_class,PROP_CROP,
      g_param_spec_boolean("crop","crop","Whether to do crop",
        DEFAULT_PROP_CROP,(GParamFlags)(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_CROP_X,
      g_param_spec_int("crop-x","Crop x","The x positon for crop, will be aligned with 128 (for ofa_flipping_none)",
        MIN_PROP_CROP_X, MAX_PROP_CROP_X, DEFAULT_PROP_CROP_X, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_CROP_Y,
      g_param_spec_int("crop-y","Crop y","The y positon for crop, will be aligned with 2 (for ofa_flipping_none)",
        MIN_PROP_CROP_Y, MAX_PROP_CROP_Y, DEFAULT_PROP_CROP_Y, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gst_element_class_set_static_metadata(gstelement_class,
      "icamerasrc",
      "Source/Video",
      "CameraSource Element",
      "Intel");

  /* try another way here */
  gst_element_class_add_pad_template(gstelement_class,
    gst_pad_template_new (GST_CAM_BASE_SRC_PAD_NAME, GST_PAD_SRC, GST_PAD_ALWAYS, gst_camerasrc_get_all_caps()));

  gst_element_class_add_static_pad_template(gstelement_class, &video_pad_template);
  gst_element_class_add_static_pad_template(gstelement_class, &still_pad_template);

  basesrc_class->get_caps = GST_DEBUG_FUNCPTR(gst_camerasrc_get_caps);
  basesrc_class->set_caps = GST_DEBUG_FUNCPTR(gst_camerasrc_set_caps);
  basesrc_class->start = GST_DEBUG_FUNCPTR(gst_camerasrc_start);
  basesrc_class->unlock = GST_DEBUG_FUNCPTR(gst_camerasrc_unlock);
  basesrc_class->unlock_stop = GST_DEBUG_FUNCPTR(gst_camerasrc_unlock_stop);
  basesrc_class->fixate = GST_DEBUG_FUNCPTR(gst_camerasrc_fixate);
  basesrc_class->stop = GST_DEBUG_FUNCPTR(gst_camerasrc_stop);
  basesrc_class->query = GST_DEBUG_FUNCPTR(gst_camerasrc_query);
  basesrc_class->negotiate = GST_DEBUG_FUNCPTR(gst_camerasrc_negotiate);
  basesrc_class->decide_allocation = GST_DEBUG_FUNCPTR(gst_camerasrc_decide_allocation);
  pushsrc_class->fill = GST_DEBUG_FUNCPTR(gst_camerasrc_fill);

  GST_DEBUG_CATEGORY_INIT (gst_camerasrc_debug, "icamerasrc", 0, "camerasrc source element");
}

static void
gst_camerasrc_init (Gstcamerasrc * camerasrc)
{
  PERF_CAMERA_ATRACE();
  GST_INFO("\n");

  /* no need to add anything to init pad*/
  gst_cam_base_src_set_format (GST_CAM_BASE_SRC (camerasrc), GST_FORMAT_TIME,
    GST_CAM_BASE_SRC_PAD_NAME);
  gst_cam_base_src_set_live (GST_CAM_BASE_SRC (camerasrc), TRUE);
  camerasrc->number_of_cameras = get_number_of_cameras();
  /* src pad is already active at the beginning */
  camerasrc->number_of_activepads = 1;
  /* strore src pad name and stream id,
   * src pad provides data in main stream */
  camerasrc->stream_map.clear();
  camerasrc->stream_map.insert(pair<string, int>(GST_CAM_BASE_SRC_PAD_NAME, GST_CAMERASRC_MAIN_STREAM_ID));
  camerasrc->pad_indexes = g_hash_table_new(NULL, NULL);

  camerasrc->number_of_buffers = DEFAULT_PROP_BUFFERCOUNT;
  camerasrc->interlace_field = DEFAULT_PROP_INTERLACE_MODE;
  camerasrc->deinterlace_method = DEFAULT_DEINTERLACE_METHOD;

  // Only use DEFAULT_PROP_DEVICE_ID when it's a valid device index, otherwise use 0 as the default.
  if (DEFAULT_PROP_DEVICE_ID < camerasrc->number_of_cameras) {
    camerasrc->device_id = DEFAULT_PROP_DEVICE_ID;
  } else {
    camerasrc->device_id = 0;
  }

  camerasrc->camera_open = FALSE;
  camerasrc->camera_init = FALSE;
  camerasrc->first_frame = TRUE;
  camerasrc->running = GST_CAMERASRC_STATUS_DEFAULT;
  camerasrc->num_vc = 0;
  camerasrc->debugLevel = 0;
  camerasrc->run_3a_cadence = DEFAULT_PROP_RUN_3A_CADENCE;
  camerasrc->video_stabilization_mode = DEFAULT_PROP_VIDEO_STABILIZATION_MODE;
  camerasrc->fisheye_dewarping_mode = DEFAULT_PROP_FISHEYE_DEWARPING_MODE;
  camerasrc->input_fmt = DEFAULT_PROP_INPUT_FORMAT;
  camerasrc->buffer_usage= DEFAULT_PROP_BUFFER_USAGE;
  camerasrc->src_stream_usage = CAMERA_STREAM_VIDEO_CAPTURE;
  camerasrc->input_config.width = DEFAULT_PROP_INPUT_WIDTH;
  camerasrc->input_config.height = DEFAULT_PROP_INPUT_HEIGHT;
  camerasrc->input_config.format = -1;
  camerasrc->start_config = FALSE;
  camerasrc->start_streams = FALSE;

  /* lock and cond are used to ensure icamerasrc only call interfaces once,
  * including: camera_device_config_streams(), camera_device_start() and
  * camera_stream_stop() */
  g_cond_init(&camerasrc->cond);
  g_mutex_init(&camerasrc->lock);

  /* qbuf_mutex is used to ensure icamerasrc has collected one buffer
  * from each stream and queue to HAL together at once */
  g_mutex_init(&camerasrc->qbuf_mutex);

  /* init buffer timestamp for main stream */
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].time_start = 0;
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].time_end = 0;
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].gstbuf_timestamp = 0;
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].stream_config_done = FALSE;
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].activated = TRUE;
  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].stream_usage = CAMERA_STREAM_VIDEO_CAPTURE;

  /* set default value for 3A manual control*/
  camerasrc->param = new Parameters;
  camerasrc->isp_control_tags = new set <unsigned int>;
  memset(&(camerasrc->man_ctl), 0, sizeof(camerasrc->man_ctl));
  memset(camerasrc->man_ctl.ae_region, 0, sizeof(camerasrc->man_ctl.ae_region));
  memset(camerasrc->man_ctl.color_transform, 0, sizeof(camerasrc->man_ctl.color_transform));
  camerasrc->man_ctl.iris_mode = DEFAULT_PROP_IRIS_MODE;
  camerasrc->man_ctl.wdr_level = DEFAULT_PROP_WDR_LEVEL;
  camerasrc->man_ctl.blc_area_mode = DEFAULT_PROP_BLC_AREA_MODE;
  camerasrc->man_ctl.awb_mode = DEFAULT_PROP_AWB_MODE;
  camerasrc->man_ctl.scene_mode = DEFAULT_PROP_SCENE_MODE;
  camerasrc->man_ctl.sensor_resolution = DEFAULT_PROP_SENSOR_RESOLUTION;
  camerasrc->man_ctl.ae_mode = DEFAULT_PROP_AE_MODE;
  camerasrc->man_ctl.exposure_time = DEFAULT_PROP_EXPOSURE_TIME;
  camerasrc->man_ctl.gain = DEFAULT_PROP_GAIN;
  camerasrc->man_ctl.weight_grid_mode = DEFAULT_PROP_WEIGHT_GRID_MODE;
  camerasrc->man_ctl.wp = DEFAULT_PROP_WP;
  camerasrc->man_ctl.antibanding_mode = DEFAULT_PROP_ANTIBANDING_MODE;
  camerasrc->man_ctl.color_range_mode = DEFAULT_PROP_COLOR_RANGE_MODE;
  camerasrc->man_ctl.exposure_priority = DEFAULT_PROP_EXPOSURE_PRIORITY;
  /* init property flags */
  camerasrc->man_ctl.manual_set_exposure_time = FALSE;
  camerasrc->man_ctl.manual_set_gain = FALSE;
  camerasrc->man_ctl.manual_set_scene_mode = FALSE;
}

static void
gst_camerasrc_3a_interface_init (GstCamerasrc3AInterface *iface)
{
  iface->get_image_enhancement = gst_camerasrc_get_image_enhancement;
  iface->set_image_enhancement = gst_camerasrc_set_image_enhancement;
  iface->set_exposure_time = gst_camerasrc_set_exposure_time;
  iface->set_iris_mode = gst_camerasrc_set_iris_mode;
  iface->set_iris_level = gst_camerasrc_set_iris_level;
  iface->set_gain = gst_camerasrc_set_gain;
  iface->set_blc_area_mode = gst_camerasrc_set_blc_area_mode;
  iface->set_wdr_level = gst_camerasrc_set_wdr_level;
  iface->set_awb_mode = gst_camerasrc_set_awb_mode;
  iface->get_awb_gain = gst_camerasrc_get_awb_gain;
  iface->set_awb_gain = gst_camerasrc_set_awb_gain;
  iface->set_scene_mode = gst_camerasrc_set_scene_mode;
  iface->set_ae_mode = gst_camerasrc_set_ae_mode;
  iface->set_weight_grid_mode = gst_camerasrc_set_weight_grid_mode;
  iface->set_ae_converge_speed = gst_camerasrc_set_ae_converge_speed;
  iface->set_awb_converge_speed = gst_camerasrc_set_awb_converge_speed;
  iface->set_ae_converge_speed_mode = gst_camerasrc_set_ae_converge_speed_mode;
  iface->set_awb_converge_speed_mode = gst_camerasrc_set_awb_converge_speed_mode;
  iface->set_exposure_ev = gst_camerasrc_set_exposure_ev;
  iface->set_exposure_priority = gst_camerasrc_set_exposure_priority;
  iface->get_awb_cct_range = gst_camerasrc_get_awb_cct_range;
  iface->set_awb_cct_range = gst_camerasrc_set_awb_cct_range;
  iface->get_white_point = gst_camerasrc_get_white_point;
  iface->set_white_point = gst_camerasrc_set_white_point;
  iface->get_awb_gain_shift = gst_camerasrc_get_awb_gain_shift;
  iface->set_awb_gain_shift = gst_camerasrc_set_awb_gain_shift;
  iface->set_ae_region = gst_camerasrc_set_ae_region;
  iface->set_color_transform = gst_camerasrc_set_color_transform;
  iface->set_custom_aic_param = gst_camerasrc_set_custom_aic_param;
  iface->set_antibanding_mode = gst_camerasrc_set_antibanding_mode;
  iface->set_color_range_mode = gst_camerasrc_set_color_range_mode;
  iface->set_exposure_time_range = gst_camerasrc_set_exposure_time_range;
  iface->set_sensitivity_gain_range = gst_camerasrc_set_sensitivity_gain_range;
}

static void
gst_camerasrc_isp_interface_init (GstCamerasrcIspInterface *ispIface)
{
  ispIface->set_isp_control = gst_camerasrc_set_isp_control;
  ispIface->get_isp_control = gst_camerasrc_get_isp_control;
  ispIface->apply_isp_control = gst_camerasrc_apply_isp_control;
  ispIface->set_ltm_tuning_data = gst_camerasrc_set_ltm_tuning_data;
  ispIface->get_ltm_tuning_data = gst_camerasrc_get_ltm_tuning_data;
}

static void
gst_camerasrc_dewarping_interface_init (GstCamerasrcDewarpingInterface *dewarpingIface)
{
  dewarpingIface->set_dewarping_mode = gst_camerasrc_set_dewarping_mode;
  dewarpingIface->get_dewarping_mode = gst_camerasrc_get_dewarping_mode;
}

#ifndef CHROME_SLIM_CAMHAL
static void
gst_camerasrc_wfov_interface_init (GstCamerasrcWFOVInterface *wfovIface)
{
  wfovIface->get_wfov_mode = gst_camerasrc_get_wfov_mode;
  wfovIface->get_sensor_mount_type = gst_camerasrc_get_sensor_mount_type;
  wfovIface->set_view_projection = gst_camerasrc_set_view_projection;
  wfovIface->get_view_projection = gst_camerasrc_get_view_projection;
  wfovIface->set_view_rotation = gst_camerasrc_set_view_rotation;
  wfovIface->get_view_rotation = gst_camerasrc_get_view_rotation;
  wfovIface->set_view_fine_adjustments = gst_camerasrc_set_view_fine_adjustments;
  wfovIface->get_view_fine_adjustments = gst_camerasrc_get_view_fine_adjustments;
  wfovIface->set_camera_rotation = gst_camerasrc_set_camera_rotation;
  wfovIface->get_camera_rotation = gst_camerasrc_get_camera_rotation;
}
#endif

/**
 * This function is to avtivate a request pad from GstCamBaseSrc
 * through interface method. The requested pad is got according
 * to user's setting in gst pipeline. Each pad added won't start
 * streaming until it finishes several steps, e.g.: fiXate, set_caps,
 * decide_allocation, along with BufferPool configuration, aquire_buffer
 * and release_buffer. RequestPad will be removed when receive EOS.
 */
static GstPad *gst_camerasrc_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name_templ, const GstCaps * caps)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(element);
  GST_INFO("CameraId=%d.", camerasrc->device_id);
  GstCamBaseSrc *basesrc = GST_CAM_BASE_SRC(element);
  GstCamBaseSrcClass *klass = GST_CAM_BASE_SRC_GET_CLASS(element);
  GstPad *req_pad = NULL;
  guint index = -1;

  GST_CAMSRC_LOCK(camerasrc);
  /* the name of request pad must follow format: video_%u or still_%u, while %u should start from 0 */
  if ((name_templ && sscanf (name_templ, "video_%u", &index) == 1)
      || (name_templ && sscanf (name_templ, "still_%u", &index) == 1)) {
    GST_INFO("CameraId=%d, current video/still pad index: %d", camerasrc->device_id, index);
    if (index >= GST_CAMERASRC_MAX_STREAM_NUM - 1) {
      GST_ERROR("index: %d is larger than limit: %d", index, GST_CAMERASRC_MAX_STREAM_NUM - 1);
      GST_CAMSRC_UNLOCK(camerasrc);
      return NULL;
    }

    if (g_hash_table_contains(camerasrc->pad_indexes, GUINT_TO_POINTER(index))) {
      GST_ERROR("pad name %s is not unique", name_templ);
      GST_CAMSRC_UNLOCK(camerasrc);
      return NULL;
    }
  } else {
    GST_ERROR("Invalid pad name: %s, please follow 'video_%%u' or 'still_%%u' as template to name it", name_templ);
    GST_CAMSRC_LOCK(camerasrc);
    return NULL;
  }

  g_hash_table_insert(camerasrc->pad_indexes, GUINT_TO_POINTER(index), NULL);

  gchar *padname = NULL;
  if (name_templ && sscanf (name_templ, "video_%u", &index) == 1) {
    padname = g_strdup_printf ("video_%u", index);
  } else if (name_templ && sscanf (name_templ, "still_%u", &index) == 1) {
    padname = g_strdup_printf ("still_%u", index);
  }
  /* store video pad name and stream id */
  guint request_pad_index = index + 1;
  camerasrc->stream_map.insert(pair<string, int>(padname, request_pad_index));
  camerasrc->number_of_activepads++;

  /* init buffer timestamp for video stream */
  camerasrc->streams[request_pad_index].time_start = 0;
  camerasrc->streams[request_pad_index].time_end = 0;
  camerasrc->streams[request_pad_index].gstbuf_timestamp = 0;
  camerasrc->streams[request_pad_index].stream_config_done = FALSE;
  camerasrc->streams[request_pad_index].activated = TRUE;
  camerasrc->streams[request_pad_index].stream_usage = CAMERA_STREAM_VIDEO_CAPTURE;

  if (name_templ && sscanf (name_templ, "still_%u", &index) == 1) {
    camerasrc->streams[request_pad_index].stream_usage = CAMERA_STREAM_STILL_CAPTURE;
  }

  gst_cam_base_src_set_format (GST_CAM_BASE_SRC (camerasrc), GST_FORMAT_TIME, padname);

  /* generate pad from template */
  GstPad *videopad = NULL;
  if (name_templ && sscanf (name_templ, "video_%u", &index) == 1) {
    videopad = gst_pad_new_from_static_template(&video_pad_template, padname);
  } else if (name_templ && sscanf (name_templ, "still_%u", &index) == 1) {
    videopad = gst_pad_new_from_static_template(&still_pad_template, padname);
  }

  /* call base class interface to add video pad */
  req_pad = GST_CAM_BASE_SRC_CLASS (parent_class)->add_video_pad(basesrc, klass, videopad, index, padname);
  if (!req_pad) {
    GST_ERROR("CameraId=%d failed to add video source pad.", camerasrc->device_id);
    GST_CAMSRC_UNLOCK(camerasrc);
    return NULL;
  }
  GST_INFO("CameraId: %d, add source pad: %s", camerasrc->device_id, padname);
  g_free(padname);
  GST_CAMSRC_UNLOCK(camerasrc);

  return req_pad;
}

static void gst_camerasrc_release_pad (GstElement * element, GstPad * pad)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(element);
  gchar *padname = gst_pad_get_name(pad);
  int stream_id = CameraSrcUtils::get_stream_id_by_pad(camerasrc->stream_map, pad);
  GST_INFO("CameraId=%d.", camerasrc->device_id);

  GST_CAMSRC_LOCK(camerasrc);
  g_hash_table_remove(camerasrc->pad_indexes, GUINT_TO_POINTER(camerasrc->stream_map[padname]));
  camerasrc->stream_map.erase(padname);
  camerasrc->number_of_activepads--;
  camerasrc->streams[stream_id].activated = FALSE;
  gst_pad_set_active(pad, FALSE);
  gst_element_remove_pad (element, pad);
  g_free (padname);
  GST_CAMSRC_UNLOCK(camerasrc);
}

/**
  * parse the region string from app, the string is similar to:
  * 161,386,186,411,1;404,192,429,217,1;246,85,271,110,1;164,271,189,296,1;418,240,443,265,1;
  */
static int
gst_camerasrc_get_region_vector(const char *reg_str, camera_window_list_t &region)
{
  if (reg_str == NULL) {
    g_print("the region string is empty\n");
    return -1;
  }

  char *attr = strdup(reg_str);
  if (attr == NULL) {
    g_print("failed to allocate attr memory\n");
    return -1;
  }
  char *str = attr;
  camera_window_t window;
  char *tmpStr = NULL, *token = NULL;

  while ((tmpStr = strstr(str, ";")) != NULL) {
      *tmpStr = '\0';

      token = strtok(str, ",");
      if (NULL == token) {
          g_print("failed to parse the left string.\n");
          break;
      }
      window.left = atoi(token);
      token = strtok(NULL, ",");
      if (NULL == token) {
          g_print("failed to parse the top string.\n");
          break;
      }
      window.top = atoi(token);
      token = strtok(NULL, ",");
      if (NULL == token) {
          g_print("failed to parse the right string.\n");
          break;
      }
      window.right = atoi(token);
      token = strtok(NULL, ",");
      if (NULL == token) {
          g_print("failed to parse the bottom string.\n");
          break;
      }
      window.bottom = atoi(token);
      token = strtok(NULL, ",");
      if (NULL == token) {
          g_print("failed to parse the weight string.\n");
          break;
      }
      window.weight = atoi(token);
      region.push_back(window);
      str = tmpStr + 1;
  };
  free(attr);

  return (NULL == token) ? -1 : 0;
}

template <typename T>
static int
gst_camerasrc_parse_range(const gchar *range_str, T &min, T &max)
{
  char *token = NULL;
  int len = strlen(range_str);
  char range_array[len + 1];
  memset(range_array, 0, sizeof(range_array));

  snprintf(range_array, sizeof(range_array), "%s", range_str);
  token = strtok(range_array, "~");
  if (token == NULL) {
    g_print("failed to aquire min range.\n");
    return -1;
  }
  min = atof(token);

  token = strtok(NULL, "~");
  if (token == NULL) {
    g_print("failed to aquire max range.\n");
    return -1;
  }
  max = atof(token);

  if (min > max) {
    g_print("invalid range, min value is bigger than max.\n");
    return -1;
  }

  return 0;
}

/**
  * parse the string to two-dimensional matrix with float type, the string is similar to:
  * -1.5, -1.5, 0, -2.0, -2.0, 1.0, 1.5, 2.0, 0.5....
  */
static int
gst_camerasrc_parse_string_to_matrix(const char *tra_str, float **array, int row, int cul)
{
  if (tra_str == NULL) {
    g_print("the color transform string is empty\n");
    return -1;
  }

  char *attr = strdup(tra_str);
  if (attr == NULL) {
    g_print("failed to allocate attr memory\n");
    return -1;
  }
  float value = 0.0;
  int i = 0, j = 0;
  char *str = attr;
  char *token = strtok(str, ", ");

  for (i = 0; i < row; i++) {
    for (j = 0; j < cul; j++) {
      if (NULL == token) {
        g_print("failed to parse the color transform string.\n");
        free(attr);
        return -1;
      }
      value = atof(token);
      if (value < -2.0)
        value = -2.0;
      if (value > 2.0)
        value = 2.0;
      *((float*)array + row*i + j) = value;
      token = strtok(NULL, ", ");
    }
  }

  free(attr);
  return 0;
}

/**
  * parse cct_range property, assign max and min value to  camera_range_t
  */
static int
gst_camerasrc_parse_cct_range(Gstcamerasrc *src, gchar *cct_range_str, camera_range_t &cct_range)
{
  char *token = NULL;
  char cct_range_array[64]={'\0'};

  snprintf(cct_range_array, sizeof(cct_range_array), "%s", cct_range_str);
  token = strtok(cct_range_array,"~");
  if (token == NULL) {
      g_print("failed to acquire cct range.");
      return -1;
  }
  cct_range.min = atoi(token);
  cct_range.max = atoi(src->man_ctl.cct_range+strlen(token)+1);
  if (cct_range.min < 1800)
      cct_range.min = 1800;
  if (cct_range.max > 15000)
      cct_range.max = 15000;

  return 0;
}

/**
  * parse white point property, assign x, y to camera_coordinate_t
  */
static int
gst_camerasrc_parse_white_point(Gstcamerasrc *src, gchar *wp_str, camera_coordinate_t &white_point)
{
  char *token = NULL;
  char white_point_array[64]={'\0'};

  snprintf(white_point_array, sizeof(white_point_array), "%s", wp_str);
  token = strtok(white_point_array,",");
  if (token == NULL) {
      g_print("failed to acquire white point.");
      return -1;
  }
  white_point.x = atoi(token);
  white_point.y = atoi(src->man_ctl.wp+strlen(token)+1);

  return 0;
}

/* Check if exposure time is out of range */
static void
gst_camerasrc_check_exposuretime_range(Gstcamerasrc *src, GEnumValue *values, int &exp)
{
  camera_info_t cam_info;

  get_camera_info(src->device_id, cam_info);
  vector<camera_ae_exposure_time_range_t> etRanges;
  if (cam_info.capability->getSupportedAeExposureTimeRange(etRanges) == 0) {
    for (auto & item : etRanges) {
      if (src->man_ctl.scene_mode == values[item.scene_mode].value &&
            (exp < item.et_range.min || exp > item.et_range.max)) {
        g_print("Warning: the exposure time: %d is out of range(%0lf~%0lf)\n", exp, item.et_range.min, item.et_range.max);
        exp = (exp < item.et_range.min) ? item.et_range.min : item.et_range.max;
        return;
      }
    }
  }
  g_print("Set exposure time to extreme value: %d\n", exp);
  return;
}

/* Check if gain is out of range */
static void
gst_camerasrc_check_aegain_range(Gstcamerasrc *src, GEnumValue *values, float &gain)
{
  camera_info_t cam_info;

  vector<camera_ae_gain_range_t> gainRange;
  get_camera_info(src->device_id, cam_info);
  if (cam_info.capability->getSupportedAeGainRange(gainRange) == 0) {
    for (auto & item : gainRange) {
      if (src->man_ctl.scene_mode == values[item.scene_mode].value &&
            (gain < item.gain_range.min || gain > item.gain_range.max)) {
        g_print("Warning: the gain value: %f is out of range(%f~%f)\n", gain, item.gain_range.min, item.gain_range.max);
        gain = (gain < item.gain_range.min) ? item.gain_range.min : item.gain_range.max;
        return;
      }
    }
  }
  g_print("Set gain to extreme value: %f\n", gain);
  return;
}

/* This function will check if 'exposure-time' and 'gain' properties are within range
   * according to current 'scene-mode' and 'device-name'. If not, adjust to extreme value */
static void
gst_camerasrc_config_ae_params(Gstcamerasrc *src)
{
  GObjectClass *oclass = G_OBJECT_GET_CLASS (src);
  GParamSpec *spec = g_object_class_find_property (oclass, "scene-mode");
  GEnumValue *values = G_ENUM_CLASS (g_type_class_ref(spec->value_type))->values;

  if (src->man_ctl.manual_set_exposure_time) {
    gst_camerasrc_check_exposuretime_range(src, values, src->man_ctl.exposure_time);
    src->param->setExposureTime(src->man_ctl.exposure_time);
  }

  if (src->man_ctl.manual_set_gain) {
    gst_camerasrc_check_aegain_range(src, values, src->man_ctl.gain);
    src->param->setSensitivityGain(src->man_ctl.gain);
  }

  if (src->man_ctl.manual_set_scene_mode)
    src->param->setSceneMode((camera_scene_mode_t)src->man_ctl.scene_mode);
}

static int
gst_camerasrc_analyze_isp_control(Gstcamerasrc *src, const char *bin_name)
{
  struct stat st;
  FILE *fp = NULL;
  void *data = NULL;
  char *buffer = NULL;
  int len = 0, offset = 0;
  isp_control_header *isp_header;
  int header_size = sizeof(isp_control_header);

  if (bin_name == NULL || (stat(bin_name, &st) != 0)) {
    GST_ERROR("The binary file for isp control doesn't exist");
    return -1;
  }

  if ((fp = fopen(bin_name, "r")) == NULL) {
    GST_ERROR("Failed to open the isp control file");
    return -1;
  }

  buffer = new char[MAX_ISP_SETTINGS_SIZE];
  len = (int)fread(buffer, 1, MAX_ISP_SETTINGS_SIZE, fp);
  fclose(fp);
  if (!len) {
    GST_ERROR("Failed to read the isp control file");
    delete[] buffer;
    return -1;
  }

  src->isp_control_tags->clear();
  g_message("%s, The isp control data length %d", __func__, len);

  while((offset + header_size) < len) {
    isp_header = (isp_control_header*)(buffer + offset);
    data = (void*) (buffer + offset + header_size);
    offset += isp_header->size;

    (src->isp_control_tags)->insert(isp_header->uuid);
#ifndef CHROME_SLIM_CAMHAL
    src->param->setIspControl(isp_header->uuid, data);
    g_message("%s, the isp control uuid: %d, size: %d, offset: %d",
                __func__, isp_header->uuid, isp_header->size, offset);
  }
  src->param->setEnabledIspControls(*(src->isp_control_tags));
#else
  }
  (void)(data);
#endif
  delete[] buffer;

  return 0;
}

static int
gst_camerasrc_set_ltm_tuning_data_from_file(Gstcamerasrc *src, const char *bin_name)
{
  int len = 0;
  struct stat st;
  FILE *fp = NULL;
  char *buffer = NULL;

  if (bin_name == NULL || (stat(bin_name, &st) != 0)) {
    GST_ERROR("The binary file for ltm tuning data doesn't exist");
    return -1;
  }

  if ((fp = fopen(bin_name, "r")) == NULL) {
    GST_ERROR("Failed to open the isp control file");
    return -1;
  }

  buffer = new char[MAX_ISP_SETTINGS_SIZE];
  len = (int)fread(buffer, 1, MAX_ISP_SETTINGS_SIZE, fp);
  fclose(fp);
  if (!len) {
    GST_ERROR("Failed to read the isp control file");
    delete[] buffer;
    return -1;
  }
  g_message("%s, The ltm tuning data length %d", __func__, len);

#ifndef CHROME_SLIM_CAMHAL
  src->param->setLtmTuningData(buffer);
#endif
  delete[] buffer;

  return 0;
}

static void
gst_camerasrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *src = GST_CAMERASRC (object);
  int ret = 0;
  const char *bin_name = NULL;
  gboolean manual_setting = true;

  camera_awb_gains_t awb_gain;
  camera_image_enhancement_t img_enhancement;
  camera_window_list_t region;
  camera_color_transform_t transform;
  camera_range_t cct_range;
  camera_range_t exposure_time_range;
  camera_range_t gain_range;
  camera_coordinate_t white_point;
  unsigned int custom_aic_param_len = 0;
  camera_crop_region_t crop_region;

  memset(&img_enhancement, 0, sizeof(camera_image_enhancement_t));
  memset(&awb_gain, 0, sizeof(camera_awb_gains_t));
  memset(&crop_region, 0, sizeof(camera_crop_region_t));

  switch (prop_id) {
    case PROP_BUFFERCOUNT:
      manual_setting = false;
      src->number_of_buffers = g_value_get_int (value);
      break;
    case PROP_PRINT_FPS:
      manual_setting = false;
      src->print_fps = g_value_get_boolean(value);
      break;
    case PROP_PRINT_FIELD:
      manual_setting = false;
      src->print_field = g_value_get_boolean(value);
      break;
    case PROP_INTERLACE_MODE:
      manual_setting = false;
      src->interlace_field = g_value_get_enum (value);
      break;
    case PROP_DEINTERLACE_METHOD:
      manual_setting = true;
      switch (g_value_get_enum(value)) {
        case GST_CAMERASRC_DEINTERLACE_METHOD_NONE:
        case GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_BOB:
        case GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE:
          src->param->setDeinterlaceMode(DEINTERLACE_OFF);
          break;
        case GST_CAMERASRC_DEINTERLACE_METHOD_HARDWARE_WEAVE:
          /* hardware weaving mode should be enabled thru
          * camera_set_parameters() interface */
          src->param->setDeinterlaceMode(DEINTERLACE_WEAVING);
          break;
        default:
          break;
      }
      src->deinterlace_method = g_value_get_enum (value);
      break;
    case PROP_IO_MODE:
      manual_setting = false;
      src->io_mode = g_value_get_enum (value);
      break;
    case PROP_DEVICE_ID:
      manual_setting = false;
      src->device_id = g_value_get_enum (value);
      gst_camerasrc_config_ae_params(src);
      break;
    case PROP_NUM_VC:
      manual_setting = false;
      src->num_vc = g_value_get_int(value);
      break;
    case PROP_DEBUG_LEVEL:
      src->param->updateDebugLevel();
      src->debugLevel = g_value_get_int (value);
      break;
    case PROP_SHARPNESS:
      src->param->getImageEnhancement(img_enhancement);
      img_enhancement.sharpness = g_value_get_int (value);
      src->param->setImageEnhancement(img_enhancement);
      src->man_ctl.sharpness = img_enhancement.sharpness;
      break;
    case PROP_BRIGHTNESS:
      src->param->getImageEnhancement(img_enhancement);
      img_enhancement.brightness = g_value_get_int (value);
      src->param->setImageEnhancement(img_enhancement);
      src->man_ctl.brightness = img_enhancement.brightness;
      break;
    case PROP_CONTRAST:
      src->param->getImageEnhancement(img_enhancement);
      img_enhancement.contrast = g_value_get_int (value);
      src->param->setImageEnhancement(img_enhancement);
      src->man_ctl.contrast = img_enhancement.contrast;
      break;
    case PROP_HUE:
      src->param->getImageEnhancement(img_enhancement);
      img_enhancement.hue = g_value_get_int (value);
      src->param->setImageEnhancement(img_enhancement);
      src->man_ctl.hue = img_enhancement.hue;
      break;
    case PROP_SATURATION:
      src->param->getImageEnhancement(img_enhancement);
      img_enhancement.saturation = g_value_get_int (value);
      src->param->setImageEnhancement(img_enhancement);
      src->man_ctl.saturation = img_enhancement.saturation;
      break;
    case PROP_IRIS_MODE:
      src->param->setIrisMode((camera_iris_mode_t)g_value_get_enum(value));
      src->man_ctl.iris_mode = g_value_get_enum (value);
      break;
    case PROP_IRIS_LEVEL:
      src->param->setIrisLevel(g_value_get_int(value));
      src->man_ctl.iris_level = g_value_get_int (value);
      break;
    case PROP_EXPOSURE_TIME:
      src->man_ctl.exposure_time = g_value_get_int(value);
      src->man_ctl.manual_set_exposure_time = TRUE;
      gst_camerasrc_config_ae_params(src);
      break;
    case PROP_GAIN:
      src->man_ctl.gain = g_value_get_float (value);
      src->man_ctl.manual_set_gain = TRUE;
      gst_camerasrc_config_ae_params(src);
      break;
    case PROP_BLC_AREA_MODE:
      src->param->setBlcAreaMode((camera_blc_area_mode_t)g_value_get_enum(value));
      src->man_ctl.blc_area_mode = g_value_get_enum (value);
      break;
    case PROP_WDR_LEVEL:
      src->param->setWdrLevel(g_value_get_int (value));
      src->man_ctl.wdr_level = g_value_get_int (value);
      break;
    case PROP_AWB_MODE:
      src->param->setAwbMode((camera_awb_mode_t)g_value_get_enum(value));
      src->man_ctl.awb_mode = g_value_get_enum (value);
      break;
    case PROP_AWB_GAIN_R:
      src->param->getAwbGains(awb_gain);
      awb_gain.r_gain = g_value_get_int (value);
      src->param->setAwbGains(awb_gain);
      src->man_ctl.awb_gain_r = awb_gain.r_gain;
      break;
    case PROP_AWB_GAIN_G:
      src->param->getAwbGains(awb_gain);
      awb_gain.g_gain = g_value_get_int (value);
      src->param->setAwbGains(awb_gain);
      src->man_ctl.awb_gain_g = awb_gain.g_gain;
      break;
    case PROP_AWB_GAIN_B:
      src->param->getAwbGains(awb_gain);
      awb_gain.b_gain = g_value_get_int (value);
      src->param->setAwbGains(awb_gain);
      src->man_ctl.awb_gain_b = awb_gain.b_gain;
      break;
    case PROP_SCENE_MODE:
      src->man_ctl.scene_mode = g_value_get_enum (value);
      src->man_ctl.manual_set_scene_mode = TRUE;
      gst_camerasrc_config_ae_params(src);
      break;
    case PROP_SENSOR_RESOLUTION:
      //implement this in the future.
      src->man_ctl.sensor_resolution = g_value_get_enum (value);
      break;
    case PROP_AE_MODE:
      src->param->setAeMode((camera_ae_mode_t)g_value_get_enum(value));
      src->man_ctl.ae_mode = g_value_get_enum(value);
      break;
    case PROP_WEIGHT_GRID_MODE:
      src->param->setWeightGridMode((camera_weight_grid_mode_t)g_value_get_enum(value));
      src->man_ctl.weight_grid_mode = g_value_get_enum(value);
      break;
    case PROP_CONVERGE_SPEED:
      src->param->setAeConvergeSpeed((camera_converge_speed_t)g_value_get_enum(value));
      src->param->setAwbConvergeSpeed((camera_converge_speed_t)g_value_get_enum(value));
      src->man_ctl.converge_speed = g_value_get_enum(value);
      break;
    case PROP_CONVERGE_SPEED_MODE:
      src->param->setAeConvergeSpeedMode((camera_converge_speed_mode_t)g_value_get_enum(value));
      src->param->setAwbConvergeSpeedMode((camera_converge_speed_mode_t)g_value_get_enum(value));
      src->man_ctl.converge_speed_mode = g_value_get_enum(value);
      break;
    case PROP_EXPOSURE_EV:
      src->param->setAeCompensation(g_value_get_int(value));
      src->man_ctl.exposure_ev = g_value_get_int (value);
      break;
    case PROP_EXPOSURE_PRIORITY:
      src->param->setAeDistributionPriority((camera_ae_distribution_priority_t)g_value_get_enum(value));
      src->man_ctl.exposure_priority = g_value_get_enum(value);
      break;
    case PROP_CCT_RANGE:
      g_free(src->man_ctl.cct_range);
      src->man_ctl.cct_range = g_strdup(g_value_get_string (value));
      src->param->getAwbCctRange(cct_range);
      ret = gst_camerasrc_parse_cct_range(src, src->man_ctl.cct_range, cct_range);
      if (ret == 0)
        src->param->setAwbCctRange(cct_range);
      break;
    case PROP_WP:
      g_free(src->man_ctl.wp);
      src->man_ctl.wp = g_strdup(g_value_get_string (value));
      src->param->getAwbWhitePoint(white_point);
      ret = gst_camerasrc_parse_white_point(src, src->man_ctl.wp, white_point);
      if (ret == 0)
        src->param->setAwbWhitePoint(white_point);
      break;
    case PROP_AWB_SHIFT_R:
      src->param->getAwbGainShift(awb_gain);
      awb_gain.r_gain = g_value_get_int (value);
      src->param->setAwbGainShift(awb_gain);
      src->man_ctl.awb_shift_r = awb_gain.r_gain;
      break;
    case PROP_AWB_SHIFT_G:
      src->param->getAwbGainShift(awb_gain);
      awb_gain.g_gain = g_value_get_int (value);
      src->param->setAwbGainShift(awb_gain);
      src->man_ctl.awb_shift_g = awb_gain.g_gain;
      break;
    case PROP_AWB_SHIFT_B:
      src->param->getAwbGainShift(awb_gain);
      awb_gain.b_gain = g_value_get_int (value);
      src->param->setAwbGainShift(awb_gain);
      src->man_ctl.awb_shift_b = awb_gain.b_gain;
      break;
    case PROP_AE_REGION:
      if (gst_camerasrc_get_region_vector(g_value_get_string (value), region) == 0) {
        src->param->setAeRegions(region);
        snprintf(src->man_ctl.ae_region, sizeof(src->man_ctl.ae_region),
                 "%s", g_value_get_string(value));
      }
      break;
    case PROP_EXPOSURE_TIME_RANGE:
      g_free(src->man_ctl.exp_time_range);
      src->man_ctl.exp_time_range = g_strdup(g_value_get_string(value));
      ret = gst_camerasrc_parse_range(src->man_ctl.exp_time_range,
              exposure_time_range.min,
              exposure_time_range.max);
      if (ret == 0)
        src->param->setExposureTimeRange(exposure_time_range);
      break;
    case PROP_GAIN_RANGE:
      g_free(src->man_ctl.gain_range);
      src->man_ctl.gain_range = g_strdup(g_value_get_string(value));
      ret = gst_camerasrc_parse_range(src->man_ctl.gain_range,
              gain_range.min,
              gain_range.max);
      if (ret == 0)
        src->param->setSensitivityGainRange(gain_range);
      break;
    case PROP_AWB_COLOR_TRANSFORM:
      ret = gst_camerasrc_parse_string_to_matrix(g_value_get_string (value),
                                    (float**)(transform.color_transform), 3, 3);
      if (ret == 0) {
        src->param->setColorTransform(transform);
        snprintf(src->man_ctl.color_transform, sizeof(src->man_ctl.color_transform),
                 "%s", g_value_get_string(value));
      }
      break;
    case PROP_CUSTOM_AIC_PARAMETER:
      g_free(src->man_ctl.custom_aic_param);
      src->man_ctl.custom_aic_param = g_strdup(g_value_get_string (value));
      custom_aic_param_len = strlen(src->man_ctl.custom_aic_param) + 1;
      src->param->setCustomAicParam(src->man_ctl.custom_aic_param, custom_aic_param_len);
      break;
    case PROP_FLIP_MODE:
      src->param->setFlipMode((camera_flip_mode_t)g_value_get_enum(value));
      src->flip_mode = g_value_get_enum (value);
      break;
    case PROP_3A_CADENCE:
      src->param->setRun3ACadence(g_value_get_int(value));
      src->run_3a_cadence = g_value_get_int(value);
      break;
    case PROP_ANTIBANDING_MODE:
      src->param->setAntiBandingMode((camera_antibanding_mode_t)g_value_get_enum(value));
      src->man_ctl.antibanding_mode = g_value_get_enum (value);
      break;
    case PROP_COLOR_RANGE_MODE:
      src->param->setYuvColorRangeMode((camera_yuv_color_range_mode_t)g_value_get_enum(value));
      src->man_ctl.color_range_mode = g_value_get_enum(value);
      break;
    case PROP_VIDEO_STABILIZATION_MODE:
      src->param->setVideoStabilizationMode((camera_video_stabilization_mode_t)g_value_get_enum(value));
      src->video_stabilization_mode = g_value_get_enum (value);
      break;
    case PROP_FISHEYE_DEWARPING_MODE:
      src->param->setFisheyeDewarpingMode((camera_fisheye_dewarping_mode_t)g_value_get_enum(value));
      src->fisheye_dewarping_mode = g_value_get_enum (value);
      break;
    case PROP_INPUT_FORMAT:
      src->input_fmt = g_strdup(g_value_get_string (value));
      break;
    case PROP_BUFFER_USAGE:
      src->buffer_usage = g_value_get_enum(value);
      break;
    case PROP_SRC_STREAM_USAGE:
      src->src_stream_usage = g_value_get_enum(value);
      break;
    case PROP_INPUT_WIDTH:
      src->input_config.width = g_value_get_int(value);
      break;
    case PROP_INPUT_HEIGHT:
      src->input_config.height = g_value_get_int(value);
      break;
    case PROP_ISP_CONTROL:
      bin_name = g_value_get_string (value);
      ret = gst_camerasrc_analyze_isp_control(src, bin_name);
      if (ret != 0)
        GST_ERROR("Failed to set isp control: please check the settings in file");
      break;
    case PROP_LTM_TUNING_DATA:
      bin_name = g_value_get_string (value);
      ret = gst_camerasrc_set_ltm_tuning_data_from_file(src, bin_name);
      if (ret != 0)
        GST_ERROR("Failed to set ltm tuning data: please check data in the bin file");
      break;
    case PROP_CROP:
      crop_region.flag = (int)g_value_get_boolean(value);
      src->param->setCropRegion(crop_region);
      break;
    case PROP_CROP_X:
      src->param->getCropRegion(crop_region);
      crop_region.x = g_value_get_int(value);
      src->param->setCropRegion(crop_region);
      break;
    case PROP_CROP_Y:
      src->param->getCropRegion(crop_region);
      crop_region.y = g_value_get_int(value);
      src->param->setCropRegion(crop_region);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  if (manual_setting && src->camera_open && src->camera_init) {
      camera_set_parameters(src->device_id, *(src->param));
  }

}

static void
gst_camerasrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstcamerasrc *src = GST_CAMERASRC (object);

  switch (prop_id) {
    case PROP_BUFFERCOUNT:
      g_value_set_int (value, src->number_of_buffers);
      break;
    case PROP_PRINT_FPS:
      g_value_set_boolean(value,src->print_fps);
      break;
    case PROP_PRINT_FIELD:
      g_value_set_boolean(value,src->print_field);
      break;
    case PROP_INTERLACE_MODE:
      g_value_set_enum (value, src->interlace_field);
      break;
    case PROP_DEINTERLACE_METHOD:
      g_value_set_enum (value, src->deinterlace_method);
      break;
    case PROP_IO_MODE:
      g_value_set_enum (value, src->io_mode);
      break;
    case PROP_DEVICE_ID:
      g_value_set_enum (value, src->device_id);
      break;
    case PROP_NUM_VC:
      g_value_set_int(value, src->num_vc);
      break;
    case PROP_DEBUG_LEVEL:
      g_value_set_int(value, src->debugLevel);
      break;
    case PROP_SHARPNESS:
      g_value_set_int (value, src->man_ctl.sharpness);
      break;
    case PROP_BRIGHTNESS:
      g_value_set_int (value, src->man_ctl.brightness);
      break;
    case PROP_CONTRAST:
      g_value_set_int (value, src->man_ctl.contrast);
      break;
    case PROP_HUE:
      g_value_set_int (value, src->man_ctl.hue);
      break;
    case PROP_SATURATION:
      g_value_set_int (value, src->man_ctl.saturation);
      break;
    case PROP_IRIS_MODE:
      g_value_set_enum (value, src->man_ctl.iris_mode);
      break;
    case PROP_IRIS_LEVEL:
      g_value_set_int (value, src->man_ctl.iris_level);
      break;
    case PROP_EXPOSURE_TIME:
      g_value_set_int (value, src->man_ctl.exposure_time);
      break;
    case PROP_GAIN:
      g_value_set_float (value, src->man_ctl.gain);
      break;
    case PROP_BLC_AREA_MODE:
      g_value_set_enum (value, src->man_ctl.blc_area_mode);
      break;
    case PROP_WDR_LEVEL:
      g_value_set_int (value, src->man_ctl.wdr_level);
      break;
    case PROP_AWB_MODE:
      g_value_set_enum (value, src->man_ctl.awb_mode);
      break;
    case PROP_AWB_GAIN_R:
      g_value_set_int (value, src->man_ctl.awb_gain_r);
      break;
    case PROP_AWB_GAIN_G:
      g_value_set_int (value, src->man_ctl.awb_gain_g);
      break;
    case PROP_AWB_GAIN_B:
      g_value_set_int (value, src->man_ctl.awb_gain_b);
      break;
    case PROP_SCENE_MODE:
      g_value_set_enum (value, src->man_ctl.scene_mode);
      break;
   case PROP_SENSOR_RESOLUTION:
      g_value_set_enum (value, src->man_ctl.sensor_resolution);
      break;
    case PROP_AE_MODE:
      g_value_set_enum (value, src->man_ctl.ae_mode);
      break;
    case PROP_WEIGHT_GRID_MODE:
      g_value_set_enum (value, src->man_ctl.weight_grid_mode);
      break;
    case PROP_CONVERGE_SPEED:
      g_value_set_enum (value, src->man_ctl.converge_speed);
      break;
    case PROP_CONVERGE_SPEED_MODE:
      g_value_set_enum (value, src->man_ctl.converge_speed_mode);
      break;
    case PROP_EXPOSURE_EV:
      g_value_set_int (value, src->man_ctl.exposure_ev);
      break;
    case PROP_EXPOSURE_PRIORITY:
      g_value_set_enum (value, src->man_ctl.exposure_priority);
      break;
    case PROP_CCT_RANGE:
      g_value_set_string (value, src->man_ctl.cct_range);
      break;
    case PROP_WP:
      g_value_set_string (value, src->man_ctl.wp);
      break;
    case PROP_AWB_SHIFT_R:
      g_value_set_int (value, src->man_ctl.awb_shift_r);
      break;
    case PROP_AWB_SHIFT_G:
      g_value_set_int (value, src->man_ctl.awb_shift_g);
      break;
    case PROP_AWB_SHIFT_B:
      g_value_set_int (value, src->man_ctl.awb_shift_b);
      break;
    case PROP_AE_REGION:
      g_value_set_string (value, src->man_ctl.ae_region);
      break;
    case PROP_EXPOSURE_TIME_RANGE:
      g_value_set_string (value, src->man_ctl.exp_time_range);
      break;
    case PROP_GAIN_RANGE:
      g_value_set_string (value, src->man_ctl.gain_range);
      break;
    case PROP_AWB_COLOR_TRANSFORM:
      g_value_set_string(value, src->man_ctl.color_transform);
      break;
    case PROP_CUSTOM_AIC_PARAMETER:
      g_value_set_string(value, src->man_ctl.custom_aic_param);
      break;
    case PROP_ANTIBANDING_MODE:
      g_value_set_enum (value, src->man_ctl.antibanding_mode);
      break;
    case PROP_COLOR_RANGE_MODE:
      g_value_set_enum (value, src->man_ctl.color_range_mode);
      break;
    case PROP_VIDEO_STABILIZATION_MODE:
      g_value_set_enum (value, src->video_stabilization_mode);
      break;
    case PROP_FISHEYE_DEWARPING_MODE:
      g_value_set_enum (value, src->fisheye_dewarping_mode);
      break;
    case PROP_FLIP_MODE:
      g_value_set_enum (value, src->flip_mode);
      break;
    case PROP_3A_CADENCE:
      g_value_set_int (value, src->run_3a_cadence);
      break;
    case PROP_INPUT_FORMAT:
      g_value_set_string (value, src->input_fmt);
      break;
    case PROP_BUFFER_USAGE:
      g_value_set_enum(value, src->buffer_usage);
      break;
    case PROP_SRC_STREAM_USAGE:
      g_value_set_enum(value, src->src_stream_usage);
      break;
    case PROP_INPUT_WIDTH:
      g_value_set_int(value, src->input_config.width);
      break;
    case PROP_INPUT_HEIGHT:
      g_value_set_int(value, src->input_config.height);
      break;
    case PROP_CROP:
      g_value_set_boolean(value, (bool)src->crop_region.flag);
      break;
    case PROP_CROP_X:
      g_value_set_int(value, src->crop_region.x);
      break;
    case PROP_CROP_Y:
      g_value_set_int(value, src->crop_region.y);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/**
 * Get the stride which is also known as aligned bype per line in some context.
 * Mainly used for locate the start of next line.
 */
static int
gst_camerasrc_get_stride(int format, int width, int bpp)
{
    int bpl = width * bpp / 8;

    if (format == V4L2_PIX_FMT_NV12 || format == V4L2_PIX_FMT_NV21
        || format == V4L2_PIX_FMT_YUV420 || format == V4L2_PIX_FMT_YVU420
        || format == V4L2_PIX_FMT_YUV422P || format == V4L2_PIX_FMT_NV16)
        bpl = width;

    return ALIGN_64(bpl);
}

/**
  * Find match stream from the support list in cam_info
  * if not found, return false
  */
static gboolean
gst_camerasrc_find_match_stream(Gstcamerasrc* camerasrc,
                                int stream_id, int format, int width, int height, int field)
{
    stream_array_t configs;
    int ret = FALSE;
    camerasrc->streams[stream_id].cam_info.capability->getSupportedStreamConfig(configs);

    camera_crop_region_t cropRegion;
    camerasrc->param->getCropRegion(cropRegion);
    if (cropRegion.flag == 0) {
        for (unsigned int i = 0; i < configs.size(); i++) {
            if (width == configs[i].width && height == configs[i].height &&
                format == configs[i].format && field == configs[i].field) {
                camerasrc->s[stream_id] = configs[i];
                camerasrc->streams[stream_id].bpl = configs[i].stride;
                return TRUE;
            }
        }
    } else {
        int bpp = 0;
        camerasrc->s[stream_id].format = format;
        camerasrc->s[stream_id].width = width;
        camerasrc->s[stream_id].height = height;
        camerasrc->s[stream_id].field = field;
        camerasrc->s[stream_id].size = get_frame_size(camerasrc->device_id, format, width, height, field, &bpp);
        camerasrc->s[stream_id].stride = gst_camerasrc_get_stride(format, width, bpp);
        camerasrc->streams[stream_id].bpl = camerasrc->s[stream_id].stride;
        return TRUE;
    }

    return ret;
}

static gboolean
gst_camerasrc_get_caps_info (Gstcamerasrc* camerasrc,
                                GstCaps * caps, int stream_id, stream_config_t *stream_list)
{
  PERF_CAMERA_ATRACE();
  GstVideoInfo info;
  gint32 fourcc = 0;
  GstStructure *structure = gst_caps_get_structure (caps, 0);
  const gchar *mimetype = gst_structure_get_name (structure);

  /* raw caps, parse into video info */
  if (!gst_video_info_from_caps (&info, caps)) {
    GST_ERROR("CameraId=%d, StreamId=%d Caps can't be parsed",
      camerasrc->device_id, stream_id);
    return FALSE;
  }
  GstVideoFormat gst_fmt = GST_VIDEO_INFO_FORMAT (&info);

  /* parse format from caps */
  if (g_str_equal (mimetype, "video/x-raw")) {
    fourcc = CameraSrcUtils::gst_fmt_2_fourcc(gst_fmt);
    if (fourcc == -1)
      return FALSE;
  } else if (g_str_equal (mimetype, "video/x-bayer"))
    fourcc = V4L2_PIX_FMT_SGRBG8;
  else {
    GST_ERROR("CameraId=%d, StreamId=%d unsupported type %s",
      camerasrc->device_id, stream_id, mimetype);
    return FALSE;
  }

  /* parse width from caps */
  if (!gst_structure_get_int (structure, "width", &info.width)) {
    GST_ERROR("CameraId=%d, StreamId=%d  failed to parse width",
      camerasrc->device_id, stream_id);
    return FALSE;
  }

  /* parse height from caps */
  if (!gst_structure_get_int (structure, "height", &info.height)) {
    GST_ERROR("CameraId=%d, StreamId=%d failed to parse height",
      camerasrc->device_id, stream_id);
    return FALSE;
  }

  int ret = gst_camerasrc_find_match_stream(camerasrc, stream_id,
                            fourcc, info.width, info.height, camerasrc->interlace_field);
  if (!ret) {
    GST_ERROR("CameraId=%d, StreamId=%d no match stream found from HAL",
      camerasrc->device_id, stream_id);
    return ret;
  }

  /* if 'framerate' label is configured in Capsfilter, call HAL interface, otherwise is 0 */
  int fps_numerator = GST_VIDEO_INFO_FPS_N(&info);
  int fps_denominator = GST_VIDEO_INFO_FPS_D(&info);
  camerasrc->param->setFrameRate(static_cast<float>(fps_numerator) / fps_denominator);

  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  camerasrc->streams[stream_id].info = info;
  camerasrc->streams[stream_id].fmt_name = gst_video_format_to_string(gst_fmt);

  GST_INFO("CameraId=%d, StreamId=%d Caps info: format=%s width=%d height=%d field=%d framerate %d/%d.",
             camerasrc->device_id, stream_id, camerasrc->streams[stream_id].fmt_name, info.width, info.height,
             camerasrc->interlace_field, fps_numerator, fps_denominator);

  return TRUE;
}

static void
gst_camerasrc_set_memtype(Gstcamerasrc* camerasrc, int stream_id)
{
  /* set memtype for stream*/
  switch(camerasrc->io_mode) {
    case GST_CAMERASRC_IO_MODE_USERPTR:
          camerasrc->s[stream_id].memType = V4L2_MEMORY_USERPTR;
          break;
    case GST_CAMERASRC_IO_MODE_DMA_IMPORT:
          camerasrc->s[stream_id].memType = V4L2_MEMORY_DMABUF;
          break;
    case GST_CAMERASRC_IO_MODE_DMA_EXPORT:
    case GST_CAMERASRC_IO_MODE_MMAP:
          camerasrc->s[stream_id].memType = V4L2_MEMORY_MMAP;
          break;
    default:
          break;
  }
}

/*
 * W/A: For some projects, the camera hal version is different, and the operation mode
 * isn't defined. So hard code to use actual value here temporary.
 */
static void
gst_camerasrc_get_configuration_mode(Gstcamerasrc* camerasrc, stream_config_t *stream_list)
{
    int scene_mode = camerasrc->man_ctl.scene_mode;

    switch(scene_mode) {
      case GST_CAMERASRC_SCENE_MODE_AUTO:
        stream_list->operation_mode = 0x2;
        break;
      case GST_CAMERASRC_SCENE_MODE_HDR:
        stream_list->operation_mode = 0x3;
        break;
      case GST_CAMERASRC_SCENE_MODE_ULL:
        stream_list->operation_mode = 0x4;
        break;
      case GST_CAMERASRC_SCENE_MODE_HLC:
        stream_list->operation_mode = 0x5;
        break;
      case GST_CAMERASRC_SCENE_MODE_CUSTOM_AIC:
        stream_list->operation_mode = 0x6;
        break;
      case GST_CAMERASRC_SCENE_MODE_VIDEO_LL:
        stream_list->operation_mode = 0x7;
        break;
      case GST_CAMERASRC_SCENE_MODE_STILL_CAPTURE:
        stream_list->operation_mode = 0x8;
        break;
      case GST_CAMERASRC_SCENE_MODE_HDR2:
        stream_list->operation_mode = 0x9;
        break;
      case GST_CAMERASRC_SCENE_MODE_NORMAL:
        stream_list->operation_mode = 0;
        break;
      default:
        stream_list->operation_mode = 0x2;
        GST_ERROR("CameraId=%d  the scene mode is invalid", camerasrc->device_id);
        break;
    }
}

static gboolean
gst_camerasrc_set_caps(GstCamBaseSrc *src, GstPad *pad, GstCaps *caps)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC (src);
  gboolean ready_config = TRUE;
  int stream_id = CameraSrcUtils::get_stream_id_by_pad(camerasrc->stream_map, pad);
  gchar *padname = gst_pad_get_name(pad);
  if (stream_id < 0) {
    g_free (padname);
    return FALSE;
  }

  GST_INFO("CameraId=%d, StreamId=%d name of pad=%s Thread Id=%ld.",
    camerasrc->device_id, stream_id, padname, gettid());

  if (get_camera_info(camerasrc->device_id, camerasrc->streams[stream_id].cam_info) < 0) {
    GST_ERROR("failed to get device name when setting device-name.");
    camera_hal_deinit();
    g_free (padname);
    return FALSE;
  }

  /* Get caps info from structure and match from HAL */
  if (!gst_camerasrc_get_caps_info (camerasrc, caps, stream_id, &camerasrc->stream_list)) {
    g_free (padname);
    return FALSE;
  }

  /* Set memory type of stream */
  gst_camerasrc_set_memtype(camerasrc, stream_id);

  /* Create buffer pool */
  camerasrc->streams[stream_id].pool = gst_camerasrc_buffer_pool_new(camerasrc, caps, stream_id);
  if (!camerasrc->streams[stream_id].pool) {
    GST_ERROR("CameraId=%d, StreamId=%d create %s buffer pool failed.",
      camerasrc->device_id, stream_id, padname);
    g_free (padname);
    return FALSE;
  }

  GST_CAMSRC_LOCK(camerasrc);
  camerasrc->streams[stream_id].stream_config_done = TRUE;

  for (int i = 0; i < GST_CAMERASRC_MAX_STREAM_NUM; i++) {
    if (camerasrc->streams[i].activated)
      ready_config *= camerasrc->streams[i].stream_config_done;
  }

  /* HAL interface:camera_device_config_streams() won't be called
  * until all other requested pats are done configuring, otherwise
  * keep waiting until it does */
  if (ready_config) {
    /* Check if input format is valid and convert to fourcc */
    if (camerasrc->input_fmt) {
      if (!CameraSrcUtils::check_format_by_name(camerasrc->input_fmt)) {
        GST_ERROR("failed to find match in supported format list.");
        GST_CAMSRC_UNLOCK(camerasrc);
        g_free (padname);
        return FALSE;
      }
      camerasrc->input_config.format =
        CameraSrcUtils::string_2_fourcc(camerasrc->input_fmt);
     }

    if (camerasrc->input_config.width < MIN_PROP_INPUT_WIDTH ||
      camerasrc->input_config.width > MAX_PROP_INPUT_WIDTH) {
        GST_ERROR("CameraId=%d, streamId=%d unsupported input width %d.",
          camerasrc->device_id, stream_id, camerasrc->input_config.width);
        GST_CAMSRC_UNLOCK(camerasrc);
        g_free (padname);
        return FALSE;
    }

    if (camerasrc->input_config.height < MIN_PROP_INPUT_HEIGHT ||
      camerasrc->input_config.height > MAX_PROP_INPUT_HEIGHT) {
        GST_ERROR("CameraId=%d, streamId=%d unsupported input height %d.",
            camerasrc->device_id, stream_id, camerasrc->input_config.height);
        GST_CAMSRC_UNLOCK(camerasrc);
        g_free (padname);
        return FALSE;
    }

    GST_INFO("CameraId=%d, streamId=%d input format: %s(fourcc=%d)."
      "input width:=%d, input height=%d.",
      camerasrc->device_id, stream_id,
      (camerasrc->input_fmt) ? (camerasrc->input_fmt) : "NULL",
      camerasrc->input_config.format,
      camerasrc->input_config.width,
      camerasrc->input_config.height);

    gst_camerasrc_get_configuration_mode(camerasrc, &camerasrc->stream_list);

    for (int i = 0; i < GST_CAMERASRC_MAX_STREAM_NUM; i++) {
        // Set usage to CAMERA_STREAM_VIDEO_CAPTURE for video user cases
        if (camerasrc->streams[i].activated) {
          GST_INFO("camera stream usage: i=[%d], stream_usage=[%d], src_stream_usate=[%d].",
              i, camerasrc->streams[i].stream_usage, camerasrc->src_stream_usage);
          if (i == 0) {
              camerasrc->s[i].usage = camerasrc->src_stream_usage;
          } else {
              camerasrc->s[i].usage = camerasrc->streams[i].stream_usage;
          }
          camerasrc->s[i].streamType = CAMERA_STREAM_OUTPUT;
        }
    }

    camerasrc->stream_start_count= camerasrc->number_of_activepads;
    camerasrc->stream_list.num_streams = camerasrc->number_of_activepads;
    camerasrc->stream_list.streams = camerasrc->s;
    int ret = camera_device_config_sensor_input(camerasrc->device_id, &camerasrc->input_config);
    ret |= camera_device_config_streams(camerasrc->device_id, &camerasrc->stream_list);
    if(ret < 0) {
      GST_ERROR("CameraId=%d, StreamId=%d failed to config stream for format %s %dx%d.",
        camerasrc->device_id, stream_id, camerasrc->streams[stream_id].fmt_name,
        camerasrc->s[stream_id].width, camerasrc->s[stream_id].height);
      GST_CAMSRC_UNLOCK(camerasrc);
      g_free (padname);
      return FALSE;
    }
    GST_INFO("CameraId=%d, StreamId=%d config stream done.", camerasrc->device_id, stream_id);

    camerasrc->start_config = TRUE;
    GST_CAMSRC_BROADCAST(camerasrc);
  } else {
    while(!camerasrc->start_config) {
      GST_CAMSRC_WAIT(camerasrc);
    }
  }
  GST_CAMSRC_UNLOCK(camerasrc);

  g_free (padname);

  return TRUE;
}

static GstCaps *
gst_camerasrc_get_caps(GstCamBaseSrc *src,GstCaps *filter)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC (src);

  GST_INFO("CameraId=%d.", camerasrc->device_id);
  return gst_pad_get_pad_template_caps (GST_CAM_BASE_SRC_PAD (GST_CAMERASRC(src)));
}

static gboolean
gst_camerasrc_check_deinterlace_params(Gstcamerasrc *camerasrc)
{
  int method = camerasrc->deinterlace_method;
  int io_mode = camerasrc->io_mode;
  int field = camerasrc->interlace_field;
  if (method == GST_CAMERASRC_DEINTERLACE_METHOD_NONE)
    return TRUE;
  else if (method == GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_BOB)
    return (io_mode == GST_CAMERASRC_IO_MODE_USERPTR)
        && ((field == GST_CAMERASRC_INTERLACE_FIELD_ANY) || (field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE));
  else if (method == GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE)
    return (io_mode == GST_CAMERASRC_IO_MODE_USERPTR) && (field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE);
  else if (method == GST_CAMERASRC_DEINTERLACE_METHOD_HARDWARE_WEAVE)
    return ((io_mode == GST_CAMERASRC_IO_MODE_USERPTR) || (io_mode == GST_CAMERASRC_IO_MODE_DMA_IMPORT))
        && (field == GST_CAMERASRC_INTERLACE_FIELD_ALTERNATE);
  else
    return FALSE;
}

static gboolean
gst_camerasrc_start(GstCamBaseSrc *basesrc)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC (basesrc);
  GST_INFO("CameraId=%d.", camerasrc->device_id);

  /* check params */
  if (!gst_camerasrc_check_deinterlace_params(camerasrc)) {
    GST_ERROR("Deinterlace params error, deinterlace_method=%d, io_mode=%d interlace_field=%d",
    camerasrc->deinterlace_method, camerasrc->io_mode, camerasrc->interlace_field);
    return FALSE;
  }
  GST_INFO("Deinterlace_method=%d, io_mode=%d interlace_field=%d",
  camerasrc->deinterlace_method, camerasrc->io_mode, camerasrc->interlace_field);

  /* Init HAL */
  int ret = camera_hal_init();
  if (ret < 0) {
    GST_ERROR("CameraId=%d failed to init HAL.", camerasrc->device_id);
    camerasrc->camera_init = false;
    return FALSE;
  }
  camerasrc->camera_init = true;
#ifndef CHROME_SLIM_CAMHAL
  ret = camera_device_open(camerasrc->device_id, camerasrc->num_vc);
#else
  ret = camera_device_open(camerasrc->device_id);
#endif
  if (ret < 0) {
     GST_ERROR("CameraId=%d failed to open libcamhal device.", camerasrc->device_id);
     camerasrc->camera_open = false;
     camera_hal_deinit();
     return FALSE;
  }
  camerasrc->camera_open = true;

  //set all the params first time.
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  return TRUE;
}

static gboolean
gst_camerasrc_stop(GstCamBaseSrc *basesrc)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(basesrc);
  GST_INFO("CameraId=%d.", camerasrc->device_id);

  camerasrc->streams[GST_CAMERASRC_MAIN_STREAM_ID].activated = FALSE;
  camerasrc->stream_map.erase(GST_CAM_BASE_SRC_PAD_NAME);

  return TRUE;
}

/**
* there're four states defined in Gstreamer: GST_STATE_NULL, GST_STATE_READY,
* GST_STATE_PAUSED, GST_STATE_PLAYING, which can be referred to simply as NULL,
* READY, PAUSED, PLAYING.
*/
static GstStateChangeReturn
gst_camerasrc_change_state (GstElement * element, GstStateChange transition)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(element);
  GST_INFO("CameraId=%d, changing state: %s -> %s.", camerasrc->device_id,
      gst_element_state_get_name (GST_STATE_TRANSITION_CURRENT (transition)),
      gst_element_state_get_name (GST_STATE_TRANSITION_NEXT (transition)));

  /* GST_STATE_CHANGE_NULL_TO_READY: state change from NULL to READY
   * GST_STATE_CHANGE_READY_TO_PAUSED: state change from READY to PAUSED
   * GST_STATE_CHANGE_PAUSED_TO_PLAYING: state change from PAUSED to PLAYING
   * GST_STATE_CHANGE_PLAYING_TO_PAUSED: state change from PLAYING to PAUSED
   * GST_STATE_CHANGE_PAUSED_TO_READY: state change from PAUSED to READY
   * GST_STATE_CHANGE_READY_TO_NULL: state change from READY to NULL */
  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      camerasrc->running = GST_CAMERASRC_STATUS_DEFAULT;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      /* element start processing buffers or data and start streaming */
      camerasrc->running = GST_CAMERASRC_STATUS_RUNNING;
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      /* element ceases processing buffers or data and exit streaming */
      camerasrc->running = GST_CAMERASRC_STATUS_STOP;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    case GST_STATE_CHANGE_READY_TO_NULL:
    default:
      camerasrc->running = GST_CAMERASRC_STATUS_DEFAULT;
      break;
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static GstCaps *
gst_camerasrc_fixate(GstCamBaseSrc * basesrc, GstCaps * caps)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(basesrc);
  GST_INFO("CameraId=%d.", camerasrc->device_id);
  GstStructure *structure;

  caps = gst_caps_make_writable(caps);

  for (guint i = 0; i < gst_caps_get_size (caps); ++i) {
    structure = gst_caps_get_structure (caps, i);
    gst_structure_fixate_field_nearest_int (structure, "width", DEFAULT_FRAME_WIDTH);
    gst_structure_fixate_field_nearest_int (structure, "height", DEFAULT_FRAME_HEIGHT);
    gst_structure_fixate_field_nearest_fraction(structure, "framerate", DEFAULT_FPS_N, DEFAULT_FPS_D);
  }
  caps = GST_CAM_BASE_SRC_CLASS (parent_class)->fixate (basesrc, caps);

  return caps;
}

static gboolean
gst_camerasrc_query(GstCamBaseSrc * bsrc, GstQuery * query )
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(bsrc);
  gboolean res = FALSE;
  GST_INFO("CameraId=%d, handling %s query.", camerasrc->device_id,
      gst_query_type_get_name (GST_QUERY_TYPE (query)));

  switch (GST_QUERY_TYPE (query)){
    case GST_QUERY_LATENCY:
      res = TRUE;
      break;
    default:
      res = GST_CAM_BASE_SRC_CLASS(parent_class)->query(bsrc,query);
      break;
  }
  return res;
}

static gboolean
gst_camerasrc_negotiate(GstCamBaseSrc *basesrc, GstPad *pad)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(basesrc);
  GST_INFO("CameraId=%d.", camerasrc->device_id);

  return GST_CAM_BASE_SRC_CLASS (parent_class)->negotiate (basesrc, pad);
}

static gboolean
gst_camerasrc_decide_allocation(GstCamBaseSrc *bsrc,GstQuery *query, GstPad *pad)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc * camerasrc = GST_CAMERASRC(bsrc);
  GST_INFO("CameraId=%d.", camerasrc->device_id);
  GstCaps *caps;
  GstAllocationParams params;
  GstBufferPool *pool = NULL;
  GstAllocator *allocator = NULL;
  GstStructure *config = NULL, *down_config = NULL;
  guint size = 0, min = 0, max = 0;
  gboolean update;
  const char* GST_MSDK_SELECT="gstMsdkSelect";

  int stream_id = CameraSrcUtils::get_stream_id_by_pad(camerasrc->stream_map, pad);
  if (stream_id < 0)
    return FALSE;
  GST_INFO("CameraId=%d, StreamId=%d.", camerasrc->device_id, stream_id);

  memset(&params, 0, sizeof(GstAllocationParams));
  gst_cam_base_src_get_allocator (bsrc, &allocator, &params);

  switch (camerasrc->io_mode) {
    case GST_CAMERASRC_IO_MODE_USERPTR:
    case GST_CAMERASRC_IO_MODE_MMAP:
    case GST_CAMERASRC_IO_MODE_DMA_EXPORT:
    {
        if(gst_query_get_n_allocation_pools(query)>0){
          gst_query_parse_nth_allocation_pool(query,0,&pool,&size,&min,&max);
          update=TRUE;
        } else {
          pool = NULL;
          max=min=0;
          size = 0;
          update = FALSE;
        }
        pool = GST_BUFFER_POOL_CAST(camerasrc->streams[stream_id].pool);
        size = (GST_CAMERASRC_BUFFER_POOL(camerasrc->streams[stream_id].pool))->size;

        if (update)
          gst_query_set_nth_allocation_pool (query, 0, pool, size, camerasrc->number_of_buffers, MAX_PROP_BUFFERCOUNT);
        else
          gst_query_add_allocation_pool (query, pool, size, camerasrc->number_of_buffers, MAX_PROP_BUFFERCOUNT);
      break;
    }
    case GST_CAMERASRC_IO_MODE_DMA_IMPORT:
    {
      gst_query_parse_allocation (query, &caps, NULL);

      if (gst_query_get_n_allocation_params (query) > 0)
        gst_query_set_nth_allocation_param (query, 0, allocator, &params);

      if (gst_query_get_n_allocation_pools (query) > 0)
        gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

      if (camerasrc->streams[stream_id].downstream_pool) {
        gst_buffer_pool_set_active(camerasrc->streams[stream_id].downstream_pool, FALSE);
        gst_object_unref (camerasrc->streams[stream_id].downstream_pool);
        camerasrc->streams[stream_id].downstream_pool = NULL;
      }

      if (pool == NULL) {
        g_print("%s: buffer pool is NULL\n", __func__);
        return FALSE;
      }

      camerasrc->streams[stream_id].downstream_pool = (GstBufferPool *)gst_object_ref(pool);

      /* config downstream buffer pool where buffers are allocated */
      if (pool) {
        config = gst_buffer_pool_get_config (camerasrc->streams[stream_id].downstream_pool);
        gst_buffer_pool_config_get_params (config, &caps, &size, &min, &max);
        down_config = config;

        /* There have two gstreamer mediasdk now. They have different defination for gst buffer pool option.
         * We use mfxxxx on yocto, for it you should use GST_BUFFER_POOL_OPTION_DMABUF_MEMORY.
         * For gstreamer vaapi, also should use GST_BUFFER_POOL_OPTION_DMABUF_MEMORY..
         * We use msdkxxx on clearlinux, for it you should use GST_BUFFER_POOL_OPTION_MSDK_USE_DMABUF.
         */
        char * gst_msdk = getenv(GST_MSDK_SELECT);
        if (gst_msdk == NULL) {
            gst_buffer_pool_config_add_option (down_config,
              GST_BUFFER_POOL_OPTION_DMABUF_MEMORY);
        } else if (!strcmp(gst_msdk, "msdk")) {
            gst_buffer_pool_config_add_option (down_config,
              GST_BUFFER_POOL_OPTION_MSDK_USE_DMABUF);
        } else {
            gst_buffer_pool_config_add_option (down_config,
              GST_BUFFER_POOL_OPTION_DMABUF_MEMORY);
        }

        gst_buffer_pool_config_add_option (down_config,
            GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);
        gst_buffer_pool_config_set_allocator (config, allocator, &params);
        gst_buffer_pool_config_set_params (config, caps, size, min, max);
        gst_buffer_pool_set_config(camerasrc->streams[stream_id].downstream_pool, down_config);
        gst_buffer_pool_set_active(camerasrc->streams[stream_id].downstream_pool, TRUE);
      }

      gst_object_unref(pool);
      pool = (GstBufferPool *)gst_object_ref(camerasrc->streams[stream_id].pool);

      /* config bufferpool */
      gst_query_set_nth_allocation_pool (query, 0, pool, size, camerasrc->number_of_buffers, max);

      if (allocator)
        gst_object_unref (allocator);
      if (pool)
        gst_object_unref (pool);
      break;
    }
    default:
      break;
 }

 return GST_CAM_BASE_SRC_CLASS (parent_class)->decide_allocation (bsrc, query, pad);
}


/* Gst Clock: |---------------|-----------------------|--------------|---....
 *     (starting_time:0) (base_time)            (get v4l2 ts) (get local ts)
 *                            |                                      |
 *                            |<--------gst buffer timestamp-------->|
 */
static GstFlowReturn
gst_camerasrc_fill(GstCamPushSrc *src, GstPad *pad, GstBuffer *buf)
{
  PERF_CAMERA_ATRACE();
  Gstcamerasrc *camerasrc = GST_CAMERASRC(src);
  GST_INFO("CameraId=%d.", camerasrc->device_id);
  GstClock *clock;
  GstClockTime base_time, timestamp, duration;
  int stream_id = CameraSrcUtils::get_stream_id_by_pad(camerasrc->stream_map, pad);
  if (stream_id < 0)
    return GST_FLOW_ERROR;

  GstCamerasrcBufferPool *bpool = GST_CAMERASRC_BUFFER_POOL(camerasrc->streams[stream_id].pool);

  timestamp = GST_BUFFER_TIMESTAMP (buf);

  if (!GST_CLOCK_TIME_IS_VALID(timestamp))
    return GST_FLOW_OK;

  if (G_LIKELY(camerasrc->streams[stream_id].time_start == 0))
    /* time_start is 0 after dqbuf at the first time */
    /* use base time as starting point*/
    camerasrc->streams[stream_id].time_start = GST_ELEMENT (camerasrc)->base_time;

  duration = (GstClockTime) (camerasrc->streams[stream_id].time_end -
    camerasrc->streams[stream_id].time_start);

  clock = GST_ELEMENT_CLOCK(camerasrc);

  if (clock) {
    base_time = GST_ELEMENT_CAST (camerasrc)->base_time;
    /* gstbuf_timestamp is the accurate timestamp since the base_time */
    camerasrc->streams[stream_id].gstbuf_timestamp = gst_clock_get_time(clock) - base_time;
  } else {
    base_time = GST_CLOCK_TIME_NONE;
  }

  GST_BUFFER_PTS(buf) = camerasrc->streams[stream_id].gstbuf_timestamp;
  GST_BUFFER_OFFSET(buf) = bpool->acquire_buffer_index;
  GST_BUFFER_OFFSET_END(buf) = bpool->acquire_buffer_index + 1;
  GST_BUFFER_DURATION(buf) = duration;
  camerasrc->streams[stream_id].time_start = camerasrc->streams[stream_id].time_end;

  GST_INFO("CameraId=%d, StreamId=%d duration=%lu\n", camerasrc->device_id, stream_id, duration);

  return GST_FLOW_OK;
}

static gboolean
gst_camerasrc_unlock(GstCamBaseSrc *src)
{
  return TRUE;
}

static gboolean
gst_camerasrc_unlock_stop(GstCamBaseSrc *src)
{
  return TRUE;
}

/* ------3A interfaces implementations------ */

/* Get customized effects
* param[in]                 cam3a    Camera Source handle
* param[in, out]       img_enhancement    image enhancement(sharpness, brightness, contrast, hue, saturation)
* return            camera_image_enhancement_t
*/
static camera_image_enhancement_t
gst_camerasrc_get_image_enhancement (GstCamerasrc3A *cam3a,
    camera_image_enhancement_t img_enhancement)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->getImageEnhancement(img_enhancement);
  g_message("Interface Called: @%s, sharpness=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d.",
                               __func__, img_enhancement.sharpness, img_enhancement.brightness,
                               img_enhancement.contrast, img_enhancement.hue, img_enhancement.saturation);
  camera_get_parameters(camerasrc->device_id, *(camerasrc->param));

  return img_enhancement;
}

/* Set customized effects
* param[in]        cam3a    Camera Source handle
* param[in]        img_enhancement    image enhancement(sharpness, brightness, contrast, hue, saturation)
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_image_enhancement(GstCamerasrc3A *cam3a,
    camera_image_enhancement_t img_enhancement)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setImageEnhancement(img_enhancement);
  g_message("Interface Called: @%s, sharpness=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d.",
                               __func__, img_enhancement.sharpness, img_enhancement.brightness,
                               img_enhancement.contrast, img_enhancement.hue, img_enhancement.saturation);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  return TRUE;
}

/* Set exposure time whose unit is microsecond
* param[in]        cam3a    Camera Source handle
* param[in]        exp_time    exposure time
* return 0 if exposure time was set, non-0 means no exposure time was set
*/
static gboolean
gst_camerasrc_set_exposure_time (GstCamerasrc3A *cam3a, guint exp_time)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setExposureTime(exp_time);
  g_message("Interface Called: @%s, exposure time=%d.", __func__, exp_time);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  return TRUE;
}

/* Set iris mode
* param[in]        cam3a    Camera Source handle
* param[in]        irisMode        IRIS_MODE_AUTO(default),
*                                  IRIS_MODE_MANUAL,
*                                  IRIS_MODE_CUSTOMIZED,
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_iris_mode (GstCamerasrc3A *cam3a,
    camera_iris_mode_t irisMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setIrisMode(irisMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, irisMode=%d.", __func__, (int)irisMode);

  return TRUE;
}

/* Set iris level
* param[in]        cam3a    Camera Source handle
* param[in]        irisLevel    iris level
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_iris_level (GstCamerasrc3A *cam3a, int irisLevel)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setIrisLevel(irisLevel);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, irisLevel=%d.", __func__, irisLevel);

  return TRUE;
}

/* Set sensor gain (unit: db)
* The sensor gain only take effect when ae mode set to manual
* param[in]        cam3a    Camera Source handle
* param[in]        gain    gain
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_gain (GstCamerasrc3A *cam3a, float gain)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setSensitivityGain(gain);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, gain=%f.", __func__, gain);

  return TRUE;
}

/* Set BLC Area mode
* param[in]        cam3a    Camera Source handle
* param[in]        blcAreaMode        BLC_AREA_MODE_OFF(default),
*                                     BLC_AREA_MODE_ON,
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_blc_area_mode (GstCamerasrc3A *cam3a,
    camera_blc_area_mode_t blcAreaMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setBlcAreaMode(blcAreaMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, Blc area mode=%d.", __func__, (int)blcAreaMode);

  return TRUE;
}

/* Set WDR level
* param[in]        cam3a    Camera Source handle
* param[in]        level    wdr level
* return 0 if set successfully, otherwise non-0 value is returned.
*/
static gboolean
gst_camerasrc_set_wdr_level (GstCamerasrc3A *cam3a, uint8_t level)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setWdrLevel(level);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, wdr level=%d.", __func__, level);

  return TRUE;
}

/* Set AWB mode
* param[in]        cam3a    Camera Source handle
* param[in]        awbMode        AWB_MODE_AUTO(default),
*                                 AWB_MODE_INCANDESCENT,
*                                 AWB_MODE_FLUORESCENT,
*                                 AWB_MODE_DAYLIGHT,
*                                 AWB_MODE_FULL_OVERCAST,
*                                 AWB_MODE_PARTLY_OVERCAST,
*                                 AWB_MODE_SUNSET,
*                                 AWB_MODE_VIDEO_CONFERENCE,
*                                 AWB_MODE_MANUAL_CCT_RANGE,
*                                 AWB_MODE_MANUAL_WHITE_POINT,
*                                 AWB_MODE_MANUAL_GAIN,
*                                 AWB_MODE_MANUAL_COLOR_TRANSFORM,
* return 0 if set successfully, otherwise non-0 value is returned.
*/
static gboolean
gst_camerasrc_set_awb_mode (GstCamerasrc3A *cam3a,
    camera_awb_mode_t awbMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbMode(awbMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, awb mode=%d.", __func__, (int)awbMode);

  return TRUE;
}

/* Get customized awb gains currently used
* param[in]        cam3a    Camera Source handle
* param[in, out]        awbGains    awb gains(r_gain, g_gain, b_gain)
* return 0 if awb gain was set, non-0 means no awb gain was set.
*/
static camera_awb_gains_t
gst_camerasrc_get_awb_gain (GstCamerasrc3A *cam3a,
    camera_awb_gains_t& awbGains)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->getAwbGains(awbGains);
  camera_get_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, r_gain=%d, g_gain=%d, b_gain=%d.", __func__,
      awbGains.r_gain, awbGains.g_gain, awbGains.b_gain);

  return awbGains;
}

/* Set AWB gain
* param[in]        cam3a    Camera Source handle
* param[in]        awbGains    awb gains(r_gain, g_gain, b_gain)
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_awb_gain (GstCamerasrc3A *cam3a,
    camera_awb_gains_t awbGains)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbGains(awbGains);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, r_gain=%d, g_gain=%d, b_gain=%d.", __func__,
      awbGains.r_gain, awbGains.g_gain, awbGains.b_gain);

  return TRUE;
}

/* Set Scene mode
* param[in]        cam3a    Camera Source handle
* param[in]        sceneMode        SCENE_MODE_AUTO(default),
*                                   SCENE_MODE_HDR,
*                                   SCENE_MODE_ULL,
*                                   SCENE_MODE_HLC,
*                                   SCENE_MODE_NORMAL,
*                                   SCENE_MODE_CUSTOM_AIC,
*                                   SCENE_MODE_VIDEO_LL,
*                                   SCENE_MODE_STILL_CAPTURE,
*                                   SCENE_MODE_MAX
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_scene_mode (GstCamerasrc3A *cam3a,
    camera_scene_mode_t sceneMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setSceneMode(sceneMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, scene mode=%d.", __func__, (int)sceneMode);

  return TRUE;
}

/* Set AE mode
* param[in]        cam3a    Camera Source handle
* param[in]        aeMode        AE_MODE_AUTO,
*                                AE_MODE_MANUAL
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_ae_mode (GstCamerasrc3A *cam3a,
    camera_ae_mode_t aeMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeMode(aeMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, ae mode=%d.", __func__, (int)aeMode);

  return TRUE;
}

/* Set weight grid mode
* param[in]        cam3a    Camera Source handle
* param[in]        weightGridMode        WEIGHT_GRID_AUTO(default),
*                                        CUSTOM_WEIGHT_GRID_1,
*                                        CUSTOM_WEIGHT_GRID_2,
*                                        CUSTOM_WEIGHT_GRID_3,
*                                        CUSTOM_WEIGHT_GRID_4,
*                                        CUSTOM_WEIGHT_GRID_5,
*                                        CUSTOM_WEIGHT_GRID_6,
*                                        CUSTOM_WEIGHT_GRID_7,
*                                        CUSTOM_WEIGHT_GRID_8,
*                                        CUSTOM_WEIGHT_GRID_9,
*                                        CUSTOM_WEIGHT_GRID_10,
*                                        CUSTOM_WEIGHT_GRID_MAX
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_weight_grid_mode (GstCamerasrc3A *cam3a,
    camera_weight_grid_mode_t weightGridMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setWeightGridMode(weightGridMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, weight grid mode=%d.", __func__, (int)weightGridMode);

  return TRUE;
}

/* Set AE converge speed
* param[in]        cam3a    Camera Source handle
* param[in]        speed        CONVERGE_NORMAL(default),
*                               CONVERGE_MID,
*                               CONVERGE_LOW,
*                               CONVERGE_MAX
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_ae_converge_speed (GstCamerasrc3A *cam3a,
    camera_converge_speed_t speed)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeConvergeSpeed(speed);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, ae converge speed=%d.", __func__, (int)speed);

  return TRUE;
}

/* Set AWB converge speed
* param[in]        cam3a    Camera Source handle
* param[in]        speed        CONVERGE_NORMAL(default),
*                               CONVERGE_MID,
*                               CONVERGE_LOW,
*                               CONVERGE_MAX
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_awb_converge_speed (GstCamerasrc3A *cam3a,
    camera_converge_speed_t speed)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbConvergeSpeed(speed);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, awb converge speed=%d.", __func__, (int)speed);

  return TRUE;
}

/* Set AE converge speed mode
* param[in]        cam3a    Camera Source handle
* param[in]        mode        CONVERGE_SPEED_MODE_AIQ(default),
*                              CONVERGE_SPEED_MODE_HAL
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_ae_converge_speed_mode (GstCamerasrc3A *cam3a,
    camera_converge_speed_mode_t mode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeConvergeSpeedMode(mode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, ae converge speed mode=%d.", __func__, (int)mode);

  return TRUE;
}

/* Set AWB converge speed mode
* param[in]        cam3a    Camera Source handle
* param[in]        mode        CONVERGE_SPEED_MODE_AIQ(default),
*                              CONVERGE_SPEED_MODE_HAL
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_awb_converge_speed_mode (GstCamerasrc3A *cam3a,
    camera_converge_speed_mode_t mode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbConvergeSpeedMode(mode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, awb converge speed mode=%d.", __func__, (int)mode);

  return TRUE;
}

/* Set exposure ev
* param[in]        cam3a    Camera Source handle
* param[in]        ev    exposure EV
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_exposure_ev (GstCamerasrc3A *cam3a, int ev)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeCompensation(ev);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, ev=%d.", __func__, ev);

  return TRUE;
}

/* Set exposure priority
* param[in]        cam3a    Camera Source handle
* param[in]        priority        DISTRIBUTION_AUTO(default),
*                                  DISTRIBUTION_SHUTTER,
*                                  DISTRIBUTION_ISO,
*                                  DISTRIBUTION_APERTURE
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_exposure_priority (GstCamerasrc3A *cam3a,
    camera_ae_distribution_priority_t priority)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeDistributionPriority(priority);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, exposure priority=%d.", __func__, (int)priority);

  return TRUE;
}

/* Get AWB cct range
* Customized cct range only take effect when awb mode is set to AWB_MODE_MANUAL_CCT_RANGE
* param[in]        cam3a    Camera Source handle
* param[in, out]        cct    cct range(min, max)
* return            camera_range_t
*/
static camera_range_t
gst_camerasrc_get_awb_cct_range (GstCamerasrc3A *cam3a,
    camera_range_t& cct)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->getAwbCctRange(cct);
  camera_get_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, get cct range, min=%f, max=%f.", __func__,
      cct.min, cct.max);

  return cct;
}

/* Set AWB cct range
* param[in]        cam3a    Camera Source handle
* param[in]        cct    cct range(min, max)
* return 0 if set successfully, otherwise non-0 value is returned
*/
static gboolean
gst_camerasrc_set_awb_cct_range (GstCamerasrc3A *cam3a,
    camera_range_t cct)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbCctRange(cct);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set cct range, min=%f, max=%f.", __func__,
      cct.min, cct.max);

  return TRUE;
}

/* Get white point
* param[in]        cam3a    Camera Source handle
* param[in, out]        whitePoint    white point coordinate(x, y)
* return            camera_coordinate_t
*/
static camera_coordinate_t
gst_camerasrc_get_white_point (GstCamerasrc3A *cam3a,
    camera_coordinate_t &whitePoint)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->getAwbWhitePoint(whitePoint);
  camera_get_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, get white point, x=%d, y=%d.", __func__,
      whitePoint.x, whitePoint.y);

  return whitePoint;
}

/* Set white point
* Only take effect when awb mode is set to AWB_MODE_MANUAL_WHITE_POINT.
* The coordinate system is based on frame which is currently displayed.
* param[in]        cam3a    Camera Source handle
* param[in]        whitePoint    white point coordinate(x, y)
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_white_point (GstCamerasrc3A *cam3a,
    camera_coordinate_t whitePoint)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbWhitePoint(whitePoint);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set white point, x=%d, y=%d.", __func__,
      whitePoint.x, whitePoint.y);

  return TRUE;
}

/* Get AWB gain shift
* param[in]        cam3a    Camera Source handle
* param[in, out]        awbGainShift    gain shift(r_gain, g_gain, b_gain)
* return camera_awb_gains_t
*/
static camera_awb_gains_t
gst_camerasrc_get_awb_gain_shift (GstCamerasrc3A *cam3a,
    camera_awb_gains_t& awbGainShift)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->getAwbGainShift(awbGainShift);
  camera_get_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, r_gain=%d, g_gain=%d, b_gain=%d.", __func__,
      awbGainShift.r_gain, awbGainShift.g_gain, awbGainShift.b_gain);

  return awbGainShift;
}

/* Set AWB gain shift
* param[in]        cam3a    Camera Source handle
* param[in]        awbGainShift    gain shift(r_gain, g_gain, b_gain)
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_awb_gain_shift (GstCamerasrc3A *cam3a,
    camera_awb_gains_t awbGainShift)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAwbGainShift(awbGainShift);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, r_gain=%d, g_gain=%d, b_gain=%d.", __func__,
      awbGainShift.r_gain, awbGainShift.g_gain, awbGainShift.b_gain);

  return TRUE;
}

/* Set AE region
* param[in]        cam3a    Camera Source handle
* param[in]        aeRegions    regions(left, top, right, bottom, weight)
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_ae_region (GstCamerasrc3A *cam3a,
    camera_window_list_t aeRegions)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAeRegions(aeRegions);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s.", __func__);

  return TRUE;
}

/* Set color transform
* param[in]        cam3a    Camera Source handle
* param[in]        colorTransform    float array
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_color_transform (GstCamerasrc3A *cam3a,
    camera_color_transform_t colorTransform)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setColorTransform(colorTransform);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s.", __func__);

  return TRUE;
}

/* Set custom Aic param
* param[in]        cam3a    Camera Source handle
* param[in]        data    the pointer of destination buffer
* param[in]        length    but buffer size
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_custom_aic_param (GstCamerasrc3A *cam3a,
    const void* data, unsigned int length)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setCustomAicParam(data, length);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s.", __func__);

  return TRUE;
}

/* Set antibanding mode
* param[in]        cam3a    Camera Source handle
* param[in]        bandingMode        ANTIBANDING_MODE_AUTO,
*                                     ANTIBANDING_MODE_50HZ,
*                                     ANTIBANDING_MODE_60HZ,
*                                     ANTIBANDING_MODE_OFF,
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean
gst_camerasrc_set_antibanding_mode (GstCamerasrc3A *cam3a,
    camera_antibanding_mode_t bandingMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setAntiBandingMode(bandingMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set andtibanding mode=%d.", __func__, (int)bandingMode);

  return TRUE;
}

/* Set color range mode
* param[in]        cam3a    Camera Source handle
* param[in]        colorRangeMode     CAMERA_FULL_MODE_YUV_COLOR_RANGE,
*                                     CAMERA_REDUCED_MODE_YUV_COLOR_RANGE,
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean gst_camerasrc_set_color_range_mode (GstCamerasrc3A *cam3a,
    camera_yuv_color_range_mode_t colorRangeMode)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setYuvColorRangeMode(colorRangeMode);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set color range mode=%d.", __func__, (int)colorRangeMode);

  return TRUE;
}

/* Set exposure time range
* param[in]        cam3a        Camera Source handle
* param[in]        exposureTimeRange        the exposure time range to be set
* return TRUE if set successfully, otherwise FASLE is returned
*/
static gboolean gst_camerasrc_set_exposure_time_range(GstCamerasrc3A *cam3a,
    camera_range_t exposureTimeRange)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setExposureTimeRange(exposureTimeRange);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set exposure time range, min=%f max=%f.", __func__,
      exposureTimeRange.min, exposureTimeRange.max);

  return TRUE;
}

/* set sensitivity gain range
* param[in]        cam3a        Camera Source handle
* param[in]        sensitivityGainRange        the sensitivity gain range to be set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_sensitivity_gain_range (GstCamerasrc3A *cam3a,
    camera_range_t sensitivityGainRange)
{
  Gstcamerasrc *camerasrc = GST_CAMERASRC(cam3a);
  camerasrc->param->setSensitivityGainRange(sensitivityGainRange);
  camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s, set sensitivity gain range, min=%lf max=%lf.", __func__,
      sensitivityGainRange.min, sensitivityGainRange.max);

  return TRUE;
}


/* Set the isp control and cache the data to param
 *
* param[in]        camIsp        Camera Source handle
* param[in]        tag           The control tag
* param[in]        data          The control data
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_isp_control (GstCamerasrcIsp *camIsp, unsigned int tag, void *data)
{
  int ret = 0;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camIsp);

  if (data == NULL) {
    (camerasrc->isp_control_tags)->erase(tag);
  } else {
    (camerasrc->isp_control_tags)->insert(tag);
  }
#ifndef CHROME_SLIM_CAMHAL
  ret = camerasrc->param->setIspControl(tag, data);
#endif
  g_message("Enter %s", __func__);

  return (ret == 0 ? TRUE : FALSE);
}

/* Get the isp data
 *
* param[in]        camIsp        Camera Source handle
* param[in]        tag           The control tag
* param[out]       data          The data pointer to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_isp_control (GstCamerasrcIsp *camIsp, unsigned int tag, void * data)
{
#ifndef CHROME_SLIM_CAMHAL
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camIsp);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getIspControl(tag, data);

  return (ret == 0 ? TRUE : FALSE);
#else
  return 0;
#endif
}

/* Apply the data cached in param to isp
 *
 * param[in]        camIsp        Camera Source handle
 * return TRUE if set successfully, otherwise FALSE is returned
 */
static gboolean gst_camerasrc_apply_isp_control (GstCamerasrcIsp *camIsp)
{
#ifndef CHROME_SLIM_CAMHAL
  int ret = 0;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camIsp);

  camerasrc->param->setEnabledIspControls(*(camerasrc->isp_control_tags));
  ret = camera_set_parameters(camerasrc->device_id, *(camerasrc->param));
  g_message("Interface Called: @%s", __func__);

  return (ret == 0 ? TRUE : FALSE);
#else
  return 0;
#endif
}

/* Get the ltm tuning data
 *
* param[in]        camIsp        Camera Source handle
* param[out]       data          The ltm tuning data pointer to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_ltm_tuning_data(GstCamerasrcIsp *camIsp, void *data)
{
#ifndef CHROME_SLIM_CAMHAL
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camIsp);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getLtmTuningData(data);

  return (ret == 0 ? TRUE : FALSE);
#else
  return 0;
#endif
}

/* Set the ltm tuning data
 *
* param[in]        camIsp        Camera Source handle
* param[in]        data          The ltm tuning data pointer to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_ltm_tuning_data(GstCamerasrcIsp *camIsp, void *data)
{
#ifndef CHROME_SLIM_CAMHAL
  int ret = 0;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camIsp);
  g_message("Enter %s", __func__);

  camerasrc->param->setLtmTuningData(data);
  ret = camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  return (ret == 0 ? TRUE : FALSE);
#else
  return 0;
#endif
}

/* Set the dewarping mode
 *
* param[in]        camDewarping         Camera Source handle
* param[in]        mode                 The dewarping mode to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_dewarping_mode (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t mode)
{
  int ret = 0;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camDewarping);
  g_message("Enter %s", __func__);

  camerasrc->param->setFisheyeDewarpingMode(mode);
  ret = camera_set_parameters(camerasrc->device_id, *(camerasrc->param));

  return (ret == 0 ? TRUE : FALSE);
}

/* Get the dewarping mode
 *
* param[in]        camDewarping         Camera Source handle
* param[out]       mode                 The dewarping mode to be returned
* return TRUE if get successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_dewarping_mode (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t &mode)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camDewarping);
  g_message("Enter %s", __func__);

  camerasrc->param->getFisheyeDewarpingMode(mode);
  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getFisheyeDewarpingMode(mode);

  return (ret == 0 ? TRUE : FALSE);
}

#ifndef CHROME_SLIM_CAMHAL
/* Get the WFOV mode if enabled or not, view set operations are only available when WFOV mode is enabled
 *
* param[in]        camWFOV       Camera Source handle
* param[out]       data          wfov mode to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_wfov_mode(GstCamerasrcWFOV *camWFOV, uint8_t &data)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getWFOV(data);

  return (ret == 0 ? TRUE : FALSE);
}

/* Get the sensor mount type
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       mount_type     The sensor mount type to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_sensor_mount_type(GstCamerasrcWFOV *camWFOV, camera_mount_type_t &mount_type)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getSensorMountType(mount_type);

  return (ret == 0 ? TRUE : FALSE);
}

/* Set the view projection setting
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       projection     The projection to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_view_projection(GstCamerasrcWFOV *camWFOV, camera_view_projection_t projection)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.setViewProjection(projection);
  if (ret != 0)
      return FALSE;

  ret = camera_set_parameters(camerasrc->device_id, param);
  return (ret == 0 ? TRUE : FALSE);
}

/* Get the view projection setting
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       projection     The projection to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_view_projection(GstCamerasrcWFOV *camWFOV, camera_view_projection_t &projection)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getViewProjection(projection);

  return (ret == 0 ? TRUE : FALSE);
}

/* Set the view rotation setting
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       rotation       The rotation to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_view_rotation(GstCamerasrcWFOV *camWFOV, camera_view_rotation_t rotation)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.setViewRotation(rotation);
  if (ret != 0)
      return FALSE;

  ret = camera_set_parameters(camerasrc->device_id, param);
  return (ret == 0 ? TRUE : FALSE);
}


/* Get the view rotation setting
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       rotation       The rotation to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_view_rotation(GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &rotation)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getViewRotation(rotation);

  return (ret == 0 ? TRUE : FALSE);
}

/* Set the view fine adjustment settings
 *
* param[in]        camWFOV             Camera Source handle
* param[out]       fine_adjustments    The view fine adjustments to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_view_fine_adjustments(GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t fine_adjustments)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.setViewFineAdjustments(fine_adjustments);
  if (ret != 0)
      return FALSE;

  ret = camera_set_parameters(camerasrc->device_id, param);
  return (ret == 0 ? TRUE : FALSE);
}

/* Get the view fine adjustment settings
 *
* param[in]        camWFOV             Camera Source handle
* param[out]       fine_adjustments    The view fine adjustments to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_view_fine_adjustments(GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t &fine_adjustments)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getViewFineAdjustments(fine_adjustments);

  return (ret == 0 ? TRUE : FALSE);
}

/* Set the camera device rotation setting
 *
* param[in]        camWFOV        Camera Source handle
* param[in]        camRotation    The camera device rotation to set
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_set_camera_rotation(GstCamerasrcWFOV *camWFOV, camera_view_rotation_t camRotation)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.setCameraRotation(camRotation);

  return (ret == 0 ? TRUE : FALSE);
}


/* Get the camera device rotation setting
 *
* param[in]        camWFOV        Camera Source handle
* param[out]       camRotation    The camera device rotation to get
* return TRUE if set successfully, otherwise FALSE is returned
*/
static gboolean gst_camerasrc_get_camera_rotation(GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &camRotation)
{
  int ret = 0;
  Parameters param;
  Gstcamerasrc *camerasrc = GST_CAMERASRC(camWFOV);
  g_message("Enter %s", __func__);

  camera_get_parameters(camerasrc->device_id, param);
  ret = param.getCameraRotation(camRotation);

  return (ret == 0 ? TRUE : FALSE);
}
#endif //CHROME_SLIM_CAMHAL

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 * then GST_PLUGIN_DEFINE will work
 */
static gboolean
plugin_init (GstPlugin * Plugin)
{
  PERF_CAMERA_ATRACE();

  return gst_element_register (Plugin, "icamerasrc", GST_RANK_NONE,
        GST_TYPE_CAMERASRC);
}

/* gstreamer looks for this structure to register camerasrcs
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    icamerasrc,
    "camera source plugins based on libcamhal",
    plugin_init, VERSION,GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
