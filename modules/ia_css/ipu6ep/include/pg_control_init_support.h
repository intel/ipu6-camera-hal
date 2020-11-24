/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
 * All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or licensors. Title to the Material remains with Intel
 * Corporation or its licensors. The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * licensors. The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No License under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#ifndef __PG_CONTROL_INIT_SUPPORT_H
#define __PG_CONTROL_INIT_SUPPORT_H

#include "ia_css_common_io_types.h"
#include "ia_css_psys_process_group.h"

/* for convert_seq_info */
#include "ia_css_kernel_user_param_instance_types.h"
#include "ipu_sequencer_info_descriptor.h"

#include "system_global.h"

/* helper class that is used in filling the program control init payload */
typedef struct load_section_desc_list_s {
	ia_css_program_control_init_load_section_desc_t *load_sec_desc;
	uint16_t mem_offset;
	uint16_t load_sec_count;
} load_section_desc_list_t;

/*!
 * Returns the frame format type of data in|out terminal.
 * @param [in]  process_group  Process group
 * @param [in]  terminal_id    Terminal ID
 * @return frame format type, IA_CSS_N_FRAME_FORMAT_TYPES on error.
 */
ia_css_frame_format_type_t pg_control_init_get_terminal_frame_format_type(
	const ia_css_process_group_t *process_group,
	int terminal_id);

/*!
 * Returns true if a terminal exists with with the given terminal ID.
 * @param [in]  process_group  Process group
 * @param [in]  terminal_id    Terminal ID
 * @return true if terminal exists, otherwise false
 */
bool pg_control_init_is_terminal_existent(
	const ia_css_process_group_t *process_group,
	int terminal_id);

/*!
 * Fills in ia_css_terminal_descriptor_s given a process group and terminal ID.
 * The terminal address being filled in will always be 0xDEADBEEF.
 * @param [out] terminal       Output
 * @param [in]  process_group  Process group
 * @param [in]  terminal_id    Terminal ID
 * @return 0 on success
 */
int pg_control_init_get_terminal_descriptor(
	struct ia_css_terminal_descriptor_s *terminal,
	const ia_css_process_group_t *process_group,
	int terminal_id,
	int frag_idx);

void pg_control_init_load_section_desc_init(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	uint16_t payload_offset);

/* get the accumulated size for all the load_section payloads for a given program */
unsigned int pg_control_init_get_program_load_section_payload_size(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc);

/* get the accumulated size for all the filled load_section payloads for a given program */
unsigned int pg_control_init_get_program_filled_load_section_payload_size(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc);

void pg_control_init_add_load_section_desc_ext(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	uint16_t size,
	uint8_t device_id,
	uint8_t instance_id,
	uint8_t channel_id,
	uint8_t section_id,
	uint16_t mode_bitmask);

void pg_control_init_add_load_section_desc(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	uint16_t size,
	uint8_t device_id,
	uint8_t instance_id,
	uint8_t channel_id,
	uint8_t section_id);

load_section_desc_list_t
pg_control_init_get_load_desc_list(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc);

load_section_desc_list_t
pg_control_init_index_load_desc_list(
	load_section_desc_list_t load_sec_list,
	unsigned index);

ia_css_program_control_init_load_section_desc_t *
pg_control_init_get_load_desc_from_list(
	load_section_desc_list_t load_sec_list);

unsigned
pg_control_init_get_mem_offset_at_index(
	load_section_desc_list_t load_sec_list,
	unsigned index);

unsigned
pg_control_init_get_mem_size_at_index(
	load_section_desc_list_t load_sec_list,
	unsigned index);

/* remove n elements from the head of the list and return them as a list */
load_section_desc_list_t
pg_control_init_load_desc_pop_list(
	load_section_desc_list_t *load_sec_list,
	unsigned n);

int pg_control_init_get_spatial_terminal_descriptor(
	struct ipu_spatial_terminal_descriptor_s *spatial_port,
	const ia_css_process_group_t *process_group,
	int terminal_id,
	unsigned int section_count,
	int frag_idx);

/*
 * Converts from the new kup-based sequencer info to the old (deprecated for vpless) sequencer info type
 * This is a temporary solution to avoid touching a lot of interfaces like dma/dfm open functions
 * where the old interface type is expected as the function argument.
 */
void convert_seq_info(const struct ia_css_kup_frag_seq_info *kup_cfg, struct ipu_kernel_fragment_sequencer_info_desc_s *kernel_frag_seq_info, unsigned int kernel_id);

/*
 * Function to map Channel ID of type vied_nci_dev_chn_ID_t to resource ID of type resource_id_t
 */
resource_id_t get_resource_id_from_dev_chn_id(vied_nci_dev_chn_ID_t chn_id);

/*
 * @brief       calculate buffer width
 *
 * @param[in]   terminal_width      terminal width
 * @param[in]   block_width         max block width

 * return optimized input block width
 */

unsigned int
calculate_isl_vmem_block_width(uint16_t terminal_width, unsigned int block_width);

int pg_control_init_get_frame_resolution(
		struct ia_css_resolution_descriptor_s *frame_res,
		const ia_css_process_group_t *process_group,
		int terminal_id);

int pg_control_init_write_file(const char *file_path, unsigned int bytes_to_write, void *data);

#endif /* __PG_CONTROL_INIT_SUPPORT_H */
