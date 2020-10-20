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

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <ICamera.h>
#include <string.h>
#include <linux/videodev2.h>

#define OK 0
#define INVALID_VALUE -1

using namespace icamera;

static int get_camera_device_id(char* input_device);
static void list_select_device(int index);
/**
* Parse Gstreamer format and return V4L2 format
**/
int string_to_integer_for_format(char* format)
{
  int int_format = 0;
  if (strcmp(format,"YUY2") == 0)  //GST_VIDEO_FORMAT_YUY2  to V4L2_PIX_FMT_YUYV
    int_format = V4L2_PIX_FMT_YUYV;
  else if (strcmp(format,"NV12") == 0) //GST_VIDEO_FORMAT_NV12 to V4L2_PIX_FMT_NV12
    int_format = V4L2_PIX_FMT_NV12;
  else if (strcmp(format,"UYVY") == 0) //GST_VIDEO_FORMAT_UYVY to V4L2_PIX_FMT_UYVY
    int_format = V4L2_PIX_FMT_UYVY;
  else if (strcmp(format,"BGRx") == 0) //GST_VIDEO_FORMAT_BGRx to V4L2_PIX_FMT_XBGR32
    int_format = V4L2_PIX_FMT_XBGR32;
  else if (strcmp(format,"RGBx") == 0) //GST_VIDEO_FORMAT_RGBx to V4L2_PIX_FMT_XRGB32
    int_format = V4L2_PIX_FMT_XRGB32;
  else if (strcmp(format,"BGRA") == 0) //GST_VIDEO_FORMAT_BGRA to V4L2_PIX_FMT_BGR32
    int_format = V4L2_PIX_FMT_BGR32;
  else if (strcmp(format,"BGR") == 0) // GST_VIDEO_FORMAT_BGR to V4L2_PIX_FMT_BGR24
    int_format = V4L2_PIX_FMT_BGR24;
  else if (strcmp(format,"RGB16") == 0) //GST_VIDEO_FORMAT_RGB16 to V4L2_PIX_FMT_RGB565
    int_format = V4L2_PIX_FMT_RGB565;
  else if (strcmp(format,"P010") == 0) //GST_VIDEO_FORMAT_P010_10BE to V4L2_PIX_FMT_P010_BE
    int_format = V4L2_PIX_FMT_P010_BE;
  else if (strcmp(format,"P01L") == 0) //GST_VIDEO_FORMAT_P010_10LE to V4L2_PIX_FMT_P010_LE
    int_format = V4L2_PIX_FMT_P010_LE;
  else {
    printf("Input format is not supported!\n");
    return INVALID_VALUE;
  }

  return int_format;
}

/**
 * Main entrance of Gstreamer tools
 *
 * run ./gst-tool --help to check supported paramters.
 *
 * Currently supported tools:
 * 1.Query one buffer from HAL and return its size
 *
 * 2.List camera device names and output the device by selection
 *
 **/
int main(int argc, char* argv[]) {
  int ch,option_index = 0;
  int camera_id,format,width,height,field,buffer_size;
  int index = -1;
  int bpp = 0;
  const char *optstring = "d:m:w:h:f:i:";
  static struct option long_options[] = {
    {"help", no_argument, 0, 'H'},
    {"getbuffersize", no_argument, 0, 0},
    {"selectdevice", no_argument, 0, 1},
    {0, 0, 0,  0}
  };

  bool get_buffer_size = false;
  bool select_device = false;

  while ((ch = getopt_long(argc,argv,optstring, long_options, &option_index)) != -1) {
      switch(ch) {
      case 0:
          if (argc != 12) {
            printf("incorrect number of input parameters!\n");
            return INVALID_VALUE;
          }
          get_buffer_size = true;
          break;
      case 1:
          select_device = true;
          break;
      case 'H':
          printf("--getbuffersize, query one buffer from HAL and return its size\n");
          printf("--help, show this message\n");
          printf("-d, followed by device name. '-d $cameraInput'\n");
          printf("-m, followed by abbreviative Gstreamer format. '-m YUY2'\n");
          printf("-w, followed by buffer width. '-w 1920'\n");
          printf("-h, followed by buffer height. '-h 1080'\n");
          printf("-f, followed by interlace field(choose from 0 and 7). '-f 0'\n");
          printf("-i, followed by device index. '-i 0'\n");
          break;
      case 'd':
          camera_id = get_camera_device_id(optarg);
          if (camera_id == INVALID_VALUE) {
              return INVALID_VALUE;
          }
          break;
      case 'm':
          format = string_to_integer_for_format(optarg);
          if (format == INVALID_VALUE) {
              return INVALID_VALUE;
          }
          break;
      case 'w':
          sscanf(optarg, "%d", &width);
          break;
      case 'h':
          sscanf(optarg, "%d", &height);
          break;
      case 'f':
          sscanf(optarg, "%d", &field);
          break;
      case 'i':
          sscanf(optarg, "%d", &index);
          break;
      default:
          printf("Input paramter is not supported!\n");
          return INVALID_VALUE;
      }
  }

  if (get_buffer_size) {
    buffer_size = get_frame_size(camera_id, format,width,height,field,&bpp);
    printf("queried buffer size is %d\n",buffer_size);
  }
  if (select_device) {
   list_select_device(index);
  }

  return OK;
}

/**
* Parse device name and return camera id
**/
static int get_camera_device_id(char* input_device)
{
  int ret;
  int count;
  int device_id;
  ret = camera_hal_init();
  if (ret < 0) {
    printf("failed to init device.\n");
    return INVALID_VALUE;
  }

  count = get_number_of_cameras();
  if (input_device != NULL ) {
    for (int id = 0; id < count; id++) {
       camera_info_t info;
       ret = get_camera_info(id, info);
       if (ret < 0) {
          printf("failed to get device name.\n");
          camera_hal_deinit();
          return INVALID_VALUE;
       }

       if(strcmp(info.name, input_device) == 0) {
          device_id = id;
          break;
       }
    }
  }

  return device_id;
}

/*
 * List currently supported devices or selected device
 */
static void list_select_device(int index) {
  camera_info_t cam_info;
  int count = get_number_of_cameras();
  for (int i=0;i<count;i++) {
    get_camera_info(i,cam_info);

    if (index == -1 || (index != -1 && i == index))
      printf("%d %s\n",i,cam_info.name);
  }
}
