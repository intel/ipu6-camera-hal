/*
 * GStreamer
 * Copyright (C) 2018 Intel Corporation
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

#define LOG_TAG "GstCameraUtils"

#include "utils.h"

struct FormatCvt {
  const char *gst_fmt_string;
  GstVideoFormat gst_pixel;
  int v4l2_pixel;
};

static const FormatCvt gFormatMapping[] = {
  { "YUY2", GST_VIDEO_FORMAT_YUY2, V4L2_PIX_FMT_YUYV },
  { "UYVY", GST_VIDEO_FORMAT_UYVY, V4L2_PIX_FMT_UYVY },
  { "NV12", GST_VIDEO_FORMAT_NV12, V4L2_PIX_FMT_NV12 },
  { "RGBx", GST_VIDEO_FORMAT_RGBx, V4L2_PIX_FMT_XRGB32 },
  { "BGRA", GST_VIDEO_FORMAT_BGRA, V4L2_PIX_FMT_BGR32 },
  { "BGR", GST_VIDEO_FORMAT_BGR, V4L2_PIX_FMT_BGR24 },
  { "RGB16", GST_VIDEO_FORMAT_RGB16, V4L2_PIX_FMT_RGB565 },
  { "NV16", GST_VIDEO_FORMAT_NV16, V4L2_PIX_FMT_NV16 },
  { "BGRx", GST_VIDEO_FORMAT_BGRx, V4L2_PIX_FMT_XBGR32 },
  { "P010", GST_VIDEO_FORMAT_P010_10BE, V4L2_PIX_FMT_P010_BE },
  { "P01L", GST_VIDEO_FORMAT_P010_10LE, V4L2_PIX_FMT_P010_LE },
};

int num_of_format = ARRAY_SIZE(gFormatMapping);

int CameraSrcUtils::gst_fmt_2_fourcc(GstVideoFormat gst_fmt)
{
  for (int i = 0; i < num_of_format; i++) {
    if (gFormatMapping[i].gst_pixel == gst_fmt)
      return gFormatMapping[i].v4l2_pixel;
  }

  return -1;
}

GstVideoFormat CameraSrcUtils::fourcc_2_gst_fmt(int fourcc)
{
  for (int j = 0; j < num_of_format; j++) {
    if (gFormatMapping[j].v4l2_pixel == fourcc)
      return gFormatMapping[j].gst_pixel;
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

gboolean CameraSrcUtils::check_format_by_name(const char *name)
{
  for (int k = 0; k < num_of_format; k++) {
    if (strcmp (gFormatMapping[k].gst_fmt_string, name) == 0)
      return TRUE;
  }

  return FALSE;
}

int CameraSrcUtils::string_2_fourcc(const char *fmt_string)
{
  for (int m = 0; m < num_of_format; m++) {
    if (strcmp(gFormatMapping[m].gst_fmt_string, fmt_string) == 0)
      return gFormatMapping[m].v4l2_pixel;
  }

  return -1;
}


/* This function is used for interlaced frame
 * It will return the number of lines that contains valid data
 * For packed format, 'Y' and 'UY' conponents are stored in a single array
 * For planar format, 'Y' and 'UY' conponents are stored separately */
int CameraSrcUtils::get_number_of_valid_lines(int format, int height)
{
  switch(format) {
    /* Planar formats */
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
    case V4L2_PIX_FMT_P010_BE:
    case V4L2_PIX_FMT_P010_LE:
      return height*3/2;
    case V4L2_PIX_FMT_NV16:
      return height*2;
    /* Packed formats */
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_XRGB32:
    case V4L2_PIX_FMT_BGR24:
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_BGR32:
      return height;
    default:
      break;
  }

  return 0;
}

void CameraSrcUtils::get_stream_info_by_caps(GstCaps *caps, const char **format, int *width, int *height)
{
  const GstStructure *structure = gst_caps_get_structure(caps, 0);
  *format = gst_structure_get_string(structure, "format");
  gst_structure_get_int(structure, "width", width);
  gst_structure_get_int(structure, "height", height);
}

int CameraSrcUtils::get_stream_id_by_pad(map<string, int> streamMap, GstPad *pad)
{
  int stream_id = -1;

  auto iter = streamMap.find(gst_pad_get_name(pad));
  if (iter != streamMap.end())
    stream_id = iter->second;
  else {
    GST_ERROR("failed to get StreamId: %d", stream_id);
  }

  return stream_id;
}
