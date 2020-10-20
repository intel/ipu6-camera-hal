/*
 * GStreamer
 * Copyright (C) 2016-2017 Intel Corporation
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

#define LOG_TAG "GstCamera3AInterface"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "gstcamera3ainterface.h"
#include <string.h>

static void gst_camerasrc_3a_iface_init (GstCamerasrc3AInterface *iface);

GType
gst_camerasrc_3a_interface_get_type (void)
{
  static GType gst_camerasrc_3a_interface_type = 0;

  if (!gst_camerasrc_3a_interface_type) {
    static const GTypeInfo gst_camerasrc_3a_interface_type_info = {
      sizeof(GstCamerasrc3AInterface),
      (GBaseInitFunc)gst_camerasrc_3a_iface_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
      NULL
    };

    gst_camerasrc_3a_interface_type = g_type_register_static(G_TYPE_INTERFACE,
                               "GstCamerasrc3AInterface", &gst_camerasrc_3a_interface_type_info, (GTypeFlags)0);

    if (!gst_camerasrc_3a_interface_type) {
      g_warning("Failed to register 3A interface type, ret:%d.", (int)gst_camerasrc_3a_interface_type);
    }
  }
  return gst_camerasrc_3a_interface_type;
}

static void
gst_camerasrc_3a_iface_init (GstCamerasrc3AInterface *iface)
{
  iface->get_image_enhancement = NULL;
  iface->set_image_enhancement = NULL;
  iface->set_exposure_time = NULL;
  iface->set_iris_mode = NULL;
  iface->set_iris_level = NULL;
  iface->set_gain = NULL;
  iface->set_blc_area_mode = NULL;
  iface->set_wdr_level = NULL;
  iface->set_awb_mode = NULL;
  iface->get_awb_gain = NULL;
  iface->set_awb_gain = NULL;
  iface->set_scene_mode = NULL;
  iface->set_ae_mode = NULL;
  iface->set_weight_grid_mode = NULL;
  iface->set_ae_converge_speed = NULL;
  iface->set_awb_converge_speed = NULL;
  iface->set_ae_converge_speed_mode = NULL;
  iface->set_awb_converge_speed_mode = NULL;
  iface->set_exposure_ev = NULL;
  iface->set_exposure_priority = NULL;
  iface->get_awb_cct_range = NULL;
  iface->set_awb_cct_range = NULL;
  iface->get_white_point = NULL;
  iface->set_white_point = NULL;
  iface->get_awb_gain_shift = NULL;
  iface->set_awb_gain_shift = NULL;
  iface->set_ae_region = NULL;
  iface->set_color_transform = NULL;
  iface->set_custom_aic_param = NULL;
  iface->set_antibanding_mode = NULL;
  iface->set_color_range_mode = NULL;
  iface->set_exposure_time_range = NULL;
  iface->set_sensitivity_gain_range = NULL;
}
