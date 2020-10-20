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

#ifndef __GST_CAM_PUSH_SRC_H__
#define __GST_CAM_PUSH_SRC_H__

#include <gst/gst.h>
#include "gstcambasesrc.h"

G_BEGIN_DECLS

/* Remove later*/
enum
{
    T_CAMERASRC_CAPTURE_MODE_STILL = 0,
    T_CAMERASRC_CAPTURE_MODE_VIDEO = 1,
    T_CAMERASRC_CAPTURE_MODE_PREVIEW = 2,
};

#define GST_TYPE_CAM_PUSH_SRC               (gst_cam_push_src_get_type())
#define GST_CAM_PUSH_SRC(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CAM_PUSH_SRC,GstCamPushSrc))
#define GST_CAM_PUSH_SRC_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CAM_PUSH_SRC,GstCamPushSrcClass))
#define GST_CAM_PUSH_SRC_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_CAM_PUSH_SRC, GstCamPushSrcClass))
#define GST_IS_CAM_PUSH_SRC(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CAM_PUSH_SRC))
#define GST_IS_CAM_PUSH_SRC_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CAM_PUSH_SRC))

typedef struct _GstCamPushSrc GstCamPushSrc;
typedef struct _GstCamPushSrcClass GstCamPushSrcClass;

/**
 * GstCamPushSrc:
 *
 * The opaque #GstCamPushSrc data structure.
 */
struct _GstCamPushSrc {
  GstCamBaseSrc     parent;

  /*< private >*/
  gpointer _gst_reserved[GST_PADDING];
};

/**
 * GstCamPushSrcClass:
 * @parent_class: Element parent class
 * @create: Ask the subclass to create a buffer. The subclass decides which
 *          size this buffer should be. Other then that, refer to
 *          #GstCamBaseSrc<!-- -->.create() for more details. If this method is
 *          not implemented, @alloc followed by @fill will be called.
 * @alloc: Ask the subclass to allocate a buffer. The subclass decides which
 *         size this buffer should be. The default implementation will create
 *         a new buffer from the negotiated allocator.
 * @fill: Ask the subclass to fill the buffer with data.
 *
 * Subclasses can override any of the available virtual methods or not, as
 * needed. At the minimum, the @fill method should be overridden to produce
 * buffers.
 */
struct _GstCamPushSrcClass {
  GstCamBaseSrcClass parent_class;

  /* ask the subclass to create a buffer, the default implementation
   * uses alloc and fill */
  GstFlowReturn (*create) (GstCamPushSrc *src, GstBuffer **buf);
  /* allocate memory for a buffer */
  GstFlowReturn (*alloc)  (GstCamPushSrc *src, GstBuffer **buf);
  /* ask the subclass to fill a buffer */
  GstFlowReturn (*fill)   (GstCamPushSrc *src, GstPad *pad, GstBuffer *buf);

  /*< private >*/
  gpointer _gst_reserved[GST_PADDING];
};

GType gst_cam_push_src_get_type(void);

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstCamPushSrc, gst_object_unref)
#endif

G_END_DECLS

#endif /* __GST_CAM_PUSH_SRC_H__ */
