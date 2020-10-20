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

#include "case_gst_common.h"
#include <string.h>
#include <math.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video-info.h>
#include <gst/allocators/gstdmabuf.h>
#include <glib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

int buf_count = 0;

struct CustomAppElement : CommonElement
{
  GstElement *videoconvert;
  GstElement *videoscale;
  GstElement *appsink;
};

static void
write_data(int flag, GstBuffer *buffer, uint32_t *plane, gsize size)
{
  FILE *fp = NULL;
  char file_name[50];
  sprintf(file_name, "flag_%d_buffer_%d", flag, buf_count);
  fp = fopen(file_name, "w+");
  if (flag == FILE_DUMP_FLAG_C_MAP)
    fwrite(plane, size, 1, fp);
  else if (flag == FILE_DUMP_FLAG_GST_MAP) {
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);
    fwrite(map.data, size, 1, fp);
  }
  buf_count++;
  fclose(fp);
}

static int
check_stream_buffer(GstBuffer *buffer)
{
  GstVideoMeta *meta;
  GstMemory *mem;
  gboolean is_dma;
  int dump_flag = get_dump_flag();
  int dmafd[GST_VIDEO_MAX_PLANES];
  int offsets[GST_VIDEO_MAX_PLANES];
  void *planes[GST_VIDEO_MAX_PLANES];

  mem = gst_buffer_peek_memory(buffer, 0);
  is_dma = gst_is_dmabuf_memory(mem);
  g_message("buffer is DMABUF:%d.", is_dma);

  meta = gst_buffer_get_video_meta(buffer);
  if (!meta) {
    g_error("Failed to get buffer metadata.");
    return GST_FLOW_ERROR;
  }

  for (int i = 0; i < meta->n_planes; i++) {
    uint32_t crc = 0;
    uint32_t *plane;

    mem = gst_buffer_peek_memory(buffer, i);
    if (is_dma) {
      dmafd[i] = gst_dmabuf_memory_get_fd(mem);

      offsets[i] = mem->offset;
      planes[i] = mmap(NULL, mem->size, PROT_READ, MAP_SHARED, dmafd[i], mem->offset);
      if (planes[i] == MAP_FAILED) {
        g_error("Failed to map memory.");
        return -ENOMEM;
      }
      plane = (uint32_t *)planes[i];

      if (dump_flag != 0)
        write_data(dump_flag, NULL, plane, mem->size);

      for (int j = 0; j < mem->size/4; j++) {
        crc ^= plane[j];
      }
      g_message("appsink frame crc: %#x", crc);
      munmap(planes[i], mem->size);
    }  else {
      if (dump_flag != 0)
        write_data(dump_flag, buffer, NULL, mem->size);
    }
  }

  return GST_FLOW_OK;
}

static void sink_eos(GstAppSink *appsink,
                       gpointer user_data)
{
  g_message("appsink [%p]: EOS called.", user_data);
}

static GstFlowReturn sink_new_preroll (GstAppSink *appsink,
                        gpointer user_data)
{
  GstSample *sample;
  GstBuffer *buffer;

  g_message("appsink [%p]: new-preroll called.", user_data);

  /* ...retrieve new sample from the pipe */
  sample = gst_app_sink_pull_preroll(appsink);
  g_assert(sample);
  buffer = gst_sample_get_buffer(sample);
  g_assert(buffer);
  g_message("appsink new preroll, buffer: %p, timestamp: %zu, refcount=%d",
          buffer, GST_BUFFER_PTS(buffer), GST_MINI_OBJECT_REFCOUNT(buffer));
  int ret = check_stream_buffer(buffer);
  if (ret < 0)
    return GST_FLOW_ERROR;

  /* unref sample and buffer after used */
  gst_sample_unref(sample);
  gst_buffer_unref(buffer);

  return GST_FLOW_OK;
}

static GstFlowReturn sink_new_sample(GstAppSink *appsink,
                        gpointer user_data)
{
  GstSample *sample;
  GstBuffer *buffer;

  sample = gst_app_sink_pull_sample(appsink);
  g_assert(sample);
  buffer = gst_sample_get_buffer(sample);
  g_assert(buffer);
  g_message("appsink new sample, buffer: %p, timestamp: %zu, refcount=%d",
    buffer, GST_BUFFER_PTS(buffer), GST_MINI_OBJECT_REFCOUNT(buffer));
  int ret = check_stream_buffer(buffer);
  if (ret < 0)
    return GST_FLOW_ERROR;

  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

/* A set of callbacks that can be installed on the appsink with
   *gst_app_sink_set_callbacks() */
static GstAppSinkCallbacks  sink_callbacks =
{
    .eos = sink_eos,
    .new_preroll = sink_new_preroll,
    .new_sample = sink_new_sample,
};

static void
event_loop (GstElement *pipeline)
{
  GstBus *bus;
  GstMessage *message = NULL;
  gboolean running = TRUE;

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  while (running) {
    message = gst_bus_timed_pop_filtered (bus, -1, GST_MESSAGE_ANY);
    g_assert (message != NULL);

    switch (message->type) {
           case GST_MESSAGE_EOS:
             g_message ("got EOS");
             running = FALSE;
             break;
           case GST_MESSAGE_WARNING:{
             GError *gerror;
             gchar *debug;

             gst_message_parse_warning (message, &gerror, &debug);
             gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
             g_error_free (gerror);
             g_free (debug);
             break;
           }
           case GST_MESSAGE_ERROR:
           {
             GError *gerror;
             gchar *debug;

             gst_message_parse_error (message, &gerror, &debug);
             gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
             g_error_free (gerror);
             g_free (debug);
             running = FALSE;
             break;
           }
           case GST_MESSAGE_STEP_DONE:
           {
             GstFormat format;
             guint64 amount;
             gdouble rate;
             gboolean flush, intermediate;
             guint64 duration;
             gboolean eos;

             gst_message_parse_step_done (message, &format, &amount, &rate,
                      &flush, &intermediate, &duration, &eos);

             if (format == GST_FORMAT_DEFAULT) {
                      g_message ("step done: %" GST_TIME_FORMAT " skipped in %"
                                  G_GUINT64_FORMAT " frames", GST_TIME_ARGS (duration), amount);
             } else {
                      g_message ("step done: %" GST_TIME_FORMAT " skipped",
                                  GST_TIME_ARGS (duration));
             }
             break;
           }
           default:
             break;
    }
    gst_message_unref (message);
  }
  gst_object_unref (bus);
}

static int
parse_pipeline_sample (CustomAppElement **pipe)
{
  gst_init (NULL, NULL);

  (*pipe)->bin = gst_pipeline_new("pipeline");
  g_assert((*pipe)->bin);

  (*pipe)->camerasrc = gst_element_factory_make ("icamerasrc", "icamerasrc");
  g_assert((*pipe)->camerasrc);
  GObjectClass *oclass = G_OBJECT_GET_CLASS ((*pipe)->camerasrc);
  GParamSpec *prop = g_object_class_find_property (oclass, "device-name");
  int cam_id = get_enum_value_from_field(prop, (*pipe)->device_name, NULL);

  g_object_set ((*pipe)->camerasrc,
                                  "device-name", cam_id,
                                  "num-vc", (*pipe)->s.num_vc,
                                  "io-mode", (*pipe)->s.io_mode,
                                  "num-buffers", (*pipe)->s.num_buffers,
                                  NULL);

  (*pipe)->filter = gst_element_factory_make ("capsfilter", "filter");
  (*pipe)->filtercaps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, (*pipe)->s.format,
                                  "width", G_TYPE_INT, (*pipe)->s.width,
                                  "height", G_TYPE_INT, (*pipe)->s.height,
                                  "interlace-mode", G_TYPE_STRING, "progressive",
                                  NULL);

  (*pipe)->videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  g_assert((*pipe)->videoconvert);
  (*pipe)->videoscale = gst_element_factory_make("videoscale", "videoscale");
  g_assert((*pipe)->videoscale);
  (*pipe)->appsink = gst_element_factory_make ("appsink", "appsink");
  g_assert((*pipe)->appsink);
   g_object_set ((*pipe)->appsink, "max-buffers", 6, NULL);

  /* this callback is designed for appsink, which can parse buffer when there's new preroll or new sample */
  gst_app_sink_set_callbacks (GST_APP_SINK_CAST((*pipe)->appsink), &sink_callbacks, NULL, NULL);
  gst_bin_add_many(GST_BIN((*pipe)->bin),
                                             (*pipe)->camerasrc,
                                             (*pipe)->filter,
                                             (*pipe)->videoconvert,
                                             (*pipe)->videoscale,
                                             (*pipe)->appsink,
                                             NULL);

  if (!gst_element_link_many ((*pipe)->camerasrc,
                                             (*pipe)->filter,
                                             (*pipe)->videoconvert,
                                             (*pipe)->videoscale,
                                             (*pipe)->appsink,
                                             NULL))
    g_error("Failed to link element.");

  g_object_set (G_OBJECT ((*pipe)->filter), "caps", (*pipe)->filtercaps, NULL);
  gst_caps_unref ((*pipe)->filtercaps);

  gst_element_set_state((*pipe)->bin, GST_STATE_PLAYING);

  event_loop((*pipe)->bin);

  gst_element_set_state((*pipe)->bin, GST_STATE_NULL);
  g_object_unref((*pipe)->bin);
}

TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_vc_userptr_YUY2_640_480_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 640;
  pipe_element->s.height = 480;
  pipe_element->s.io_mode = 0;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 1;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}


TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_vc_dma_export_YUY2_640_480_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 640;
  pipe_element->s.height = 480;
  pipe_element->s.io_mode = 2;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 1;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}

TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_vc_userptr_YUY2_1280_800_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 1280;
  pipe_element->s.height = 800;
  pipe_element->s.io_mode = 0;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 1;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}

TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_vc_dma_export_YUY2_1280_800_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 1280;
  pipe_element->s.height = 800;
  pipe_element->s.io_mode = 2;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 1;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}

TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_userptr_YUY2_1920_1080_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 1920;
  pipe_element->s.height = 1080;
  pipe_element->s.io_mode = 0;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 0;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}

TEST (CameraSrcTest, gst_stream_data_icamerasrc_appsink_dma_export_YUY2_1920_1080_progressive)
{
  int ret = 0;
  CustomAppElement *pipe_element = new CustomAppElement;

  pipe_element->device_name = get_input_device_env();
  ASSERT_NE(strlen(pipe_element->device_name), 0);

  pipe_element->s.format = "YUY2";
  pipe_element->s.width = 1920;
  pipe_element->s.height = 1080;
  pipe_element->s.io_mode = 2;
  pipe_element->s.num_buffers = 50;
  pipe_element->s.num_vc = 0;

  parse_pipeline_sample(&pipe_element);
  delete pipe_element;
  pipe_element = NULL;
}
