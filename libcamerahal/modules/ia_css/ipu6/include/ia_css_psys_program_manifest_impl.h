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

#ifndef __IA_CSS_PSYS_PROGRAM_MANIFEST_IMPL_H
#define __IA_CSS_PSYS_PROGRAM_MANIFEST_IMPL_H

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

/** Function only to be used in FW */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_manifest_ext_t *ia_css_program_manifest_get_extension(const ia_css_program_manifest_t *program)
{
    DECLARE_ERRVAL
    verifexitval(program != NULL, EFAULT);
    if (program->program_extension_offset != 0) {
        return (ia_css_program_manifest_ext_t *)((uint8_t *)program + program->program_extension_offset);
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_extension invalid argument\n");
    }
    return NULL;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_program_manifest_get_int_mem_size(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_mem_type_ID_t     mem_type_id)
{
    (void)manifest;
    (void)mem_type_id;
    IA_CSS_TRACE_0(PSYSAPI_STATIC, INFO,
                   "ia_css_program_manifest_get_int_mem_size always returns 0"
                   "(internally memory feature not used).\n");
    return 0;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_input_terminal_id(
    ia_css_program_manifest_t *manifest,
    unsigned char              idx)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_input_stream_id(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(idx < IA_CSS_MAX_INPUT_DEC_RESOURCES, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
EXIT:
    if (!noerror()) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
#if IA_CSS_MAX_INPUT_DEC_RESOURCES > 0
    return manifest_ext->dec_resources_input_terminal[idx];
#else
    (void) idx;
    return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
#endif
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_output_terminal_id(
    ia_css_program_manifest_t *manifest,
    unsigned char              idx)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_output_terminal_id(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(idx < IA_CSS_MAX_OUTPUT_DEC_RESOURCES, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
EXIT:
    if (!noerror()) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
#if IA_CSS_MAX_OUTPUT_DEC_RESOURCES > 0
    return manifest_ext->dec_resources_output_terminal[idx];
#else
    (void) idx;
    return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
#endif

}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_input_stream_id(
    ia_css_program_manifest_t  *manifest,
    unsigned char               idx)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_input_stream_id(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(idx < IA_CSS_MAX_INPUT_DEC_RESOURCES, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
EXIT:
    if (!noerror()) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
#if IA_CSS_MAX_INPUT_DEC_RESOURCES > 0
    return manifest_ext->dec_resources_input[idx];
#else
    (void) idx;
    return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
#endif
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_output_stream_id(
    ia_css_program_manifest_t *manifest,
    unsigned char              idx)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_output_stream_id(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(idx < IA_CSS_MAX_OUTPUT_DEC_RESOURCES, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
EXIT:
    if (!noerror()) {
        return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
    }
#if IA_CSS_MAX_OUTPUT_DEC_RESOURCES > 0
    return manifest_ext->dec_resources_output[idx];
#else
    (void) idx;
    return IA_CSS_PROGRAM_INVALID_DEC_CONFIG;
#endif
}

#if HAS_DFM
IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_is_dfm_relocatable(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_dev_dfm_id_t      dfm_type_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_is_dfm_relocatable(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(dfm_type_id < VIED_NCI_N_DEV_DFM_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_is_dfm_relocatable invalid argument\n");
        return 0;
    }
    return manifest_ext->is_dfm_relocatable[dfm_type_id];
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t ia_css_program_manifest_get_dfm_port_bitmap(
    const ia_css_program_manifest_t  *manifest,
    const vied_nci_dev_dfm_id_t       dfm_type_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_dfm_port_bitmap(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(dfm_type_id < VIED_NCI_N_DEV_DFM_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_dfm_port_bitmap invalid argument\n");
        return 0;
    }
    return manifest_ext->dfm_port_bitmap[dfm_type_id];
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t ia_css_program_manifest_get_dfm_active_port_bitmap(
    const ia_css_program_manifest_t  *manifest,
    const vied_nci_dev_dfm_id_t       dfm_type_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_dfm_active_port_bitmap(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(dfm_type_id < VIED_NCI_N_DEV_DFM_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_dfm_active_port_bitmap invalid argument\n");
        return 0;
    }
    return manifest_ext->dfm_active_port_bitmap[dfm_type_id];
}
#endif /* HAS_DFM */

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_program_manifest_get_ext_mem_size(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_mem_type_ID_t     mem_type_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_ext_mem_size(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_ext_mem_size invalid argument\n");
        return 0;
    }
    return manifest_ext->ext_mem_size[mem_type_id];
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_program_manifest_get_ext_mem_offset(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_mem_type_ID_t     mem_type_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_ext_mem_offset(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_ext_mem_offset invalid argument\n");
         return 0;
    }
    return manifest_ext->ext_mem_offset[mem_type_id];
}

#if HAS_DFM
IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_program_manifest_get_dev_chn_size(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_dev_chn_ID_t      dev_chn_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_dev_chn_size(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(dev_chn_id < VIED_NCI_N_DEV_CHN_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_dev_chn_size invalid argument\n");
        return 0;
    }
    return manifest_ext->dev_chn_size[dev_chn_id];
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_program_manifest_get_dev_chn_offset(
    const ia_css_program_manifest_t *manifest,
    const vied_nci_dev_chn_ID_t      dev_chn_id)
{
    DECLARE_ERRVAL
    ia_css_program_manifest_ext_t *manifest_ext = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_dev_chn_offset(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    verifexitval(dev_chn_id < VIED_NCI_N_DEV_CHN_ID, EFAULT);
    manifest_ext = ia_css_program_manifest_get_extension(manifest);
    if (manifest_ext == NULL) {
        return 0;
    }
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_dev_chn_offset invalid argument\n");
         return 0;
    }
    return manifest_ext->dev_chn_offset[dev_chn_id];
}
#endif /* HAS_DFM */

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
size_t ia_css_program_manifest_get_size(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_size(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_size invalid argument\n");
        return 0;
    }

    return manifest->size;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_ID_t ia_css_program_manifest_get_program_ID(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_program_ID(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_program_ID invalid argument\n");
        return IA_CSS_PROGRAM_INVALID_ID;
    }
    return manifest->ID;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_group_manifest_t *ia_css_program_manifest_get_parent(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL
    char *base = NULL;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_parent(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);

    base = (char *)((char *)manifest + manifest->parent_offset);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_parent invalid argument\n");
        return NULL;
    }
    return (ia_css_program_group_manifest_t *)(base);
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
vied_nci_cell_type_ID_t ia_css_program_manifest_get_cell_type_ID(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_cell_type_ID(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_cell_type_ID invalid argument\n");
        return VIED_NCI_N_CELL_TYPE_ID;
    }
    return manifest->cell_type_id;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_type_t ia_css_program_manifest_get_type(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_type(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_type invalid argument\n");
        return IA_CSS_N_PROGRAM_TYPES;
    }
    return manifest->program_type;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_terminal_dependency_count(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_terminal_dependency_count(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_terminal_dependency_count invalid argument\n");
        return 0;
    }
    return manifest->terminal_dependency_count;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_terminal_dependency(
    const ia_css_program_manifest_t *manifest,
    const unsigned int index)
{
    DECLARE_ERRVAL
    uint8_t *terminal_dep_ptr = NULL;
    uint8_t terminal_dependency_count;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_terminal_dependency(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
    terminal_dependency_count =
        ia_css_program_manifest_get_terminal_dependency_count(manifest);
    verifexitval(index < terminal_dependency_count, EFAULT);

    terminal_dep_ptr = (uint8_t *)((uint8_t *)manifest +
            manifest->terminal_dependency_offset + index);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_terminal_dependency invalid argument\n");
        return IA_CSS_PROGRAM_INVALID_DEPENDENCY;
    }
    return *terminal_dep_ptr;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
bool ia_css_is_program_manifest_singular_program_type(
    const ia_css_program_manifest_t *manifest)
{
    ia_css_program_type_t        program_type;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_is_program_manifest_singular_program_type(): enter:\n");

    program_type = ia_css_program_manifest_get_type(manifest);

/* The error return is the limit value, so no need to check on the manifest
 * pointer
 */
    return (program_type == IA_CSS_PROGRAM_TYPE_SINGULAR);
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_program_dependency_count(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
        "ia_css_program_manifest_get_program_dependency_count(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_program_dependency_count invalid argument\n");
        return 0;
    }
    return manifest->program_dependency_count;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_manifest_get_program_dependency(
    const ia_css_program_manifest_t *manifest,
    const unsigned int index)
{
    DECLARE_ERRVAL
    uint8_t *program_dep_ptr = NULL;
    uint8_t program_dependency_count;

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_program_dependency(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);

    program_dependency_count =
        ia_css_program_manifest_get_program_dependency_count(manifest);

    verifexitval(index < program_dependency_count, EFAULT);
    program_dep_ptr =  (uint8_t *)((uint8_t *)manifest +
            manifest->program_dependency_offset +
            index * sizeof(uint8_t));
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
                       "ia_css_program_manifest_get_program_dependency invalid argument\n");
        return IA_CSS_PROGRAM_INVALID_DEPENDENCY;
    }
    return *program_dep_ptr;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_kernel_bitmap_t ia_css_program_manifest_get_kernel_bitmap(
    const ia_css_program_manifest_t *manifest)
{
    DECLARE_ERRVAL

    IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
                   "ia_css_program_manifest_get_kernel_bitmap(): enter:\n");
    verifexitval(manifest != NULL, EFAULT);
EXIT:
    if (!noerror()) {
        IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
          "ia_css_program_manifest_get_kernel_bitmap invalid argument\n");
        return ia_css_kernel_bitmap_clear();
    }
    return manifest->kernel_bitmap;
}

#endif /* __IA_CSS_PSYS_PROGRAM_MANIFEST_IMPL_H */
