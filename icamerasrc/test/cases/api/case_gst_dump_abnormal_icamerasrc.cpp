/*
 * GStreamer
 * Copyright (C) 2015-2016 Intel Corporation
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

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_YUY2_1_1)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=YUY2,width=1,height=1");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_yuy2_1_1.yuv", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_NV12_1_1)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=NV12,width=1,height=1");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_nv12_1_1.nv12", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_YUY2_0_0)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=YUY2,width=0,height=0");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_yuy2_0_0.yuv", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);

}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_NV12_0_0)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=NV12,width=0,height=0");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_nv12_0_0.nv12", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_YUY2_3264_2448)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=YUY2,width=3264,height=2448");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_yuy2_3264_2448.yuv", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_NV12_3264_2448)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=NV12,width=3264,height=2448");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_nv12_3264_2448.nv12", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}

TEST(CameraSrcTest, gst_dump_abnormal_icamerasrc_filesink_I420_1920_1080)
{
    int ret = 0;
    clear_plugins();

    ret = add_plugins("icamerasrc", "num-buffers=1", "video/x-raw,format=I420,width=1920,height=1080");
    ASSERT_EQ(ret, 0);
    ret = add_plugins("filesink", "location=video_x-raw_i420_1920_1080.yuv", NULL);
    ASSERT_EQ(ret, 0);

    ret = do_pipline(NULL);
    ASSERT_NE(ret, 0);
}
