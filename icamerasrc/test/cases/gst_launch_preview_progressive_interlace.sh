#!/bin/bash

#
#  GStreamer
#  Copyright (C) 2015-2016 Intel Corporation
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
#  Alternatively, the contents of this file may be used under the
#  GNU Lesser General Public License Version 2.1 (the "LGPL"), in
#  which case the following provisions apply instead of the ones
#  mentioned above:
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public
#  License along with this library; if not, write to the
#  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#  Boston, MA 02111-1307, USA.
#

#Progressive preview of YUYV format
function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_320_240_fps() {
    template_preview_progressive_interlace YUY2 320 240
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_640_480_fps() {
    template_preview_progressive_interlace YUY2 640 480
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_720_576_fps() {
    template_preview_progressive_interlace YUY2 720 576
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1280_720_fps() {
    template_preview_progressive_interlace YUY2 1280 720
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1280_800_fps() {
    template_preview_progressive_interlace YUY2 1280 800
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1920_1080_fps() {
    template_preview_progressive_interlace YUY2 1920 1080
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_720_480_fps() {
  template_preview_progressive_interlace YUY2 720 480
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_720_576_fps() {
  template_preview_progressive_interlace YUY2 720 576
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_1920_1080_fps() {
  template_preview_progressive_interlace YUY2 1920 1080
}

#Progressive preview of NV12 format
function gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_320_240_fps() {
    template_preview_progressive_interlace NV12 320 240
}

function gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_640_480_fps() {
    template_preview_progressive_interlace NV12 640 480
}

function gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1280_720_fps() {
    template_preview_progressive_interlace NV12 1280 720
}

function gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1280_800_fps() {
    template_preview_progressive_interlace NV12 1280 800
}

function gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1920_1080_fps_3A() {
    template_preview_progressive_interlace NV12 1920 1080
}


#Progressive/interlace preview of UYVY format
function gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_640_480_fps() {
    template_preview_progressive_interlace UYVY 640 480
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_720_576_fps() {
    template_preview_progressive_interlace UYVY 720 576
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_1280_720_fps() {
    template_preview_progressive_interlace UYVY 1280 720
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_1920_1080_fps() {
    template_preview_progressive_interlace UYVY 1920 1080
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_720_480_fps() {
  template_preview_progressive_interlace UYVY 720 480
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_720_576_fps() {
  template_preview_progressive_interlace UYVY 720 576
}

function gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_1920_1080_fps() {
  template_preview_progressive_interlace UYVY 1920 1080
}

function gst_launch_none_icamerasrc_UYVY_fakesink_progressive_1920_1080_fps() {
  template_preview_progressive_interlace UYVY 1920 1080
}

function gst_launch_none_icamerasrc_UYVY_fakesink_interlace_720_480_fps() {
  template_preview_progressive_interlace UYVY 720 480
}


#interlace preview of BGRx format
function gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_720_480_fps() {
  template_preview_progressive_interlace BGRx 720 480
}

function gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_720_576_fps() {
  template_preview_progressive_interlace BGRx 720 576
}

function gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_1920_1080_fps() {
  template_preview_progressive_interlace BGRx 1920 1080
}

function gst_launch_none_icamerasrc_BGRx_fakesink_interlace_720_480_fps() {
  template_preview_progressive_interlace BGRx 720 480
}

#interlace preview of RGBx format
function gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_720_480_fps() {
  template_preview_progressive_interlace RGBx 720 480
}

function gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_720_576_fps() {
  template_preview_progressive_interlace RGBx 720 576
}

function gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_1920_1080_fps() {
  template_preview_progressive_interlace RGBx 1920 1080
}

function gst_launch_none_icamerasrc_RGBx_fakesink_interlace_720_480_fps() {
  template_preview_progressive_interlace RGBx 720 480
}

#Progressive preview of BGR(V4L2_PIX_FMT_BGR24) format
function gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_640_480_fps() {
  template_preview_progressive_interlace BGR 640 480
}

function gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_720_576_fps() {
  template_preview_progressive_interlace BGR 720 576
}

function gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_1280_720_fps() {
  template_preview_progressive_interlace BGR 1280 720
}

function gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_1920_1080_fps() {
  template_preview_progressive_interlace BGR 1920 1080
}

#Progressive preview of RGB16(V4L2_PIX_FMT_RGB565) format
function gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_640_480_fps() {
  template_preview_progressive_interlace RGB16 640 480
}

function gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_720_576_fps() {
  template_preview_progressive_interlace RGB16 720 576
}

function gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_1280_720_fps() {
  template_preview_progressive_interlace RGB16 1280 720
}

function gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_1920_1080_fps() {
  template_preview_progressive_interlace RGB16 1920 1080
}

#Progressive preview of NV16(V4L2_PIX_FMT_NV16) format
function gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_640_480_fps() {
  template_preview_progressive_interlace NV16 640 480
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_720_576_fps() {
  template_preview_progressive_interlace NV16 720 576
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_1280_720_fps() {
  template_preview_progressive_interlace NV16 1280 720
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_1920_1080_fps() {
  template_preview_progressive_interlace NV16 1920 1080
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_720_480_fps() {
  template_preview_progressive_interlace NV16 720 480
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_720_576_fps() {
  template_preview_progressive_interlace NV16 720 576
}

function gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_1920_1080_fps() {
  template_preview_progressive_interlace NV16 1920 1080
}

function gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1920_1080_inputformat_UYVY() {
  template_preview_progressive_interlace YUY2 1920 1080 UYVY
}

# Preview of Dual Camera
function gst_launch_preview_dual_icamerasrc_UYVY_vaapisink_1920_1080() {
  template_multiple_streams 2 UYVY 1920x1080 UYVY 1920x1080
}

function gst_launch_preview_dual_icamerasrc_BGR_ximagesink_1920_1080() {
  template_multiple_streams 2 BGR 1920x1080 BGR 1920x1080
}

function gst_launch_preview_dual_icamerasrc_RGB16_ximagesink_1920_1080() {
  template_multiple_streams 2 RGB16 1920x1080 RGB16 1920x1080
}

function gst_launch_preview_dual_icamerasrc_NV16_ximagesink_1920_1080() {
  template_multiple_streams 2 NV16 1920x1080 NV16 1920x1080
}

# Preview of Virtual Channel
function gst_launch_preview_virtual_icamerasrc_UYVY_vaapisink_640_480() {
  template_multiple_streams 4 UYVY 640x480 UYVY 640x480 UYVY 640x480 UYVY 640x480
}

function gst_launch_preview_virtual_icamerasrc_UYVY_vaapisink_1280_800() {
  template_multiple_streams 4 UYVY 1280x800 UYVY 1280x800 UYVY 1280x800 UYVY 1280x800
}

REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_320_240_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1280_800_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1920_1080_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_720_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_interlace_1920_1080_fps

REGISTER_CASES gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_320_240_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1280_800_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV12_vaapisink_progressive_1920_1080_fps_3A

REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_progressive_1920_1080_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_720_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_UYVY_vaapisink_interlace_1920_1080_fps
REGISTER_CASES gst_launch_none_icamerasrc_UYVY_fakesink_progressive_1920_1080_fps
REGISTER_CASES gst_launch_none_icamerasrc_UYVY_fakesink_interlace_720_480_fps

REGISTER_CASES gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_720_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_BGRx_vaapisink_interlace_1920_1080_fps
REGISTER_CASES gst_launch_none_icamerasrc_BGRx_fakesink_interlace_720_480_fps

REGISTER_CASES gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_720_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_RGBx_vaapisink_interlace_1920_1080_fps
REGISTER_CASES gst_launch_none_icamerasrc_RGBx_fakesink_interlace_720_480_fps

REGISTER_CASES gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_BGR_ximagesink_progressive_1920_1080_fps

REGISTER_CASES gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_RGB16_ximagesink_progressive_1920_1080_fps

REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_640_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_1280_720_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_progressive_1920_1080_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_720_480_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_720_576_fps
REGISTER_CASES gst_launch_preview_icamerasrc_NV16_ximagesink_interlace_1920_1080_fps

REGISTER_CASES gst_launch_preview_icamerasrc_YUY2_vaapisink_progressive_1920_1080_inputformat_UYVY

REGISTER_CASES gst_launch_preview_dual_icamerasrc_UYVY_vaapisink_1920_1080
REGISTER_CASES gst_launch_preview_dual_icamerasrc_BGR_ximagesink_1920_1080
REGISTER_CASES gst_launch_preview_dual_icamerasrc_RGB16_ximagesink_1920_1080
REGISTER_CASES gst_launch_preview_dual_icamerasrc_NV16_ximagesink_1920_1080

REGISTER_CASES gst_launch_preview_virtual_icamerasrc_UYVY_vaapisink_640_480
REGISTER_CASES gst_launch_preview_virtual_icamerasrc_UYVY_vaapisink_1280_800
