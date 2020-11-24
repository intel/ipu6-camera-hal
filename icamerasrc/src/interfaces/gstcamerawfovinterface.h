/*
 * GStreamer
 * Copyright (C) 2018 Intel Corporation
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
#ifndef CHROME_SLIM_CAMHAL
#ifndef __GST_CAMERASRC_WFOV_INTERFACE_H__
#define __GST_CAMERASRC_WFOV_INTERFACE_H__

#include <gst/gst.h>
#include <ICamera.h>
#include <Parameters.h>

using namespace icamera;

G_BEGIN_DECLS
GST_DEBUG_CATEGORY_EXTERN(gst_camerasrc_debug);
#define GST_CAT_DEFAULT gst_camerasrc_debug

/* Get icamerasrc WFOV interface type */
#define GST_TYPE_CAMERASRC_WFOV_IF (gst_camerasrc_wfov_interface_get_type())

/* Get icamerasrc WFOV handle */
#define GST_CAMERASRC_WFOV(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CAMERASRC_WFOV_IF, GstCamerasrcWFOV))

/* Get icamerasrc WFOV interface */
#define GST_CAMERASRC_WFOV_GET_INTERFACE(inst) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GST_TYPE_CAMERASRC_WFOV_IF, GstCamerasrcWFOVInterface))

/* Usage:
   * - GstCamerasrcWFOV *camWFOV = GST_CAMERASRC_WFOV(camsrc);
   * - GstCamerasrcWFOVInterface *wfovIface = GST_CAMERASRC_MISC_GET_INTERFACE(camWFOV);
   * - wfovIface->set_wfov_mode(camWFOV, mode);
   * Note: camsrc could be instance of Gstcamerasrc or GstElement type.
   */
typedef struct _GstCamerasrcWFOV GstCamerasrcWFOV;
typedef struct _GstCamerasrcWFOVInterface GstCamerasrcWFOVInterface;

struct _GstCamerasrcWFOVInterface {
  /* Inherent from GTypeInterface */
  GTypeInterface base;

  /* Get wfov mode
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]        mode         enabled or not
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_wfov_mode)   (GstCamerasrcWFOV *camWFOV, uint8_t &mode);

  /* Get sensor mount type mode
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]        mount_type   sensor mount type: WALL_MOUNTED or CEILING_MOUNT
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_sensor_mount_type)   (GstCamerasrcWFOV *camWFOV, camera_mount_type_t &mount_type);

  /* Set view projection
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in]        projection   projection type and cone angle if conical projection
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_view_projection)   (GstCamerasrcWFOV *camWFOV, camera_view_projection_t projection);

  /* Get view projection
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]   projection   projection type and cone angle if conical projection
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_view_projection)   (GstCamerasrcWFOV *camWFOV, camera_view_projection_t &projection);

  /* Set view rotation
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in]        rotation     view rotation: pitch, yaw and roll
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_view_rotation)   (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t rotation);

  /* Get view rotation
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]   rotation     view rotation: pitch, yaw and roll
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_view_rotation)   (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &rotation);

  /* Set view fine adjustments
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in]        view_fine_adjustments     window shift and rotation
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_view_fine_adjustments)   (GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t fine_adjustments);

  /* Get view fine adjustments
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]   view_fine_adjustments     window shift and rotation
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_view_fine_adjustments)   (GstCamerasrcWFOV *camWFOV, camera_view_fine_adjustments_t &fine_adjustments);

  /* Set camera device rotation
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in]        camRotation  Camera device rotation: pitch, yaw and roll
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*set_camera_rotation)   (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t camRotation);

  /* Get camera device rotation
   *
   * param[in]        camWFOV      Camera Source handle
   * param[in, out]   camRotation  Camera device rotation: pitch, yaw and roll
   * return TRUE if get successfully, otherwise non-0 value is returned
   */
  gboolean      (*get_camera_rotation)   (GstCamerasrcWFOV *camWFOV, camera_view_rotation_t &camRotation);
};

GType gst_camerasrc_wfov_interface_get_type(void);

G_END_DECLS

#endif /* __GST_CAMERASRC_WFOV_INTERFACE_H__ */
#endif // CHROME_SLIM_CAMHAL
