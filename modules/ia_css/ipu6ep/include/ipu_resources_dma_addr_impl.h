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

#ifndef __IPU_RESOURCES_DMA_ADDR_IMPL_H
#define __IPU_RESOURCES_DMA_ADDR_IMPL_H

#include "ipu_resources_dma_addr.h"
#include "ipu_resources_dma.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "nci_dma_descriptors.h"
#include "vied_nci_psys_resource_model.h"
#include "assert_support.h"
#include "storage_class.h"
/* todo let tproxy depends on this module, instead of the other way around */
#include "ia_css_tproxy_dma_utils.h"
#include "vied_nci_dma_dev_access.h" /* nci_dma_reg_addr */

STORAGE_CLASS_INLINE void ipu_resources_dma_get_descriptor_addresses(
	enum nci_dma_device_id dev_id,
	enum nci_dma_bank_mode bank_mode,
	struct nci_dma_descriptor_addresses *base)
{
	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		base->channel_descriptor_offset = nci_dma_reg_addr(dev_id, IPU_DEVICE_DMA_CHANNEL_GROUP_ID, 0, 0);
		base->span_descriptor_offset = nci_dma_reg_addr(dev_id, IPU_DEVICE_DMA_SPAN_GROUP_ID, 0, 0);
		base->terminal_descriptor_offset = nci_dma_reg_addr(dev_id, IPU_DEVICE_DMA_TERMINAL_GROUP_ID, 0, 0);
		base->unit_descriptor_offset = nci_dma_reg_addr(dev_id, IPU_DEVICE_DMA_UNIT_GROUP_ID, 0, 0);
		base->cmem_descriptor_base_address = 0;
	} else {
		/* This one is only for logical (cached) channels. */
		ia_css_tproxy_server_get_descriptor_addresses(dev_id, base);
	}
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_channel_descriptor_address(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_offset)
{
	struct nci_dma_descriptor_addresses base;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	ipu_resources_dma_get_descriptor_addresses(dev_id, bank_mode, &base);

	return nci_dma_get_channel_descriptor_offset(dev_id,
		nci_dma_get_channel_id(dev_id, resource_offset, bank_mode), bank_mode) +
		base.channel_descriptor_offset +
		base.cmem_descriptor_base_address;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_unit_descriptor_address_for_unit_id(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	uint32_t unit_id)
{
	struct nci_dma_descriptor_addresses base;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	ipu_resources_dma_get_descriptor_addresses(dev_id, bank_mode, &base);

	return nci_dma_get_unit_descriptor_offset(dev_id,
		unit_id, bank_mode) +
		base.unit_descriptor_offset +
		base.cmem_descriptor_base_address;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_unit_descriptor_address(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_offset)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	return ipu_resources_dma_get_unit_descriptor_address_for_unit_id(
			dev,
			bank_mode,
			nci_dma_get_unit_id(dev_id, resource_offset, bank_mode));
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_terminal_descriptor_address_for_terminal_id(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	uint32_t terminal_id)
{
	uint32_t addr;
	struct nci_dma_descriptor_addresses base;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	ipu_resources_dma_get_descriptor_addresses(dev_id, bank_mode, &base);
	addr = nci_dma_get_terminal_descriptor_offset(dev_id,
			terminal_id, bank_mode) + base.terminal_descriptor_offset + base.cmem_descriptor_base_address;
	return addr;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_terminal_descriptor_address_private(
	const vied_nci_dev_chn_ID_t    dev,
	enum nci_dma_bank_mode         bank_mode,
	const vied_nci_resource_size_t resource_offset,
	const enum nci_dma_port        port)
{
	uint32_t addr_A, addr_B;
	int terminal_id_A, terminal_id_B;
	struct nci_dma_descriptor_addresses base;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	ipu_resources_dma_get_descriptor_addresses(dev_id, bank_mode, &base);

	terminal_id_A = nci_dma_get_terminal_id(dev_id, resource_offset, bank_mode, NCI_DMA_PORT_A);
	terminal_id_B = nci_dma_get_terminal_id(dev_id, resource_offset, bank_mode, NCI_DMA_PORT_B);
	addr_A = nci_dma_get_terminal_descriptor_offset(dev_id,
		terminal_id_A, bank_mode) + base.terminal_descriptor_offset;
	addr_B = nci_dma_get_terminal_descriptor_offset(dev_id,
		terminal_id_B, bank_mode) + base.terminal_descriptor_offset;
	assert(addr_B - addr_A == (uint32_t)nci_dma_get_terminal_descriptor_size(dev_id, bank_mode));

	switch (port) {
	case NCI_DMA_PORT_A:
		return addr_A + base.cmem_descriptor_base_address;
	case NCI_DMA_PORT_B:
		return addr_B + base.cmem_descriptor_base_address;
	}
	return 0;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_terminal_descriptor_address(
	const vied_nci_dev_chn_ID_t    dev,
	enum nci_dma_bank_mode         bank_mode,
	const vied_nci_resource_size_t resource_offset)
{
	return ipu_resources_dma_get_terminal_descriptor_address_private(dev, bank_mode, resource_offset, NCI_DMA_PORT_A);
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_request_bank_dequeue_ack_section_address(
	const vied_nci_dev_chn_ID_t    dev,
	const uint16_t request_bank_id)
{
#if IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK
	return nci_dma_get_request_bank_dequeue_ack_section_address(ipu_resources_dma_get_nci_dev_id(dev), request_bank_id);
#else
	(void)dev;
	(void)request_bank_id;
	assert(0);
	return 0;
#endif
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_region_origin_address(
	const vied_nci_dev_chn_ID_t             dev,
	const vied_nci_resource_size_t          resource_offset,
	const vied_nci_dma_connect_section_id_t section_id)
{
	switch (section_id) {
	case VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_A:
		return ipu_resources_dma_get_terminal_descriptor_address_private(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset, NCI_DMA_PORT_A);
	case VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_B:
		return ipu_resources_dma_get_terminal_descriptor_address_private(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset, NCI_DMA_PORT_B);
	case VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_A_NON_CACHED:
		return ipu_resources_dma_get_terminal_descriptor_address_private(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset, NCI_DMA_PORT_A);
	case VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_B_NON_CACHED:
		return ipu_resources_dma_get_terminal_descriptor_address_private(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset, NCI_DMA_PORT_B);

	}
	return 0;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_span_descriptor_address_for_span_id(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	uint8_t span_id)
{
	uint32_t addr;
	struct nci_dma_descriptor_addresses base;

	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	ipu_resources_dma_get_descriptor_addresses(dev_id, bank_mode, &base);

	addr = nci_dma_get_span_descriptor_offset(dev_id, span_id, bank_mode) + base.span_descriptor_offset;
	return addr + base.cmem_descriptor_base_address;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_span_descriptor_address(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_offset)
{
	uint32_t addr_A, addr_B;
	int span_id_A, span_id_B;
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	span_id_A = nci_dma_get_span_id(dev_id, resource_offset, bank_mode, NCI_DMA_PORT_A);
	span_id_B = nci_dma_get_span_id(dev_id, resource_offset, bank_mode, NCI_DMA_PORT_B);
	addr_A = ipu_resources_dma_get_span_descriptor_address_for_span_id(dev, bank_mode, span_id_A);
	addr_B = ipu_resources_dma_get_span_descriptor_address_for_span_id(dev, bank_mode, span_id_B);
	assert(addr_B - addr_A == (uint32_t)nci_dma_get_span_descriptor_size(dev_id, bank_mode));
	return addr_A;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_descriptor_address(
	const vied_nci_dev_chn_ID_t	dev,
	const vied_nci_resource_size_t	resource_offset,
	const vied_nci_dma_section_id_t	section_id)
{
	uint32_t addr = 0;

	switch (section_id) {
	case VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED:
		addr = ipu_resources_dma_get_channel_descriptor_address(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED:
		addr = ipu_resources_dma_get_channel_descriptor_address(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_UNIT_CACHED:
		addr = ipu_resources_dma_get_unit_descriptor_address(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED:
		addr = ipu_resources_dma_get_unit_descriptor_address(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_SPAN_CACHED:
		addr = ipu_resources_dma_get_span_descriptor_address(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED:
		addr = ipu_resources_dma_get_span_descriptor_address(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED:
		addr = ipu_resources_dma_get_terminal_descriptor_address(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED:
		addr = ipu_resources_dma_get_terminal_descriptor_address(dev, NCI_DMA_BANK_MODE_CACHED, resource_offset);
		break;

	case VIED_NCI_DMA_SECTION_ID_REQUESTOR_BANK:
		addr = ipu_resources_dma_get_request_bank_dequeue_ack_section_address(dev, resource_offset);
		break;
	}
	return addr;
}

#endif /* __IPU_RESOURCES_DMA_ADDR_IMPL_H */
