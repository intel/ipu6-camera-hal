/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2018 Intel Corporation.
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

#ifndef __IA_CSS_TPROXY_DMA_UTILS_H
#define __IA_CSS_TPROXY_DMA_UTILS_H

#include "assert_support.h" /* for assert... */
#include "misc_support.h"
#include "ipu_device_dma_devices.h"  /* for nci_dma_device_id, NCI_DMA_NUM_DEVICES */
#include "ia_css_tproxy_global.h" /* for ia_css_tproxy_chan_id_t... */
#include "nci_dma_descriptor_manager.h" /* for nci_dma_descriptor_manager_channel_offset... */
#include "ia_css_tproxy_resources.h" /* for DMA_EXT0_BASE_CHANNEL_OFFSET ... */

/*
 * Get NCI descriptor cache addresses from device ID
 */
STORAGE_CLASS_INLINE void ia_css_tproxy_server_get_descriptor_addresses(
	const enum nci_dma_device_id dev_id,
	struct nci_dma_descriptor_addresses * const descriptor_addresses)
{
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	assert(NULL != descriptor_addresses);

	assert(sizeof(struct nci_dma_descriptor_addresses) == sizeof(struct ia_css_dma_descriptor_addresses));
	nci_dma_descriptor_manager_offsets((struct ia_css_dma_descriptor_addresses *)descriptor_addresses, dev_id);
}

/*
 * Get NCI bank mode from device ID
 */
STORAGE_CLASS_INLINE enum nci_dma_bank_mode ia_css_tproxy_server_get_bank_mode(
	const enum nci_dma_device_id dev_id)
{
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);

	return nci_dma_descriptor_manager_bank_mode(dev_id);
}

/*
 * Get NCI request type from device ID
 */
STORAGE_CLASS_INLINE enum nci_dma_request_type ia_css_tproxy_server_get_request_type(
	const enum nci_dma_device_id dev_id)
{
	NOT_USED(dev_id);
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return NCI_DMA_SHARED_REQUESTOR;
}

/*
 * Get NCI device ID and channel ID from Tproxy channel ID
 */
STORAGE_CLASS_INLINE void ia_css_tproxy_server_get_nci_dev_id_chan_id(
	const ia_css_tproxy_chan_id_t tproxy_chan_id,
	enum nci_dma_device_id * const nci_dma_dev_id,
	uint32_t * const nci_dma_chan_id)
{
	assert(IA_CSS_N_TPROXY_CHAN > tproxy_chan_id);
	assert(NULL != nci_dma_dev_id);
	assert(NULL != nci_dma_chan_id);

	*nci_dma_dev_id = (enum nci_dma_device_id)NCI_DMA_NUM_DEVICES;
	*nci_dma_chan_id = IA_CSS_N_TPROXY_CHAN;

#if DMA_EXT0_BASE_CHANNEL_OFFSET > 0
	/* In case DMA_EXT0_BASE_CHANNEL_OFFSET = 0, the following 2 lines might give compiler error:
	 * comparison of unsigned expression >= 0 is always true.
	 */
	if (tproxy_chan_id >= DMA_EXT0_BASE_CHANNEL_OFFSET &&
		tproxy_chan_id < (DMA_EXT0_BASE_CHANNEL_OFFSET + DMA_EXT0_NUM_LOGICAL_CHANNELS)) {
#else
	if (tproxy_chan_id < (DMA_EXT0_BASE_CHANNEL_OFFSET + DMA_EXT0_NUM_LOGICAL_CHANNELS)) {
#endif
		*nci_dma_dev_id = NCI_DMA_EXT0;
		*nci_dma_chan_id = tproxy_chan_id - DMA_EXT0_BASE_CHANNEL_OFFSET;
	} else

	if (tproxy_chan_id >= DMA_EXT1R_BASE_CHANNEL_OFFSET &&
		tproxy_chan_id < (DMA_EXT1R_BASE_CHANNEL_OFFSET + DMA_EXT1R_NUM_LOGICAL_CHANNELS)) {
		*nci_dma_dev_id = NCI_DMA_EXT1R;
		*nci_dma_chan_id = tproxy_chan_id - DMA_EXT1R_BASE_CHANNEL_OFFSET;
	} else if (tproxy_chan_id >= DMA_EXT1W_BASE_CHANNEL_OFFSET &&
		tproxy_chan_id < (DMA_EXT1W_BASE_CHANNEL_OFFSET + DMA_EXT1W_NUM_LOGICAL_CHANNELS)) {
		*nci_dma_dev_id = NCI_DMA_EXT1W;
		*nci_dma_chan_id = tproxy_chan_id - DMA_EXT1W_BASE_CHANNEL_OFFSET;
#if HAS_DMA_INTERNAL
	} else if (tproxy_chan_id >= DMA_INT_BASE_CHANNEL_OFFSET &&
		tproxy_chan_id < (DMA_INT_BASE_CHANNEL_OFFSET + DMA_INT_NUM_LOGICAL_CHANNELS)) {
		*nci_dma_dev_id = NCI_DMA_INT;
		*nci_dma_chan_id = tproxy_chan_id - DMA_INT_BASE_CHANNEL_OFFSET;
#endif /* HAS_DMA_INTERNAL */
	} else if (tproxy_chan_id >= DMA_ISA_BASE_CHANNEL_OFFSET &&
		tproxy_chan_id < (DMA_ISA_BASE_CHANNEL_OFFSET + DMA_ISA_NUM_LOGICAL_CHANNELS)) {
		*nci_dma_dev_id = NCI_DMA_ISA;
		*nci_dma_chan_id = tproxy_chan_id - DMA_ISA_BASE_CHANNEL_OFFSET;
 	}
	assert(NCI_DMA_NUM_DEVICES != (unsigned int)*nci_dma_dev_id);
	assert(IA_CSS_N_TPROXY_CHAN > (unsigned int)*nci_dma_chan_id);
}

/*
 * Get NCI device ID from channel ID
 */
STORAGE_CLASS_INLINE enum nci_dma_device_id ia_css_tproxy_server_get_nci_dev_id(
	const ia_css_tproxy_chan_id_t tproxy_chan_id)
{
	enum nci_dma_device_id nci_dma_dev_id;
	uint32_t nci_dma_chan_id;

	ia_css_tproxy_server_get_nci_dev_id_chan_id(tproxy_chan_id, &nci_dma_dev_id, &nci_dma_chan_id);
	return nci_dma_dev_id;
}

/*
 * Get NCI channel ID from channel ID
 */
STORAGE_CLASS_INLINE uint32_t ia_css_tproxy_server_get_nci_chan_id(
	const ia_css_tproxy_chan_id_t tproxy_chan_id)
{
	enum nci_dma_device_id nci_dma_dev_id;
	uint32_t nci_dma_chan_id;

	ia_css_tproxy_server_get_nci_dev_id_chan_id(tproxy_chan_id, &nci_dma_dev_id, &nci_dma_chan_id);
	return nci_dma_chan_id;
}

#endif /* __IA_CSS_TPROXY_DMA_UTILS_H */
