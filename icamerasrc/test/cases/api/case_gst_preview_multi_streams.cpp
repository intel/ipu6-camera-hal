/*
 * GStreamer
 * Copyright (C) 2017 Intel Corporation
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

#include "case_gst_common.h"
#include <glib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

TEST (CameraSrcTest, gst_multi_streams_icamerasrc_vaapisink_NV12_1920_1080_progressive)
{
  GstElement *pipeline, *icamerasrc;
  GstElement *queue0, *filter0, *vaapisink0, *queue1, *filter1, *vaapisink1;
  GstCaps *filtercaps0, *filtercaps1;
  GstPadTemplate *videopad_templ;
  GstPad *srcpad, *videopad;
  GstPad *queue_srcpad, *queue_videopad;
  GstBus *bus;
  GstMessage *msg;

  /* Set environment */
  if (putenv((char *)("DISPLAY=:0")) != 0) {
      g_error("Failed to set display env.");
      FAIL();
  }

  /* Init Gstreamer */
  gst_init(NULL, NULL);

  /* Create elements */
  icamerasrc = gst_element_factory_make("icamerasrc", "icamerasrc");
  /* create stream0 elements */
  queue0 = gst_element_factory_make("queue", "queue0");
  filter0 = gst_element_factory_make("capsfilter", "filter0");
  filtercaps0 = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "NV12",
                  "width", G_TYPE_INT, 1920,
                  "height", G_TYPE_INT, 1080,
                  NULL);
  vaapisink0 = gst_element_factory_make("vaapisink", "vaapisink0");
  /* create stream1 elements */
  queue1 = gst_element_factory_make("queue", "queue1");
  filter1 = gst_element_factory_make("capsfilter", "filter1");
  filtercaps1 = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "NV12",
                  "width", G_TYPE_INT, 1920,
                  "height", G_TYPE_INT, 1080,
                  NULL);
  vaapisink1 = gst_element_factory_make("vaapisink", "vaapisink1");

  /* Create empty pipeline */
  pipeline = gst_pipeline_new("multi-streams-pipeline");

  if (!pipeline || !icamerasrc || !filter0 || !queue0 || !vaapisink0 ||
    !filter1 || !queue1 || !vaapisink1) {
    g_error("failed to create element");
    FAIL();
  }

  /* Configure elements */
  char *device_name = get_input_device_env();
  ASSERT_NE(strlen(device_name), 0);
  GObjectClass *oclass = G_OBJECT_GET_CLASS(icamerasrc);
  GParamSpec *prop = g_object_class_find_property (oclass, "device-name");
  int cam_id = get_enum_value_from_field(prop, device_name, NULL);
  g_object_set(icamerasrc, "device-name", cam_id,
    "scene-mode", 4/* normal mode */,
    "num-buffers", 500,
    "printfps", 1,
    NULL);
  g_object_set(G_OBJECT(filter0), "caps", filtercaps0, NULL);
  g_object_set(G_OBJECT(filter1), "caps", filtercaps1, NULL);
  gst_caps_unref(filtercaps0);
  gst_caps_unref(filtercaps1);

  /* Link all elements */
  gst_bin_add_many(GST_BIN(pipeline), icamerasrc, queue0, filter0, vaapisink0,
    queue1, filter1, vaapisink1, NULL);
  if (gst_element_link_many(icamerasrc, queue0, filter0, vaapisink0, NULL) != TRUE ||
      gst_element_link_many(queue1, filter1, vaapisink1, NULL) != TRUE) {
    g_error("failed to link elements.");
    gst_object_unref(pipeline);
    FAIL();
  }

  videopad_templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(icamerasrc), "video");
  videopad = gst_element_request_pad(icamerasrc, videopad_templ, "video", NULL);
  queue_videopad = gst_element_get_static_pad(queue1, "sink");
  if (gst_pad_link(videopad, queue_videopad) != GST_PAD_LINK_OK) {
    g_error("failed to link request pad.");
    FAIL();
  }

  gst_object_unref(queue_videopad);

  /* Start playing the pipeline */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus,
    GST_CLOCK_TIME_NONE, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  /* Release the request pad and unref it */
  gst_element_release_request_pad(icamerasrc, videopad);
  gst_object_unref(videopad);


  /* Cease pipeline */
  if (msg != NULL)
    gst_message_unref(msg);
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);

  gst_object_unref(pipeline);
}
