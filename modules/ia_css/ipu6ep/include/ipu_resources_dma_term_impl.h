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

#ifndef __IPU_RESOURCES_DMA_TERM_IMPL_H
#define __IPU_RESOURCES_DMA_TERM_IMPL_H

#include "ipu_resources_dma.h"
#include "ipu_resources_dma_private.h"
#include "ipu_resources_section_count.h"
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_type_properties.h"
#include "nci_dma_descriptors.h"
#include "nci_dma_descriptors_print.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"
#include "assert_support.h"
#include "storage_class.h"
#include "error_support.h"
#include "ia_css_psys_terminal_private_types.h"
/* todo let tproxy depends on this module, instead of the other way around */
#include "ia_css_tproxy_dma_utils.h"
#include "pg_control_init_support.h"

enum {
	IPU_RESOURCES_CHANNEL_SECTION_INDEX  = 0,
	IPU_RESOURCES_TERMINAL_SECTION_INDEX = 1,
	IPU_RESOURCES_SPAN_SECTION_INDEX     = 2,
	IPU_RESOURCES_UNIT_SECTION_INDEX     = 3
};

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_set_progctrlinit_section(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t		dev,
	const vied_nci_resource_size_t		resource_offset,
	const vied_nci_resource_size_t		resource_size,
	const vied_nci_dma_section_id_t	section_id)
{
	pg_control_init_add_load_section_desc(
		prog_ctrl_init_prog_desc,
		ipu_resources_dma_get_descriptor_size(dev, resource_size, section_id),
		VIED_NCI_DEVICE_ID_DMA,
		dev,
		resource_offset, /* should not be needed */
		section_id);

	return ipu_resources_dma_get_descriptor_size(dev, resource_size, section_id);
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_set_progctrlinit_sections(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t		dev,
	const vied_nci_resource_size_t		resource_offset,
	const vied_nci_resource_size_t		resource_size)
{
	uint32_t payload_size = 0;
	vied_nci_dma_section_id_t section_id;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	enum nci_dma_bank_mode bank_mode = ia_css_tproxy_server_get_bank_mode(dev_id);

	assert(IPU_RESOURCES_CHANNEL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_SPAN_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_TERMINAL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_UNIT_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);

	section_id = (bank_mode == NCI_DMA_BANK_MODE_CACHED) ?
			VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED : VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED;
	payload_size += ipu_resources_dma_set_progctrlinit_section(prog_ctrl_init_prog_desc,
		dev, resource_offset, resource_size, section_id);

	section_id = (bank_mode == NCI_DMA_BANK_MODE_CACHED) ?
			VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED : VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED;
	payload_size += ipu_resources_dma_set_progctrlinit_section(prog_ctrl_init_prog_desc,
		dev, resource_offset, resource_size, section_id);

	section_id = (bank_mode == NCI_DMA_BANK_MODE_CACHED) ?
			VIED_NCI_DMA_SECTION_ID_SPAN_CACHED : VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED;
	payload_size += ipu_resources_dma_set_progctrlinit_section(prog_ctrl_init_prog_desc,
		dev, resource_offset, resource_size, section_id);

	section_id = (bank_mode == NCI_DMA_BANK_MODE_CACHED) ?
			VIED_NCI_DMA_SECTION_ID_UNIT_CACHED : VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED;
	payload_size += ipu_resources_dma_set_progctrlinit_section(prog_ctrl_init_prog_desc,
		dev, resource_offset, resource_size, section_id);

	assert(ipu_resources_dma_get_payload_size(dev, resource_size) == payload_size);
	return payload_size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_print_progctrlinit_payload_single_channel(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	const vied_nci_resource_size_t					resource_offset_offset,
	load_section_desc_list_t			load_sec_desc_list,
	const void							*buffer)
{
	/*
	 * A user requests a contiguous block of resources with size resource_size.
	 * Resource management grants a specified block of resources with resource_size starting at
	 * resource_offset (the base). The granting can be fixed (compile time), or dynamic (runtime).
	 * resource_offset_offset is the resource value relative to the assigned block of resources.
	 *
	 * Per descriptor type (unit/channel/terminal/span), all payload bytes for the block of resources are
	 * contiguous in memory.
	 */
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	enum nci_dma_bank_mode bank_mode = ia_css_tproxy_server_get_bank_mode(dev_id);

	uint32_t channel_id    = ipu_resources_dma_get_nci_chan_id(dev, bank_mode, resource_offset + resource_offset_offset);
	uint32_t terminal_id_A = nci_dma_get_terminal_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_A);
	uint32_t terminal_id_B = nci_dma_get_terminal_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_B);
	uint32_t span_id_A     = nci_dma_get_span_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_A);
	uint32_t span_id_B     = nci_dma_get_span_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_B);
	uint32_t unit_id       = nci_dma_get_unit_id(dev_id, channel_id, bank_mode);
	uint8_t *addr;
	uint32_t offset_in_section;
	unsigned section_mem_offset;
	(void)resource_size;

	assert(IPU_RESOURCES_CHANNEL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_SPAN_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_TERMINAL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_UNIT_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);

	/************ CHANNEL ************/
	offset_in_section  = ipu_resources_dma_get_channel_descriptor_size(dev, bank_mode, resource_offset_offset);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_CHANNEL_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_channel_descriptor(dev_id, channel_id, bank_mode, addr);

	/************ TERMINAL ************/
	offset_in_section  = ipu_resources_dma_get_terminal_descriptor_size(dev, bank_mode, resource_offset_offset);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_TERMINAL_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_terminal_descriptor(dev_id, terminal_id_A, bank_mode, addr);

	offset_in_section += nci_dma_get_terminal_descriptor_size(dev_id, bank_mode);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_TERMINAL_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_terminal_descriptor(dev_id, terminal_id_B, bank_mode, addr);

	/************ SPAN ************/
	offset_in_section  = ipu_resources_dma_get_span_descriptor_size(dev, bank_mode, resource_offset_offset);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_SPAN_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_span_descriptor(dev_id, span_id_A, bank_mode, addr);

	offset_in_section += nci_dma_get_span_descriptor_size(dev_id, bank_mode);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_SPAN_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_span_descriptor(dev_id, span_id_B, bank_mode, addr);

	/************ UNIT ************/
	offset_in_section  = ipu_resources_dma_get_unit_descriptor_size(dev, bank_mode, resource_offset_offset);
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_UNIT_SECTION_INDEX);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	nci_dma_descriptors_print_private_unit_descriptor(dev_id, unit_id, bank_mode, addr);

	return 0;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_print_progctrlinit_payload(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	load_section_desc_list_t			load_sec_desc_list,
	const void							*buffer)
{
	int retval = -1;

	unsigned int resource_offset_offset;

	for (resource_offset_offset = 0; resource_offset_offset < resource_size; resource_offset_offset++) {
		verifjmpexit(ipu_resources_dma_print_progctrlinit_payload_single_channel(
			dev, resource_offset, resource_size, resource_offset_offset, load_sec_desc_list, buffer) == 0);
	}

	retval = 0;
EXIT:
	return retval;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_fill_progctrlinit_payload_single_channel(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	const vied_nci_resource_size_t					resource_offset_offset,
	struct ipu_resources_dma_descriptor_config			*public_dma_descriptor_config,
	load_section_desc_list_t			load_sec_desc_list,
	void								*buffer)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	enum nci_dma_bank_mode bank_mode = ia_css_tproxy_server_get_bank_mode(dev_id);

	uint32_t channel_id    = ipu_resources_dma_get_nci_chan_id(dev, bank_mode, resource_offset + resource_offset_offset);
	uint32_t terminal_id_A = nci_dma_get_terminal_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_A);
	uint32_t terminal_id_B = nci_dma_get_terminal_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_B);
	uint32_t span_id_A     = nci_dma_get_span_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_A);
	uint32_t span_id_B     = nci_dma_get_span_id(dev_id, channel_id, bank_mode, NCI_DMA_PORT_B);
	uint32_t unit_id       = nci_dma_get_unit_id(dev_id, channel_id, bank_mode);
	uint8_t *addr;
	uint32_t offset_in_section;
	uint32_t bytes_written;
	uint32_t total_bytes_written = 0;

	struct ipu_resources_dma_descriptor_config *cfg = public_dma_descriptor_config;
	unsigned section_mem_offset;
	unsigned section_size;

	/* the bank mode is only dependent on vied_nci_dev_chn_ID_t dev */
	cfg->channel_desc.bank_mode = bank_mode;
	cfg->terminal_desc[0].bank_mode = bank_mode;
	cfg->terminal_desc[1].bank_mode = bank_mode;
	cfg->span_desc[0].bank_mode = bank_mode;
	cfg->span_desc[1].bank_mode = bank_mode;
	cfg->unit_desc.bank_mode = bank_mode;

	/* check inputs */
	assert(resource_offset_offset < resource_size);

	assert(IPU_RESOURCES_CHANNEL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_SPAN_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_TERMINAL_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);
	assert(IPU_RESOURCES_UNIT_SECTION_INDEX < DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS);

	/************ CHANNEL ************/
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_CHANNEL_SECTION_INDEX);
	section_size = pg_control_init_get_mem_size_at_index(load_sec_desc_list, IPU_RESOURCES_CHANNEL_SECTION_INDEX);
	assert(section_size == ipu_resources_dma_get_channel_descriptor_size(dev, bank_mode, resource_size));
	offset_in_section     = ipu_resources_dma_get_channel_descriptor_size(dev, bank_mode, resource_offset_offset);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;

	bytes_written = nci_dma_fill_channel_descriptor(dev_id, channel_id, bank_mode, &cfg->channel_desc, addr);
	total_bytes_written += bytes_written;
	assert(section_size == bytes_written * resource_size);

	/************ TERMINAL A+B ************/
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_TERMINAL_SECTION_INDEX);
	section_size = pg_control_init_get_mem_size_at_index(load_sec_desc_list, IPU_RESOURCES_TERMINAL_SECTION_INDEX);
	assert(section_size == ipu_resources_dma_get_terminal_descriptor_size(dev, bank_mode, resource_size));
	offset_in_section     = ipu_resources_dma_get_terminal_descriptor_size(dev, bank_mode, resource_offset_offset);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;

	bytes_written  = nci_dma_fill_terminal_descriptor(dev_id,
		terminal_id_A, bank_mode, &cfg->terminal_desc[NCI_DMA_PORT_A], addr);

	offset_in_section += nci_dma_get_terminal_descriptor_size(dev_id, bank_mode);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	bytes_written += nci_dma_fill_terminal_descriptor(dev_id,
		terminal_id_B, bank_mode, &cfg->terminal_desc[NCI_DMA_PORT_B], addr);
	total_bytes_written += bytes_written;
	assert(section_size == bytes_written * resource_size);

	/************ SPAN A+B ************/
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_SPAN_SECTION_INDEX);
	section_size = pg_control_init_get_mem_size_at_index(load_sec_desc_list, IPU_RESOURCES_SPAN_SECTION_INDEX);
	assert(section_size == ipu_resources_dma_get_span_descriptor_size(dev, bank_mode, resource_size));
	offset_in_section     = ipu_resources_dma_get_span_descriptor_size(dev, bank_mode, resource_offset_offset);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;

	bytes_written  = nci_dma_fill_span_descriptor(dev_id,
		span_id_A, bank_mode, &cfg->span_desc[NCI_DMA_PORT_A], addr);

	offset_in_section += nci_dma_get_span_descriptor_size(dev_id, bank_mode);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;
	bytes_written += nci_dma_fill_span_descriptor(dev_id,
		span_id_B, bank_mode, &cfg->span_desc[NCI_DMA_PORT_B], addr);
	total_bytes_written += bytes_written;
	assert(section_size == bytes_written * resource_size);

	/************ UNIT ************/
	section_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, IPU_RESOURCES_UNIT_SECTION_INDEX);
	section_size = pg_control_init_get_mem_size_at_index(load_sec_desc_list, IPU_RESOURCES_UNIT_SECTION_INDEX);
	assert(section_size == ipu_resources_dma_get_unit_descriptor_size(dev, bank_mode, resource_size));
	offset_in_section     = ipu_resources_dma_get_unit_descriptor_size(dev, bank_mode, resource_offset_offset);
	addr = (uint8_t *)buffer + section_mem_offset + offset_in_section;

	bytes_written = nci_dma_fill_unit_descriptor(dev_id, unit_id, bank_mode, &cfg->unit_desc, addr);
	total_bytes_written += bytes_written;
	assert(section_size == bytes_written * resource_size);

	return total_bytes_written;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_fill_progctrlinit_payload(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	struct ipu_resources_dma_descriptor_config			*public_dma_descriptor_config,
	load_section_desc_list_t			load_sec_desc_list,
	void								*buffer)
{
	uint32_t resource_offset_offset;
	uint32_t total_bytes_written = 0;

	for (resource_offset_offset = 0; resource_offset_offset < resource_size; resource_offset_offset++) {
		total_bytes_written += ipu_resources_dma_fill_progctrlinit_payload_single_channel(
			dev,
			resource_offset,
			resource_size,
			resource_offset_offset,
			&public_dma_descriptor_config[resource_offset_offset],
			load_sec_desc_list,
			buffer);
	}
	return total_bytes_written;
}

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_fill_request_bank_progctrlinit_payload(
		const vied_nci_dev_chn_ID_t					dev,
		enum nci_dma_requestor_id 					request_bank_id,
		void								*public_request_reg_bank,
		load_section_desc_list_t load_sec_desc_list,
		void								*buffer)
{
	uint32_t bytes_written = 0;
#if (IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK)
	unsigned payload_mem_offset = pg_control_init_get_mem_offset_at_index(load_sec_desc_list, 0);
	unsigned payload_size = pg_control_init_get_mem_size_at_index(load_sec_desc_list, 0);

	assert(payload_size == ipu_resources_dma_get_request_bank_payload_size());

	struct nci_dma_public_request_bank_dequeue_ack_config *public_params = (struct nci_dma_public_request_bank_dequeue_ack_config *)public_request_reg_bank;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	buffer = (uint8_t *)buffer + payload_mem_offset;

	bytes_written = nci_dma_fill_request_bank_dequeue_ack(dev_id, request_bank_id, public_params, buffer);
#else
	(void)dev;
	(void)request_bank_id;
	(void)public_request_reg_bank;
	(void)load_sec_desc_list;
	(void)buffer;
#endif
	return bytes_written;
}

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_get_request_bank_progctrlinit_section_count()
{
	uint32_t section_count = 0;
#if (IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK)
	section_count  = 1; /* Only one section, corresponding to de-queue ack fields */
#endif
	return section_count;
}

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_get_request_bank_payload_size()
{
	uint32_t payload_size = 0;
#if (IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK)
	payload_size = nci_dma_get_request_bank_dequeue_ack_section_size();
#endif
	return payload_size;

}

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_set_request_bank_progctrlinit_section_desc(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_requestor_id   request_bank_id)
{
	uint32_t payload_size = 0;
#if (IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK)
	payload_size = nci_dma_get_request_bank_dequeue_ack_section_size();

	pg_control_init_add_load_section_desc(
		prog_ctrl_init_prog_desc,
		payload_size,
		VIED_NCI_DEVICE_ID_DMA,
		dev,
		request_bank_id,
		VIED_NCI_DMA_SECTION_ID_REQUESTOR_BANK);
#else
	(void)prog_ctrl_init_prog_desc;
	(void)dev;
	(void)request_bank_id;
#endif
	return payload_size;
}

#endif /* __IPU_RESOURCES_DMA_TERM_IMPL_H */
