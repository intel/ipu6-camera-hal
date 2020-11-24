/**
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

#ifndef __NCI_DMA_DESCRIPTORS_IMPL_DEP_H
#define __NCI_DMA_DESCRIPTORS_IMPL_DEP_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "math_support.h"
#include "misc_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "nci_dma_descriptors_types.h"
#include "nci_dma_descriptors_storage_class.h"
#include "nci_dma_descriptors_print.h"
#include "nci_dma_descriptors_checks_dep.h"
#include "nci_dma_descriptors_trace.h"
#include "nci_dma_descriptors_pretty_print.h"
#include "nci_dma_descriptors_ids.h"

#define NCI_DMA_INVALIDATE_RESERVED_BITS     5

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C unsigned int nci_dma_invalidate_instruction(
	const uint32_t dev_id,
	const enum nci_dma_descriptor_kind descriptor_kind,
	const uint32_t lower_id,
	const uint32_t upper_id)
{
	uint32_t instruction;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT((uint32_t)(1 << ipu_device_dma_descriptor_id_bits(dev_id)) > upper_id);
	IA_CSS_ASSERT((uint32_t)(1 << ipu_device_dma_descriptor_id_bits(dev_id)) > lower_id);
	IA_CSS_ASSERT((1 << NCI_DMA_EXECUTION_FORMAT_BITS) > NCI_DMA_REQUEST_INVALIDATION_FORMAT);

	/* TODO: Optimize for the new fixed-format instruction scheme */
	instruction = upper_id;
	instruction <<= ipu_device_dma_descriptor_id_bits(dev_id);
	instruction |= lower_id;
	instruction <<= NCI_DMA_INVALIDATE_RESERVED_BITS;
	instruction <<= NCI_DMA_DESCRIPTOR_KIND_BITS;
	instruction |= descriptor_kind;
	instruction <<= NCI_DMA_EXECUTION_FORMAT_BITS;
	instruction |= NCI_DMA_REQUEST_INVALIDATION_FORMAT;

	return instruction;
}

STORAGE_CLASS_INLINE int nci_dma_get_channel_descriptor_size_impl(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	nci_dma_check_channel_structures(dev_id, bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		return sizeof(struct nci_dma_non_cached_channel_descriptor);
	} else {
		return sizeof(struct nci_dma_cached_channel_descriptor);
	}
}

STORAGE_CLASS_INLINE int nci_dma_get_span_descriptor_size_impl(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	nci_dma_check_span_structures(dev_id, bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		return sizeof(struct nci_dma_non_cached_span_descriptor);
	} else {
		return sizeof(struct nci_dma_cached_span_descriptor);
	}
}

STORAGE_CLASS_INLINE int nci_dma_get_terminal_descriptor_size_impl(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	nci_dma_check_terminal_structures(dev_id, bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		return sizeof(struct nci_dma_non_cached_terminal_descriptor);
	} else {
		return sizeof(struct nci_dma_cached_terminal_descriptor);
	}
}

STORAGE_CLASS_INLINE int nci_dma_get_unit_descriptor_size_impl(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	nci_dma_check_unit_structures(dev_id, bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		return sizeof(struct nci_dma_non_cached_unit_descriptor);
	} else {
		return sizeof(struct nci_dma_cached_unit_descriptor);
	}
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_cached_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id,
	const uint32_t bank_id)
{
	uint32_t offset;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);

	IA_CSS_ASSERT(IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id ||
		IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id ||
		IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id ||
		IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id);

	IA_CSS_ASSERT(IPU_DEVICE_DMA_CHANNEL_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id &&
		ipu_device_dma_channels(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_SPAN_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id &&
		ipu_device_dma_spans(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_TERMINAL_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id &&
		ipu_device_dma_terminals(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_UNIT_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id &&
		ipu_device_dma_units(dev_id) > bank_id));

	switch (group_id) {
	case IPU_DEVICE_DMA_CHANNEL_GROUP_ID:
		offset = nci_dma_get_channel_descriptor_size_impl(dev_id, NCI_DMA_BANK_MODE_CACHED) * bank_id;
		break;
	case IPU_DEVICE_DMA_SPAN_GROUP_ID:
		offset = nci_dma_get_span_descriptor_size_impl(dev_id, NCI_DMA_BANK_MODE_CACHED) * bank_id;
		break;
	case IPU_DEVICE_DMA_UNIT_GROUP_ID:
		offset = nci_dma_get_unit_descriptor_size_impl(dev_id, NCI_DMA_BANK_MODE_CACHED) * bank_id;
		break;
	case IPU_DEVICE_DMA_TERMINAL_GROUP_ID:
		offset = nci_dma_get_terminal_descriptor_size_impl(dev_id, NCI_DMA_BANK_MODE_CACHED) * bank_id;
		break;
	default:
		offset = 0xFFFFFFFF;
		IA_CSS_ASSERT(0);
	}

	return offset;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_channel_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_channel_descriptor * const public_channel_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(NULL != public_channel_descriptor);
	IA_CSS_ASSERT(NULL != buffer);

	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_element_extend_mode_bits(dev_id)) >=
		public_channel_descriptor->element_extend_mode);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_element_init_data_bits(dev_id)) >=
		public_channel_descriptor->element_init_data);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_padding_mode_bits(dev_id)) >=
		public_channel_descriptor->padding_mode);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_sampling_setup_bits(dev_id)) >=
		public_channel_descriptor->sampling_setup);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_global_set_id_bits(dev_id)) >=
		public_channel_descriptor->global_set_id);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_ack_mode_bits(dev_id)) >=
		public_channel_descriptor->ack_mode);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_ack_addr_bits(dev_id)) >=
		public_channel_descriptor->ack_addr);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_ack_data_bits(dev_id)) >=
		public_channel_descriptor->ack_data);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_completed_count_bits(dev_id)) >=
		public_channel_descriptor->completed_counter);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE >= public_channel_descriptor->bank_mode);

	nci_dma_check_channel_structures(dev_id, bank_mode);

	if (ipu_device_dma_channels(dev_id) > channel_id) {
		if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
			filled_bytes = nci_dma_fill_non_cached_channel_descriptor(public_channel_descriptor,
				(struct nci_dma_non_cached_channel_descriptor *)buffer);
		} else {
			filled_bytes = nci_dma_fill_cached_channel_descriptor(public_channel_descriptor,
				(struct nci_dma_cached_channel_descriptor *)buffer);
		}
	}
	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_span_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_span_descriptor * const public_span_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_spans(dev_id) > span_id);
	IA_CSS_ASSERT(NULL != public_span_descriptor);
	IA_CSS_ASSERT(NULL != buffer);

	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_unit_location_bits(dev_id)) >=
		public_span_descriptor->unit_location);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_span_row_bits(dev_id)) >=
		public_span_descriptor->span_row);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_span_column_bits(dev_id)) >=
		public_span_descriptor->span_column);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_span_width_bits(dev_id)) >=
		public_span_descriptor->span_width);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_span_height_bits(dev_id)) >=
		public_span_descriptor->span_height);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE >= public_span_descriptor->bank_mode);

	nci_dma_check_span_structures(dev_id, bank_mode);

	if (ipu_device_dma_spans(dev_id) > span_id) {
		if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
			filled_bytes = nci_dma_fill_non_cached_span_descriptor(public_span_descriptor,
				(struct nci_dma_non_cached_span_descriptor *)buffer);
		} else {
			filled_bytes = nci_dma_fill_cached_span_descriptor(public_span_descriptor,
				(struct nci_dma_cached_span_descriptor *)buffer);
		}
	}
	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_terminal_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_terminal_descriptor * const public_terminal_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_terminals(dev_id) > terminal_id);
	IA_CSS_ASSERT(NULL != public_terminal_descriptor);
	IA_CSS_ASSERT(NULL != buffer);

	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_region_origin_bits(dev_id)) >=
		public_terminal_descriptor->region_origin);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_region_width_bits(dev_id)) >=
		public_terminal_descriptor->region_width);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_region_stride_bits(dev_id)) >=
		public_terminal_descriptor->region_stride);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_element_setup_bits(dev_id)) >=
		public_terminal_descriptor->element_setup);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_cio_info_setup_bits(dev_id)) >=
		public_terminal_descriptor->cio_info_setup);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_port_mode_bits(dev_id)) >=
		public_terminal_descriptor->port_mode);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE >= public_terminal_descriptor->bank_mode);

	nci_dma_check_terminal_structures(dev_id, bank_mode);

	if (ipu_device_dma_terminals(dev_id) > terminal_id) {
		if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
			filled_bytes = nci_dma_fill_non_cached_terminal_descriptor(public_terminal_descriptor,
				(struct nci_dma_non_cached_terminal_descriptor *)buffer);
		} else {
			filled_bytes = nci_dma_fill_cached_terminal_descriptor(public_terminal_descriptor,
				(struct nci_dma_cached_terminal_descriptor *)buffer);
		}
	}
	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_unit_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_unit_descriptor * const public_unit_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_units(dev_id) > unit_id);
	IA_CSS_ASSERT(NULL != public_unit_descriptor);
	IA_CSS_ASSERT(NULL != buffer);

	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_unit_width_bits(dev_id)) >= public_unit_descriptor->unit_width);
	IA_CSS_ASSERT(max_value_bits(ipu_device_dma_unit_height_bits(dev_id)) >= public_unit_descriptor->unit_height);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE >= public_unit_descriptor->bank_mode);

	nci_dma_check_unit_structures(dev_id, bank_mode);

	if (ipu_device_dma_units(dev_id) > unit_id) {
		if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
			filled_bytes = nci_dma_fill_non_cached_unit_descriptor(public_unit_descriptor,
				(struct nci_dma_non_cached_unit_descriptor *)buffer);
		} else {
			filled_bytes = nci_dma_fill_cached_unit_descriptor(public_unit_descriptor,
				(struct nci_dma_cached_unit_descriptor *)buffer);
		}
	}
	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_request_bank_dequeue_ack(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_bank_id,
	const struct nci_dma_public_request_bank_dequeue_ack_config * const public_request_reg_bank_config,
	void * const buffer)
{
	int filled_bytes = -1;

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_bank_id);
	IA_CSS_ASSERT(NULL != public_request_reg_bank_config);
	IA_CSS_ASSERT(NULL != buffer);

	filled_bytes = nci_dma_fill_request_bank_dequeue_ack_config(public_request_reg_bank_config,
				(struct nci_dma_non_cached_request_dequeue_ack *)buffer);

	return filled_bytes;
}

STORAGE_CLASS_INLINE int nci_dma_get_request_bank_dequeue_ack_section_size_impl(void)
{
	{
		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_request_bank_dequeue_ack_section_size_impl enter()\n");
	}
	return sizeof(struct nci_dma_non_cached_request_dequeue_ack);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_request_bank_dequeue_ack_section_size(void)
{
	return nci_dma_get_request_bank_dequeue_ack_section_size_impl();
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_request_bank_dequeue_ack_address_impl(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);

	return (nci_dma_get_request_descriptor_address(dev_id, request_id) +
			NCI_DMA_REQUEST_BANK_REQ_ACC_ACK_ADDR_OFFSET);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_request_bank_dequeue_ack_section_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_request_descriptor_address enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_request_id(dev_id, request_id);
	}
	return nci_dma_get_request_bank_dequeue_ack_address_impl(dev_id, request_id);
}

#endif /* __NCI_DMA_DESCRIPTORS_IMPL_DEP_H */
