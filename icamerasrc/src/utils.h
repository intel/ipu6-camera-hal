/*
 * GStreamer
 * Copyright (C) 2016-2017 Intel Corporation
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

#ifndef _GST_CAMERASRC_UTILS_H_
#define _GST_CAMERASRC_UTILS_H_

#include <gst/gst.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/syscall.h>
#include "gst/video/video.h"
#include <linux/videodev2.h>
#include <map>

using namespace std;

#define ALIGN(val, alignment) (((val)+(alignment)-1) & ~((alignment)-1))
#define ALIGN_64(val) ALIGN(val, 64)

// Macro for memcpy
#define MEMCPY_S(dest, dmax, src, smax) \
  memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))

#define ARRAY_SIZE(array)    (sizeof(array) / sizeof((array)[0]))

#define gettid() syscall(SYS_gettid)

#define PRINT_FIELD(a, f) \
      do { \
              if (a == 2)  {f = "top";} \
              else if (a == 3) {f = "bottom";} \
              else if (a == 7) {f = "alternate";} \
              else {f = "none";} \
      } while(0)

/**
* An option that can be activated on bufferpool to request dmabuf
* handles on buffers from the pool.
*/
#define GST_BUFFER_POOL_OPTION_DMABUF_MEMORY \
  "GstBufferPoolOptionDMABUFMemory"

/**
* The definition is for gstreamer msdk,which is
* in gst-bad-plugin.
*/
#define GST_BUFFER_POOL_OPTION_MSDK_USE_DMABUF \
  "GstBufferPoolOptionMsdkUseDMABuf"

namespace CameraSrcUtils {

  int gst_fmt_2_fourcc(GstVideoFormat gst_fmt);

  GstVideoFormat fourcc_2_gst_fmt(int fourcc);

  gboolean check_format_by_name(const char *name);

  int string_2_fourcc(const char *fmt_string);

  int get_number_of_valid_lines(int format, int height);

  void get_stream_info_by_caps(GstCaps *caps, const char **format, int *width, int *height);

  int get_stream_id_by_pad(map<string, int> streamMap, GstPad *pad);
}

#endif
