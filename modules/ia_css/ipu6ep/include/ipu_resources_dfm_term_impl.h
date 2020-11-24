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

#ifndef __IPU_RESOURCES_DFM_TERM_IMPL_H
#define __IPU_RESOURCES_DFM_TERM_IMPL_H

#include "ipu_resources_dfm.h"
#include "dev_api_dfm.h"
#include "vied_nci_psys_resource_model_private.h"
#include "ipu_resources_section_count.h"
#include "pg_control_init_support.h"

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_set_progctrlinit_section(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_dfm_id_t  dev_id,
	const uint32_t               port_number,
	const vied_nci_dfm_section_id_t section_id,
	const uint32_t               mode_bitmask)
{
	pg_control_init_add_load_section_desc_ext(
		prog_ctrl_init_prog_desc,
		ipu_resources_dfm_get_section_payload_size(dev_id, port_number, section_id),
		VIED_NCI_DEVICE_ID_DFM,
		dev_id,
		port_number,
		section_id,
		mode_bitmask);

	return ipu_resources_dfm_get_section_payload_size(dev_id, port_number, section_id);
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_set_progctrlinit_sections(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_dfm_id_t  dev_id,
	const uint32_t               port_number,
	const uint32_t               mode_bitmask)
{
	uint32_t size = 0;

	/***  Fill descriptor of cmdbank section ***/
	size += ipu_resources_dfm_set_progctrlinit_section(
		prog_ctrl_init_prog_desc,
		dev_id, port_number, VIED_NCI_DFM_SECTION_ID_CMDBANK,
		mode_bitmask);

	/***  Fill descriptor of port section ***/
	size += ipu_resources_dfm_set_progctrlinit_section(
		prog_ctrl_init_prog_desc,
		dev_id, port_number, VIED_NCI_DFM_SECTION_ID_PORT,
		mode_bitmask);

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_set_progctrlinit_sections_resource(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
		dfm_port_resource_t dfm_resource,
		unsigned int mode_bitmask)
{
	unsigned int i;
	uint32_t size = 0;

	for (i = 0; i < dfm_resource.value; i++, dfm_resource.offset++) {
		size += ipu_resources_dfm_set_progctrlinit_sections(
			prog_ctrl_init_prog_desc,
			dfm_resource.id,
			dfm_resource.offset,
			mode_bitmask);
	}

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_sections_size(
		dfm_port_resource_t dfm_resource)
{
	unsigned int i;
	uint32_t size = 0;

	for (i = 0; i < dfm_resource.value; i++, dfm_resource.offset++) {
		size += ipu_resources_dfm_get_section_payload_size(dfm_resource.id, dfm_resource.offset, VIED_NCI_DFM_SECTION_ID_CMDBANK);
		size += ipu_resources_dfm_get_section_payload_size(dfm_resource.id, dfm_resource.offset, VIED_NCI_DFM_SECTION_ID_PORT);
	}

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_fill_progctrlinit_payload(
	const vied_nci_dev_dfm_id_t     dev,
	const uint32_t                  port_number,
	struct ipu_resources_dfm_descriptor_config *public_config,
	load_section_desc_list_t		load_sec_desc_list,
	void                            *buffer)
{
	uint32_t size = 0, size_in_desc = 0;
	uint32_t nci_port_number = ipu_resources_dfm_get_nci_port_number(dev, port_number);
	unsigned payload_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, VIED_NCI_DFM_SECTION_ID_CMDBANK);
	unsigned payload_size0 = pg_control_init_get_mem_size_at_index(load_sec_desc_list, VIED_NCI_DFM_SECTION_ID_CMDBANK);
	unsigned payload_size1 = pg_control_init_get_mem_size_at_index(load_sec_desc_list, VIED_NCI_DFM_SECTION_ID_PORT);

	assert(public_config->port_config.port_num == nci_port_number);
	assert(buffer);

	public_config->port_config.config_buffer = (uint8_t *)buffer + payload_mem_offset;
	size = dev_api_dfm_config_port(&public_config->stream_config, &public_config->port_config);
	size_in_desc = payload_size0 + payload_size1;

	assert(size == size_in_desc);

	return size;
}

STORAGE_CLASS_INLINE void ipu_resources_dfm_print_progctrlinit_payload(
	const vied_nci_dev_dfm_id_t     dev,
	const uint32_t                  port_number,
	load_section_desc_list_t		load_sec_desc_list,
	const void                      *buffer)
{
	void *addr = NULL;
	unsigned payload_mem_offset0 = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, VIED_NCI_DFM_SECTION_ID_CMDBANK);
	unsigned payload_mem_offset1 = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, VIED_NCI_DFM_SECTION_ID_PORT);

	enum ipu_device_dfm_id dai_dev_id = ipu_resources_dfm_get_nci_dev_id(dev);
	uint32_t nci_port_number = ipu_resources_dfm_get_nci_port_number(dev, port_number);
	addr = (uint8_t *)buffer + payload_mem_offset0;
	ipu_nci_dfm_port_print_section0(dai_dev_id, nci_port_number, addr);
	addr = (uint8_t *)buffer + payload_mem_offset1;
	ipu_nci_dfm_port_print_section1(addr);

}

#endif /* __IPU_RESOURCES_DFM_TERM_IMPL_H */
