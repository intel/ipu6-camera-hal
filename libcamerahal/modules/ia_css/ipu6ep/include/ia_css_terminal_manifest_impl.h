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

#ifndef __IA_CSS_TERMINAL_MANIFEST_IMPL_H
#define __IA_CSS_TERMINAL_MANIFEST_IMPL_H

#include "ia_css_terminal_manifest.h"
#include "error_support.h"
#include "assert_support.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE void __terminal_manifest_dummy_check_alignment(void)
{
    COMPILATION_ERROR_IF(
        SIZE_OF_PARAM_TERMINAL_MANIFEST_STRUCT_IN_BITS !=
            (CHAR_BIT * sizeof(ia_css_param_terminal_manifest_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_param_terminal_manifest_t) % sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_PARAM_TERMINAL_MANIFEST_SEC_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(ia_css_param_manifest_section_desc_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_param_manifest_section_desc_t) %
            sizeof(uint32_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_SPATIAL_PARAM_TERM_MAN_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(ia_css_spatial_param_terminal_manifest_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_spatial_param_terminal_manifest_t) %
            sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_FRAME_GRID_PARAM_MAN_SEC_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(
            ia_css_frame_grid_param_manifest_section_desc_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_frame_grid_param_manifest_section_desc_t) %
            sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_PROG_TERM_MAN_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(ia_css_program_terminal_manifest_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_program_terminal_manifest_t)%sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_FRAG_PARAM_MAN_SEC_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(
            ia_css_fragment_param_manifest_section_desc_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_fragment_param_manifest_section_desc_t) %
            sizeof(uint32_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_KERNEL_FRAG_SEQ_INFO_MAN_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(
            ia_css_kernel_fragment_sequencer_info_manifest_desc_t))
        );

    COMPILATION_ERROR_IF(0 != sizeof(
        ia_css_kernel_fragment_sequencer_info_manifest_desc_t) %
            sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_PARAM_TERMINAL_MANIFEST_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof(ia_css_sliced_param_terminal_manifest_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_sliced_param_terminal_manifest_t) %
            sizeof(uint64_t));

    COMPILATION_ERROR_IF(
        SIZE_OF_SLICED_PARAM_MAN_SEC_STRUCT_IN_BITS !=
        (CHAR_BIT * sizeof
            (ia_css_sliced_param_manifest_section_desc_t)));

    COMPILATION_ERROR_IF(0 !=
        sizeof(ia_css_sliced_param_manifest_section_desc_t) %
            sizeof(uint64_t));
}

/* Parameter Terminal */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
unsigned int ia_css_param_terminal_manifest_get_size(
    const unsigned int nof_sections)
{

    return sizeof(ia_css_param_terminal_manifest_t) +
        nof_sections*sizeof(ia_css_param_manifest_section_desc_t);
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
int ia_css_param_terminal_manifest_init(
    ia_css_param_terminal_manifest_t *param_terminal,
    const uint16_t section_count)
{
    if (param_terminal == NULL) {
        return -EFAULT;
    }

    param_terminal->param_manifest_section_desc_count = section_count;
    param_terminal->param_manifest_section_desc_offset = sizeof(
                ia_css_param_terminal_manifest_t);

    return 0;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
ia_css_param_manifest_section_desc_t *
ia_css_param_terminal_manifest_get_prm_sct_desc(
    const ia_css_param_terminal_manifest_t *param_terminal_manifest,
    const unsigned int section_index)
{
    ia_css_param_manifest_section_desc_t *param_manifest_section_base;
    ia_css_param_manifest_section_desc_t *
        param_manifest_section_desc = NULL;

    verifjmpexit(param_terminal_manifest != NULL);

    param_manifest_section_base =
        (ia_css_param_manifest_section_desc_t *)
        (((const char *)param_terminal_manifest)
        + param_terminal_manifest->param_manifest_section_desc_offset);

    param_manifest_section_desc =
        &(param_manifest_section_base[section_index]);

EXIT:
    return param_manifest_section_desc;
}

/* Spatial Parameter Terminal */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
unsigned int ia_css_spatial_param_terminal_manifest_get_size(
    const unsigned int nof_frame_param_sections)
{
    return sizeof(ia_css_spatial_param_terminal_manifest_t) +
        nof_frame_param_sections * sizeof(
            ia_css_frame_grid_param_manifest_section_desc_t);
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
int ia_css_spatial_param_terminal_manifest_init(
    ia_css_spatial_param_terminal_manifest_t *spatial_param_terminal,
    const uint16_t section_count)
{
    if (spatial_param_terminal == NULL) {
        return -EFAULT;
    }

    spatial_param_terminal->
        frame_grid_param_manifest_section_desc_count = section_count;
    spatial_param_terminal->
        frame_grid_param_manifest_section_desc_offset =
        sizeof(ia_css_spatial_param_terminal_manifest_t);

    return 0;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
ia_css_frame_grid_param_manifest_section_desc_t *
ia_css_spatial_param_terminal_manifest_get_frm_grid_prm_sct_desc(
    const ia_css_spatial_param_terminal_manifest_t *
        spatial_param_terminal_manifest,
    const unsigned int section_index)
{
    ia_css_frame_grid_param_manifest_section_desc_t *
        frame_param_manifest_section_base;
    ia_css_frame_grid_param_manifest_section_desc_t *
        frame_param_manifest_section_desc = NULL;

    verifjmpexit(spatial_param_terminal_manifest != NULL);

    frame_param_manifest_section_base =
        (ia_css_frame_grid_param_manifest_section_desc_t *)
        (((const char *)spatial_param_terminal_manifest) +
            spatial_param_terminal_manifest->
            frame_grid_param_manifest_section_desc_offset);
    frame_param_manifest_section_desc =
        &(frame_param_manifest_section_base[section_index]);

EXIT:
    return frame_param_manifest_section_desc;
}

/* Sliced Terminal */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
unsigned int ia_css_sliced_param_terminal_manifest_get_size(
    const unsigned int nof_slice_param_sections)
{
    return sizeof(ia_css_spatial_param_terminal_manifest_t) +
        nof_slice_param_sections *
        sizeof(ia_css_sliced_param_manifest_section_desc_t);
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
int ia_css_sliced_param_terminal_manifest_init(
    ia_css_sliced_param_terminal_manifest_t *sliced_param_terminal,
    const uint16_t section_count)
{
    if (sliced_param_terminal == NULL) {
        return -EFAULT;
    }

    sliced_param_terminal->sliced_param_section_count = section_count;
    sliced_param_terminal->sliced_param_section_offset =
        sizeof(ia_css_sliced_param_terminal_manifest_t);

    return 0;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
ia_css_sliced_param_manifest_section_desc_t *
ia_css_sliced_param_terminal_manifest_get_sliced_prm_sct_desc(
    const ia_css_sliced_param_terminal_manifest_t *
        sliced_param_terminal_manifest,
    const unsigned int section_index)
{
    ia_css_sliced_param_manifest_section_desc_t *
        sliced_param_manifest_section_base;
    ia_css_sliced_param_manifest_section_desc_t *
        sliced_param_manifest_section_desc = NULL;

    verifjmpexit(sliced_param_terminal_manifest != NULL);

    sliced_param_manifest_section_base =
        (ia_css_sliced_param_manifest_section_desc_t *)
        (((const char *)sliced_param_terminal_manifest) +
            sliced_param_terminal_manifest->
            sliced_param_section_offset);
    sliced_param_manifest_section_desc =
        &(sliced_param_manifest_section_base[section_index]);

EXIT:
    return sliced_param_manifest_section_desc;
}

/* Program Terminal */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
unsigned int ia_css_program_terminal_manifest_get_size(
    const unsigned int nof_fragment_param_sections,
    const unsigned int nof_kernel_fragment_sequencer_infos)
{
    return sizeof(ia_css_program_terminal_manifest_t) +
        nof_fragment_param_sections *
        sizeof(ia_css_fragment_param_manifest_section_desc_t) +
        nof_kernel_fragment_sequencer_infos *
        sizeof(ia_css_kernel_fragment_sequencer_info_manifest_desc_t);
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
int ia_css_program_terminal_manifest_init(
    ia_css_program_terminal_manifest_t *program_terminal,
    const uint16_t fragment_param_section_count,
    const uint16_t kernel_fragment_seq_info_section_count)
{
    if (program_terminal == NULL) {
        return -EFAULT;
    }

    program_terminal->fragment_param_manifest_section_desc_count =
        fragment_param_section_count;
    program_terminal->fragment_param_manifest_section_desc_offset =
        sizeof(ia_css_program_terminal_manifest_t);

    program_terminal->kernel_fragment_sequencer_info_manifest_info_count =
        kernel_fragment_seq_info_section_count;
    program_terminal->kernel_fragment_sequencer_info_manifest_info_offset =
        sizeof(ia_css_program_terminal_manifest_t) +
        fragment_param_section_count*sizeof(
            ia_css_fragment_param_manifest_section_desc_t);

    return 0;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
ia_css_fragment_param_manifest_section_desc_t *
ia_css_program_terminal_manifest_get_frgmnt_prm_sct_desc(
    const ia_css_program_terminal_manifest_t *program_terminal_manifest,
    const unsigned int section_index)
{
    ia_css_fragment_param_manifest_section_desc_t *
        fragment_param_manifest_section_base;
    ia_css_fragment_param_manifest_section_desc_t *
        fragment_param_manifest_section = NULL;

    verifjmpexit(program_terminal_manifest != NULL);

    fragment_param_manifest_section_base =
        (ia_css_fragment_param_manifest_section_desc_t *)
        (((const char *)program_terminal_manifest) +
        program_terminal_manifest->
        fragment_param_manifest_section_desc_offset);
    fragment_param_manifest_section =
        &(fragment_param_manifest_section_base[section_index]);

EXIT:
    return fragment_param_manifest_section;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
ia_css_kernel_fragment_sequencer_info_manifest_desc_t *
ia_css_program_terminal_manifest_get_kernel_frgmnt_seq_info_desc(
    const ia_css_program_terminal_manifest_t *program_terminal_manifest,
    const unsigned int info_index)
{
    ia_css_kernel_fragment_sequencer_info_manifest_desc_t *
        kernel_manifest_fragment_sequencer_info_manifest_desc_base;
    ia_css_kernel_fragment_sequencer_info_manifest_desc_t *
        kernel_manifest_fragment_sequencer_info_manifest_desc = NULL;

    verifjmpexit(program_terminal_manifest != NULL);

    kernel_manifest_fragment_sequencer_info_manifest_desc_base =
        (ia_css_kernel_fragment_sequencer_info_manifest_desc_t *)
        (((const char *)program_terminal_manifest) +
        program_terminal_manifest->
        kernel_fragment_sequencer_info_manifest_info_offset);

    kernel_manifest_fragment_sequencer_info_manifest_desc =
        &(kernel_manifest_fragment_sequencer_info_manifest_desc_base[
                info_index]);

EXIT:
    return kernel_manifest_fragment_sequencer_info_manifest_desc;
}

/* Start ...*/

IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_param_manifest_section_desc_get_kernel_id(
    const ia_css_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_KERNEL_ID_OFFSET) & MANIFEST_SECTION_DESC_KERNEL_ID_MASK;
}
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_param_manifest_section_desc_get_region_id(
    const ia_css_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_REGION_ID_OFFSET) & MANIFEST_SECTION_DESC_REGION_ID_MASK;
}
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_param_manifest_section_desc_get_mem_type_id(
    const ia_css_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_MEM_TYPE_ID_OFFSET) & MANIFEST_SECTION_DESC_MEM_TYPE_ID_MASK;
}

IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_kernel_id(
    const ia_css_fragment_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_KERNEL_ID_OFFSET) & MANIFEST_SECTION_DESC_KERNEL_ID_MASK;
}
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_region_id(
    const ia_css_fragment_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_REGION_ID_OFFSET) & MANIFEST_SECTION_DESC_REGION_ID_MASK;
}
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_mem_type_id(
    const ia_css_fragment_param_manifest_section_desc_t *section)
{
    return (section->info >> MANIFEST_SECTION_DESC_MEM_TYPE_ID_OFFSET) & MANIFEST_SECTION_DESC_MEM_TYPE_ID_MASK;
}

/* End ...*/

#endif /* __IA_CSS_TERMINAL_MANIFEST_IMPL_H */
