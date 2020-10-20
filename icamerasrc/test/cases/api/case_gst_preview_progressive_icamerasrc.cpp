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

#include <string.h>
#include <math.h>
#include "case_gst_common.h"

TEST(CameraSrcTest, gst_preview_icamerasrc_fpsdisplay_YUY2_1920_1080_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("videoconvert", NULL, "video/x-raw,format=YUY2");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fpsdisplaysink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}


TEST(CameraSrcTest, gst_preview_icamerasrc_YUY2_320_240_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=320,height=240");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_YUY2_640_480_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=640,height=480");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_YUY2_1280_720_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=1280,height=720");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_YUY2_1280_800_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=1280,height=800");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_YUY2_1920_1080_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_NV12_320_240_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=320,height=240");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_NV12_640_480_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=640,height=480");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_NV12_1280_720_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1280,height=720");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_NV12_1280_800_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1280,height=800");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_NV12_1920_1080_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_UYVY_640_480_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=UYVY,width=640,height=480");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_UYVY_720_576_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=UYVY,width=720,height=576");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_UYVY_1280_720_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=UYVY,width=1280,height=720");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_UYVY_1920_1080_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=500 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=UYVY,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);
    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

// fps test cases.
TEST(CameraSrcTest, gst_preview_icamerasrc_fakesink_NV12_320_240_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=320,height=240");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fakesink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_fakesink_NV12_640_480_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=640,height=480");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fakesink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_fakesink_NV12_1280_720_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1280,height=720");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fakesink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_fakesink_NV12_1280_800_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1280,height=800");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fakesink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_preview_icamerasrc_fakesink_NV12_1920_1080_fps)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));

    memset(&check, 0, sizeof(CheckField));
    ret = add_plugins("icamerasrc", property, "video/x-raw,format=NV12,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("fakesink", NULL, NULL);
    ASSERT_EQ(ret, 0);

    check.check_fps = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    g_print("the fps is %02f, target fps: %d\n", check.info.fps, DEFAULT_TARGET_FPS);
    if (check.info.fps < DEFAULT_TARGET_FPS)
        ret = -1;

    ASSERT_EQ(ret, 0);
}

TEST(CameraSrcTest, gst_dewarping_mode_switch)
{
    int ret = 0;
    CheckField check;
    clear_plugins();
    char property[256] = "DISPLAY=:0 input-format=UYVY input-width=1280 input-height=720 fisheye-dewarping-mode=rearview num-buffers=1000 device-name=";
    char *device_name = get_input_device_env();
    ASSERT_NE(strlen(device_name), 0);
    strncat(property,device_name,strlen(device_name));
    char *sink_name = get_sink_plugin_name();
    const char *convert = NULL, *sink = NULL, *sink_filter = NULL;

    if (strcmp(sink_name, "mfx") == 0) {
        sink = "mfxsink";
        convert = "mfxvpp";
    } else {
        sink = "vaapisink";
        convert = "vaapipostproc";
    }

    memset(&check, 0, sizeof(CheckField));

    ret = add_plugins("icamerasrc", property, "video/x-raw,format=YUY2,width=1280,height=768");
    ASSERT_EQ(ret, 0);
    ret = add_plugins(convert, NULL, sink_filter);
    ASSERT_EQ(ret, 0);

    ret = add_plugins(sink, NULL, NULL);
    ASSERT_EQ(ret, 0);
    g_print("gst command line = %s\n", property);

    check.check_dewarping_mode_switch = true;
    ret = do_pipline(&check);
    ASSERT_EQ(ret, 0);

    if ((check.check_fps) && (check.info.fps < TARGET_FPS)) {
        g_print("the fps is %02f, target fps: %d\n", check.info.fps, TARGET_FPS);
        ret = -1;
    }

    ASSERT_EQ(ret, 0);
}
