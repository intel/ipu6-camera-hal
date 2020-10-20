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

#ifndef __GST_CAMERASRC_BUFFER_POOL_H__
#define __GST_CAMERASRC_BUFFER_POOL_H__

#include <gst/gst.h>
#include "gstcampushsrc.h"
#include "gstcamerasrc.h"

typedef struct _GstCamerasrcBufferPool GstCamerasrcBufferPool;//in use of qbuf&dqbuf
typedef struct _GstCamerasrcBufferPoolClass GstCamerasrcBufferPoolClass;//in use of _class_init
typedef struct _GstCamerasrcMeta GstCamerasrcMeta;

G_BEGIN_DECLS
GST_DEBUG_CATEGORY_EXTERN(gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

#define GST_TYPE_CAMERASRC_BUFFER_POOL      (gst_camerasrc_buffer_pool_get_type())
#define GST_IS_CAMERASRC_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_CAMERASRC_BUFFER_POOL))
#define GST_CAMERASRC_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CAMERASRC_BUFFER_POOL, GstCamerasrcBufferPool))
#define GST_CAMERASRC_BUFFER_POOL_CAST(obj) ((GstCamerasrcBufferPool *)(obj))
#define GST_CAMERASRC_META_GET(buf) ((GstCamerasrcMeta *)gst_buffer_get_meta(buf,gst_camerasrc_meta_api_get_type()))
#define GST_CAMERASRC_META_ADD(buf) ((GstCamerasrcMeta *)gst_buffer_add_meta(buf,gst_camerasrc_meta_get_info(),NULL))

#define FPS_TIME_INTERVAL 2000000
#define FPS_BUF_COUNT_START 10

struct _GstCamerasrcBufferPool
{
  GstBufferPool parent;

  GstAllocator *allocator;
  GstAllocationParams params;

  Gstcamerasrc *src;
  GstBuffer **buffers;

  gint number_of_buffers;
  gint number_allocated;
  gint acquire_buffer_index;
  gint size;

  int stream_id;
  gboolean alloc_done;
};

struct _GstCamerasrcBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

struct _GstCamerasrcMeta {
  GstMeta meta;

  int index;
  gpointer mem;
  camera_buffer_t *buffer;
};

GType gst_camerasrc_meta_api_get_type (void);
GType gst_camerasrc_buffer_pool_get_type(void);
const GstMetaInfo * gst_camerasrc_meta_get_info (void);
GstBufferPool *gst_camerasrc_buffer_pool_new(Gstcamerasrc *src,
          GstCaps *caps, int stream_id);

G_END_DECLS
#endif
