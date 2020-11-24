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

#ifndef __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_DEP_H
#define __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_DEP_H

#include "ipu_device_dma_devices.h" /* for NCI_DMA_NUM_DEVICES ... */
#include "nci_dma_descriptor_manager_struct.h" /* for ia_css_dma_descriptor_manager ... */
#include "ipu_device_memory_properties_defs.h" /* for IPU_DEVICE_MEMORY_TRANSFER_DMEM_CONTROL_BUS_ADDR */
#include "nci_dma_descriptor_manager_struct_dep.h" /* for DMA_EXT0_NUM_LOGICAL_CHANNELS ... */

static const union ia_css_dma_descriptor_manager dma_descriptor_setup[NCI_DMA_NUM_DEVICES] = {
	{
		{
		/* DMA_EXT0 */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext0_descriptors) +
			offsetof(struct ia_css_dma_cached_ext0_descriptor, ia_css_dma_cached_ext0_channel_descriptor)),
		0,  /* request_descriptor_offset    */
		0,  /* global_descriptor_offset     */
		0,  /* master_descriptor_offset     */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext0_descriptors) +
			offsetof(struct ia_css_dma_cached_ext0_descriptor, ia_css_dma_cached_ext0_span_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext0_descriptors) +
			offsetof(struct ia_css_dma_cached_ext0_descriptor, ia_css_dma_cached_ext0_unit_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext0_descriptors) +
			offsetof(struct ia_css_dma_cached_ext0_descriptor, ia_css_dma_cached_ext0_terminal_descriptor)),
		IPU_DEVICE_MEMORY_TRANSFER_DMEM_CONTROL_BUS_ADDR,
		},
	},
	{
		{
		/* DMA_EXT1R */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1r_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1r_descriptor, ia_css_dma_cached_ext1r_channel_descriptor)),
		0,  /* request_descriptor_offset    */
		0,  /* global_descriptor_offset     */
		0,  /* master_descriptor_offset     */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1r_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1r_descriptor, ia_css_dma_cached_ext1r_span_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1r_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1r_descriptor, ia_css_dma_cached_ext1r_unit_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1r_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1r_descriptor, ia_css_dma_cached_ext1r_terminal_descriptor)),
		IPU_DEVICE_MEMORY_TRANSFER_DMEM_CONTROL_BUS_ADDR,
		},
	},
	{
		{
		/* DMA_EXT1W */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1w_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1w_descriptor, ia_css_dma_cached_ext1w_channel_descriptor)),
		0,  /* request_descriptor_offset    */
		0,  /* global_descriptor_offset     */
		0,  /* master_descriptor_offset     */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1w_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1w_descriptor, ia_css_dma_cached_ext1w_span_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1w_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1w_descriptor, ia_css_dma_cached_ext1w_unit_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_ext1w_descriptors) +
			offsetof(struct ia_css_dma_cached_ext1w_descriptor, ia_css_dma_cached_ext1w_terminal_descriptor)),
		IPU_DEVICE_MEMORY_TRANSFER_DMEM_CONTROL_BUS_ADDR,
		},
	},
#if HAS_DMA_INTERNAL
	{
		{
		/* DMA_INT */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_int_descriptors) +
			offsetof(struct ia_css_dma_cached_int_descriptor, ia_css_dma_cached_int_channel_descriptor)),
		0,  /* request_descriptor_offset    */
		0,  /* global_descriptor_offset     */
		0,  /* master_descriptor_offset     */
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_int_descriptors) +
			offsetof(struct ia_css_dma_cached_int_descriptor, ia_css_dma_cached_int_span_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_int_descriptors) +
			offsetof(struct ia_css_dma_cached_int_descriptor, ia_css_dma_cached_int_unit_descriptor)),
		(offsetof(struct ia_css_dma_cached_descriptor, ia_css_dma_cached_int_descriptors) +
			offsetof(struct ia_css_dma_cached_int_descriptor, ia_css_dma_cached_int_terminal_descriptor)),
		IPU_DEVICE_MEMORY_TRANSFER_DMEM_CONTROL_BUS_ADDR,
		},
	},
#endif /* HAS_DMA_INTERNAL */
	{
		{
		/* DMA_ISA */
		0,  /* channel_descriptor_offset    */
		0,  /* request_descriptor_offset    */
		0,  /* global_descriptor_offset     */
		0,  /* master_descriptor_offset     */
		0,  /* span_descriptor_offset       */
		0,  /* unit_descriptor_offset       */
		0,  /* terminal_descriptor_offset   */
		0,  /* cmem_descriptor_base_address */
		},
	},
};

STORAGE_CLASS_INLINE enum nci_dma_bank_mode nci_dma_descriptor_manager_bank_mode_dep(
	const enum nci_dma_device_id dev_id)
{
	enum nci_dma_bank_mode bank_mode = (enum nci_dma_bank_mode)N_NCI_DMA_BANK_MODE;

	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);

	switch(dev_id) {
	case NCI_DMA_ISA:
#if HAS_DMA_INTERNAL
	case NCI_DMA_INT:
#endif /* HAS_DMA_INTERNAL */
		bank_mode = NCI_DMA_BANK_MODE_NON_CACHED;
		break;
	case NCI_DMA_EXT0:
	case NCI_DMA_EXT1R:
	case NCI_DMA_EXT1W:
		bank_mode = NCI_DMA_BANK_MODE_CACHED;
		break;
	default:
		assert(0);
		break;
	}
	assert(N_NCI_DMA_BANK_MODE != (unsigned int)bank_mode);
	return bank_mode;
}

STORAGE_CLASS_INLINE unsigned int nci_dma_descriptor_manager_num_chan_dep(
	const enum nci_dma_device_id dev_id)
{
	unsigned int num_chan;

	assert(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	switch(dev_id) {
	case NCI_DMA_EXT0:
		num_chan = DMA_EXT0_NUM_LOGICAL_CHANNELS;
		break;
	case NCI_DMA_EXT1R:
		num_chan = DMA_EXT1R_NUM_LOGICAL_CHANNELS;
		break;
	case NCI_DMA_EXT1W:
		num_chan = DMA_EXT1W_NUM_LOGICAL_CHANNELS;
		break;
#if HAS_DMA_INTERNAL
	case NCI_DMA_INT:
		num_chan = DMA_INT_NUM_LOGICAL_CHANNELS;
		break;
#endif /* HAS_DMA_INTERNAL */
	case NCI_DMA_ISA:
		num_chan = DMA_ISA_NUM_LOGICAL_CHANNELS;
		break;
	default:
		assert(0);
		num_chan = 0;
		break;
	}
	return num_chan;
}

STORAGE_CLASS_INLINE void nci_dma_descriptor_manager_offsets_dep(
	struct ia_css_dma_descriptor_addresses * const descriptor_addresses,
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(NULL != descriptor_addresses);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);

	switch(dev_id) {
		case NCI_DMA_ISA:
			descriptor_addresses->channel_descriptor_offset = 0;
			descriptor_addresses->span_descriptor_offset = 0;
			descriptor_addresses->terminal_descriptor_offset = 0;
			descriptor_addresses->unit_descriptor_offset = 0;
			descriptor_addresses->cmem_descriptor_base_address = 0;
			break;
		case NCI_DMA_EXT0:
			descriptor_addresses->channel_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT0].data[IPU_DEVICE_DMA_CHANNEL_GROUP_ID];
			descriptor_addresses->span_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT0].data[IPU_DEVICE_DMA_SPAN_GROUP_ID];
			descriptor_addresses->terminal_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT0].data[IPU_DEVICE_DMA_TERMINAL_GROUP_ID];
			descriptor_addresses->unit_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT0].data[IPU_DEVICE_DMA_UNIT_GROUP_ID];
			descriptor_addresses->cmem_descriptor_base_address = dma_descriptor_setup[NCI_DMA_EXT0].data[N_IPU_DEVICE_DMA_GROUP_ID];
			break;
		case NCI_DMA_EXT1R:
			descriptor_addresses->channel_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1R].data[IPU_DEVICE_DMA_CHANNEL_GROUP_ID];
			descriptor_addresses->span_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1R].data[IPU_DEVICE_DMA_SPAN_GROUP_ID];
			descriptor_addresses->terminal_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1R].data[IPU_DEVICE_DMA_TERMINAL_GROUP_ID];
			descriptor_addresses->unit_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1R].data[IPU_DEVICE_DMA_UNIT_GROUP_ID];
			descriptor_addresses->cmem_descriptor_base_address = dma_descriptor_setup[NCI_DMA_EXT1R].data[N_IPU_DEVICE_DMA_GROUP_ID];
			break;
		case NCI_DMA_EXT1W:
			descriptor_addresses->channel_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1W].data[IPU_DEVICE_DMA_CHANNEL_GROUP_ID];
			descriptor_addresses->span_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1W].data[IPU_DEVICE_DMA_SPAN_GROUP_ID];
			descriptor_addresses->terminal_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1W].data[IPU_DEVICE_DMA_TERMINAL_GROUP_ID];
			descriptor_addresses->unit_descriptor_offset = dma_descriptor_setup[NCI_DMA_EXT1W].data[IPU_DEVICE_DMA_UNIT_GROUP_ID];
			descriptor_addresses->cmem_descriptor_base_address = dma_descriptor_setup[NCI_DMA_EXT1W].data[N_IPU_DEVICE_DMA_GROUP_ID];
			break;
#if HAS_DMA_INTERNAL
		case NCI_DMA_INT:
			descriptor_addresses->channel_descriptor_offset = dma_descriptor_setup[NCI_DMA_INT].data[IPU_DEVICE_DMA_CHANNEL_GROUP_ID];
			descriptor_addresses->span_descriptor_offset = dma_descriptor_setup[NCI_DMA_INT].data[IPU_DEVICE_DMA_SPAN_GROUP_ID];
			descriptor_addresses->terminal_descriptor_offset = dma_descriptor_setup[NCI_DMA_INT].data[IPU_DEVICE_DMA_TERMINAL_GROUP_ID];
			descriptor_addresses->unit_descriptor_offset = dma_descriptor_setup[NCI_DMA_INT].data[IPU_DEVICE_DMA_UNIT_GROUP_ID];
			descriptor_addresses->cmem_descriptor_base_address = dma_descriptor_setup[NCI_DMA_INT].data[N_IPU_DEVICE_DMA_GROUP_ID];
			break;
#endif /* HAS_DMA_INTERNAL */
		default:
			assert(0);
			descriptor_addresses->channel_descriptor_offset = 0;
			descriptor_addresses->span_descriptor_offset = 0;
			descriptor_addresses->terminal_descriptor_offset = 0;
			descriptor_addresses->unit_descriptor_offset = 0;
			descriptor_addresses->cmem_descriptor_base_address = 0;
			break;
	}
}

#endif /* __NCI_DMA_DESCRIPTOR_MANAGER_IMPL_DEP_H */
