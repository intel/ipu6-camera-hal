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

#ifndef __IA_CSS_PSYS_TERMINAL_IMPL_H
#define __IA_CSS_PSYS_TERMINAL_IMPL_H

#include <ia_css_psys_terminal.h>

#include <ia_css_psys_process_types.h>
#include <ia_css_psys_terminal_manifest.h>

#include <ia_css_program_group_data.h>
#include <ia_css_program_group_param.h>

#include <ia_css_psys_process_group.h>
#include <ia_css_psys_program_group_manifest.h>
#include <ia_css_psys_process_group.hsys.user.h>

#include <type_support.h>
#include <error_support.h>    /* for verifexit, verifjmpexit */
#include <assert_support.h>    /* for COMPILATION_ERROR_IF */
#include <misc_support.h>    /* for NOT_USED */
#include "ia_css_psys_terminal_private_types.h"
#include "ia_css_terminal_manifest_types.h"
#include "ia_css_psys_dynamic_trace.h"
#include "ia_css_psys_manifest_types.h"
#include "ia_css_psys_program_group_private.h"
#include "ia_css_terminal_types.h"

STORAGE_CLASS_INLINE int ia_css_data_terminal_print(const ia_css_terminal_t *terminal,
    void *fid) {

    DECLARE_ERRVAL
    int retval = -1;
    int i;
    ia_css_data_terminal_t *dterminal = (ia_css_data_terminal_t *)terminal;
    uint16_t fragment_count =
        ia_css_data_terminal_get_fragment_count(dterminal);
    verifexitval(fragment_count != 0, EINVAL);

    retval = ia_css_frame_descriptor_print(
        ia_css_data_terminal_get_frame_descriptor(dterminal),
        fid);
    verifexitval(retval == 0, EINVAL);

    retval = ia_css_frame_print(
        ia_css_data_terminal_get_frame(dterminal), fid);
    verifexitval(retval == 0, EINVAL);

    for (i = 0; i < (int)fragment_count; i++) {
        retval = ia_css_fragment_descriptor_print(
            ia_css_data_terminal_get_fragment_descriptor(
                dterminal, i), fid);
        verifexitval(retval == 0, EINVAL);
    }
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "stream2gen_buffer_size = %u\n", dterminal->stream2gen_buffer_size);

    retval = 0;
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_print failed (%i)\n", retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_print(
    const ia_css_terminal_t *terminal,
    void *fid)
{
    DECLARE_ERRVAL
    int retval = -1;
    ia_css_terminal_type_t term_type = ia_css_terminal_get_type(terminal);

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
        "ia_css_terminal_print(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
        "\tTerminal 0x%lx sizeof %d, typeof %d, parent 0x%lx\n",
        (unsigned long int)terminal,
        (int)ia_css_terminal_get_size(terminal),
        (int)ia_css_terminal_get_type(terminal),
        (unsigned long int)ia_css_terminal_get_parent(terminal));

    switch (term_type) {
    case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
        ia_css_program_control_init_terminal_print(
            (ia_css_program_control_init_terminal_t *)terminal);
    break;
    case IA_CSS_TERMINAL_TYPE_DATA_IN:
    case IA_CSS_TERMINAL_TYPE_DATA_OUT:
        ia_css_data_terminal_print(terminal, fid);
    break;
    default:
        /* other terminal prints are currently not supported */
    break;
    }

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_print invalid argument terminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_print failed (%i)\n", retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_input(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    bool is_input = false;
    ia_css_terminal_type_t terminal_type;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_input(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    terminal_type = ia_css_terminal_get_type(terminal);

    switch (terminal_type) {
    case IA_CSS_TERMINAL_TYPE_DATA_IN:        /* Fall through */
    case IA_CSS_TERMINAL_TYPE_STATE_IN:        /* Fall through */
    case IA_CSS_TERMINAL_TYPE_PARAM_STREAM:        /* Fall through */
    case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
    case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
    case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN:
    case IA_CSS_TERMINAL_TYPE_PROGRAM:
    case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
        is_input = true;
        break;
    case IA_CSS_TERMINAL_TYPE_DATA_OUT:        /* Fall through */
    case IA_CSS_TERMINAL_TYPE_STATE_OUT:
    case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
    case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT:
    case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
        is_input = false;
        break;
    default:
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_input: Unknown terminal type (%d)\n",
            terminal_type);
        goto EXIT;
    }

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_input invalid argument\n");
    }
    return is_input;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
size_t ia_css_terminal_get_size(
    const ia_css_terminal_t    *terminal)
{
    DECLARE_ERRVAL
    size_t size = 0;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_size(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    size = terminal->size;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_size invalid argument\n");
    }
    return size;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_terminal_type_t ia_css_terminal_get_type(
    const ia_css_terminal_t    *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t    terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_type(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    terminal_type = terminal->terminal_type;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_type invalid argument\n");
    }
    return terminal_type;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_type(
    ia_css_terminal_t *terminal,
    const ia_css_terminal_type_t terminal_type)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_type(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    terminal->terminal_type = terminal_type;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_type invalid argument terminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_type failed (%i)\n", retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint16_t ia_css_terminal_get_terminal_manifest_index(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    uint16_t terminal_manifest_index;

    terminal_manifest_index = 0xffff;
    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_terminal_manifest_index(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    terminal_manifest_index = terminal->tm_index;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_terminal_manifest_index: invalid argument\n");
    }
    return terminal_manifest_index;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_terminal_manifest_index(
    ia_css_terminal_t *terminal,
    const uint16_t terminal_manifest_index)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_terminal_manifest_index(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);
    terminal->tm_index = terminal_manifest_index;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_terminal_manifest_index: invalid argument terminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_terminal_manifest_index: failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint32_t ia_css_terminal_get_payload_size(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    uint32_t payload_size;

    payload_size = 0;
    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_payload_size(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    payload_size = terminal->payload_size;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_payload_size: invalid argument\n");
    }
    return payload_size;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_payload_size(
    ia_css_terminal_t *terminal,
    uint32_t payload_size)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_payload_size(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);
    terminal->payload_size = payload_size;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_payload_size: invalid argument terminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_payload_size: failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_terminal_ID_t ia_css_terminal_get_ID(
    const ia_css_terminal_t    *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_ID_t retval = IA_CSS_TERMINAL_INVALID_ID;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_ID(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    retval = terminal->ID;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_ID invalid argument\n");
        retval = 0;
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_data_terminal_get_kernel_id(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    uint8_t retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_kernel_id(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    retval = dterminal->kernel_id;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_kernel_id: invalid argument\n");
        retval =  0;
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_data_terminal_is_compressed(
    const ia_css_data_terminal_t *dterminal,
    uint8_t *val)
{
    DECLARE_ERRVAL
    int retval = -1;
    const ia_css_frame_descriptor_t *frame_descriptor;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_is_compressed enter\n");

    frame_descriptor =
        ia_css_data_terminal_get_frame_descriptor(dterminal);

    verifexitval(frame_descriptor != NULL, EFAULT);

    *val = frame_descriptor->is_compressed;
    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_is_compressed: Null frame descriptor\n");
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_connection_type_t ia_css_data_terminal_get_connection_type(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    ia_css_connection_type_t connection_type = IA_CSS_N_CONNECTION_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_connection_type(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    connection_type = dterminal->connection_type;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_connection_type: invalid argument\n");
    }
    return connection_type;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_data_terminal_get_link_id(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    uint8_t link_id = 0;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_link_id(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    link_id = dterminal->link_id;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_link_id: invalid argument\n");
    }
    return link_id;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_data_terminal_set_link_id(
    ia_css_data_terminal_t *dterminal,
    const uint8_t link_id)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_set_link_id(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);
    dterminal->link_id = link_id;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_link_id: invalid argument terminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_link_id: failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_data_terminal_set_connection_type(
    ia_css_data_terminal_t *dterminal,
    const ia_css_connection_type_t connection_type)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_set_connection_type(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    dterminal->connection_type = connection_type;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_connection_type: invalid argument dterminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_connection_type failed (%i)\n",
            retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_data_terminal_set_stream2gen_buffer_size(
    ia_css_data_terminal_t *dterminal,
    const uint16_t stream2gen_buffer_size)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_set_stream2gen_buffer_size(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, DEBUG,
        "ia_css_data_terminal_set_stream2gen_buffer_size(): stream2gen_buffer_size = %u\n", stream2gen_buffer_size);
    dterminal->stream2gen_buffer_size = stream2gen_buffer_size;

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_stream2gen_buffer_size: invalid argument dterminal\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_set_stream2gen_buffer_size failed (%i)\n",
            retval);
    }

    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_group_t *ia_css_terminal_get_parent(
    const ia_css_terminal_t    *terminal)
{
    DECLARE_ERRVAL
    ia_css_process_group_t *parent = NULL;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_parent(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);
    if (terminal->parent_offset == 0) {
        return NULL;
    }

    parent = (ia_css_process_group_t *) ((char *)terminal +
                    terminal->parent_offset);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_parent invalid argument\n");
    }
    return parent;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_parent(
    ia_css_terminal_t *terminal,
    ia_css_process_group_t *parent)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_parent(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);
    verifexitval(parent != NULL, EFAULT);

    terminal->parent_offset = (uint16_t) ((char *)parent -
                        (char *)terminal);

    retval = 0;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_parent invalid argument\n");
    }
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_parent failed (%i)\n", retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_frame_t *ia_css_data_terminal_get_frame(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    ia_css_frame_t *frame = NULL;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_frame(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    frame = (ia_css_frame_t    *)(&(dterminal->frame));
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_frame invalid argument\n");
    }
    return frame;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_frame_descriptor_t *ia_css_data_terminal_get_frame_descriptor(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    ia_css_frame_descriptor_t *frame_descriptor = NULL;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_frame_descriptor(): enter:\n");

    verifexitval(dterminal != NULL, EFAULT);

    frame_descriptor =
        (ia_css_frame_descriptor_t *)(&(dterminal->frame_descriptor));
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_frame_descriptor: invalid argument\n");
    }
    return frame_descriptor;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_fragment_descriptor_t *ia_css_data_terminal_get_fragment_descriptor(
    const ia_css_data_terminal_t *dterminal,
    const unsigned int fragment_index)
{
    DECLARE_ERRVAL
    ia_css_fragment_descriptor_t *fragment_descriptor = NULL;
    uint16_t fragment_count = 0;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_frame_descriptor(): enter:\n");

    fragment_count = ia_css_data_terminal_get_fragment_count(dterminal);

    verifexitval(dterminal != NULL, EFAULT);
    verifexitval(fragment_count != 0, EINVAL);
    verifexitval(fragment_index < fragment_count, EINVAL);

    fragment_descriptor = (ia_css_fragment_descriptor_t *)
        ((char *)dterminal + dterminal->fragment_descriptor_offset);

    fragment_descriptor += fragment_index;
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_frame_descriptor: invalid argument\n");
    }
    return fragment_descriptor;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint16_t ia_css_data_terminal_get_fragment_count(
    const ia_css_data_terminal_t *dterminal)
{
    DECLARE_ERRVAL
    ia_css_process_group_t *parent;
    uint16_t fragment_count = 0;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_get_fragment_count(): enter:\n");

    parent = ia_css_terminal_get_parent((ia_css_terminal_t *)dterminal);

    verifexitval(dterminal != NULL, EFAULT);
    verifexitval(parent != NULL, EFAULT);

    fragment_count = ia_css_process_group_get_fragment_count(parent);
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_get_fragment_count: invalid argument\n");
    }
    return fragment_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_parameter_terminal(
    const ia_css_terminal_t    *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_parameter_terminal(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    /* will return an error value on error */
    terminal_type = ia_css_terminal_get_type(terminal);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_parameter_terminal: invalid argument\n");
    }
    return (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT);
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_data_terminal(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_data_terminal(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    /* will return an error value on error */
    terminal_type = ia_css_terminal_get_type(terminal);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_data_terminal invalid argument\n");
    }
    return (terminal_type == IA_CSS_TERMINAL_TYPE_DATA_IN ||
            terminal_type == IA_CSS_TERMINAL_TYPE_DATA_OUT);
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_program_terminal(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t    terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_program_terminal(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    /* will return an error value on error */
    terminal_type = ia_css_terminal_get_type(terminal);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_program_terminal: invalid argument\n");
    }
    return (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM);
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_program_control_init_terminal(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t    terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_program_control_init_terminal(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    /* will return an error value on error */
    terminal_type = ia_css_terminal_get_type(terminal);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_program_control_init_terminal: invalid argument\n");
    }
    return (terminal_type == IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT);
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_spatial_parameter_terminal(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    ia_css_terminal_type_t terminal_type = IA_CSS_N_TERMINAL_TYPES;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_spatial_parameter_terminal(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);

    /* will return an error value on error */
    terminal_type = ia_css_terminal_get_type(terminal);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_spatial_param_terminal: invalid argument\n");
    }
    return (terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN ||
        terminal_type == IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT);
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_data_terminal_compute_plane_count(
    const ia_css_terminal_manifest_t *manifest,
    const ia_css_program_group_param_t *param)
{
    DECLARE_ERRVAL
    uint8_t    plane_count = 1;

    NOT_USED(manifest);
    NOT_USED(param);

    verifexitval(manifest != NULL, EFAULT);
    verifexitval(param != NULL, EFAULT);
    /* TODO: Implementation Missing*/

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_data_terminal_compute_plane_count(): enter:\n");
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_data_terminal_compute_plane_count: invalid argument\n");
    }
    return plane_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_vaddress_t  ia_css_terminal_get_buffer(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    vied_vaddress_t buffer = VIED_NULL;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_buffer(): enter:\n");

    verifexitval(terminal != NULL, EFAULT);
    if (ia_css_is_terminal_data_terminal(terminal)) {
        ia_css_frame_t *frame = ia_css_data_terminal_get_frame(
                    (ia_css_data_terminal_t *)terminal);

        verifexitval(frame != NULL, EFAULT);
        buffer = ia_css_frame_get_buffer(frame);
    } else if (ia_css_is_terminal_parameter_terminal(terminal)) {
        const ia_css_param_terminal_t *param_terminal =
            (const ia_css_param_terminal_t *)terminal;

        verifexitval(param_terminal != NULL, EFAULT);
        buffer = param_terminal->param_payload.buffer;
    }  else if (ia_css_is_terminal_program_terminal(terminal)) {
        const ia_css_program_terminal_t *program_terminal =
            (const ia_css_program_terminal_t *)terminal;

        verifexitval(program_terminal != NULL, EFAULT);
        buffer = program_terminal->param_payload.buffer;
    }  else if (ia_css_is_terminal_program_control_init_terminal(terminal)) {
        const ia_css_program_control_init_terminal_t *program_ctrl_init_terminal =
            (const ia_css_program_control_init_terminal_t *)terminal;

        verifexitval(program_ctrl_init_terminal != NULL, EFAULT);
        buffer = program_ctrl_init_terminal->param_payload.buffer;
    } else if (ia_css_is_terminal_spatial_parameter_terminal(terminal)) {
        const ia_css_spatial_param_terminal_t *spatial_terminal =
            (const ia_css_spatial_param_terminal_t *)terminal;

        verifexitval(spatial_terminal != NULL, EFAULT);
        buffer = spatial_terminal->param_payload.buffer;
    }
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_buffer: invalid argument terminal\n");
    }
    return buffer;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_buffer(
    ia_css_terminal_t *terminal,
    vied_vaddress_t buffer)
{
    DECLARE_ERRVAL
    int retval = -1;
    ia_css_param_terminal_t *pterminal = NULL;
    ia_css_program_terminal_t *prog_terminal = NULL;
    ia_css_program_control_init_terminal_t *progctrl_terminal = NULL;
    ia_css_spatial_param_terminal_t *spatial_terminal = NULL;
    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_buffer(): enter:\n");

    if (ia_css_is_terminal_data_terminal(terminal) == true) {
        /* Currently using Frames inside data terminal ,
         * TODO: start directly using data.
         */
        ia_css_data_terminal_t *dterminal =
            (ia_css_data_terminal_t *)terminal;
        ia_css_frame_t *frame =
            ia_css_data_terminal_get_frame(dterminal);

        verifexitval(frame != NULL, EFAULT);
        retval = ia_css_frame_set_buffer(frame, buffer);
        verifexitval(retval == 0, EINVAL);
    } else if (ia_css_is_terminal_parameter_terminal(terminal) == true) {
        pterminal = (ia_css_param_terminal_t *)terminal;
        verifexitval(pterminal != NULL, EFAULT);
        pterminal->param_payload.buffer = buffer;
        retval = 0;
    } else if (ia_css_is_terminal_program_terminal(terminal) == true) {
        prog_terminal = (ia_css_program_terminal_t *)terminal;
        verifexitval(prog_terminal != NULL, EFAULT);
        prog_terminal->param_payload.buffer = buffer;
        retval = 0;
    } else if (ia_css_is_terminal_program_control_init_terminal(terminal) == true) {
        progctrl_terminal = (ia_css_program_control_init_terminal_t *)terminal;
        verifexitval(progctrl_terminal != NULL, EFAULT);
        progctrl_terminal->param_payload.buffer = buffer;
        retval = 0;
    } else if (ia_css_is_terminal_spatial_parameter_terminal(terminal) ==
            true) {
        spatial_terminal = (ia_css_spatial_param_terminal_t *)terminal;
        verifexitval(spatial_terminal != NULL, EFAULT);
        spatial_terminal->param_payload.buffer = buffer;
        retval = 0;
    } else {
        return retval;
    }

    retval = 0;
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_buffer failed (%i)\n", retval);
    }
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_get_terminal_index(
    const ia_css_terminal_t *terminal)
{
    DECLARE_ERRVAL
    int terminal_index = -1;
    verifexitval(terminal != NULL, EFAULT);

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_get_terminal_index(): enter:\n");

    if (ia_css_is_terminal_data_terminal(terminal)) {
        ia_css_frame_t *frame = ia_css_data_terminal_get_frame(
                    (ia_css_data_terminal_t *)terminal);

        verifexitval(frame != NULL, EFAULT);
        terminal_index = ia_css_frame_get_data_index(frame);
    } else {
        if (ia_css_is_terminal_parameter_terminal(terminal)) {
            const ia_css_param_terminal_t *param_terminal =
                (const ia_css_param_terminal_t *)terminal;
            verifexitval(param_terminal != NULL, EFAULT);
            terminal_index = param_terminal->param_payload.terminal_index;
        }  else if (ia_css_is_terminal_program_terminal(terminal)) {
            const ia_css_program_terminal_t *program_terminal =
                (const ia_css_program_terminal_t *)terminal;
            verifexitval(program_terminal != NULL, EFAULT);
            terminal_index = program_terminal->param_payload.terminal_index;
        }  else if (ia_css_is_terminal_program_control_init_terminal(terminal)) {
            const ia_css_program_control_init_terminal_t *program_ctrl_init_terminal =
                (const ia_css_program_control_init_terminal_t *)terminal;
            verifexitval(program_ctrl_init_terminal != NULL, EFAULT);
            terminal_index = program_ctrl_init_terminal->param_payload.terminal_index;
        } else if (ia_css_is_terminal_spatial_parameter_terminal(terminal)) {
            const ia_css_spatial_param_terminal_t *spatial_terminal =
                (const ia_css_spatial_param_terminal_t *)terminal;
            verifexitval(spatial_terminal != NULL, EFAULT);
            terminal_index = spatial_terminal->param_payload.terminal_index;
        } else {
            verifjmpexit(0);
        }
    }
EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_get_terminal_index: invalid argument\n");
    }
    return terminal_index;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_terminal_set_terminal_index(
    ia_css_terminal_t *terminal,
    unsigned int terminal_index)
{
    DECLARE_ERRVAL
    int retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_terminal_set_terminal_index(): enter:\n");

    if (ia_css_is_terminal_data_terminal(terminal) == true) {
        /* Currently using Frames inside data terminal ,
         * TODO: start directly using data.
         */
        ia_css_data_terminal_t *dterminal =
            (ia_css_data_terminal_t *)terminal;
        ia_css_frame_t *frame =
            ia_css_data_terminal_get_frame(dterminal);

        verifexitval(frame != NULL, EFAULT);
        retval = ia_css_frame_set_data_index(frame, terminal_index);
        verifexitval(retval == 0, EINVAL);
    } else {
        if (ia_css_is_terminal_parameter_terminal(terminal) == true) {
            ia_css_param_terminal_t *pterminal =
                (ia_css_param_terminal_t *)terminal;

            pterminal->param_payload.terminal_index = terminal_index;
            retval = 0;
        } else if (ia_css_is_terminal_program_terminal(terminal) == true) {
            ia_css_program_terminal_t *pterminal =
                (ia_css_program_terminal_t *)terminal;

            pterminal->param_payload.terminal_index = terminal_index;
            retval = 0;
        } else if (ia_css_is_terminal_program_control_init_terminal(terminal)
                == true) {
            ia_css_program_control_init_terminal_t *pterminal =
                (ia_css_program_control_init_terminal_t *)terminal;

            pterminal->param_payload.terminal_index = terminal_index;
            retval = 0;
        } else if (ia_css_is_terminal_spatial_parameter_terminal(terminal) ==
                true) {
            ia_css_spatial_param_terminal_t *pterminal =
                (ia_css_spatial_param_terminal_t *)terminal;

            pterminal->param_payload.terminal_index = terminal_index;
            retval = 0;
        } else {
            return retval;
        }
    }

    retval = 0;
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_terminal_set_terminal_index failed (%i)\n",
            retval);
    }
    return retval;
}

STORAGE_CLASS_INLINE bool ia_css_is_data_terminal_valid(
    const ia_css_terminal_t *terminal,
    const ia_css_terminal_manifest_t *terminal_manifest,
    const uint16_t nof_fragments)
{
    DECLARE_ERRVAL
    bool invalid_flag = false;

    const ia_css_data_terminal_t *dterminal =
        (ia_css_data_terminal_t *)terminal;
    const ia_css_data_terminal_manifest_t *dt_manifest =
        (ia_css_data_terminal_manifest_t *)terminal_manifest;
    const ia_css_frame_descriptor_t *frame_descriptor;
    ia_css_frame_format_bitmap_t man_frame_format_bitmap;
    ia_css_frame_format_bitmap_t proc_frame_format_bitmap;
    uint16_t max_value[IA_CSS_N_DATA_DIMENSION];
    uint16_t min_value[IA_CSS_N_DATA_DIMENSION];

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_data_terminal_valid enter\n");

    frame_descriptor =
        ia_css_data_terminal_get_frame_descriptor(dterminal);
    verifexitval(frame_descriptor != NULL, EFAULT);
    man_frame_format_bitmap =
        ia_css_data_terminal_manifest_get_frame_format_bitmap(
                    dt_manifest);
    proc_frame_format_bitmap =
        ia_css_frame_format_bit_mask(
                frame_descriptor->frame_format_type);
    /*
     * TODO: Replace by 'validation of frame format type'.
     * Currently frame format type is not correctly set by manifest,
     * waiting for HSD 1804260604
     */
    if (man_frame_format_bitmap > 0) {
        if ((man_frame_format_bitmap &
                    proc_frame_format_bitmap) == 0) {
            uint32_t *bitmap_arr =
                (uint32_t *)&man_frame_format_bitmap;

            NOT_USED(bitmap_arr);
            IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
                "Frame format type not defined in manifest\n");
            IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
                " man bitmap_arr[]: %d,%d\n",
                bitmap_arr[1], bitmap_arr[0]);
            bitmap_arr = (uint32_t *)&proc_frame_format_bitmap;
            IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
                " proc bitmap_arr[]: %d,%d\n",
                bitmap_arr[1], bitmap_arr[0]);
        }
    } else {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "Frame format bitmap not defined in manifest\n");
    }
    ia_css_data_terminal_manifest_get_min_size(dt_manifest, min_value);
    /*
     * TODO: Replace by validation of Minimal frame column dimensions.
     *  Currently not correctly set by manifest yet,
     *  waiting for HSD 1804260604
     */
    if ((frame_descriptor->dimension[IA_CSS_COL_DIMENSION] <
        min_value[IA_CSS_COL_DIMENSION])) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "Minimal frame column dimensions not set correctly (by manifest)\n");
    }
    /*
     * TODO: Replace by validation of Minimal frame row dimensions.
     * Currently not correctly set by manifest yet,
     * waiting for HSD 1804260604
     */
    if (frame_descriptor->dimension[IA_CSS_ROW_DIMENSION] <
        min_value[IA_CSS_ROW_DIMENSION]) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "Minimal frame row dimensions not set correctly (by manifest)\n");
    }

    ia_css_data_terminal_manifest_get_max_size(dt_manifest, max_value);
    /*
     * TODO: Replace by validation of Maximal frame column dimensions.
     * Currently not correctly set by manifest yet,
     * waiting for HSD 1804260604
     */
    if (frame_descriptor->dimension[IA_CSS_COL_DIMENSION] >
        max_value[IA_CSS_COL_DIMENSION]) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "Maximal frame column dimensions not set correctly (by manifest)\n");
    }
    /*
     * TODO: Replace by validation of Maximal frame row dimensions.
     * Currently not correctly set by manifest yet,
     * waiting for HSD 1804260604
     */
    if (frame_descriptor->dimension[IA_CSS_ROW_DIMENSION] >
        max_value[IA_CSS_ROW_DIMENSION]) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "Maximal frame row dimensions not set correctly (by manifest)\n");
    }
    IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, VERBOSE, "min_value: [%d,%d]\n",
        min_value[IA_CSS_COL_DIMENSION],
        min_value[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, VERBOSE, "max_value: [%d,%d]\n",
        max_value[IA_CSS_COL_DIMENSION],
        max_value[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, VERBOSE, "frame dim: [%d,%d]\n",
        frame_descriptor->dimension[IA_CSS_COL_DIMENSION],
        frame_descriptor->dimension[IA_CSS_ROW_DIMENSION]);
    /*
     * TODO: Add validation of fragment dimensions.
     * Currently not set by manifest yet, waiting for HSD 1804260604
     */
    NOT_USED(nof_fragments);

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_data_terminal_valid() invalid argument\n");
        return false;
    } else {
        return (!invalid_flag);
    }
}

STORAGE_CLASS_INLINE void ia_css_program_terminal_seq_info_print(
    const ia_css_kernel_fragment_sequencer_info_manifest_desc_t
        *man_seq_info_desc,
    const ia_css_kernel_fragment_sequencer_info_desc_t
        *term_seq_info_desc)
{
    NOT_USED(man_seq_info_desc);
    NOT_USED(term_seq_info_desc);

    /* slice dimension column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_slice_dimension: %d\n",
        term_seq_info_desc->
        fragment_grid_slice_dimension[IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_slice_dimension: %d\n",
        man_seq_info_desc->
        max_fragment_grid_slice_dimension[IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_slice_dimension: %d\n",
        man_seq_info_desc->
        min_fragment_grid_slice_dimension[IA_CSS_COL_DIMENSION]);

    /* slice dimension row */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_slice_dimension: %d\n",
        term_seq_info_desc->
        fragment_grid_slice_dimension[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_slice_dimension: %d\n",
        man_seq_info_desc->
        max_fragment_grid_slice_dimension[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_slice_dimension: %d\n",
        man_seq_info_desc->
        min_fragment_grid_slice_dimension[IA_CSS_ROW_DIMENSION]);

    /* slice count column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_slice_count: %d\n",
        term_seq_info_desc->
        fragment_grid_slice_count[IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_slice_count: %d\n",
        man_seq_info_desc->
        max_fragment_grid_slice_count[IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_slice_count: %d\n",
        man_seq_info_desc->
        min_fragment_grid_slice_count[IA_CSS_COL_DIMENSION]);

    /* slice count row */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_slice_count: %d\n",
        term_seq_info_desc->
        fragment_grid_slice_count[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_slice_count: %d\n",
        man_seq_info_desc->
        max_fragment_grid_slice_count[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_slice_count: %d\n",
        man_seq_info_desc->
        min_fragment_grid_slice_count[IA_CSS_ROW_DIMENSION]);

    /* decimation factor column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_point_decimation_factor: %d\n",
        term_seq_info_desc->
        fragment_grid_point_decimation_factor[IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_point_decimation_factor: %d\n",
        man_seq_info_desc->
        max_fragment_grid_point_decimation_factor[IA_CSS_COL_DIMENSION]
        );
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_point_decimation_factor: %d\n",
        man_seq_info_desc->
        min_fragment_grid_point_decimation_factor[IA_CSS_COL_DIMENSION]
        );

    /* decimation factor row */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_point_decimation_factor: %d\n",
        term_seq_info_desc->
        fragment_grid_point_decimation_factor[IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_point_decimation_factor: %d\n",
        man_seq_info_desc->
        max_fragment_grid_point_decimation_factor[
                    IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_point_decimation_factor: %d\n",
        man_seq_info_desc->
        min_fragment_grid_point_decimation_factor[
                    IA_CSS_ROW_DIMENSION]);

    /* index column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_overlay_pixel_topleft_index: %d\n",
        term_seq_info_desc->
        fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_overlay_pixel_topleft_index: %d\n",
        man_seq_info_desc->
        max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_overlay_pixel_topleft_index: %d\n",
        man_seq_info_desc->
        min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION]);

    /* index row */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_overlay_pixel_topleft_index: %d\n",
        term_seq_info_desc->
        fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_overlay_pixel_topleft_index: %d\n",
        man_seq_info_desc->
        max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_overlay_pixel_topleft_index: %d\n",
        man_seq_info_desc->
        min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);

    /* dimension column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_overlay_pixel_dimension: %d\n",
        term_seq_info_desc->
        fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_overlay_pixel_dimension: %d\n",
        man_seq_info_desc->
        max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_overlay_pixel_dimension: %d\n",
        man_seq_info_desc->
        min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION]);

    /* dimension column */
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "fragment_grid_overlay_pixel_dimension: %d\n",
        term_seq_info_desc->
        fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "max_fragment_grid_overlay_pixel_dimension: %d\n",
        man_seq_info_desc->
        max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, VERBOSE,
        "min_fragment_grid_overlay_pixel_dimension: %d\n",
        man_seq_info_desc->
        min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
}

STORAGE_CLASS_INLINE bool ia_css_is_program_terminal_valid(
    const ia_css_terminal_t    *terminal,
    const ia_css_terminal_manifest_t *terminal_manifest,
    const uint16_t nof_fragments)
{
    DECLARE_ERRVAL
    bool invalid_flag = false;
    uint16_t frag_idx;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_program_terminal_valid enter\n");

    for (frag_idx = 0; frag_idx < nof_fragments; frag_idx++) {
        uint16_t frag_seq_info_count, seq_idx;
        const ia_css_program_terminal_t *prog_term;
        const ia_css_program_terminal_manifest_t *prog_term_man;

        prog_term = (const ia_css_program_terminal_t *)terminal;
        prog_term_man =
            (const ia_css_program_terminal_manifest_t *)
            terminal_manifest;
        frag_seq_info_count =
            prog_term_man->
            kernel_fragment_sequencer_info_manifest_info_count;

        for (seq_idx = 0; seq_idx < frag_seq_info_count; seq_idx++) {
            const ia_css_kernel_fragment_sequencer_info_desc_t
            *term_seq_info_desc;
            const
            ia_css_kernel_fragment_sequencer_info_manifest_desc_t *
            man_seq_info_desc;

            term_seq_info_desc =
        ia_css_program_terminal_get_kernel_frgmnt_seq_info_desc(
                    prog_term, frag_idx, seq_idx,
                    frag_seq_info_count);
            verifexitval(term_seq_info_desc != NULL, EFAULT);
            man_seq_info_desc =
        ia_css_program_terminal_manifest_get_kernel_frgmnt_seq_info_desc
                    (prog_term_man, seq_idx);
            verifexitval(man_seq_info_desc != NULL, EFAULT);

            ia_css_program_terminal_seq_info_print(
                man_seq_info_desc, term_seq_info_desc);
            /* slice dimension column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_slice_dimension[
                        IA_CSS_COL_DIMENSION]);

            /* slice dimension row */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_slice_dimension[
                        IA_CSS_ROW_DIMENSION]);

            /* slice count column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_slice_count[
                        IA_CSS_COL_DIMENSION]);

            /* slice count row */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_slice_count[
                        IA_CSS_ROW_DIMENSION]);

            /* decimation factor column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_point_decimation_factor[
                        IA_CSS_COL_DIMENSION]);

            /* decimation factor row */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_point_decimation_factor[
                        IA_CSS_ROW_DIMENSION]);

            /* index column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_COL_DIMENSION]);

            /* index row */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_overlay_pixel_topleft_index[
                        IA_CSS_ROW_DIMENSION]);

            /* dimension column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_COL_DIMENSION]);

            /* dimension column */
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION] >
                man_seq_info_desc->
                max_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
            invalid_flag = invalid_flag ||
                (term_seq_info_desc->
                fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION] <
                man_seq_info_desc->
                min_fragment_grid_overlay_pixel_dimension[
                        IA_CSS_ROW_DIMENSION]);
        }
    }

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_program_terminal_valid() invalid argument\n");
        return false;
    }
    if (invalid_flag == true) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_is_program_terminal_valid(): validation failed\n");
        /* TODO: program terminal parameters not correctly defined,
         * disable validation result until issues has been solved
         */
        return true;
    }
    return (!invalid_flag);
}

STORAGE_CLASS_INLINE bool ia_css_is_sliced_terminal_valid(
    const ia_css_terminal_t *terminal,
    const ia_css_terminal_manifest_t *terminal_manifest,
    const uint16_t nof_fragments)
{
    DECLARE_ERRVAL
    bool invalid_flag = false;
    uint16_t frag_idx;

    uint16_t slice_idx, section_idx;

    const ia_css_sliced_param_terminal_t *sliced_term =
        (const ia_css_sliced_param_terminal_t *)terminal;
    verifexitval(sliced_term != NULL, EFAULT);
    const ia_css_sliced_param_terminal_manifest_t *sliced_term_man =
        (const ia_css_sliced_param_terminal_manifest_t *)
        terminal_manifest;
    verifexitval(sliced_term_man != NULL, EFAULT);

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_sliced_terminal_valid enter\n");

    for (frag_idx = 0; frag_idx < nof_fragments; frag_idx++) {
        const ia_css_fragment_slice_desc_t *fragment_slice_desc =
            ia_css_sliced_param_terminal_get_fragment_slice_desc(
                sliced_term, frag_idx);

        verifexitval(fragment_slice_desc != NULL, EFAULT);

        for (slice_idx = 0;
            slice_idx < fragment_slice_desc->slice_count;
            slice_idx++) {
            for (section_idx = 0;
                section_idx <
                sliced_term_man->sliced_param_section_count;
                section_idx++) {
                const
                ia_css_sliced_param_manifest_section_desc_t *
                    slice_man_section_desc;
                const ia_css_slice_param_section_desc_t *
                    slice_section_desc;

                slice_man_section_desc =
        ia_css_sliced_param_terminal_manifest_get_sliced_prm_sct_desc(
                        sliced_term_man, section_idx);
                slice_section_desc =
        ia_css_sliced_param_terminal_get_slice_param_section_desc(
                        sliced_term, frag_idx,
                        slice_idx, section_idx,
                        sliced_term_man->
                        sliced_param_section_count);
                verifexitval(slice_man_section_desc != NULL, EFAULT);
                verifexitval(slice_section_desc != NULL, EFAULT);

                invalid_flag = invalid_flag ||
                    (slice_section_desc->mem_size >
                    slice_man_section_desc->max_mem_size);
            }
        }
    }

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
                "ia_css_is_sliced_terminal_valid() invalid argument\n");
        return false;
    } else {
        return (!invalid_flag);
    }

}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_terminal_valid(
    const ia_css_terminal_t    *terminal,
    const ia_css_terminal_manifest_t *terminal_manifest)
{
    DECLARE_ERRVAL
    bool is_valid = false;
    uint16_t nof_fragments;
    ia_css_terminal_type_t terminal_type = IA_CSS_TERMINAL_INVALID_ID;

    verifexitval(NULL != terminal, EFAULT);
    verifexitval(NULL != terminal_manifest, EFAULT);

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_is_terminal_valid enter\n");

    nof_fragments = ia_css_data_terminal_get_fragment_count(
                (const ia_css_data_terminal_t *)terminal);
    terminal_type = ia_css_terminal_get_type(terminal);

    switch (terminal_type) {
    case IA_CSS_TERMINAL_TYPE_DATA_IN:
    case IA_CSS_TERMINAL_TYPE_DATA_OUT:
        is_valid = ia_css_is_data_terminal_valid(terminal,
                terminal_manifest, nof_fragments);
        break;
    case IA_CSS_TERMINAL_TYPE_PROGRAM:
        is_valid = ia_css_is_program_terminal_valid(terminal,
                terminal_manifest, nof_fragments);
        break;
    case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN:
    case IA_CSS_TERMINAL_TYPE_PARAM_CACHED_OUT:
    case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_IN:
    case IA_CSS_TERMINAL_TYPE_PARAM_SPATIAL_OUT:
    case IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT:
        /* Nothing to be validated for cached and spatial
        *  parameters, return valid
        */
        is_valid = true;
        break;
    case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_IN:
    case IA_CSS_TERMINAL_TYPE_PARAM_SLICED_OUT:
        is_valid = ia_css_is_sliced_terminal_valid(terminal,
                terminal_manifest, nof_fragments);
        break;
    default:
        /* Terminal type unknown, return invalid */
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_is_terminal_valid() Terminal type %x unknown\n",
            (int)terminal_type);
        is_valid = false;
        break;
    }

EXIT:
    if (haserror(EFAULT)) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_is_terminal_valid() invalid argument\n");
        return false;
    }
    /* TODO: to be removed once all PGs pass validation */
    if (is_valid == false) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "ia_css_is_terminal_valid(): type: %d validation failed\n",
            terminal_type);
    }
    return is_valid;
}

/* ================= Program Control Init Terminal - START ================= */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int
ia_css_program_control_init_terminal_init(
    ia_css_program_control_init_terminal_t *terminal,
    const ia_css_program_control_init_terminal_manifest_t *manifest,
    const ia_css_program_group_manifest_t *pg_manifest,
    ia_css_kernel_bitmap_t kernel_bitmap)
{
    int retval = -1;
    unsigned int i;
    unsigned int base_load_sec;
    unsigned int base_connect_sec;
    unsigned int load_index = 0;
    unsigned int connect_index = 0;
    unsigned int load_section_count = 0;
    unsigned int connect_section_count = 0;

    ia_css_program_control_init_manifest_program_desc_t *man_progs;
    ia_css_program_manifest_t *prog_manifest;

    verifjmpexit(terminal != NULL);

    man_progs =
    ia_css_program_control_init_terminal_manifest_get_program_desc(manifest, 0);
    verifjmpexit(man_progs != NULL);

    for (i = 0; i < manifest->program_count; i++) {
        prog_manifest = ia_css_program_group_manifest_get_prgrm_mnfst(pg_manifest, i);
        verifjmpexit(prog_manifest != NULL);
        if (!ia_css_process_group_is_program_enabled(prog_manifest, kernel_bitmap)) {
            continue;
        }
        load_section_count += man_progs[i].load_section_count;
        connect_section_count += man_progs[i].connect_section_count;
    }

    terminal->program_count = manifest->program_count;
    terminal->program_section_desc_offset =
        sizeof(ia_css_program_control_init_terminal_t);

    base_load_sec = /* base_load_sec relative to first program */
        terminal->program_count *
        sizeof(ia_css_program_control_init_program_desc_t);

    base_connect_sec = base_load_sec +
        load_section_count *
        sizeof(ia_css_program_control_init_load_section_desc_t);

    for (i = 0; i < terminal->program_count; i++) {
        ia_css_program_control_init_program_desc_t *prog;

        prog = ia_css_program_control_init_terminal_get_program_desc(
                terminal, i);
        verifjmpexit(prog != NULL);

        prog_manifest = ia_css_program_group_manifest_get_prgrm_mnfst(pg_manifest, i);
        verifjmpexit(prog_manifest != NULL);
        if (!ia_css_process_group_is_program_enabled(prog_manifest, kernel_bitmap)) {
            prog->load_section_count = 0;
            prog->connect_section_count = 0;
            prog->load_section_desc_offset = 0;
            prog->connect_section_desc_offset  = 0;
            continue;
        }

        prog->load_section_count = man_progs[i].load_section_count;
        prog->connect_section_count = man_progs[i].connect_section_count;

        prog->load_section_desc_offset =
            base_load_sec +
            load_index *
            sizeof(ia_css_program_control_init_load_section_desc_t) -
            i * sizeof(ia_css_program_control_init_program_desc_t);
        prog->connect_section_desc_offset =
            base_connect_sec +
            connect_index *
            sizeof(ia_css_program_control_init_connect_section_desc_t) -
            i * sizeof(ia_css_program_control_init_program_desc_t);

        load_index += man_progs[i].load_section_count;
        connect_index += man_progs[i].connect_section_count;
    }
    retval = 0;
EXIT:
    return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
unsigned int
ia_css_program_control_init_terminal_get_descriptor_size(
        const ia_css_program_control_init_terminal_manifest_t *manifest,
        const ia_css_program_group_manifest_t *pg_manifest,
        ia_css_kernel_bitmap_t kernel_bitmap)
{
    unsigned int i;
    unsigned size = 0;
    unsigned load_section_count = 0;
    unsigned connect_section_count = 0;
    ia_css_program_control_init_manifest_program_desc_t *man_progs;
    ia_css_program_manifest_t *prog_manifest;
    verifjmpexit(manifest != NULL);

    man_progs =
        ia_css_program_control_init_terminal_manifest_get_program_desc(
            manifest, 0);
    verifjmpexit(man_progs != NULL);

    for (i = 0; i < manifest->program_count; i++) {
        prog_manifest = ia_css_program_group_manifest_get_prgrm_mnfst(pg_manifest, i);
        verifjmpexit(prog_manifest != NULL);
        if (!ia_css_process_group_is_program_enabled(prog_manifest, kernel_bitmap)) {
            continue;
        }
        load_section_count += man_progs[i].load_section_count;
        connect_section_count += man_progs[i].connect_section_count;
    }

    size = sizeof(ia_css_program_control_init_terminal_t) +
        manifest->program_count *
        sizeof(struct ia_css_program_control_init_program_desc_s) +
        load_section_count *
        sizeof(struct ia_css_program_control_init_load_section_desc_s) +
        connect_section_count  *
        sizeof(struct ia_css_program_control_init_connect_section_desc_s);
EXIT:
    return size;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
void ia_css_program_control_init_terminal_print(
    const ia_css_program_control_init_terminal_t *terminal)
{
    unsigned int prog_idx, sec_idx;
    ia_css_program_control_init_program_desc_t *prog;
    ia_css_program_control_init_load_section_desc_t *load_sec;
    ia_css_program_control_init_connect_section_desc_t *connect_sec;
    unsigned mem_offset;
    verifjmpexit(terminal != NULL);

    IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
        "program_count: %d, payload_fragment_stride: %d\n",
        terminal->program_count,
        terminal->payload_fragment_stride);

    for (prog_idx = 0; prog_idx < terminal->program_count; prog_idx++) {
        prog = ia_css_program_control_init_terminal_get_program_desc(
            terminal, prog_idx);
        verifjmpexit(prog != NULL);
        mem_offset = ia_css_program_control_init_terminal_get_load_section_mem_offset(prog);

        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO, "program_idx: %d\n", prog_idx);
        IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO, "\tprocess_id: %d, num_done_events: %d\n",
            prog->control_info.process_id,
            prog->control_info.num_done_events);

        if (prog->load_section_count > 0) {
            IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO, "\tload_sections:\n");
        }
        for (sec_idx = 0; sec_idx < prog->load_section_count; sec_idx++) {
            load_sec =
                ia_css_program_control_init_terminal_get_load_section_desc(
                    prog, sec_idx);
            verifjmpexit(load_sec != NULL);
            IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
                "\t\tdevice_descriptor_id: 0x%08x, mem_offset: %d, "
                "mem_size: %d, mode_bitmask: %x\n",
                load_sec->device_descriptor_id.data,
                mem_offset,
                load_sec->mem_size,
                load_sec->mode_bitmask);
            mem_offset += load_sec->mem_size;
        }
        if (prog->connect_section_count > 0) {
            IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO, "\tconnect_sections:\n");
        }
        for (sec_idx = 0; sec_idx < prog->connect_section_count; sec_idx++) {
            connect_sec =
                ia_css_program_control_init_terminal_get_connect_section_desc(
                    prog, sec_idx);
            verifjmpexit(connect_sec != NULL);
            IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
                "\t\tdevice_descriptor_id: 0x%08x, "
                "connect_terminal_ID: %d, connect_section_idx: %d, "
                "mode_bitmask: %x\n",
                connect_sec->device_descriptor_id.data,
                connect_sec->connect_terminal_ID,
                connect_sec->connect_section_idx,
                connect_sec->mode_bitmask);
        }
    }
EXIT:
    return;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_program_control_init_program_desc_t *
ia_css_program_control_init_terminal_get_program_desc(
    const ia_css_program_control_init_terminal_t *prog_ctrl_init_terminal,
    const unsigned int program_index)
{
    ia_css_program_control_init_program_desc_t *program_desc_base;
    ia_css_program_control_init_program_desc_t *program_desc = NULL;

    verifjmpexit(prog_ctrl_init_terminal != NULL);
    verifjmpexit(program_index < prog_ctrl_init_terminal->program_count);

    program_desc_base = (ia_css_program_control_init_program_desc_t *)
        (((const char *)prog_ctrl_init_terminal) +
         prog_ctrl_init_terminal->program_section_desc_offset);
    program_desc = &(program_desc_base[program_index]);

EXIT:
    return program_desc;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_id_t ia_css_program_control_init_terminal_get_process_id(
    const ia_css_program_control_init_program_desc_t *program_desc)
{
    ia_css_process_id_t process_id = 0;

    verifjmpexit(program_desc != NULL);

    process_id = program_desc->control_info.process_id;

EXIT:
    return process_id;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_program_control_init_terminal_get_num_done_events(
    const ia_css_program_control_init_program_desc_t *program_desc)
{
    uint8_t num_done_events = 0;

    verifjmpexit(program_desc != NULL);

    num_done_events = program_desc->control_info.num_done_events;

EXIT:
    return num_done_events;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
void ia_css_program_control_init_terminal_set_control_info(
    ia_css_program_control_init_program_desc_t *program_desc,
    ia_css_process_id_t process_id,
    uint8_t num_done_events)
{
    verifjmpexit(program_desc != NULL);

    program_desc->control_info.process_id = process_id;
    program_desc->control_info.num_done_events = num_done_events;

EXIT:
    return;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
unsigned int ia_css_program_control_init_terminal_get_connect_section_count(
    const ia_css_program_control_init_program_desc_t *program_desc)
{
    assert(program_desc != NULL);
    return program_desc->connect_section_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
unsigned int ia_css_program_control_init_terminal_get_load_section_count(
    const ia_css_program_control_init_program_desc_t *program_desc)
{
    assert(program_desc != NULL);
    return program_desc->load_section_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
unsigned int ia_css_program_control_init_terminal_get_load_section_mem_offset(
    const ia_css_program_control_init_program_desc_t *program_desc)
{
    assert(program_desc != NULL);
    return program_desc->load_section_mem_offset;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_program_control_init_load_section_desc_t *
ia_css_program_control_init_terminal_get_load_section_desc(
    const ia_css_program_control_init_program_desc_t *program_desc,
    const unsigned int load_section_index)
{
    ia_css_program_control_init_load_section_desc_t *load_section_desc_base;
    ia_css_program_control_init_load_section_desc_t *load_section_desc = NULL;

    verifjmpexit(program_desc != NULL);
    verifjmpexit(load_section_index < program_desc->load_section_count);

    load_section_desc_base = (ia_css_program_control_init_load_section_desc_t *)
        (((const char *)program_desc) +
         program_desc->load_section_desc_offset);
    load_section_desc = &(load_section_desc_base[load_section_index]);

EXIT:
    return load_section_desc;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_program_control_init_connect_section_desc_t *
ia_css_program_control_init_terminal_get_connect_section_desc(
    const ia_css_program_control_init_program_desc_t *program_desc,
    const unsigned int connect_section_index)
{
    ia_css_program_control_init_connect_section_desc_t *connect_sec_desc_base;
    ia_css_program_control_init_connect_section_desc_t *connect_sec_desc = NULL;

    verifjmpexit(program_desc != NULL);
    verifjmpexit(connect_section_index < program_desc->connect_section_count);

    connect_sec_desc_base =
        (ia_css_program_control_init_connect_section_desc_t *)
        (((const char *)program_desc) +
         program_desc->connect_section_desc_offset);
    connect_sec_desc = &(connect_sec_desc_base[connect_section_index]);

EXIT:
    return connect_sec_desc;
}

/* ================= Program Control Init Terminal - END ================= */

#endif /* __IA_CSS_PSYS_TERMINAL_IMPL_H */
