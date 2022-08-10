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

#ifndef __IA_CSS_TERMINAL_H
#define __IA_CSS_TERMINAL_H

#include "type_support.h"
#include "ia_css_terminal_types.h"
#include "ia_css_param_storage_class.h"

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_param_in_terminal_get_descriptor_size(
	const unsigned int nof_sections
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_param_section_desc_t *
ia_css_param_in_terminal_get_param_section_desc(
	const ia_css_param_terminal_t *param_terminal,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_param_out_terminal_get_descriptor_size(
	const unsigned int nof_sections,
	const unsigned int nof_fragments
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_param_section_desc_t *
ia_css_param_out_terminal_get_param_section_desc(
	const ia_css_param_terminal_t *param_terminal,
	const unsigned int section_index,
	const unsigned int nof_sections,
	const unsigned int fragment_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_param_terminal_create(
	ia_css_param_terminal_t *param_terminal,
	const uint16_t terminal_offset,
	const uint16_t terminal_size,
	const uint16_t is_input_terminal
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_spatial_param_terminal_get_descriptor_size(
	const unsigned int nof_frame_param_sections,
	const unsigned int nof_fragments
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_fragment_grid_desc_t *
ia_css_spatial_param_terminal_get_fragment_grid_desc(
	const ia_css_spatial_param_terminal_t *spatial_param_terminal,
	const unsigned int fragment_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_frame_grid_param_section_desc_t *
ia_css_spatial_param_terminal_get_frame_grid_param_section_desc(
	const ia_css_spatial_param_terminal_t *spatial_param_terminal,
	const unsigned int section_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_spatial_param_terminal_create(
	ia_css_spatial_param_terminal_t *spatial_param_terminal,
	const uint16_t terminal_offset,
	const uint16_t terminal_size,
	const uint16_t is_input_terminal,
	const unsigned int nof_fragments,
	const uint32_t kernel_id
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_sliced_param_terminal_get_descriptor_size(
	const unsigned int nof_slice_param_sections,
	const unsigned int nof_slices[],
	const unsigned int nof_fragments
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_fragment_slice_desc_t *
ia_css_sliced_param_terminal_get_fragment_slice_desc(
	const ia_css_sliced_param_terminal_t *sliced_param_terminal,
	const unsigned int fragment_index
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_slice_param_section_desc_t *
ia_css_sliced_param_terminal_get_slice_param_section_desc(
	const ia_css_sliced_param_terminal_t *sliced_param_terminal,
	const unsigned int fragment_index,
	const unsigned int slice_index,
	const unsigned int section_index,
	const unsigned int nof_slice_param_sections
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_sliced_param_terminal_create(
	ia_css_sliced_param_terminal_t *sliced_param_terminal,
	const uint16_t terminal_offset,
	const uint16_t terminal_size,
	const uint16_t is_input_terminal,
	const unsigned int nof_slice_param_sections,
	const unsigned int nof_slices[],
	const unsigned int nof_fragments,
	const uint32_t kernel_id
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_program_terminal_get_descriptor_size(
	const unsigned int nof_fragments,
	const unsigned int nof_fragment_param_sections,
	const unsigned int nof_kernel_fragment_sequencer_infos,
	const unsigned int nof_command_objs
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_fragment_param_section_desc_t *
ia_css_program_terminal_get_frgmnt_prm_sct_desc(
	const ia_css_program_terminal_t *program_terminal,
	const unsigned int fragment_index,
	const unsigned int section_index,
	const unsigned int nof_fragment_param_sections
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
ia_css_kernel_fragment_sequencer_info_desc_t *
ia_css_program_terminal_get_kernel_frgmnt_seq_info_desc(
	const ia_css_program_terminal_t *program_terminal,
	const unsigned int fragment_index,
	const unsigned int info_index,
	const unsigned int nof_kernel_fragment_sequencer_infos
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_program_terminal_create(
	ia_css_program_terminal_t *program_terminal,
	const uint16_t terminal_offset,
	const uint16_t terminal_size,
	const unsigned int nof_fragments,
	const unsigned int nof_kernel_fragment_sequencer_infos,
	const unsigned int nof_command_objs
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
int ia_css_program_terminal_get_command_base_offset(
	const ia_css_program_terminal_t *program_terminal,
	const unsigned int nof_fragments,
	const unsigned int nof_kernel_fragment_sequencer_infos,
	const unsigned int commands_slots_used,
	uint16_t *command_desc_offset
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
uint16_t *ia_css_program_terminal_get_line_count(
	const ia_css_kernel_fragment_sequencer_command_desc_t
	*kernel_fragment_sequencer_command_desc_base,
	const unsigned int set_count
);

IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_spatial_param_terminal_get_descriptor_size(
	const unsigned int nof_frame_param_sections,
	const unsigned int nof_fragments
);

#ifdef __INLINE_PARAMETERS__
#include "ia_css_terminal_impl.h"
#endif /* __INLINE_PARAMETERS__ */

#endif /* __IA_CSS_TERMINAL_H */
