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

#ifndef __IPU_RESOURCES_DMA_TERM_H
#define __IPU_RESOURCES_DMA_TERM_H

#include "nci_dma_descriptors.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "storage_class.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"
#include "ia_css_psys_terminal_private_types.h"
#include "pg_control_init_support.h"

/*!
 * (public) DMA descriptor configuration for a single channel.
 */
struct ipu_resources_dma_descriptor_config {
	struct nci_dma_public_channel_descriptor	channel_desc;
	struct nci_dma_public_span_descriptor		span_desc[N_NCI_DMA_PORT];
	struct nci_dma_public_terminal_descriptor	terminal_desc[N_NCI_DMA_PORT];
	struct nci_dma_public_unit_descriptor		unit_desc;
};

/*!
 * Fills in program control init load section descriptor for a single DMA descrtiptor
 * (it sets mem_offset, mem_size and device_descriptor_id).
 *
 * @param desc            Pointer to program descriptor
 * @param dev             Device ID from the resource model (instance id).
 * @param resource_offset DMA resource offset (channel id).
 * @param resource_size   Number of assigned resources.
 *
 * @return                The total size of the payloads.
 */

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_set_progctrlinit_section(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t         dev,
	const vied_nci_resource_size_t      resource_offset,
	const vied_nci_resource_size_t      resource_size,
	const vied_nci_dma_section_id_t     section_id);

/*!
 * Fills in the program control init load section descriptor for DMA
 * resources (it sets mem_offset, mem_size and device_descriptor_id).
 * @param prog_ctrl_init_prog_desc           pointer to program descriptor
 * @param dev            Device channel ID from the resource model.
 * @param resource_offset Assigned resource value according to the resource model.
 * @param resource_size  Number of assigned resources.
 * @return               The total size of the payloads.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_set_progctrlinit_sections(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t		dev,
	const vied_nci_resource_size_t		resource_offset,
	const vied_nci_resource_size_t		resource_size);

/*!
 * Fills in the program control init terminal payload for a single DMA channel.
 * @param dev				Device channel ID from the resource model.
 * @param resource_offset		Assigned resource value according to the resource model.
 * @param resource_size			Number of assigned resources.
 * @param public_dma_descriptor_config	DMA NCI public configuration array of size resource_size.
 *					This function will override the bank modes in
 *					public_dma_descriptor_config.
 * @param load_section_desc		Program control init terminal oad section descriptor.
 *					(of size ipu_resources_dma_get_descriptor_size).
 * @param buffer			Program control init terminal (payload) pointer.
 *					(the address a payload is written to, is
 *					bufer + load_section_desc[i].mem_offset).
 *
 * @return bytes written (<=0 on error)
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_fill_progctrlinit_payload(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	struct ipu_resources_dma_descriptor_config			*public_dma_descriptor_config,
	load_section_desc_list_t			load_sec_desc_list,
	void								*buffer);

/*!
 * Print the program control init terminal payload for a block of DMA resources.
 * @param dev				Device channel ID from the resource model.
 * @param resource_offset		Assigned resource value according to the resource model.
 * @param resource_size			Number of assigned resources.
 * @param load_sec_desc_list		Program control init terminal oad section descriptor list.
 *					(of size ipu_resources_dma_get_descriptor_size).
 * @param buffer			Program control init terminal (payload) pointer.
 *					(the address a payload is written to, is
 *					bufer + load_section_desc[i].mem_offset).
 *
 * @return < 0 on error
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_print_progctrlinit_payload(
	const vied_nci_dev_chn_ID_t					dev,
	const vied_nci_resource_size_t					resource_offset,
	const vied_nci_resource_size_t					resource_size,
	load_section_desc_list_t			load_sec_desc_list,
	const void							*buffer);

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_fill_request_bank_progctrlinit_payload(
		const vied_nci_dev_chn_ID_t					dev,
		enum nci_dma_requestor_id 					request_bank_id,
		void								*public_request_reg_bank,
		load_section_desc_list_t			load_sec_desc_list,
		void								*buffer);

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_get_request_bank_progctrlinit_section_count(void);

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_get_request_bank_payload_size(void);

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dma_set_request_bank_progctrlinit_section_desc(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_requestor_id   request_bank_id);

#include "ipu_resources_dma_term_impl.h"

#endif /* __IPU_RESOURCES_DMA_TERM_H */
