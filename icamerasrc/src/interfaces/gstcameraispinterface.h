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

#ifndef __GST_CAMERASRC_ISP_INTERFACE_H__
#define __GST_CAMERASRC_ISP_INTERFACE_H__

#include <gst/gst.h>
#include <ICamera.h>
#include <Parameters.h>

using namespace icamera;

G_BEGIN_DECLS
GST_DEBUG_CATEGORY_EXTERN(gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

/* Get icamerasrc isp interface type */
#define GST_TYPE_CAMERASRC_ISP_IF (gst_camerasrc_isp_interface_get_type())

/* Get icamerasrc isp handle */
#define GST_CAMERASRC_ISP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CAMERASRC_ISP_IF, GstCamerasrcIsp))

/* Get icamerasrc isp interface */
#define GST_CAMERASRC_ISP_GET_INTERFACE(inst) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GST_TYPE_CAMERASRC_ISP_IF, GstCamerasrcIspInterface))

/* Usage:
   * - GstCamerasrcIsp *camIsp = GST_CAMERASRC_ISP(camsrc);
   * - GstCamerasrcIspInterface *ispIface = GST_CAMERASRC_ISP_GET_INTERFACE(camIsp);
   * - ispIface->set_isp_control(camIsp, tag, data);
   * - ispIface->apply_isp_control();
   * Note: camsrc could be instance of Gstcamerasrc or GstElement type.
   */
typedef struct _GstCamerasrcIsp GstCamerasrcIsp;
typedef struct _GstCamerasrcIspInterface GstCamerasrcIspInterface;

struct _GstCamerasrcIspInterface {
  /* Inherent from GTypeInterface */
  GTypeInterface base;

  /* Set isp low level control and cache the data in camerasrc
   *
   * param[in]        camIsp        Camera Source handle
   * param[in]        tag           The control type
   * param[in]        data          The control data pointer
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_isp_control)      (GstCamerasrcIsp *camIsp, unsigned int tag, void *data);

  /* Get isp low level control
   *
   * param[in]        camIsp        Camera Source handle
   * param[in]        tag           The tag type
   * param[out]       data          the data pointer to get
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_isp_control)      (GstCamerasrcIsp *camIsp, unsigned int tag, void *data);

  /* Apply the data cached in camerasrc to isp
   *
   * param[in]        camIsp        Camera Source handle
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*apply_isp_control)      (GstCamerasrcIsp *camIsp);

  /* Set ltm tuning data
   *
   * param[in]        camIsp        Camera Source handle
   * param[in]        data          The ltm tuning data pointer
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_ltm_tuning_data)   (GstCamerasrcIsp *camIsp, void *data);

  /* Get ltm tuning data
   *
   * param[in]        camIsp        Camera Source handle
   * param[out]       data          the ltm tuning data pointer to get
   * return TRUE if set successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_ltm_tuning_data)   (GstCamerasrcIsp *camIsp, void *data);
};

GType gst_camerasrc_isp_interface_get_type(void);

G_END_DECLS

#endif /* __GST_CAMERASRC_ISP_INTERFACE_H__ */
