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

#include <ia_css_psys_program_manifest.h>
#include <ia_css_psys_program_group_manifest.h>
/* for ia_css_kernel_bitmap_t, ia_css_kernel_bitmap_print */
#include <ia_css_kernel_bitmap.h>

#include <vied_nci_psys_system_global.h>
#include "ia_css_psys_program_group_private.h"
#include "ia_css_psys_static_trace.h"

#include <error_support.h>
#include <misc_support.h>
#include <storage_class.h>
#include <math_support.h>

/* *****************************************************
 * Functions to possibly inline
 * ******************************************************/
#ifndef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_program_manifest_impl.h"
#endif /* __IA_CSS_PSYS_STATIC_INLINE__ */

/* *****************************************************
 * Functions not to inline and available to firmware
 * ******************************************************/
bool ia_css_has_program_manifest_fixed_cell(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL
    bool has_fixed_cell = false;
    vied_nci_cell_ID_t cell_id;
    vied_nci_cell_type_ID_t    cell_type_id;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_has_program_manifest_fixed_cell(): enter:\n");

    verifexitval(manifest != NULL, EFAULT);

    cell_id      = ia_css_program_manifest_get_cell_ID(manifest);
    cell_type_id = ia_css_program_manifest_get_cell_type_ID(manifest);

    has_fixed_cell = ((cell_id != VIED_NCI_N_CELL_ID) &&
              (cell_type_id == VIED_NCI_N_CELL_TYPE_ID));
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
           "ia_css_has_program_manifest_fixed_cell invalid argument\n");
        return false;
    }
    return has_fixed_cell;
}

vied_nci_resource_bitmap_t ia_css_program_manifest_get_cells_bitmap(
    const ia_css_program_manifest_t            *manifest)
{
    DECLARE_ERRVAL
    vied_nci_resource_bitmap_t    bitmap = 0;
    int i = 0;
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_get_cells_bitmap(): enter:\n");

    verifexitval(manifest != NULL, EFAULT);

    for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
        if (VIED_NCI_N_CELL_ID != manifest->cells[i]) {
            bitmap |= (1 << manifest->cells[i]);
        }
#ifdef __HIVECC
#pragma hivecc unroll
#endif
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_get_cells_bitmap invalid argument\n");
    }
    return bitmap;
}

bool ia_css_is_program_manifest_subnode_program_type(
    const ia_css_program_manifest_t *manifest)
{
    ia_css_program_type_t        program_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_is_program_manifest_subnode_program_type(): enter:\n");

    program_type = ia_css_program_manifest_get_type(manifest);
/* The error return is the limit value, so no need to check on the manifest
 * pointer
 */
    return (program_type == IA_CSS_PROGRAM_TYPE_PARALLEL_SUB) ||
            (program_type == IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUB) ||
            (program_type == IA_CSS_PROGRAM_TYPE_VIRTUAL_SUB);
}

bool ia_css_is_program_manifest_supernode_program_type(
    const ia_css_program_manifest_t *manifest)
{
    ia_css_program_type_t program_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
           "ia_css_is_program_manifest_supernode_program_type(): enter:\n");

    program_type = ia_css_program_manifest_get_type(manifest);

/* The error return is the limit value, so no need to check on the manifest
 * pointer
 */
    return (program_type == IA_CSS_PROGRAM_TYPE_PARALLEL_SUPER) ||
            (program_type == IA_CSS_PROGRAM_TYPE_EXCLUSIVE_SUPER) ||
            (program_type == IA_CSS_PROGRAM_TYPE_VIRTUAL_SUPER);
}

vied_nci_cell_ID_t ia_css_program_manifest_get_cell_ID(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    vied_nci_cell_ID_t cell_id = VIED_NCI_N_CELL_ID;
    int i = 0;
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_get_cell_ID(): enter:\n");

    verifexitval(manifest != NULL, EFAULT);

    for (i = 1; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
        assert(VIED_NCI_N_CELL_ID == manifest->cells[i]);
#ifdef __HIVECC
#pragma hivecc unroll
#endif
    }
    cell_id = manifest->cells[0];
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
          "ia_css_program_manifest_get_cell_ID invalid argument\n");
        return VIED_NCI_N_CELL_ID;
    }
    return cell_id;
}

/* *****************************************************
 * Functions not to inline and not available to firmware
 * ******************************************************/
#if !defined(__HIVECC)
size_t ia_css_sizeof_program_manifest(
    const uint8_t program_needs_extension,
    const uint8_t program_dependency_count,
    const uint8_t terminal_dependency_count)
{
    size_t    size = 0;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_sizeof_program_manifest(): enter:\n");

    size += sizeof(ia_css_program_manifest_t);
    if (program_needs_extension == 1) {
        /* align manifest extension to 32 bit */
        size = CEIL_MUL(size, sizeof(uint32_t));
        size += sizeof(ia_css_program_manifest_ext_t);
    }
    size += program_dependency_count * sizeof(uint8_t);
    size += terminal_dependency_count * sizeof(uint8_t);
    /* align whole structure to 32 bit */
    size = ceil_mul(size, sizeof(uint32_t));

    return size;
}

int ia_css_program_manifest_set_program_ID(
    ia_css_program_manifest_t            *manifest,
    ia_css_program_ID_t id)
{
    int ret = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_program_ID(): enter:\n");

    if (manifest != NULL) {
        assert(id < UINT8_MAX && IA_CSS_PROGRAM_ID_BITS == 8);
        manifest->ID = id;
        ret = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
           "ia_css_program_manifest_set_program_ID failed (%i)\n", ret);
    }
    return ret;
}

int ia_css_program_manifest_set_parent_offset(
    ia_css_program_manifest_t    *manifest,
    int32_t program_offset)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_parent_offset(): enter:\n");

    verifexit(manifest != NULL);

    /* parent is at negative offset away from current program offset*/
    assert((int16_t)(-program_offset) == -program_offset);
    manifest->parent_offset = (int16_t)(-program_offset);

    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_parent_offset failed (%i)\n",
            retval);
    }
    return retval;
}

int ia_css_program_manifest_set_type(
    ia_css_program_manifest_t                *manifest,
    const ia_css_program_type_t                program_type)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_type(): enter:\n");

    if (manifest != NULL) {
        assert(program_type < UINT8_MAX && IA_CSS_PROGRAM_TYPE_BITS == 8);
        manifest->program_type = (ia_css_program_type_t)program_type;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
              "ia_css_program_manifest_set_type failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_program_manifest_set_kernel_bitmap(
    ia_css_program_manifest_t            *manifest,
    const ia_css_kernel_bitmap_t            kernel_bitmap)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_kernel_bitmap(): enter:\n");

    if (manifest != NULL) {
        manifest->kernel_bitmap = kernel_bitmap;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_kernel_bitmap failed (%i)\n",
            retval);
    }
    return retval;
}

int ia_css_program_manifest_set_cell_ID(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_cell_ID_t            cell_id)
{
    int    retval = -1;
    int i = 0;
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_cell_ID(): enter:\n");
    if (manifest != NULL) {
        manifest->cells[0] = cell_id;
        for (i = 1; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
            manifest->cells[i] = VIED_NCI_N_CELL_ID;
        }
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
           "ia_css_program_manifest_set_cell_ID failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_program_manifest_set_cell_type_ID(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_cell_type_ID_t            cell_type_id)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_cell_type_ID(): enter:\n");
    if (manifest != NULL) {
        manifest->cell_type_id = cell_type_id;
        retval = 0;
    } else {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_cell_type_ID failed (%i)\n",
            retval);
    }
    return retval;
}

void ia_css_program_manifest_set_input_terminal_id(
    ia_css_program_manifest_t   *manifest,
    unsigned char                   idx,
    unsigned char                   val)
{
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_input_terminal_id(): enter:\n");
    if (manifest != NULL) {
#if IA_CSS_MAX_INPUT_DEC_RESOURCES > 0
        ia_css_program_manifest_ext_t *manifest_ext = NULL;

        manifest_ext = ia_css_program_manifest_get_extension(manifest);
        if (manifest_ext == NULL) {
            return;
        }
        assert(idx < IA_CSS_MAX_INPUT_DEC_RESOURCES);
        manifest_ext->dec_resources_input_terminal[idx] = val;
#else
        (void) idx;
        (void) val;
#endif
    }
}

void ia_css_program_manifest_set_output_terminal_id(
    ia_css_program_manifest_t   *manifest,
    unsigned char                    idx,
    unsigned char                    val)
{
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_output_terminal_id(): enter:\n");
    if (manifest != NULL) {
#if IA_CSS_MAX_OUTPUT_DEC_RESOURCES > 0
        ia_css_program_manifest_ext_t *manifest_ext = NULL;

        manifest_ext = ia_css_program_manifest_get_extension(manifest);
        if (manifest_ext == NULL) {
            return;
        }
        assert(idx < IA_CSS_MAX_OUTPUT_DEC_RESOURCES);
        manifest_ext->dec_resources_output_terminal[idx] = val;
#else
        (void) idx;
        (void) val;
        (void) manifest_ext;
#endif
    }
}

void ia_css_program_manifest_set_input_stream_id(
    ia_css_program_manifest_t   *manifest,
    unsigned char                   idx,
    unsigned char                   val)
{
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_input_stream_id(): enter:\n");
    if (manifest != NULL) {
#if IA_CSS_MAX_INPUT_DEC_RESOURCES > 0
        ia_css_program_manifest_ext_t *manifest_ext = NULL;

        manifest_ext = ia_css_program_manifest_get_extension(manifest);
        if (manifest_ext == NULL) {
            return;
        }
        assert(idx < IA_CSS_MAX_INPUT_DEC_RESOURCES);
        manifest_ext->dec_resources_input[idx] = val;
#else
        (void) idx;
        (void) val;
#endif
    }
}

void ia_css_program_manifest_set_output_stream_id(
    ia_css_program_manifest_t   *manifest,
    unsigned char                    idx,
    unsigned char                    val)
{
    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_output_stream_id(): enter:\n");
    if (manifest != NULL) {
#if IA_CSS_MAX_OUTPUT_DEC_RESOURCES > 0
        ia_css_program_manifest_ext_t *manifest_ext = NULL;

        manifest_ext = ia_css_program_manifest_get_extension(manifest);
        if (manifest_ext == NULL) {
            return;
        }
        assert(idx < IA_CSS_MAX_OUTPUT_DEC_RESOURCES);
        manifest_ext->dec_resources_output[idx] = val;
#else
        (void) idx;
        (void) val;
#endif
    }
}

int ia_css_program_manifest_set_cells_bitmap(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_resource_bitmap_t    bitmap)
{
    int retval = -1;
    int array_index = 0;
    int bit_index;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_cells_bitmap(): enter:\n");

    if (manifest != NULL) {
        for (bit_index = 0; bit_index < VIED_NCI_N_CELL_ID; bit_index++) {
            if (vied_nci_is_bit_set_in_bitmap(bitmap, bit_index)) {
                verifexit(array_index < IA_CSS_PROCESS_MAX_CELLS);
                manifest->cells[array_index] = (vied_nci_cell_ID_t)bit_index;
                array_index++;
            }
        }
        for (; array_index < IA_CSS_PROCESS_MAX_CELLS; array_index++) {
            manifest->cells[array_index] = VIED_NCI_N_CELL_ID;
        }
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_set_cells_bitmap invalid argument\n");
    }
EXIT:
    return retval;
}

#if HAS_DFM
int ia_css_program_manifest_set_dfm_port_bitmap(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_dev_dfm_id_t          dfm_type_id,
    const vied_nci_resource_bitmap_t     bitmap)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_dfm_port_bitmap(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(dfm_type_id < VIED_NCI_N_DEV_DFM_ID);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && bitmap == 0) {
        return 0;
    }
    verifexit(manifest_ext != NULL);
    manifest_ext->dfm_port_bitmap[dfm_type_id] = bitmap;
    retval = 0;

EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                "ia_css_program_manifest_set_dfm_port_bitmap invalid argument\n");
    }
    return retval;
}

int ia_css_program_manifest_set_dfm_active_port_bitmap(
    ia_css_program_manifest_t           *manifest,
    const vied_nci_dev_dfm_id_t          dfm_type_id,
    const vied_nci_resource_bitmap_t     bitmap)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
            "ia_css_program_manifest_set_dfm_active_port_bitmap(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(dfm_type_id < VIED_NCI_N_DEV_DFM_ID);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && bitmap == 0) {
        return 0;
    }
    verifexit(manifest_ext != NULL);
    manifest_ext->dfm_active_port_bitmap[dfm_type_id] = bitmap;
    retval = 0;

EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                "ia_css_program_manifest_set_dfm_active_port_bitmap invalid argument\n");
    }
    return retval;
}

int ia_css_program_manifest_set_is_dfm_relocatable(
    ia_css_program_manifest_t       *manifest,
    const vied_nci_dev_dfm_id_t      dfm_type_id,
    const uint8_t                    is_relocatable)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
            "ia_css_program_manifest_set_is_dfm_relocatable(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(dfm_type_id < VIED_NCI_N_DEV_DFM_ID);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
    manifest_ext->is_dfm_relocatable[dfm_type_id] = is_relocatable;
    retval = 0;

    EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                "ia_css_program_manifest_set_is_dfm_relocatable invalid argument\n");
    }

    return retval;
}
#endif /* HAS_DFM */

int ia_css_program_manifest_set_int_mem_size(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_mem_type_ID_t            mem_type_id,
    const vied_nci_resource_size_t            int_mem_size)
{
    (void)manifest;
    (void)mem_type_id;
    if (int_mem_size != 0) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_int_mem_size should not be called"
            "(internally memory feature not used).\n");
        return -1;
    }
    return 0;
}

int ia_css_program_manifest_set_ext_mem_size(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_mem_type_ID_t            mem_type_id,
    const vied_nci_resource_size_t            ext_mem_size)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    if (manifest == NULL) {
        return retval;
    }
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && ext_mem_size == 0) {
        return 0;
    } else if (manifest_ext == NULL) {
        return retval;
    }

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_ext_mem_size(): enter:\n");

    if (manifest != NULL && mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID) {
        manifest_ext->ext_mem_size[mem_type_id] = ext_mem_size;
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_set_ext_mem_size invalid argument\n");
    }

    return retval;
}

int ia_css_program_manifest_set_ext_mem_offset(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_mem_type_ID_t            mem_type_id,
    const vied_nci_resource_size_t            ext_mem_offset)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    if (manifest == NULL) {
        return retval;
    }
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && ext_mem_offset ==
        (vied_nci_resource_size_t)IA_CSS_PROGRAM_MANIFEST_RESOURCE_OFFSET_IS_RELOCATABLE) {
        return 0;
    } else if (manifest_ext == NULL) {
        return retval;
    }

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_ext_mem_offset(): enter:\n");

    if (manifest != NULL && mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID) {
        manifest_ext->ext_mem_offset[mem_type_id] = ext_mem_offset;
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_set_ext_mem_offset invalid argument\n");
    }

    return retval;
}

int ia_css_program_manifest_set_dev_chn_size(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_dev_chn_ID_t            dev_chn_id,
    const vied_nci_resource_size_t            dev_chn_size)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    if (manifest == NULL) {
        return retval;
    }
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && dev_chn_size == 0) {
        return 0;
    } else if (manifest_ext == NULL) {
        return retval;
    }

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_dev_chn_size(): enter:\n");
#ifndef IPU_SYSVER_ipu7v1
    if (manifest != NULL && dev_chn_id < VIED_NCI_N_DEV_CHN_ID) {
        manifest_ext->dev_chn_size[dev_chn_id] = dev_chn_size;
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_set_dev_chn_size invalid argument\n");
    }
#else
    NOT_USED(dev_chn_id);
#endif
    return retval;
}

int ia_css_program_manifest_set_dev_chn_offset(
    ia_css_program_manifest_t            *manifest,
    const vied_nci_dev_chn_ID_t            dev_chn_id,
    const vied_nci_resource_size_t            dev_chn_offset)
{
    int retval = -1;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    if (manifest == NULL) {
        return retval;
    }
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL && dev_chn_offset ==
        (vied_nci_resource_size_t)IA_CSS_PROGRAM_MANIFEST_RESOURCE_OFFSET_IS_RELOCATABLE) {
        return 0;
    } else if (manifest_ext == NULL) {
        return retval;
    }

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
               "ia_css_program_manifest_set_dev_chn_offset(): enter:\n");
#ifndef IPU_SYSVER_ipu7v1
    if (manifest != NULL && dev_chn_id < VIED_NCI_N_DEV_CHN_ID) {
        manifest_ext->dev_chn_offset[dev_chn_id] = dev_chn_offset;
        retval = 0;
    } else {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
         "ia_css_program_manifest_set_dev_chn_offset invalid argument\n");
    }
#else
    NOT_USED(dev_chn_id);
#endif
    return retval;
}

int ia_css_program_manifest_set_program_dependency(
    ia_css_program_manifest_t    *manifest,
    const uint8_t            program_dependency,
    const unsigned int        index)
{
    int    retval = -1;
    uint8_t *program_dep_ptr;
    uint8_t    program_dependency_count;
    uint8_t    program_count;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_program_dependency(): enter:\n");

    program_dependency_count =
        ia_css_program_manifest_get_program_dependency_count(manifest);
    program_count =
        ia_css_program_group_manifest_get_program_count(
            ia_css_program_manifest_get_parent(manifest));

    if ((index < program_dependency_count) &&
        (program_dependency < program_count)) {
        program_dep_ptr = (uint8_t *)((uint8_t *)manifest +
                  manifest->program_dependency_offset +
                  index*sizeof(uint8_t));
         *program_dep_ptr = program_dependency;
        retval = 0;
    }

    if (retval != 0) {
        IA_CSS_TRACE_3(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_program_dependency(m, %d, %d) failed (%i)\n",
            program_dependency, index, retval);
    }
    return retval;
}

int ia_css_program_manifest_set_terminal_dependency(
    ia_css_program_manifest_t            *manifest,
    const uint8_t                    terminal_dependency,
    const unsigned int                index)
{
    int    retval = -1;
    uint8_t *terminal_dep_ptr;
    uint8_t terminal_dependency_count =
        ia_css_program_manifest_get_terminal_dependency_count(manifest);
    uint8_t terminal_count =
        ia_css_program_group_manifest_get_terminal_count(
            ia_css_program_manifest_get_parent(manifest));

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_set_terminal_dependency(): enter:\n");

    if ((index < terminal_dependency_count) &&
            (terminal_dependency < terminal_count)) {
        terminal_dep_ptr = (uint8_t *)((uint8_t *)manifest +
                  manifest->terminal_dependency_offset + index);
         *terminal_dep_ptr = terminal_dependency;
        retval = 0;
    }

    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_set_terminal_dependency failed (%i)\n",
            retval);
    }
    return retval;
}

void ia_css_program_manifest_init(
    ia_css_program_manifest_t    *blob,
    const uint8_t    program_needs_extension,
    const uint8_t    program_dependency_count,
    const uint8_t    terminal_dependency_count)
{
    size_t size = 0;
    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
        "ia_css_program_manifest_init(): enter:\n");

    /*TODO: add assert*/
    if (!blob)
        return;

    blob->ID = 1;
    blob->program_dependency_count = program_dependency_count;
    blob->terminal_dependency_count = terminal_dependency_count;

    size += sizeof(ia_css_program_manifest_t);
    if (program_needs_extension == 1) {
        /* align start of extension to 32 bit */
        size = CEIL_MUL(size, sizeof(uint32_t));
        blob->program_extension_offset = size;
        size += sizeof(ia_css_program_manifest_ext_t);
    } else {
        blob->program_extension_offset = 0;
    }
    blob->program_dependency_offset = size;
    size += sizeof(uint8_t) * program_dependency_count;
    blob->terminal_dependency_offset = size;
    size += sizeof(uint8_t) * terminal_dependency_count;
    /* align whole structure to 32 bit */
    size = CEIL_MUL(size, sizeof(uint32_t));

    blob->size = (uint8_t)ia_css_sizeof_program_manifest(
        program_needs_extension,
        program_dependency_count,
        terminal_dependency_count);

    assert(blob->size == size);
    /* The following assert also implies that:
     * blob->program_extension_offset,
     * blob->program_dependency_offset,
     * blob->terminal_dependency_offset,
     * are <= UINT8_MAX
     */
    assert(size <= UINT8_MAX);
}

int ia_css_program_manifest_print(
    const ia_css_program_manifest_t *manifest,
    void *fid)
{
    int            retval = -1;
    int            i, mem_index, dev_chn_index;

    vied_nci_cell_type_ID_t    cell_type_id;
    uint8_t                    program_dependency_count;
    uint8_t                    terminal_dependency_count;
    ia_css_kernel_bitmap_t    bitmap;
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
               "ia_css_program_manifest_print(): enter:\n");

    verifexit(manifest != NULL);
    NOT_USED(fid);

    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "sizeof(manifest) = %d\n",
        (int)ia_css_program_manifest_get_size(manifest));
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "program ID = %d\n",
        (int)ia_css_program_manifest_get_program_ID(manifest));

    bitmap = ia_css_program_manifest_get_kernel_bitmap(manifest);
    verifexit(ia_css_kernel_bitmap_print(bitmap, fid) == 0);

    if (ia_css_has_program_manifest_fixed_cell(manifest)) {
        vied_nci_cell_ID_t cell_id =
                  ia_css_program_manifest_get_cell_ID(manifest);

        cell_type_id = vied_nci_cell_get_type(cell_id);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "cell(program) = %d\n",
            (int)cell_id);
    } else {
        cell_type_id =
            ia_css_program_manifest_get_cell_type_ID(manifest);
    }
    IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "cell type(program) = %d\n",
        (int)cell_type_id);

    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext != NULL) {
    for (mem_index = 0; mem_index < (int)VIED_NCI_N_MEM_TYPE_ID;
         mem_index++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(internal mem) type = %d\n",
        (int)vied_nci_cell_type_get_mem_type(cell_type_id, mem_index));
    }

    for (mem_index = 0; mem_index < (int)VIED_NCI_N_DATA_MEM_TYPE_ID;
         mem_index++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(external mem) type = %d\n",
            (int)(vied_nci_mem_type_ID_t)mem_index);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(external mem) size = %d\n",
            manifest_ext->ext_mem_size[mem_index]);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(external mem) offset = %d\n",
            manifest_ext->ext_mem_offset[mem_index]);
    }
#if HAS_DFM
    for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_CHN_ID;
         dev_chn_index++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(device channel) type = %d\n",
            (int)dev_chn_index);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(device channel) size = %d\n",
            manifest_ext->dev_chn_size[dev_chn_index]);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(device channel) offset = %d\n",
            manifest_ext->dev_chn_offset[dev_chn_index]);
    }
    for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_DFM_ID;
        dev_chn_index++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(dfm port) type = %d\n",
            (int)dev_chn_index);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(dfm port) port_bitmap = %x\n",
            manifest_ext->dfm_port_bitmap[dev_chn_index]);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(dfm port) active_port_bitmap = %x\n",
            manifest_ext->dfm_active_port_bitmap[dev_chn_index]);

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(dfm port) is_dfm_relocatable = %d\n",
            manifest_ext->is_dfm_relocatable[dev_chn_index]);
    }
#else
    NOT_USED(dev_chn_index);
#endif /* HAS_DFM */
#ifdef USE_DEC400
    for (i = 0; i < IA_CSS_MAX_INPUT_DEC_RESOURCES; i++) {
        IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
            "\tEncoder stream id for idx = %d is: %d \n",
            i,
            manifest_ext->dec_resources_input[i]);

    }
    for (i = 0; i < IA_CSS_MAX_OUTPUT_DEC_RESOURCES; i++) {
        IA_CSS_TRACE_2(PSYSAPI_STATIC, INFO,
            "\tDecoder stream id for idx: = %d is: %d\n",
            i,
            manifest_ext->dec_resources_output[i]);

    }
#endif
    }

    for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "\ttype(cells) bitmap = %d\n",
            manifest->cells[i]);
    }
    program_dependency_count =
        ia_css_program_manifest_get_program_dependency_count(manifest);
    if (program_dependency_count == 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "program_dependencies[%d] {};\n",
            program_dependency_count);
    } else {
        uint8_t prog_dep;

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "program_dependencies[%d] {\n",
            program_dependency_count);
        for (i = 0; i < (int)program_dependency_count - 1; i++) {
            prog_dep =
            ia_css_program_manifest_get_program_dependency(
                manifest, i);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "\t %4d,\n", prog_dep);
        }
        prog_dep =
        ia_css_program_manifest_get_program_dependency(manifest, i);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\t %4d }\n", prog_dep);
        (void)prog_dep;
    }

    terminal_dependency_count =
        ia_css_program_manifest_get_terminal_dependency_count(manifest);
    if (terminal_dependency_count == 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "terminal_dependencies[%d] {};\n",
            terminal_dependency_count);
    } else {
        uint8_t term_dep;

        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
            "terminal_dependencies[%d] {\n",
            terminal_dependency_count);
        for (i = 0; i < (int)terminal_dependency_count - 1; i++) {
            term_dep =
            ia_css_program_manifest_get_terminal_dependency(
                manifest, i);
            IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO,
                "\t %4d,\n", term_dep);
        }
        term_dep =
           ia_css_program_manifest_get_terminal_dependency(manifest, i);
        IA_CSS_TRACE_1(PSYSAPI_STATIC, INFO, "\t %4d }\n", term_dep);
        (void)term_dep;
    }
    (void)cell_type_id;

    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_STATIC, ERROR,
            "ia_css_program_manifest_print failed (%i)\n", retval);
    }
    return retval;
}

#endif /* !defined(__HIVECC) */
