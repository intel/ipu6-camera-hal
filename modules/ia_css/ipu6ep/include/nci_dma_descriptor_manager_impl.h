/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2018 - 2018 Intel Corporation.
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

#ifndef __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_H
#define __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_H

#include "assert_support.h" /* for IA_CSS_ASSERT */
#include "ipu_device_dma_devices.h" /* for NCI_DMA_NUM_DEVICES ... */
#include "ipu_device_dma_type_properties.h" /* for nci_dma_bank_mode ... */
#include "nci_dma_descriptor_manager.h"
#include "nci_dma_descriptor_manager_impl_dep.h"
#include "nci_dma_descriptor_manager_struct.h" /* for ia_css_dma_descriptor_manager ... */
#include "nci_dma_descriptor_manager_trace.h"

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C unsigned int nci_dma_descriptor_manager_num_chan(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);

	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_num_chan enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	return nci_dma_descriptor_manager_num_chan_dep(dev_id);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C enum nci_dma_bank_mode nci_dma_descriptor_manager_bank_mode(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);

	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_bank_mode enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	return nci_dma_descriptor_manager_bank_mode_dep(dev_id);
}

STORAGE_CLASS_INLINE unsigned int nci_dma_descriptor_manager_offset(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID >= (unsigned int)group_id);

	return dma_descriptor_setup[dev_id].data[group_id];
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C void nci_dma_descriptor_manager_offsets(
	struct ia_css_dma_descriptor_addresses * const descriptor_addresses,
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_offsets enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	nci_dma_descriptor_manager_offsets_dep(descriptor_addresses, dev_id);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C unsigned int nci_dma_descriptor_manager_channel_offset(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_channel_offset enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return (nci_dma_descriptor_manager_offset(dev_id, IPU_DEVICE_DMA_CHANNEL_GROUP_ID));
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C unsigned int nci_dma_descriptor_manager_span_offset(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_span_offset enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return (nci_dma_descriptor_manager_offset(dev_id, IPU_DEVICE_DMA_SPAN_GROUP_ID));
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C unsigned int nci_dma_descriptor_manager_terminal_offset(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_terminal_offset enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return (nci_dma_descriptor_manager_offset(dev_id, IPU_DEVICE_DMA_TERMINAL_GROUP_ID));
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_C unsigned int nci_dma_descriptor_manager_unit_offset(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_unit_offset enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return (nci_dma_descriptor_manager_offset(dev_id, IPU_DEVICE_DMA_UNIT_GROUP_ID));
}

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_cmem_base(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "nci_dma_descriptor_manager_cmem_base enter()\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTOR_MANAGER, VERBOSE, "dev_id %d()\n", dev_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	return dma_descriptor_setup[dev_id].values.cmem_descriptor_base_address;
}

#endif /* __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_H */
