/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __NCI_DMA_DESCRIPTORS_IDS_IMPL_V4_H
#define __NCI_DMA_DESCRIPTORS_IDS_IMPL_V4_H

#include "storage_class.h"
#include "misc_support.h"
#include "assert_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "ipu_device_dma_properties.h"

STORAGE_CLASS_INLINE int nci_dma_get_channel_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	int chan_id = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	if (ipu_device_dma_channels(dev_id) > channel_id) {
		chan_id = channel_id;
	}
	return chan_id;
}

STORAGE_CLASS_INLINE int nci_dma_get_span_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode,
	const enum nci_dma_port port)
{
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_PORT > (unsigned int)port);

	return port + (ipu_device_dma_span_banks(dev_id) -
			(N_NCI_DMA_PORT * (ipu_device_dma_channel_banks(dev_id)))) +
			(channel_id * N_NCI_DMA_PORT);
}

STORAGE_CLASS_INLINE int nci_dma_get_terminal_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode,
	const enum nci_dma_port port)
{
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_PORT > (unsigned int)port);

	return port + (channel_id * N_NCI_DMA_PORT);
}

STORAGE_CLASS_INLINE int nci_dma_get_unit_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);

	return ((ipu_device_dma_unit_banks(dev_id) - ipu_device_dma_channel_banks(dev_id) + channel_id));
}

#endif /* __NCI_DMA_DESCRIPTORS_IDS_IMPL_V4_H */
