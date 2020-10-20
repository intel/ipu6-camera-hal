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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "gstcampushsrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_cam_push_src_debug);
#define GST_CAT_DEFAULT gst_cam_push_src_debug

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_cam_push_src_debug, "campushsrc", 0, \
        "campushsrc element");

#define gst_cam_push_src_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstCamPushSrc, gst_cam_push_src, GST_TYPE_CAM_BASE_SRC, _do_init);

static gboolean gst_cam_push_src_query (GstCamBaseSrc * src, GstQuery * query);
static GstFlowReturn gst_cam_push_src_create (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** ret);
static GstFlowReturn gst_cam_push_src_alloc (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset,
    guint length, GstBuffer ** ret);
static GstFlowReturn gst_cam_push_src_fill (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset,
    guint length, GstBuffer * ret);

static void
gst_cam_push_src_class_init (GstCamPushSrcClass * klass)
{
  GstCamBaseSrcClass *gstbasesrc_class = (GstCamBaseSrcClass *) klass;

  gstbasesrc_class->create = GST_DEBUG_FUNCPTR (gst_cam_push_src_create);
  gstbasesrc_class->alloc = GST_DEBUG_FUNCPTR (gst_cam_push_src_alloc);
  gstbasesrc_class->fill = GST_DEBUG_FUNCPTR (gst_cam_push_src_fill);
  gstbasesrc_class->query = GST_DEBUG_FUNCPTR (gst_cam_push_src_query);
}

static void
gst_cam_push_src_init (GstCamPushSrc * pushsrc) {}

static gboolean
gst_cam_push_src_query (GstCamBaseSrc * src, GstQuery * query)
{
  gboolean ret;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_SCHEDULING:
    {
      /* a pushsrc can by default never operate in pull mode override
       * if you want something different. */
      gst_query_set_scheduling (query, GST_SCHEDULING_FLAG_SEQUENTIAL, 1, -1,
          0);
      gst_query_add_scheduling_mode (query, GST_PAD_MODE_PUSH);

      ret = TRUE;
      break;
    }
    default:
      ret = GST_CAM_BASE_SRC_CLASS (parent_class)->query (src, query);
      break;
  }
  return ret;
}

static GstFlowReturn
gst_cam_push_src_create (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset, guint length,
    GstBuffer ** ret)
{
  GstFlowReturn fret;
  GstCamPushSrc *src;
  GstCamPushSrcClass *pclass;

  src = GST_CAM_PUSH_SRC (bsrc);
  pclass = GST_CAM_PUSH_SRC_GET_CLASS (src);
  if (pclass->create)
    fret = pclass->create (src, ret);
  else
    fret =
        GST_CAM_BASE_SRC_CLASS (parent_class)->create (bsrc, pad, offset, length, ret);

  return fret;
}

static GstFlowReturn
gst_cam_push_src_alloc (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset, guint length,
    GstBuffer ** ret)
{
  GstFlowReturn fret;
  GstCamPushSrc *src;
  GstCamPushSrcClass *pclass;

  src = GST_CAM_PUSH_SRC (bsrc);
  pclass = GST_CAM_PUSH_SRC_GET_CLASS (src);
  if (pclass->alloc)
    fret = pclass->alloc (src, ret);
  else
    fret = GST_CAM_BASE_SRC_CLASS (parent_class)->alloc (bsrc, pad, offset, length, ret);

  return fret;
}

static GstFlowReturn
gst_cam_push_src_fill (GstCamBaseSrc * bsrc, GstPad *pad, guint64 offset, guint length,
    GstBuffer * ret)
{
  GstFlowReturn fret;
  GstCamPushSrc *src;
  GstCamPushSrcClass *pclass;

  src = GST_CAM_PUSH_SRC (bsrc);
  pclass = GST_CAM_PUSH_SRC_GET_CLASS (src);
  if (pclass->fill)
    fret = pclass->fill (src, pad, ret);
  else
    fret = GST_CAM_BASE_SRC_CLASS (parent_class)->fill (bsrc, pad, offset, length, ret);

  return fret;
}
