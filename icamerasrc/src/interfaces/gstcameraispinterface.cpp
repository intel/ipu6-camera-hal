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

#define LOG_TAG "GstCameraIspInterface"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "gstcameraispinterface.h"
#include <string.h>

static void gst_camerasrc_isp_iface_init (GstCamerasrcIspInterface *ispIface);

GType
gst_camerasrc_isp_interface_get_type (void)
{
  static GType gst_camerasrc_isp_interface_type = 0;

  if (!gst_camerasrc_isp_interface_type) {
    static const GTypeInfo gst_camerasrc_isp_interface_type_info = {
      sizeof(GstCamerasrcIspInterface),
      (GBaseInitFunc)gst_camerasrc_isp_iface_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
      NULL
    };

    gst_camerasrc_isp_interface_type = g_type_register_static(G_TYPE_INTERFACE,
                               "GstCamerasrcIspInterface", &gst_camerasrc_isp_interface_type_info, (GTypeFlags)0);

    if (!gst_camerasrc_isp_interface_type) {
      g_warning("Failed to register isp interface type, ret:%d.", (int)gst_camerasrc_isp_interface_type);
    }
  }
  return gst_camerasrc_isp_interface_type;
}

static void
gst_camerasrc_isp_iface_init (GstCamerasrcIspInterface *ispIface)
{
  ispIface->set_isp_control = NULL;
  ispIface->get_isp_control = NULL;
  ispIface->apply_isp_control = NULL;
  ispIface->set_ltm_tuning_data = NULL;
  ispIface->get_ltm_tuning_data = NULL;
}
