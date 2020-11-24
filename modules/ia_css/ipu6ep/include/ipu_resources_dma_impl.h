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

#ifndef __IPU_RESOURCES_DMA_IMPL_H
#define __IPU_RESOURCES_DMA_IMPL_H

#include "ipu_resources_dma.h"
#include "ipu_resources_dma_private.h"
#include "ipu_resources_dep.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "nci_dma_descriptors.h"
#include "nci_dma_descriptors_ids.h"
#include "vied_nci_psys_resource_model.h"
#include "assert_support.h"
#include "storage_class.h"
/* todo let tproxy depends on this module, instead of the other way around */
#include "ia_css_tproxy_dma_utils.h"

/**
 * ASSUMPTION: for each DMA resource in vied_nci_dev_chn_ID_t,
 * we assume that from resource 0 to vied_nci_dev_chn_size[dev]
 * all resources have the same size, identical properties and are
 * contiguous either in a physical DMA register bank
 * or contiguous in a (transfer) DMEM for logical channels.
 */

STORAGE_CLASS_INLINE enum nci_dma_device_id ipu_resources_dma_get_nci_dev_id(
	const vied_nci_dev_chn_ID_t dev)
{
	enum nci_dma_device_id nci_dma_device_id;

	assert(dev < VIED_NCI_N_DEV_CHN_ID);

	nci_dma_device_id = resource_model_dev_chn_2_nci_dma_dev[dev];
	assert(nci_dma_device_id < NCI_DMA_NUM_DEVICES);
	return nci_dma_device_id;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_nci_chan_id(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	vied_nci_resource_size_t resource_offset)
{
	enum nci_dma_device_id nci_dma_dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	assert(resource_offset < vied_nci_dev_chn_size[dev]);

	return nci_dma_get_channel_id(nci_dma_dev_id, resource_offset, bank_mode);
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_channel_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	int size = nci_dma_get_channel_descriptor_size(dev_id, bank_mode) * resource_size;

	assert(resource_size <= vied_nci_dev_chn_size[dev]);

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_unit_descr_size_per_unit_descr(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	return nci_dma_get_unit_descriptor_size(dev_id, bank_mode);

}
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_unit_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size)
{
	int size = ipu_resources_dma_get_unit_descr_size_per_unit_descr(dev, bank_mode) * resource_size;

	assert(resource_size <= vied_nci_dev_chn_size[dev]);

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_term_descr_size_per_term_descr(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	return nci_dma_get_terminal_descriptor_size(dev_id, bank_mode);
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_terminal_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size)
{
	int size = ipu_resources_dma_get_term_descr_size_per_term_descr(dev, bank_mode) * resource_size * 2;

	assert(resource_size <= vied_nci_dev_chn_size[dev]);

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_request_bank_dequeue_ack_section_size(void)
{
#if IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK
	return nci_dma_get_request_bank_dequeue_ack_section_size();
#else
	assert(0);
	return 0;
#endif
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_span_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);

	int size = nci_dma_get_span_descriptor_size(dev_id, bank_mode) * resource_size * 2;

	assert(resource_size <= vied_nci_dev_chn_size[dev]);

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_descriptor_size(
	const vied_nci_dev_chn_ID_t	dev,
	const vied_nci_resource_size_t	resource_size,
	const vied_nci_dma_section_id_t	section_id)
{
	uint32_t size = 0;

	switch (section_id) {
	case VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED:
		size = ipu_resources_dma_get_channel_descriptor_size(dev, NCI_DMA_BANK_MODE_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED:
		size = ipu_resources_dma_get_channel_descriptor_size(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_UNIT_CACHED:
		size = ipu_resources_dma_get_unit_descriptor_size(dev, NCI_DMA_BANK_MODE_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED:
		size = ipu_resources_dma_get_unit_descriptor_size(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_SPAN_CACHED:
		size = ipu_resources_dma_get_span_descriptor_size(dev, NCI_DMA_BANK_MODE_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED:
		size = ipu_resources_dma_get_span_descriptor_size(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED:
		size = ipu_resources_dma_get_terminal_descriptor_size(dev, NCI_DMA_BANK_MODE_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED:
		size = ipu_resources_dma_get_terminal_descriptor_size(dev, NCI_DMA_BANK_MODE_NON_CACHED, resource_size);
		break;

	case VIED_NCI_DMA_SECTION_ID_REQUESTOR_BANK:
		size = ipu_resources_dma_get_request_bank_dequeue_ack_section_size();
		break;
	}

	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_payload_size(
	const vied_nci_dev_chn_ID_t		dev,
	const vied_nci_resource_size_t		resource_size)
{
	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	enum nci_dma_bank_mode bank_mode = ia_css_tproxy_server_get_bank_mode(dev_id);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		return ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED);
	} else {
		return ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_SPAN_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_UNIT_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED)
			+ ipu_resources_dma_get_descriptor_size(dev, resource_size, VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED);
	}
}

STORAGE_CLASS_INLINE bool ipu_resources_dma_section_is_managed_resource(
		const vied_nci_dev_chn_ID_t dev,
		const vied_nci_dma_section_id_t section_id)
{
	/*
	 * The resoucre manager only manages channel, span, terminal and unit
	 * descritptor banks when use in  the default bank mode of the device
	 * (as set by tproxy). If a section id refers a bank mode that is
	 * different from the defualt bank mode of dma dev, then the
	 * dma descriptor bank is not managed by SW.
	 *
	 */
	bool is_managed = false;

	enum nci_dma_device_id dev_id = ipu_resources_dma_get_nci_dev_id(dev);
	enum nci_dma_bank_mode bank_mode = ia_css_tproxy_server_get_bank_mode(dev_id);
	bool is_cached = (bank_mode == NCI_DMA_BANK_MODE_CACHED);

	switch (section_id) {
	case VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED:
	case VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED:
	case VIED_NCI_DMA_SECTION_ID_SPAN_CACHED:
	case VIED_NCI_DMA_SECTION_ID_UNIT_CACHED:
		is_managed = is_cached;
		break;
	case VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED:
	case VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED:
	case VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED:
	case VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED:
		is_managed = !(is_cached);
		break;
	default:
		/* The requestor bank is not a manqaged resource */
		break;
	}

	return is_managed;
}

#endif /* __IPU_RESOURCES_DMA_IMPL_H */
