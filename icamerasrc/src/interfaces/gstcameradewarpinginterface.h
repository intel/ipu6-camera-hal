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

#ifndef __GST_CAMERASRC_DEWARPING_INTERFACE_H__
#define __GST_CAMERASRC_DEWARPING_INTERFACE_H__

#include <gst/gst.h>
#include <ICamera.h>
#include <Parameters.h>

using namespace icamera;

G_BEGIN_DECLS
GST_DEBUG_CATEGORY_EXTERN(gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

/* Get icamerasrc dewarping interface type */
#define GST_TYPE_CAMERASRC_DEWARPING_IF (gst_camerasrc_dewarping_interface_get_type())

/* Get icamerasrc dewarping handle */
#define GST_CAMERASRC_DEWARPING(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CAMERASRC_DEWARPING_IF, GstCamerasrcDewarping))

/* Get icamerasrc dewarping interface */
#define GST_CAMERASRC_DEWARPING_GET_INTERFACE(inst) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GST_TYPE_CAMERASRC_DEWARPING_IF, GstCamerasrcDewarpingInterface))

/* Usage:
   * - GstCamerasrcDewarping *camDewarping = GST_CAMERASRC_DEWARPING(camsrc);
   * - GstCamerasrcDewarpingInterface *dewarpingIface = GST_CAMERASRC_MISC_GET_INTERFACE(camDewarping);
   * - dewarpingIface->set_dewarping_mode(camDewarping, mode);
   * Note: camsrc could be instance of Gstcamerasrc or GstElement type.
   */
typedef struct _GstCamerasrcDewarping GstCamerasrcDewarping;
typedef struct _GstCamerasrcDewarpingInterface GstCamerasrcDewarpingInterface;

struct _GstCamerasrcDewarpingInterface {
  /* Inherent from GTypeInterface */
  GTypeInterface base;

  /* Set dewarping mode
   *
   * param[in]        camDewarping      Camera Source handle
   * param[in]        mode              the dewarping mode to be set
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_dewarping_mode)   (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t mode);
    /* Get dewarping mode
   *
   * param[in]        camDewarping      Camera Source handle
   * param[in]        mode              the dewarping mode to be returned
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_dewarping_mode)   (GstCamerasrcDewarping *camDewarping, camera_fisheye_dewarping_mode_t &mode);
};

GType gst_camerasrc_dewarping_interface_get_type(void);

G_END_DECLS

#endif /* __GST_CAMERASRC_DEWARPING_INTERFACE_H__ */
