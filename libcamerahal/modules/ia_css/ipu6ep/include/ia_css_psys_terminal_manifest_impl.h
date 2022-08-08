/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IA_CSS_PSYS_TERMINAL_MANIFEST_IMPL_H
#define __IA_CSS_PSYS_TERMINAL_MANIFEST_IMPL_H

#include <ia_css_psys_terminal_manifest.h>

/* Data object types on the terminals */
#include <ia_css_program_group_data.h>
/* for ia_css_kernel_bitmap_t, ia_css_kernel_bitmap_clear, ia_css_... */
#include <ia_css_kernel_bitmap.h>

#include "ia_css_psys_program_group_private.h"
#include "ia_css_terminal_manifest.h"
#include "ia_css_terminal_manifest_types.h"

#include <error_support.h>
#include <print_support.h>
#include <misc_support.h>
#include "ia_css_psys_static_trace.h"
#include "ia_css_psys_static_storage_class.h"

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_spatial_parameter_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_parameter_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return ((terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN) ||
        (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT));
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_program_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_parameter_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM);
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_program_control_init_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_program_control_init_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT);
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_parameter_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    /* will return an error value on error */
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_parameter_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT);
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_data_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    /* will return an error value on error */
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_data_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return ((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN) ||
        (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT));
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_terminal_manifest_sliced_terminal(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_terminal_manifest_sliced_terminal(): enter:\n");

    terminal_type = ia_css_terminal_manifest_get_type(manifest);

    return ((terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN) ||
        (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT));
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
size_t ia_css_terminal_manifest_get_size(
    const ia_css_terminal_manifest_t *manifest)
{
    size_t size = 0;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_size(): enter:\n");

    if (manifest != NULL) {
        size = manifest->size;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_terminal_manifest_get_size: invalid argument\n");
    }
    return size;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_terminal_type_t ia_css_terminal_manifest_get_type(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_type_t terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_type(): enter:\n");

    if (manifest != NULL) {
        terminal_type = manifest->terminal_type;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_terminal_manifest_get_type: invalid argument\n");
    }
    return terminal_type;
}

/** Temporary implementation of attributes based on terminal type ID
 *
 *  A later implementation in the PG manifest generator tool will
 *  replace this and thus remove the dependency on the legacy
 *  terminal type.
 */
static void ia_css_terminal_manifest_set_attributes_by_type(
    ia_css_terminal_manifest_t *manifest,
    const ia_css_terminal_type_t terminal_type)
{
    /* Set defaults for most cases, and change in the later code for the rest. */
    ia_css_terminal_cat_t category = IA_CSS_TERMINAL_CAT_LOAD;
    ia_css_terminal_dir_t direction = IA_CSS_TERMINAL_DIR_IN;
    ia_css_terminal_rou_t rou = IA_CSS_TERMINAL_ROU_FRAME;
    ia_css_connect_buf_type_t connect_buf_type = IA_CSS_CONNECT_BUF_TYPE_UNKNOWN;

    if ((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT)) {
        category = IA_CSS_TERMINAL_CAT_CONNECT;
    }
    ia_css_terminal_manifest_set_category(manifest, category);

    if ((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_STATE_OUT)) {
        direction = IA_CSS_TERMINAL_DIR_OUT;
    }
    ia_css_terminal_manifest_set_direction(manifest, direction);

    if ((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT)) {
        rou = IA_CSS_TERMINAL_ROU_FRAG;
    } else if (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT) {
            rou = IA_CSS_TERMINAL_ROU_STREAM;
    }
    ia_css_terminal_manifest_set_rate_of_update(manifest, rou);

    if ((terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN) ||
         (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT)) {
        connect_buf_type = IA_CSS_CONNECT_BUF_TYPE_DATA;
    } else if ((terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN) ||
             (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT)) {
            connect_buf_type = IA_CSS_CONNECT_BUF_TYPE_META;
    }
    ia_css_terminal_manifest_set_connect_buffer_type(manifest, connect_buf_type);
}

/** Temporary implementation for new fields until the manfifest generator
 *  tool will be enhance to set values directly.
 *
 *  - Attibutes are set to meaningful values by mapping terminal type to
 *  attributes
 *  - connect_ID is set to the invalid terminal ID value: IA_CSS_TERMINAL_INVALID_ID
 *  - max_payload_size is set to UINT32_MAX
 */
static void ia_css_terminal_manifest_set_new_fields(
    ia_css_terminal_manifest_t *manifest,
    const ia_css_terminal_type_t terminal_type)
{
    ia_css_terminal_manifest_set_attributes_by_type(manifest, terminal_type);
    assert(ia_css_terminal_manifest_set_connect_ID(manifest, IA_CSS_TERMINAL_INVALID_ID));
    assert(ia_css_terminal_manifest_set_max_payload_size(manifest, UINT32_MAX));
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_terminal_manifest_set_type(
    ia_css_terminal_manifest_t *manifest,
    const ia_css_terminal_type_t terminal_type)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_type(): enter:\n");

    if (manifest != NULL) {
        manifest->terminal_type = terminal_type;
        ia_css_terminal_manifest_set_new_fields(manifest, terminal_type);
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_type failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_terminal_manifest_set_ID(
    ia_css_terminal_manifest_t *manifest,
    const ia_css_terminal_ID_t ID)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_ID(): enter:\n");

    if (manifest != NULL) {
        manifest->ID = ID;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_ID failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_terminal_ID_t ia_css_terminal_manifest_get_ID(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_ID_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_ID(): enter:\n");

    if (manifest != NULL) {
        retval = manifest->ID;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_ID failed\n");
        retval = IA_CSS_TERMINAL_INVALID_ID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_ID_t ia_css_terminal_manifest_get_connect_ID(
    const ia_css_terminal_manifest_t        *manifest)
{
    ia_css_terminal_ID_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_connect_ID(): enter:\n");

    if (manifest != NULL) {
        retval = manifest->assoc_ID;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_connect_ID failed\n");
        retval = IA_CSS_TERMINAL_INVALID_ID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_cat_t ia_css_terminal_manifest_get_category(
    const ia_css_terminal_manifest_t        *manifest)
{
    ia_css_terminal_cat_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_category(): enter:\n");

    if (manifest != NULL) {
        retval = (ia_css_terminal_cat_t)manifest->attributes.category;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_category failed\n");
        retval = IA_CSS_TERMINAL_CAT_INVALID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_category(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_cat_t category)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_category(): enter:\n");

    if (manifest != NULL) {
        manifest->attributes.category = category;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_category failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_dir_t ia_css_terminal_manifest_get_direction(
    const ia_css_terminal_manifest_t        *manifest)
{
    ia_css_terminal_dir_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_direction(): enter:\n");

    if (manifest != NULL) {
        retval = (ia_css_terminal_dir_t)manifest->attributes.direction;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_direction failed\n");
        retval = IA_CSS_TERMINAL_DIR_INVALID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_direction(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_dir_t direction)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_direction(): enter:\n");

    if (manifest != NULL) {
        manifest->attributes.direction = direction;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_direction failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_rou_t ia_css_terminal_manifest_get_rate_of_update(
    const ia_css_terminal_manifest_t        *manifest)
{
    ia_css_terminal_rou_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_rate_of_update(): enter:\n");

    if (manifest != NULL) {
        retval = (ia_css_terminal_rou_t)manifest->attributes.rou;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_rate_of_update failed\n");
        retval = IA_CSS_TERMINAL_ROU_INVALID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_rate_of_update(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_rou_t rate_of_update)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_rate_of_update(): enter:\n");

    if (manifest != NULL) {
        manifest->attributes.rou = rate_of_update;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_rate_of_update failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_connect_buf_type_t ia_css_terminal_manifest_get_connect_buffer_type(
    const ia_css_terminal_manifest_t        *manifest)
{
    ia_css_connect_buf_type_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_connect_buffer_type(): enter:\n");

    if (manifest != NULL) {
        retval = (ia_css_connect_buf_type_t)manifest->attributes.connect_buf_type;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_connect_buffer_type failed\n");
        retval = IA_CSS_CONNECT_BUF_TYPE_INVALID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_connect_buffer_type(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_connect_buf_type_t connect_buf_type)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_connect_buffer_type(): enter:\n");

    if (manifest != NULL) {
        manifest->attributes.connect_buf_type = connect_buf_type;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_connect_buffer_type failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_connect_ID(
    ia_css_terminal_manifest_t            *manifest,
    const ia_css_terminal_ID_t            ID)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_connect_ID(): enter:\n");

    if (manifest != NULL &&
        ((IA_CSS_TERMINAL_CAT_LOAD == ia_css_terminal_manifest_get_category(manifest)) ||
        (IA_CSS_TERMINAL_INVALID_ID == ID))) {
        manifest->assoc_ID = ID;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_connect_ID failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_terminal_manifest_get_max_payload_size(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_terminal_ID_t retval;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_max_payload_size(): enter:\n");

    if (manifest != NULL) {
        retval = manifest->max_payload_size;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_get_max_payload_size failed\n");
        retval = IA_CSS_TERMINAL_INVALID_ID;
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_max_payload_size(
    ia_css_terminal_manifest_t            *manifest,
    uint32_t                             max_payload_size)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_max_payload_size(): enter:\n");

    if (manifest != NULL) {
        manifest->max_payload_size = max_payload_size;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_max_payload_size failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_group_manifest_t *ia_css_terminal_manifest_get_parent(
    const ia_css_terminal_manifest_t *manifest)
{
    ia_css_program_group_manifest_t    *parent = NULL;
    char *base;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_get_parent(): enter:\n");

    verifexit(manifest != NULL);

    base = (char *)((char *)manifest + manifest->parent_offset);

    parent = (ia_css_program_group_manifest_t *)(base);
EXIT:
    return parent;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_terminal_manifest_set_parent_offset(
    ia_css_terminal_manifest_t *manifest,
    int32_t terminal_offset)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_terminal_manifest_set_parent_offset(): enter:\n");

    verifexit(manifest != NULL);

    /* parent is at negative offset away from current terminal offset*/
    manifest->parent_offset = -terminal_offset;

    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_terminal_manifest_set_parent_offset failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C ia_css_frame_format_bitmap_t
ia_css_data_terminal_manifest_get_frame_format_bitmap(
    const ia_css_data_terminal_manifest_t *manifest)
{
    ia_css_frame_format_bitmap_t bitmap = 0;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_frame_format_bitmap(): enter:\n");

    if (manifest != NULL) {
        bitmap = manifest->frame_format_bitmap;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_data_terminal_manifest_get_frame_format_bitmap invalid argument\n");
    }
    return bitmap;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_data_terminal_manifest_set_frame_format_bitmap(
    ia_css_data_terminal_manifest_t *manifest,
    ia_css_frame_format_bitmap_t bitmap)
{
    int ret = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_frame_format_bitmap(): enter:\n");

    if (manifest != NULL) {
        manifest->frame_format_bitmap = bitmap;
        ret = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_frame_format_bitmap failed (%i)\n",
            ret);
    }

    return ret;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_data_terminal_manifest_can_support_compression(
    const ia_css_data_terminal_manifest_t *manifest)
{
    bool compression_support = false;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_compression_support(): enter:\n");

    if (manifest != NULL) {
        /* compression_support is used boolean encoded in uint8_t.
         * So we only need to check
         * if this is non-zero
         */
        compression_support = (manifest->compression_support != 0);
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_can_support_compression invalid argument\n");
    }

    return compression_support;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_data_terminal_manifest_set_compression_support(
    ia_css_data_terminal_manifest_t *manifest,
    bool compression_support)
{
    int ret = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_compression_support(): enter:\n");

    if (manifest != NULL) {
        manifest->compression_support =
            (compression_support == true) ? 1 : 0;
        ret = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_compression_support failed (%i)\n",
            ret);
    }

    return ret;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_connection_bitmap_t ia_css_data_terminal_manifest_get_connection_bitmap(
    const ia_css_data_terminal_manifest_t *manifest)
{
    ia_css_connection_bitmap_t connection_bitmap = 0;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_connection_bitmap(): enter:\n");

    if (manifest != NULL) {
        connection_bitmap = manifest->connection_bitmap;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_data_terminal_manifest_get_connection_bitmap invalid argument\n");
    }
    return connection_bitmap;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_data_terminal_manifest_set_connection_bitmap(
    ia_css_data_terminal_manifest_t *manifest, ia_css_connection_bitmap_t bitmap)
{
    int ret = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_connection_bitmap(): enter:\n");

    if (manifest != NULL) {
        assert(bitmap != 0); /* zero means there is no connection, this is invalid. */
        assert((bitmap >> IA_CSS_N_CONNECTION_TYPES) == 0);

        manifest->connection_bitmap = bitmap;
        ret = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_data_terminal_manifest_set_connection_bitmap invalid argument\n");
    }
    return ret;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_kernel_bitmap_t ia_css_data_terminal_manifest_get_kernel_bitmap(
    const ia_css_data_terminal_manifest_t *manifest)
{
    ia_css_kernel_bitmap_t kernel_bitmap = ia_css_kernel_bitmap_clear();

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_get_kernel_bitmap(): enter:\n");

    if (manifest != NULL) {
        kernel_bitmap = manifest->kernel_bitmap;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
            "ia_css_data_terminal_manifest_get_kernel_bitmap: invalid argument\n");
    }
    return kernel_bitmap;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_data_terminal_manifest_set_kernel_bitmap(
    ia_css_data_terminal_manifest_t    *manifest,
    const ia_css_kernel_bitmap_t kernel_bitmap)
{
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_data_terminal_manifest_set_kernel_bitmap(): enter:\n");

    if (manifest != NULL) {
        manifest->kernel_bitmap = kernel_bitmap;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_data_terminal_manifest_set_kernel_bitmap: failed (%i)\n",
            retval);
    }

    return retval;
}

#endif /* __IA_CSS_PSYS_TERMINAL_MANIFEST_IMPL_H */
