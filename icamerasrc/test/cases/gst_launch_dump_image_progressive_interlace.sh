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

#Dump progressive image file for YUY2 format
function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_320_240() {
  template_dump_image_progressive_interlace YUY2 320 240
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace YUY2 640 480
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_720_576_fps() {
    template_preview_progressive_interlace YUY2 720 576
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace YUY2 1280 720
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1280_800() {
  template_dump_image_progressive_interlace YUY2 1280 800
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace YUY2 1920 1080
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_720_480() {
  template_dump_image_progressive_interlace YUY2 720 480
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_720_576() {
  template_dump_image_progressive_interlace YUY2 720 576
}

function gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_1920_1080() {
  template_dump_image_progressive_interlace YUY2 1920 1080
}

#Dump progressive image file for NV12 format
function gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_320_240() {
  template_dump_image_progressive_interlace NV12 320 240
}

function gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace NV12 640 480
}

function gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace NV12 1280 720
}

function gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1280_800() {
  template_dump_image_progressive_interlace NV12 1280 800
}

function gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace NV12 1920 1080
}


#Dump progressive/interlace image file for UYVY format
function gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace UYVY 640 480
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_720_576() {
  template_dump_image_progressive_interlace UYVY 720 576
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace UYVY 1280 720
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace UYVY 1920 1080
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_720_480() {
  template_dump_image_progressive_interlace UYVY 720 480
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_720_576() {
  template_dump_image_progressive_interlace UYVY 720 576
}

function gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_1920_1080() {
  template_dump_image_progressive_interlace UYVY 1920 1080
}


#Dump interlace image file for BGRx format
function gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_720_480() {
  template_dump_image_progressive_interlace BGRx 720 480
}

function gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_720_576() {
  template_dump_image_progressive_interlace BGRx 720 576
}

function gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_1920_1080() {
  template_dump_image_progressive_interlace BGRx 1920 1080
}


#Dump interlace image file for RGBx format
function gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_720_480() {
  template_dump_image_progressive_interlace RGBx 720 480
}

function gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_720_576() {
  template_dump_image_progressive_interlace RGBx 720 576
}

function gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_1920_1080() {
  template_dump_image_progressive_interlace RGBx 1920 1080
}

#Dump progressive image file for BGR(V4L2_PIX_FMT_BGR24) format
function gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace BGR 640 480
}

function gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_720_576() {
  template_dump_image_progressive_interlace BGR 720 576
}

function gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace BGR 1280 720
}

function gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace BGR 1920 1080
}

#Dump progressive image file for RGB16(V4L2_PIX_FMT_RGB565) format
function gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace RGB16 640 480
}

function gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_720_576() {
  template_dump_image_progressive_interlace RGB16 720 576
}

function gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace RGB16 1280 720
}

function gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace RGB16 1920 1080
}

#Dump Progressive image file for NV16 format
function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_640_480() {
  template_dump_image_progressive_interlace NV16 640 480
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_720_576() {
  template_dump_image_progressive_interlace NV16 720 576
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_800_480() {
  template_dump_image_progressive_interlace NV16 800 480
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1280_720() {
  template_dump_image_progressive_interlace NV16 1280 720
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1280_800() {
  template_dump_image_progressive_interlace NV16 1280 800
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1920_1080() {
  template_dump_image_progressive_interlace NV16 1920 1080
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_720_480() {
  template_dump_image_progressive_interlace NV16 720 480
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_720_576() {
  template_dump_image_progressive_interlace NV16 720 576
}

function gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_1920_1080() {
  template_dump_image_progressive_interlace NV16 1920 1080
}

#Dump image file for Dual Camera
function gst_launch_dump_dual_image_icamerasrc_UYVY_filesink_1920_1080() {
  template_multiple_streams 2 UYVY 1920x1080 UYVY 1920x1080
}

function gst_launch_dump_dual_image_icamerasrc_BGR_filesink_1920_1080() {
  template_multiple_streams 2 BGR 1920x1080 BGR 1920x1080
}

function gst_launch_dump_dual_image_icamerasrc_RGB16_filesink_1920_1080() {
  template_multiple_streams 2 RGB16 1920x1080 RGB16 1920x1080
}

function gst_launch_dump_dual_image_icamerasrc_NV16_filesink_1920_1080() {
  template_multiple_streams 2 NV16 1920x1080 NV16 1920x1080
}

#Dump image file for Virtual Channel
function gst_launch_dump_virtual_image_icamerasrc_UYVY_filesink_640_480() {
  template_multiple_streams 4 UYVY 640x480 UYVY 640x480 UYVY 640x480 UYVY 640x480
}

function gst_launch_dump_virtual_image_icamerasrc_UYVY_filesink_1280_800() {
  template_multiple_streams 4 UYVY 1280x800 UYVY 1280x800 UYVY 1280x800 UYVY 1280x800
}

function gst_launch_dump_virtual_image_icamerasrc_NV16_filesink_1920_1080() {
  template_multiple_streams 4 NV16 1920x1080 NV16 1920x1080 NV16 1920x1080 NV16 1920x1080
}

REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_320_240
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1280_800
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_progressive_1920_1080
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_720_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_YUY2_filesink_interlace_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_320_240
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1280_800
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV12_filesink_progressive_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_progressive_1920_1080
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_720_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_UYVY_filesink_interlace_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_720_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_BGRx_filesink_interlace_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_720_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_RGBx_filesink_interlace_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_BGR_filesink_progressive_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_RGB16_filesink_progressive_1920_1080

REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_640_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_800_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1280_720
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1280_800
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_progressive_1920_1080
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_720_480
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_720_576
REGISTER_CASES gst_launch_dump_image_icamerasrc_NV16_filesink_interlace_1920_1080

REGISTER_CASES gst_launch_dump_dual_image_icamerasrc_UYVY_filesink_1920_1080
REGISTER_CASES gst_launch_dump_dual_image_icamerasrc_BGR_filesink_1920_1080
REGISTER_CASES gst_launch_dump_dual_image_icamerasrc_RGB16_filesink_1920_1080
REGISTER_CASES gst_launch_dump_dual_image_icamerasrc_NV16_filesink_1920_1080

REGISTER_CASES gst_launch_dump_virtual_image_icamerasrc_UYVY_filesink_640_480
REGISTER_CASES gst_launch_dump_virtual_image_icamerasrc_UYVY_filesink_1280_800
REGISTER_CASES gst_launch_dump_virtual_image_icamerasrc_NV16_filesink_1920_1080
