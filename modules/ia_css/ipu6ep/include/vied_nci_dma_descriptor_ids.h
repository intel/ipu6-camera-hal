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

#ifndef __VIED_NCI_DMA_IMPL_DEP_H_V4
#define __VIED_NCI_DMA_IMPL_DEP_H_V4

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"

/**
 * This file has functions to convert between per bank mode channel IDs and descriptor IDs
 * (in code both types are typically referred to as channel_id).
 *
 * The per bank mode channel ID correlates with a DMA resource as used
 * in the process group, manifest and IPU resources module.
 *  if NCI_DMA_BANK_MODE_NON_CACHED range: [0, ipu_device_dma_channel_banks >
 *  if NCI_DMA_BANK_MODE_CACHED     range: [0, ipu_device_dma_channels >
 *
 * The channel/terminal/span/unit (=<*>) descriptor IDs are used for DMA instructions (next, invalidate, ...)
 *  if NCI_DMA_BANK_MODE_NON_CACHED range: [0, ipu_device_dma_<*>_banks >
 *  if NCI_DMA_BANK_MODE_CACHED     range: [0, ipu_device_dma_<*>_s >
 *
 * The following functions convert from per bank mode channel ID to a descriptor ID:
 *  vied_nci_dma_get_channel_id  (this file)
 *  nci_dma_get_channel_id       (nci_dma_descriptor module)
 *  nci_dma_get_terminal_id      (nci_dma_descriptor module)
 *  nci_dma_get_span_id          (nci_dma_descriptor module)
 *  nci_dma_get_unit_id          (nci_dma_descriptor module)
 *
 * The following functions convert from a channel descriptor ID to another descriptor ID,
 * (only valid for NCI_DMA_BANK_MODE_CACHED)
 *  vied_nci_dma_get_channel_id
 *  vied_nci_dma_get_terminal_id
 *  vied_nci_dma_get_span_id
 *  vied_nci_dma_get_unit_id
 *
 * NOTE: v3 of this file has different conversion logic.
 */

STORAGE_CLASS_INLINE int vied_nci_dma_get_per_mode_channel_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	(void)dev_id;
	(void)bank_mode;

	return channel_id;
}

STORAGE_CLASS_INLINE int vied_nci_dma_get_channel_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	(void)dev_id;
	(void)bank_mode;

	return channel_id;
}

STORAGE_CLASS_INLINE uint32_t vied_nci_dma_get_span_id(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const uint32_t idx)
{
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	assert(ipu_device_dma_channels(dev_id) > chan_id);

	return (chan_id * NCI_DMA_MAX_NUM_PORTS) + idx;
}

STORAGE_CLASS_INLINE uint32_t vied_nci_dma_get_terminal_id(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const uint32_t idx)
{
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	assert(ipu_device_dma_channels(dev_id) > chan_id);
	assert(NCI_DMA_MAX_NUM_PORTS > idx);

	return (chan_id * NCI_DMA_MAX_NUM_PORTS) + idx;
}

STORAGE_CLASS_INLINE uint32_t vied_nci_dma_get_unit_id(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id)
{
	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	assert(ipu_device_dma_channels(dev_id) > chan_id);

	return chan_id;
}

#endif /* __VIED_NCI_DMA_IMPL_DEP_H_V4 */
