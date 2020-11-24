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

#ifndef __NCI_DMA_DESCRIPTORS_IMPL_H
#define __NCI_DMA_DESCRIPTORS_IMPL_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"

#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_type_properties.h"

#include "nci_dma_descriptors.h"
#include "nci_dma_descriptors_ids.h"
#include "nci_dma_descriptors_types.h"
#include "nci_dma_descriptors_storage_class.h"
#include "nci_dma_descriptors_impl_dep.h"
#include "nci_dma_descriptors_pretty_print.h"

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_non_cached_descriptor_base(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_non_cached_descriptor_base enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_group_id(group_id);
	}

	return ipu_device_dma_base_address(dev_id) + (group_id << ipu_device_dma_group_id_idx(dev_id));
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_non_cached_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id,
	const uint32_t bank_id)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);

		IA_CSS_ASSERT(IPU_DEVICE_DMA_CHANNEL_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id &&
			ipu_device_dma_channel_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_REQUEST_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_REQUEST_GROUP_ID == group_id &&
			ipu_device_dma_request_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_GLOBAL_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_GLOBAL_GROUP_ID == group_id &&
			ipu_device_dma_global_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_MASTER_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_MASTER_GROUP_ID == group_id &&
			ipu_device_dma_master_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_SPAN_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id &&
			ipu_device_dma_span_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_TERMINAL_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id &&
			ipu_device_dma_terminal_banks(dev_id) > bank_id));
		IA_CSS_ASSERT(IPU_DEVICE_DMA_UNIT_GROUP_ID != group_id ||
			(IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id &&
			ipu_device_dma_unit_banks(dev_id) > bank_id));

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_non_cached_descriptor_offset enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_group_id(group_id);
		nci_dma_print_bank_id(dev_id, group_id, bank_id);
	}

	return (bank_id << ipu_device_dma_bank_id_idx(dev_id));
}

STORAGE_CLASS_INLINE int nci_dma_get_request_descriptor_size_impl(void)
{
	{
		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_request_descriptor_size_impl enter()\n");
	}
	return sizeof(struct nci_dma_non_cached_request_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_get_global_descriptor_size_impl(void)
{
	{
		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_global_descriptor_size_impl enter()\n");
	}
	return sizeof(struct nci_dma_non_cached_global_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_get_master_descriptor_size_impl(void)
{
	{
		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_master_descriptor_size_impl enter()\n");
	}
	return sizeof(struct nci_dma_non_cached_master_descriptor);
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_descriptors_offset_impl(
	const enum nci_dma_device_id dev_id,
	const unsigned int bank_id,
	const enum ipu_device_dma_group_id group_id,
	const enum nci_dma_bank_mode bank_mode)
{
	int address;

	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);
		IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

		if (IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id) {
			IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > bank_id);
		} else if (IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id) {
			IA_CSS_ASSERT(ipu_device_dma_spans(dev_id) > bank_id);
		} else if (IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id) {
			IA_CSS_ASSERT(ipu_device_dma_terminals(dev_id) > bank_id);
		} else if (IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id) {
			IA_CSS_ASSERT(ipu_device_dma_units(dev_id) > bank_id);
		} else {
			IA_CSS_ASSERT(0);
		}

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_descriptors_offset_impl enter()\n");
		nci_dma_print_dev_id(dev_id);
		if (IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id) {
			nci_dma_print_channel_id(dev_id, bank_id, bank_mode);
		} else if (IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id) {
			nci_dma_print_span_id(dev_id, bank_id, bank_mode);
		} else if (IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id) {
			nci_dma_print_terminal_id(dev_id, bank_id, bank_mode);
		} else if (IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id) {
			nci_dma_print_unit_id(dev_id, bank_id, bank_mode);
		} else {
			IA_CSS_ASSERT(0);
		}
		nci_dma_print_group_id(group_id);
		nci_dma_print_bank_mode(bank_mode);
	}

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		address = nci_dma_get_non_cached_descriptor_offset(dev_id, group_id, bank_id);
	} else {
		address = nci_dma_get_cached_descriptor_offset(dev_id, group_id, bank_id);
	}

	return address;
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_global_descriptor_address_impl(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_global_banks(dev_id) > global_id);

	return nci_dma_get_non_cached_descriptor_base(dev_id, IPU_DEVICE_DMA_GLOBAL_GROUP_ID) +
		nci_dma_get_non_cached_descriptor_offset(dev_id, IPU_DEVICE_DMA_GLOBAL_GROUP_ID, global_id);
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_master_descriptor_address_impl(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_master_banks(dev_id) > master_id);

	return nci_dma_get_non_cached_descriptor_base(dev_id, IPU_DEVICE_DMA_MASTER_GROUP_ID) +
		nci_dma_get_non_cached_descriptor_offset(dev_id, IPU_DEVICE_DMA_MASTER_GROUP_ID, master_id);
}

STORAGE_CLASS_INLINE unsigned int nci_dma_get_request_descriptor_address_impl(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);

	return nci_dma_get_non_cached_descriptor_base(dev_id, IPU_DEVICE_DMA_REQUEST_GROUP_ID) +
		nci_dma_get_non_cached_descriptor_offset(dev_id, IPU_DEVICE_DMA_REQUEST_GROUP_ID, request_id);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_request_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id,
	const struct nci_dma_public_request_descriptor * const public_request_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);
		IA_CSS_ASSERT(NULL != public_request_descriptor);
		IA_CSS_ASSERT(NULL != buffer);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_fill_request_descriptor enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_request_id(dev_id, request_id);
		nci_dma_descriptors_print_public_request_descriptor(public_request_descriptor);
	}

	if (ipu_device_dma_request_banks(dev_id) > request_id) {
		filled_bytes = nci_dma_fill_non_cached_request_descriptor(public_request_descriptor,
			(struct nci_dma_non_cached_request_descriptor *)buffer);
	}

	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_global_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id,
	const struct nci_dma_public_global_descriptor * const public_global_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_global_banks(dev_id) > global_id);
		IA_CSS_ASSERT(NULL != public_global_descriptor);
		IA_CSS_ASSERT(NULL != buffer);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_fill_global_descriptor enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_global_id(dev_id, global_id);
		nci_dma_descriptors_print_public_global_descriptor(public_global_descriptor);
	}

	if (ipu_device_dma_global_banks(dev_id) > global_id) {
		filled_bytes = nci_dma_fill_non_cached_global_descriptor(public_global_descriptor,
			(struct nci_dma_non_cached_global_descriptor *)buffer);
	}

	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_fill_master_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id,
	const struct nci_dma_public_master_descriptor * const public_master_descriptor,
	void * const buffer)
{
	int filled_bytes = -1;

	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_master_banks(dev_id) > master_id);
		IA_CSS_ASSERT(NULL != public_master_descriptor);
		IA_CSS_ASSERT(NULL != buffer);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_fill_master_descriptor enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_master_id(dev_id, master_id);
		nci_dma_descriptors_print_public_master_descriptor(public_master_descriptor);
	}

	if (ipu_device_dma_master_banks(dev_id) > master_id) {
		filled_bytes = nci_dma_fill_non_cached_master_descriptor(public_master_descriptor,
			(struct nci_dma_non_cached_master_descriptor *)buffer);
	}

	return filled_bytes;
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_channel_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_channel_descriptor_size enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_bank_mode(bank_mode);
	}
	return nci_dma_get_channel_descriptor_size_impl(dev_id, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_request_descriptor_size(void)
{
	return nci_dma_get_request_descriptor_size_impl();
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_global_descriptor_size(void)
{
	return nci_dma_get_global_descriptor_size_impl();
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_master_descriptor_size(void)
{
	return nci_dma_get_master_descriptor_size_impl();
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_span_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_span_descriptor_size enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_bank_mode(bank_mode);
	}
	return nci_dma_get_span_descriptor_size_impl(dev_id, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_terminal_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_terminal_descriptor_size enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_bank_mode(bank_mode);
	}
	return nci_dma_get_terminal_descriptor_size_impl(dev_id, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_unit_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_unit_descriptor_size enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_bank_mode(bank_mode);
	}
	return nci_dma_get_unit_descriptor_size_impl(dev_id, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_channel_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	return nci_dma_get_descriptors_offset_impl(dev_id, channel_id, IPU_DEVICE_DMA_CHANNEL_GROUP_ID, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_span_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode)
{
	return nci_dma_get_descriptors_offset_impl(dev_id, span_id, IPU_DEVICE_DMA_SPAN_GROUP_ID, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_terminal_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode)
{
	return nci_dma_get_descriptors_offset_impl(dev_id, terminal_id, IPU_DEVICE_DMA_TERMINAL_GROUP_ID, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_unit_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode)
{
	return nci_dma_get_descriptors_offset_impl(dev_id, unit_id, IPU_DEVICE_DMA_UNIT_GROUP_ID, bank_mode);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_global_descriptor_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_global_banks(dev_id) > global_id);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_global_descriptor_address enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_global_id(dev_id, global_id);
	}
	return nci_dma_get_global_descriptor_address_impl(dev_id, global_id);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_master_descriptor_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id)
{
	{
		IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
		IA_CSS_ASSERT(ipu_device_dma_master_banks(dev_id) > master_id);

		IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, VERBOSE, "nci_dma_get_master_descriptor_address enter()\n");
		nci_dma_print_dev_id(dev_id);
		nci_dma_print_master_id(dev_id, master_id);
	}
	return nci_dma_get_master_descriptor_address_impl(dev_id, master_id);
}

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_C int nci_dma_get_request_descriptor_address(
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
	return nci_dma_get_request_descriptor_address_impl(dev_id, request_id);
}

#endif /* __NCI_DMA_DESCRIPTORS_IMPL_H */
