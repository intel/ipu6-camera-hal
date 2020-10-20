/*
 * GStreamer
 * Copyright (C) 2015-2017 Intel Corporation
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

#define LOG_TAG "GstCameraDeinterlace"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "gst/video/video.h"
#include "gst/video/gstvideometa.h"
#include "gst/video/gstvideopool.h"

#include "ICamera.h"
#include "ScopedAtrace.h"

#include "gstcamerasrcbufferpool.h"
#include "gstcamerasrc.h"
#include <iostream>
#include <time.h>
#include "utils.h"

using namespace icamera;

void gst_camerasrc_copy_field(Gstcamerasrc *camerasrc,
       camera_buffer_t *src,
       camera_buffer_t *dst)
{
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL_CAST(camerasrc);
  int stream_id = pool->stream_id;

  if (camerasrc->deinterlace_method != GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE)
    return;

  const int bytes_of_line = camerasrc->streams[stream_id].bpl;
  const int height = CameraSrcUtils::get_number_of_valid_lines(camerasrc->s[stream_id].format,
                         camerasrc->s[stream_id].height);

  int total_len = bytes_of_line * height / 2;
  MEMCPY_S((char *)dst->addr, total_len, (char *)src->addr, total_len);
}

static int
gst_camerasrc_deinterlace_sw_bob(Gstcamerasrc *camerasrc,
               camera_buffer_t *buffer)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL_CAST(camerasrc);
  int stream_id = pool->stream_id;
  char *addr = (char *)buffer->addr;
  const int bytes_of_line = camerasrc->streams[stream_id].bpl;
  const int height = CameraSrcUtils::get_number_of_valid_lines(camerasrc->s[stream_id].format,
                         camerasrc->s[stream_id].height);

  for (int i = height/2; i > 0 ; i--) {
    MEMCPY_S(addr + (i*2-1)*bytes_of_line, bytes_of_line, addr + (i-1)*bytes_of_line, bytes_of_line);
    MEMCPY_S(addr + (i*2-2)*bytes_of_line, bytes_of_line, addr + (i-1)*bytes_of_line, bytes_of_line);
  }

  /* Update buffer flag because it's progressive frame*/
  buffer->s.field = V4L2_FIELD_NONE;

  return 0;
}

static int
gst_camerasrc_deinterlace_sw_weave(Gstcamerasrc *camerasrc,
               camera_buffer_t * top,
               camera_buffer_t * bottom,
               camera_buffer_t * dest)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL_CAST(camerasrc);
  int stream_id = pool->stream_id;
  char *addr = (char *)dest->addr;
  const int bytes_of_line = camerasrc->streams[stream_id].bpl;
  const int height = CameraSrcUtils::get_number_of_valid_lines(camerasrc->s[stream_id].format,
                         camerasrc->s[stream_id].height);

  /* Weave topfield buffer and bottomfield buffer into output buffer */
  for (int i = height/2; i > 0; i--) {
    MEMCPY_S(addr + (i*2-1)*bytes_of_line, bytes_of_line,
              (char *)bottom->addr + (i-1)*bytes_of_line, bytes_of_line);
    MEMCPY_S(addr + (i*2-2)*bytes_of_line, bytes_of_line,
              (char *)top->addr + (i-1)*bytes_of_line, bytes_of_line);
  }
  /* Update buffer flag because it's progressive frame*/
  dest->s.field = V4L2_FIELD_NONE;
  return 0;
}

int gst_camerasrc_deinterlace_frame(Gstcamerasrc *camerasrc, camera_buffer_t *buffer)
{
  PERF_CAMERA_ATRACE();
  GstCamerasrcBufferPool *pool = GST_CAMERASRC_BUFFER_POOL_CAST(camerasrc);
  int stream_id = pool->stream_id;

  switch (camerasrc->deinterlace_method) {
    case GST_CAMERASRC_DEINTERLACE_METHOD_NONE:
    case GST_CAMERASRC_DEINTERLACE_METHOD_HARDWARE_WEAVE:
      break;
    case GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_BOB:
      return gst_camerasrc_deinterlace_sw_bob(camerasrc, buffer);
    case GST_CAMERASRC_DEINTERLACE_METHOD_SOFTWARE_WEAVE:
      return gst_camerasrc_deinterlace_sw_weave(camerasrc,
                 camerasrc->streams[stream_id].top,
                 camerasrc->streams[stream_id].bottom,
                 buffer);
    default:
      break;
  }

  return 0;
}
