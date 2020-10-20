/*
 * GStreamer
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

#define LOG_TAG "GstCameraFormat"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <gst/gst.h>
#include <linux/videodev2.h>
#include <gst/video/video.h>
#include <vector>

#include "ICamera.h"
#include "ScopedAtrace.h"
#include "gstcamerasrc.h"
#include "gstcameraformat.h"
#include "Parameters.h"
#include "utils.h"

using namespace icamera;
using std::vector;

/**
  * Used to save the max/min width and height of corresponding format when parsing camera info
  */
typedef struct
{
  int max_w;
  int max_h;
  int min_w;
  int min_h;
} cameraSrc_Res_Range;

/**
 * Used to save the union set of resolution of corresponding format
 * This struct will be updated if the resolution range in cameraSrc_Res_Range has larger scale
 */
typedef struct
{
  int format;
  cameraSrc_Res_Range range;
} cameraSrc_Main_Res_Range;

static int register_format_and_resolution(const stream_array_t configs,
               vector <camera_resolution_t> fmt_res,
               vector <cameraSrc_Main_Res_Range> &main_res_range);
static void get_max_and_min_resolution(vector <camera_resolution_t> r, cameraSrc_Res_Range *res_range);
static void update_main_resolution(int format,
               cameraSrc_Res_Range res_range,
               vector <cameraSrc_Main_Res_Range> &main_res_range);
static GstStructure *create_structure (guint32 fourcc);
static void set_structure_to_caps(vector <cameraSrc_Main_Res_Range> main_res_range, GstCaps **caps);

static GstStructure *
create_structure (guint32 fourcc)
{
  PERF_CAMERA_ATRACE();
  GstStructure *structure = NULL;

  switch (fourcc) {
    case V4L2_PIX_FMT_NV12:{   /* 12  Y/CbCr 4:2:0  */
      structure = gst_structure_new ("video/x-raw",
          "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_NV12), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_UYVY:{
      structure = gst_structure_new ("video/x-raw",
          "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_UYVY), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_YUYV:{
      structure = gst_structure_new ("video/x-raw",
          "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_YUY2), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_SGRBG8:{
      structure = gst_structure_new_empty ("video/x-bayer");
    }
    break;
    case V4L2_PIX_FMT_XRGB32:{
      structure = gst_structure_new ("video/x-raw",
           "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_RGBx), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_BGR32:{
      structure = gst_structure_new ("video/x-raw",
           "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_BGRA), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_BGR24:{
      structure = gst_structure_new ("video/x-raw",
           "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_BGR), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_RGB565:{
      structure = gst_structure_new ("video/x-raw",
           "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_RGB16), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_NV16:{
      structure = gst_structure_new ("video/x-raw",
           "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_NV16), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_XBGR32:{
      structure = gst_structure_new ("video/x-raw",
            "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_BGRx), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_P010_BE:{
      structure = gst_structure_new ("video/x-raw",
            "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_P010_10BE), (void *)NULL);
    }
    break;
    case V4L2_PIX_FMT_P010_LE:{
      structure = gst_structure_new ("video/x-raw",
            "format", G_TYPE_STRING, gst_video_format_to_string (GST_VIDEO_FORMAT_P010_10LE), (void *)NULL);
    }
    break;
    default:
    break;
  }

  return structure;
}

/**
  * Set format and maximum range of resolution into structure
  * Merge all structures into caps
  */
#define GST_CAPS_FEATURE_MEMORY_DMABUF "memory:DMABuf"
static void
set_structure_to_caps(vector <cameraSrc_Main_Res_Range> main_res_range, GstCaps **caps)
{
  GstStructure *structure = NULL;
  int feature_index = 0;
  /* Set caps with dmabuffer */
  for (auto&res_range : main_res_range) {
    structure = create_structure (res_range.format);
    if (structure) {
      /* If has only one resolution */
      if ( res_range.range.max_w == res_range.range.min_w &&
           res_range.range.max_h == res_range.range.min_h )
        gst_structure_set (structure,
              "width", G_TYPE_INT, res_range.range.max_w,
              "height", G_TYPE_INT, res_range.range.max_h,
              "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
              NULL);
      else
        gst_structure_set (structure,
              "width", GST_TYPE_INT_RANGE, res_range.range.min_w, res_range.range.max_w,
              "height", GST_TYPE_INT_RANGE, res_range.range.min_h, res_range.range.max_h,
              "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
              NULL);

      *caps = gst_caps_merge_structure (*caps, structure);
      gst_caps_set_features(*caps,feature_index,gst_caps_features_new(GST_CAPS_FEATURE_MEMORY_DMABUF,NULL));
      feature_index++;
    }
  }
  /* Set caps with userptr */
  for (auto&res_range : main_res_range) {
    structure = create_structure (res_range.format);
    if (structure) {
      /* If has only one resolution */
      if ( res_range.range.max_w == res_range.range.min_w &&
           res_range.range.max_h == res_range.range.min_h )
        gst_structure_set (structure,
              "width", G_TYPE_INT, res_range.range.max_w,
              "height", G_TYPE_INT, res_range.range.max_h,
              "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
              NULL);
      else
        gst_structure_set (structure,
              "width", GST_TYPE_INT_RANGE, res_range.range.min_w, res_range.range.max_w,
              "height", GST_TYPE_INT_RANGE, res_range.range.min_h, res_range.range.max_h,
              "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
              NULL);

      *caps = gst_caps_merge_structure (*caps, structure);
    }
  }
}

/**
  * Parse all the resolutions saved in camera_resolution_t, select the max/min width and height
  * and assign to cameraSrc_Res_Range
  */
static void
get_max_and_min_resolution(vector <camera_resolution_t> r, cameraSrc_Res_Range *res_range)
{
  PERF_CAMERA_ATRACE();
  gboolean isFirstElem = true;

  for (auto&res : r) {
    if (isFirstElem) {
      res_range->max_w = res.width;
      res_range->max_h = res.height;
      res_range->min_w = res.width;
      res_range->min_h = res.height;
      isFirstElem = false;
    }
    camera_resolution_t rz = res;
    res_range->max_w = MAX(res_range->max_w, rz.width);
    res_range->max_h = MAX(res_range->max_h, rz.height);
    res_range->min_w = MIN(res_range->min_w, rz.width);
    res_range->min_h = MIN(res_range->min_h, rz.height);
  }
}

/**
  *  Generate the maximum range of resolution for each format, and add a new format if the format is not exist before
  */
static void
update_main_resolution(int format,
    cameraSrc_Res_Range res_range,
    vector <cameraSrc_Main_Res_Range> &main_res_range)
{
  cameraSrc_Main_Res_Range r;
  gboolean is_new_format = true;

  for (auto&main_res : main_res_range) {
    if (main_res.format == format) {
      is_new_format = false;
      main_res.range.max_w = MAX(main_res.range.max_w, res_range.max_w);
      main_res.range.max_h = MAX(main_res.range.max_h, res_range.max_h);
      main_res.range.min_w = MIN(main_res.range.min_w, res_range.min_w);
      main_res.range.min_h = MIN(main_res.range.min_h, res_range.min_h);
    }
  }

  if (is_new_format) {
    r.format = format;
    r.range = res_range;
    main_res_range.push_back(r);
  }
}

/**
  * Read all supported formats, width, height from Camera info
  * Register the union set of resolution of each format into cameraSrc_Main_Res_Range
  */
static int
register_format_and_resolution(const stream_array_t configs,
    vector <camera_resolution_t> fmt_res,
    vector <cameraSrc_Main_Res_Range> &main_res_range)
{
    PERF_CAMERA_ATRACE();
    int currentFormat = -1;
    size_t next_res_idx = 0;
    cameraSrc_Res_Range res_range;
    camera_resolution_t r;

    for (size_t j = 0; j < configs.size(); j++) {
      next_res_idx = j+1;
      memset(&res_range, 0, sizeof(cameraSrc_Res_Range));
      currentFormat = configs[j].format;

      if (configs[j].field == GST_CAMERASRC_INTERLACE_FIELD_ANY) {
        r.width = configs[j].width;
        r.height = configs[j].height;
        fmt_res.push_back(r);
      }

      //calculate the main resolution for a single format
      if (fmt_res.size() > 0 &&
                      ((next_res_idx < configs.size() && currentFormat != configs[next_res_idx].format) ||
                      next_res_idx == configs.size())) {
        get_max_and_min_resolution(fmt_res, &res_range);
        update_main_resolution(currentFormat, res_range, main_res_range);
        fmt_res.clear();
      }
    }
    return (currentFormat == -1)?-1:0;
}

GstCaps *gst_camerasrc_get_all_caps ()
{
  PERF_CAMERA_ATRACE();

  vector <camera_resolution_t> fmt_res;
  vector <cameraSrc_Main_Res_Range> main_res_range;

  static GstCaps *caps = gst_caps_new_empty ();
  int count = get_number_of_cameras();

  for(int i = 0; i < count; i++) {
    stream_array_t configs;
    camera_info_t info;

    //get configuration of camera
    int ret = get_camera_info(i, info);
    if (ret != 0) {
      GST_ERROR("failed to get camera info from libcamhal");
      gst_caps_unref(caps);
      return NULL;
    }
    info.capability->getSupportedStreamConfig(configs);

    ret = register_format_and_resolution(configs, fmt_res, main_res_range);
    if (ret != 0) {
        GST_ERROR("failed to get format info from libcamhal");
        gst_caps_unref(caps);
        return NULL;
    }
  }

  set_structure_to_caps(main_res_range, &caps);
  main_res_range.clear();

  return gst_caps_simplify(caps);
}
