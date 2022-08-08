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

#ifndef __IA_CSS_TERMINAL_MANIFEST_H
#define __IA_CSS_TERMINAL_MANIFEST_H

#include "type_support.h"
#include "ia_css_param_storage_class.h"
#include "ia_css_terminal_manifest_types.h"

#define MANIFEST_SECTION_DESC_KERNEL_ID_MASK     0xFF
#define MANIFEST_SECTION_DESC_KERNEL_ID_OFFSET   0
#define MANIFEST_SECTION_DESC_REGION_ID_MASK     0xF
#define MANIFEST_SECTION_DESC_REGION_ID_OFFSET   8
#define MANIFEST_SECTION_DESC_MEM_TYPE_ID_MASK   0xF
#define MANIFEST_SECTION_DESC_MEM_TYPE_ID_OFFSET 12

/** Returns the size in bytes required to store this
 *  terminal's manifest entry.
 *
 *  Used as part of binary manifest generation. It helps
 *  to determine the amount of memory required
 *  to store a parameter terminal manifest entry for
 *  allocation before initialization
 *
 *  @param [in] nof_sections Number of sections (==
 *  number of descriptors)
 *
 *  @return Size in bytes required for an entry.
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_param_terminal_manifest_get_size(
	const unsigned int nof_sections
);

/** Initialize a parameter terminal manifest entry.
 *
 *  Used as part of binary manifest generation.
 *
 *  @param param_terminal Terminal entry to initialize.  Must
 *  have been allocated with the size determined by
 *  ia_css_param_terminal_manifest_get_size().
 *
 *  @return 0 on success, -EFAULT if param_terminal is NULL.
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_param_terminal_manifest_init(
	ia_css_param_terminal_manifest_t *param_terminal,
	const uint16_t section_count
);

/** Returns a section descriptor for a parameter terminal.
 *
 *  @param param_terminal_manifest Parameter terminal manifest handle
 *  @param section_index Index of descriptor.  Must be smaller than the
 *  value returned by ia_css_param_terminal_manifest_get_prm_sct_desc_count()
 *
 *  @return Pointer to a descriptor structure
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_param_manifest_section_desc_t *
ia_css_param_terminal_manifest_get_prm_sct_desc(
	const ia_css_param_terminal_manifest_t *param_terminal_manifest,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_spatial_param_terminal_manifest_get_size(
	const unsigned int nof_frame_param_sections
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_spatial_param_terminal_manifest_init(
	ia_css_spatial_param_terminal_manifest_t *spatial_param_terminal,
	const uint16_t section_count
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_frame_grid_param_manifest_section_desc_t *
ia_css_spatial_param_terminal_manifest_get_frm_grid_prm_sct_desc(
	const ia_css_spatial_param_terminal_manifest_t *
		spatial_param_terminal_manifest,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_sliced_param_terminal_manifest_get_size(
	const unsigned int nof_slice_param_sections
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_sliced_param_terminal_manifest_init(
	ia_css_sliced_param_terminal_manifest_t *sliced_param_terminal,
	const uint16_t section_count
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_sliced_param_manifest_section_desc_t *
ia_css_sliced_param_terminal_manifest_get_sliced_prm_sct_desc(
	const ia_css_sliced_param_terminal_manifest_t *
		sliced_param_terminal_manifest,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_program_terminal_manifest_get_size(
	const unsigned int nof_fragment_param_sections,
	const unsigned int nof_kernel_fragment_sequencer_infos
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_program_terminal_manifest_init(
	ia_css_program_terminal_manifest_t *program_terminal,
	const uint16_t fragment_param_section_count,
	const uint16_t kernel_fragment_seq_info_section_count
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_fragment_param_manifest_section_desc_t *
ia_css_program_terminal_manifest_get_frgmnt_prm_sct_desc(
	const ia_css_program_terminal_manifest_t *program_terminal_manifest,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_kernel_fragment_sequencer_info_manifest_desc_t *
ia_css_program_terminal_manifest_get_kernel_frgmnt_seq_info_desc(
	const ia_css_program_terminal_manifest_t *program_terminal_manifest,
	const unsigned int info_index
);

/*! Getter for kernel id

  The function return the kernel id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] param manifest section desc

 @return kernel_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C uint8_t
ia_css_param_manifest_section_desc_get_kernel_id(
	const ia_css_param_manifest_section_desc_t *section);

/*! Getter for  region_id

  The function return the region_id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] param manifest section desc

 @return region_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_param_manifest_section_desc_get_region_id(
	const ia_css_param_manifest_section_desc_t *section);

/*! Getter for mem_type_id

  The function return the mem_type_id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] param manifest section desc

 @return mem_type_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_param_manifest_section_desc_get_mem_type_id(
	const ia_css_param_manifest_section_desc_t *section);

/*! Getter for kernel id

  The function return the kernel id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] fragment manifest section desc

 @return kernel_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_kernel_id(
	const ia_css_fragment_param_manifest_section_desc_t *section);

/*! Getter for  region_id

  The function return the region_id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] fragment manifest section desc

 @return region_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_region_id(
	const ia_css_fragment_param_manifest_section_desc_t *section);

/*! Getter for mem_type_id

  The function return the mem_type_id
  field from the section
  This field shouldn't accessed directly
  only with the getter function
 @param	[in] fragment manifest section desc

 @return mem_type_id
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_C
uint8_t ia_css_fragment_param_manifest_section_desc_get_mem_type_id(
	const ia_css_fragment_param_manifest_section_desc_t *section);

#ifdef __INLINE_PARAMETERS__
#include "ia_css_terminal_manifest_impl.h"
#endif /* __INLINE_PARAMETERS__ */

#endif /* __IA_CSS_TERMINAL_MANIFEST_H */
