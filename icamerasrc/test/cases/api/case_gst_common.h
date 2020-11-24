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

#include "gtest/gtest.h"
#include <sys/time.h>
#include <gst/gst.h>
#include <Parameters.h>
#ifndef CHROME_SLIM_CAMHAL
#include <IspControl.h>
#include <AlgoTuning.h>
#endif
#include <gstcamera3ainterface.h>
#include <gstcameraispinterface.h>
#include <gstcameradewarpinginterface.h>
#include <gstcamerawfovinterface.h>

#define MAX_PLUGIN_SIZE 10
#define FPS_COUNT_START 20 //about 2s.
#define ISP_CONTROL_TEST_NUM 3
#define DEFAULT_TARGET_FPS 60 //60 fps.
#define TARGET_FPS 30 //30 fps.

#define FILE_DUMP_FLAG_C_MAP 1<<0
#define FILE_DUMP_FLAG_GST_MAP 1<<1
#define LTM_TUNING_DATA_TAG 1000

using namespace icamera;

struct Field3A {
  gboolean update_img_enhancement;
  gboolean update_exp_time;
  gboolean update_iris_mode;
};

struct Custom3AData {
  int exp_time = 0;
  camera_iris_mode_t irismode;
};

struct CustomIspControl {
    unsigned int tag;
    void * data;
};

typedef struct _CustomData {
    float fps;
    gboolean update_error;
    gint buffer_num_fps;
    gint buffer_num_prop;
    gint buffer_num_3a;
    gint buffer_num_isp;
    gint buffer_num_frame;
    gint isp_control_index;
    gboolean bus_error;
    gboolean test_fps;
    gboolean test_property;
    gboolean test_3a_iface;
    gboolean test_wfov;
    timeval start_time;
    timeval last_time;
    GstCamerasrc3A *cam3a;
    GstCamerasrc3AInterface *cam3a_iface;
    Field3A cam3a_field;
    Custom3AData cam3a_data;

#ifndef CHROME_SLIM_CAMHAL
    uint8_t wfov_mode;
    camera_mount_type_t mount_type;
    gboolean set_projection;
    camera_view_projection_t projection;
    gboolean set_rotation;
    camera_view_rotation_t rotation;
    gboolean set_fine_adjustments;
    camera_view_fine_adjustments_t fine_adjustments;
    gboolean set_camera_rotation;
    camera_view_rotation_t camera_rotation;
#endif //CHROME_SLIM_CAMHAL

    gboolean test_isp_control;
    CustomIspControl camisp_control;
    GstCamerasrcIsp *camisp;
    GstCamerasrcIspInterface *camisp_iface;
    gboolean test_dewarping_mode_switch;
    GstCamerasrcDewarping *camdewarping;
    GstCamerasrcDewarpingInterface *camdewarping_iface;
#ifndef CHROME_SLIM_CAMHAL
    GstCamerasrcWFOV *camwfov;
    GstCamerasrcWFOVInterface *camwfov_iface;
#endif //CHROME_SLIM_CAMHAL
    GMainLoop *main_loop;  /* GLib's Main Loop */
} CustomData;

struct FrameInfo {
    float fps;
};

struct UpdateProperty {
    gboolean update_error;
};

typedef struct _CheckField  {
    gboolean check_fps;
    gboolean check_property;
    gboolean check_3a_iface;
    gboolean check_isp_control;
    gboolean check_dewarping_mode_switch;

    Field3A field_3a;
    CustomIspControl field_isp;
    struct FrameInfo info;
    struct UpdateProperty property;

#ifndef CHROME_SLIM_CAMHAL
    gboolean check_wfov;
    gboolean set_projection;
    camera_view_projection_t projection;
    gboolean set_rotation;
    camera_view_rotation_t rotation;
    gboolean set_fine_adjustments;
    camera_view_fine_adjustments_t fine_adjustments;
#endif //CHROME_SLIM_CAMHAL
} CheckField;

typedef struct _PropertyRange {
     gint min;
     gint max;
     const char *enum_str;
} PropertyRange;

struct Plugin {
    const char *plugin_name;
    const char *pro_attrs;
    const char *caps_filter;
};

/* for customized type of cases: configure in capsfilter */
struct StreamConfig
{
  char *format;
  int width;
  int height;
  int io_mode;
  int num_buffers;
  int num_vc;
};

/* for customized type of cases: common element that are used */
struct CommonElement
{
  GstCaps *filtercaps;
  char *device_name;
  StreamConfig s;

  GstElement *bin;
  GstElement *camerasrc;
  GstElement *filter;
};

int do_pipline(CheckField *check);
void clear_plugins(void);
int add_plugins(const char *plugin_name, const char *pro_attrs, const char *caps_filter);
char *get_input_device_env(void);
char *get_sink_plugin_name(void);
int get_dump_flag(void);
int get_enum_value_from_field(GParamSpec * prop, const char *nick, PropertyRange *range);
int get_random_value(int min,int max);
