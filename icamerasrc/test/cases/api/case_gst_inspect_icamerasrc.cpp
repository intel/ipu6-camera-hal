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

#include "gtest/gtest.h"
#include <string.h>
#include <math.h>
#include <gst/gst.h>

static gboolean format_nv12 = false;
static gboolean format_yuy2 = false;

static gboolean check_field_value(GQuark field, const GValue * value, gpointer pfx)
{
    //we can use g_quark_to_string() to get filed name.
    gchar *str_value = gst_value_serialize (value);
    if (!strcmp(str_value, "YUY2"))
        format_yuy2 = true;
    if (!strcmp(str_value, "NV12"))
        format_nv12 = true;

    g_print ("        %s: %s\n", g_quark_to_string (field), str_value);
    g_free (str_value);
    return TRUE;
}
static int check_inspectinfo_for_factory(const char* factoryname)
{
    int i = 0;
    const GstCaps * caps;
    const GList *templs;
    GObjectClass *oclass;
    GstElement * new_element;
    GstStructure *structure;
    GstElementFactory * factory;
    GParamSpec *property = NULL;
    GstStaticPadTemplate *padtemplate;

    format_nv12 = false;
    format_yuy2 = false;
    gst_init (NULL, NULL);
    factory = gst_element_factory_find (factoryname);
    templs = gst_element_factory_get_static_pad_templates (factory);
    while (templs) {
        padtemplate = (GstStaticPadTemplate *) templs->data;
        if (padtemplate->static_caps.string) {
            g_print ("    Capabilities:\n");
            caps = gst_static_caps_get(&padtemplate->static_caps);
            for (i = 0; i < gst_caps_get_size (caps); i++) {
                structure = gst_caps_get_structure (caps, i);
                g_print ("    %s\n", gst_structure_get_name (structure));
                gst_structure_foreach (structure, check_field_value, NULL);
            }
        }
        templs = g_list_next (templs);
    }
    if (!format_nv12) {
        g_print("donesn't support NV12 format\n");
        return -1;
    }
    if (!format_yuy2) {
        g_print("donesn't support YUY2 format\n");
        return -1;
    }

    new_element = gst_element_factory_make(factoryname, "check");
    if (!new_element) {
        g_print("Failed to create element\n");
        return -1;
    }
    oclass = G_OBJECT_GET_CLASS (new_element);
    property = g_object_class_find_property (oclass, "interlace-mode");
    if (!property) {
        gst_object_unref(GST_OBJECT(new_element));
        g_print("There isn't interlace-mode property in factory: %s\n", factoryname);
        return -1;
    }
    property = NULL;
    property = g_object_class_find_property (oclass, "deinterlace-method");
    if (!property) {
        gst_object_unref(GST_OBJECT(new_element));
        g_print("There isn't deinterlace-method property in factory: %s\n", factoryname);
        return -1;
    }
    gst_object_unref(GST_OBJECT(new_element));

    return 0;
}

TEST(CameraSrcTest, gst_inspect_icamerasrc)
{
    int ret = check_inspectinfo_for_factory("icamerasrc");
    ASSERT_EQ(ret, 0);
}
