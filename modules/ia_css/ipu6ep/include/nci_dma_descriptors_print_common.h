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

#ifndef __NCI_DMA_DESCRIPTORS_PRINT_COMMON_V4_H
#define __NCI_DMA_DESCRIPTORS_PRINT_COMMON_V4_H

#include "storage_class.h"
#include "misc_support.h"
#include "assert_support.h"
#include "ia_css_trace.h"
#include "nci_dma_descriptors.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_properties_defs.h"
#include "nci_dma_descriptors_trace.h"

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_channel_descriptor(
	const struct nci_dma_public_channel_descriptor * const public_channel_descriptor)
{
	NOT_USED(public_channel_descriptor);
	IA_CSS_ASSERT(NULL != public_channel_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_channel_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_extend_mode = 0x%08x\n",
		public_channel_descriptor->element_extend_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_init_data   = 0x%08x\n",
		public_channel_descriptor->element_init_data);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t padding_mode        = 0x%08x\n",
		public_channel_descriptor->padding_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t sampling_setup      = 0x%08x\n",
		public_channel_descriptor->sampling_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t global_set_id       = 0x%08x\n",
		public_channel_descriptor->global_set_id);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_mode            = 0x%08x\n",
		public_channel_descriptor->ack_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_addr            = 0x%08x\n",
		public_channel_descriptor->ack_addr);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_data            = 0x%08x\n",
		public_channel_descriptor->ack_data);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t completed_counter   = 0x%08x\n",
		public_channel_descriptor->completed_counter);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy  = 0x%08x\n",
		public_channel_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t sign_bit_toggle_value	= 0x%08x\n",
		public_channel_descriptor->sign_bit_toggle_value);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode			= 0x%08x\n",
		public_channel_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_channel_descriptor(
	const struct nci_dma_non_cached_channel_descriptor * const non_cached_channel_descriptor)
{
	NOT_USED(non_cached_channel_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_channel_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_channel_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_extend_mode = 0x%08x\n",
		non_cached_channel_descriptor->element_extend_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_init_data   = 0x%08x\n",
		non_cached_channel_descriptor->element_init_data);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t padding_mode        = 0x%08x\n",
		non_cached_channel_descriptor->padding_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t sampling_setup      = 0x%08x\n",
		non_cached_channel_descriptor->sampling_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t global_set_id       = 0x%08x\n",
		non_cached_channel_descriptor->global_set_id);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_mode            = 0x%08x\n",
		non_cached_channel_descriptor->ack_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_addr            = 0x%08x\n",
		non_cached_channel_descriptor->ack_addr);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_data            = 0x%08x\n",
		non_cached_channel_descriptor->ack_data);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t extended_pad	= 0x%08x\n",
		non_cached_channel_descriptor->extended_pad);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy	= 0x%08x\n",
		non_cached_channel_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t completed_counter	= 0x%08x\n",
		non_cached_channel_descriptor->completed_counter);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode		= 0x%08x\n",
		non_cached_channel_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_request_descriptor(
	const struct nci_dma_non_cached_request_descriptor * const non_cached_request_descriptor)
{
	NOT_USED(non_cached_request_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_request_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_request_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t instruction                  = 0x%08x\n",
		non_cached_request_descriptor->instruction);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t descriptor_id_setup_1        = 0x%08x\n",
		non_cached_request_descriptor->descriptor_id_setup_1);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t descriptor_id_setup_2        = 0x%08x\n",
		non_cached_request_descriptor->descriptor_id_setup_2);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t request_valid                = 0x%08x\n",
		non_cached_request_descriptor->request_valid);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t request_resourced            = 0x%08x\n",
		non_cached_request_descriptor->request_resourced);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_request_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(request_id);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);
	IA_CSS_ASSERT(NULL != buffer);

	nci_dma_descriptors_print_non_cached_request_descriptor(
		(const struct nci_dma_non_cached_request_descriptor * const)buffer);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_global_descriptor(
	const struct nci_dma_public_global_descriptor * const public_global_descriptor)
{
	uint32_t idx;

	NOT_USED(public_global_descriptor);
	IA_CSS_ASSERT(NULL != public_global_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_global_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_descriptor_base_address     = 0x%08x\n",
		public_global_descriptor->unit_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_descriptor_base_address     = 0x%08x\n",
		public_global_descriptor->span_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t terminal_descriptor_base_address = 0x%08x\n",
		public_global_descriptor->terminal_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t channel_descriptor_base_address  = 0x%08x\n",
		public_global_descriptor->channel_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t maximum_physical_block_height    = 0x%08x\n",
		public_global_descriptor->maximum_physical_block_height);
	for (idx = 0; idx < NCI_DMA_MAX_GLOBAL_SETS; idx++) {
		IA_CSS_TRACE_2(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_1d_block_width[%d]            = 0x%08x\n",
			idx, public_global_descriptor->maximum_physical_block_width[idx].max_1d_block_width);
		IA_CSS_TRACE_2(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_2d_block_width[%d]            = 0x%08x\n",
			idx, public_global_descriptor->maximum_physical_block_width[idx].max_2d_block_width);
	}
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_global_descriptor(
	const struct nci_dma_non_cached_global_descriptor * const non_cached_global_descriptor)
{
	NOT_USED(non_cached_global_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_global_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_global_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t error                            = 0x%08x\n",
		non_cached_global_descriptor->error);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t idle                             = 0x%08x\n",
		non_cached_global_descriptor->idle);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_descriptor_base_address     = 0x%08x\n",
		non_cached_global_descriptor->unit_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_descriptor_base_address     = 0x%08x\n",
		non_cached_global_descriptor->span_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t terminal_descriptor_base_address = 0x%08x\n",
		non_cached_global_descriptor->terminal_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t channel_descriptor_base_address  = 0x%08x\n",
		non_cached_global_descriptor->channel_descriptor_base_address);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t maximum_physical_block_height    = 0x%08x\n",
		non_cached_global_descriptor->maximum_physical_block_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_1d_block_width_0             = 0x%08x\n",
		non_cached_global_descriptor->max_1d_block_width_0);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_2d_block_width_0             = 0x%08x\n",
		non_cached_global_descriptor->max_2d_block_width_0);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_1d_block_width_1             = 0x%08x\n",
		non_cached_global_descriptor->max_1d_block_width_1);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t max_2d_block_width_1             = 0x%08x\n",
		non_cached_global_descriptor->max_2d_block_width_1);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_global_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(global_id);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_global_banks(dev_id) > global_id);
	IA_CSS_ASSERT(NULL != buffer);

	nci_dma_descriptors_print_non_cached_global_descriptor(
		(const struct nci_dma_non_cached_global_descriptor * const)buffer);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_master_descriptor(
	const struct nci_dma_public_master_descriptor * const public_master_descriptor)
{
	NOT_USED(public_master_descriptor);
	IA_CSS_ASSERT(NULL != public_master_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_master_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t srmd_support            = 0x%08x\n",
		public_master_descriptor->srmd_support);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t burst_support           = 0x%08x\n",
		public_master_descriptor->burst_support);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t maximum_physical_stride = 0x%08x\n",
		public_master_descriptor->maximum_physical_stride);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_master_descriptor(
	const struct nci_dma_non_cached_master_descriptor * const non_cached_master_descriptor)
{
	NOT_USED(non_cached_master_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_master_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_master_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t srmd_support            = 0x%08x\n",
		non_cached_master_descriptor->srmd_support);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unused_1                = 0x%08x\n",
		non_cached_master_descriptor->unused_1);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t burst_support           = 0x%08x\n",
		non_cached_master_descriptor->burst_support);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t maximum_physical_stride = 0x%08x\n",
		non_cached_master_descriptor->maximum_physical_stride);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_master_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(master_id);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_master_banks(dev_id) > master_id);
	IA_CSS_ASSERT(NULL != buffer);

	nci_dma_descriptors_print_non_cached_master_descriptor(
		(const struct nci_dma_non_cached_master_descriptor * const)buffer);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_span_descriptor(
	const struct nci_dma_public_span_descriptor * const public_span_descriptor)
{
	NOT_USED(public_span_descriptor);
	IA_CSS_ASSERT(NULL != public_span_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_span_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_location  = 0x%08x\n",
		public_span_descriptor->unit_location);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_row       = 0x%08x\n",
		public_span_descriptor->span_row);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_column    = 0x%08x\n",
		public_span_descriptor->span_column);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_width     = 0x%08x\n",
		public_span_descriptor->span_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_height    = 0x%08x\n",
		public_span_descriptor->span_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_mode      = 0x%08x\n",
		public_span_descriptor->span_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode      = 0x%08x\n",
		public_span_descriptor->bank_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy      = 0x%08x\n",
		public_span_descriptor->replacement_policy);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_span_descriptor(
	const struct nci_dma_non_cached_span_descriptor * const non_cached_span_descriptor)
{
	NOT_USED(non_cached_span_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_span_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_span_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_location  = 0x%08x\n",
		non_cached_span_descriptor->unit_location);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_column    = 0x%08x\n",
		non_cached_span_descriptor->span_column);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_row       = 0x%08x\n",
		non_cached_span_descriptor->span_row);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_width     = 0x%08x\n",
		non_cached_span_descriptor->span_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_height    = 0x%08x\n",
		non_cached_span_descriptor->span_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy      = 0x%08x\n",
		non_cached_span_descriptor->replacement_policy);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_terminal_descriptor(
	const struct nci_dma_public_terminal_descriptor * const public_terminal_descriptor)
{
	NOT_USED(public_terminal_descriptor);
	IA_CSS_ASSERT(NULL != public_terminal_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_terminal_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_origin  = 0x%08x\n",
		public_terminal_descriptor->region_origin);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_stride  = 0x%08x\n",
		public_terminal_descriptor->region_stride);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_width   = 0x%08x\n",
		public_terminal_descriptor->region_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_height  = 0x%08x\n",
		public_terminal_descriptor->region_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t cio_info_setup = 0x%08x\n",
		public_terminal_descriptor->cio_info_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy = 0x%08x\n",
		public_terminal_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_setup  = 0x%08x\n",
		public_terminal_descriptor->element_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t port_mode      = 0x%08x\n",
		public_terminal_descriptor->port_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode      = 0x%08x\n",
		public_terminal_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_terminal_descriptor(
	const struct nci_dma_non_cached_terminal_descriptor * const non_cached_terminal_descriptor)
{
	NOT_USED(non_cached_terminal_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_terminal_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_terminal_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_origin  = 0x%08x\n",
		non_cached_terminal_descriptor->region_origin);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_width   = 0x%08x\n",
		non_cached_terminal_descriptor->region_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_stride  = 0x%08x\n",
		non_cached_terminal_descriptor->region_stride);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_setup  = 0x%08x\n",
		non_cached_terminal_descriptor->element_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t cio_info_setup = 0x%08x\n",
		non_cached_terminal_descriptor->cio_info_setup);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t port_mode;     = 0x%08x\n",
		non_cached_terminal_descriptor->port_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy	= 0x%08x\n",
		non_cached_terminal_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode		= 0x%08x\n",
		non_cached_terminal_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_unit_descriptor(
	const struct nci_dma_public_unit_descriptor * const public_unit_descriptor)
{
	NOT_USED(public_unit_descriptor);
	IA_CSS_ASSERT(NULL != public_unit_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_unit_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_width  = 0x%08x\n",
		public_unit_descriptor->unit_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_height = 0x%08x\n",
		public_unit_descriptor->unit_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy = 0x%08x\n",
		public_unit_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode = 0x%08x\n",
		public_unit_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_non_cached_unit_descriptor(
	const struct nci_dma_non_cached_unit_descriptor * const non_cached_unit_descriptor)
{
	NOT_USED(non_cached_unit_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_unit_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_non_cached_unit_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_width  = 0x%08x\n",
		non_cached_unit_descriptor->unit_width);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_height = 0x%08x\n",
		non_cached_unit_descriptor->unit_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy = 0x%08x\n",
		non_cached_unit_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t bank_mode = 0x%08x\n",
		non_cached_unit_descriptor->bank_mode);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_public_request_descriptor(
	const struct nci_dma_public_request_descriptor * const public_request_descriptor)
{
	NOT_USED(public_request_descriptor);
	IA_CSS_ASSERT(NULL != public_request_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_public_request_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t instruction                  = 0x%08x\n",
		public_request_descriptor->instruction);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t descriptor_id_setup_1        = 0x%08x\n",
		public_request_descriptor->descriptor_id_setup_1);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t descriptor_id_setup_2        = 0x%08x\n",
		public_request_descriptor->descriptor_id_setup_2);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_cached_channel_descriptor_type(
	const struct nci_dma_cached_channel_descriptor * const cached_channel_descriptor)
{
	NOT_USED(cached_channel_descriptor);
	IA_CSS_ASSERT(NULL != cached_channel_descriptor);

	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_cached_channel_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_addr		= 0x%08x\n",
		*((uint32_t *)cached_channel_descriptor->ack_addr));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_data		= 0x%08x\n",
		*((uint32_t *)cached_channel_descriptor->ack_data));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t extended_pad	= 0x%08x\n",
		cached_channel_descriptor->extended_pad);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy	= 0x%08x\n",
		cached_channel_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t global_set_id	= 0x%08x\n",
		cached_channel_descriptor->global_set_id);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t ack_mode		= 0x%08x\n",
		cached_channel_descriptor->ack_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t completed_counter	= 0x%08x\n",
		cached_channel_descriptor->completed_counter);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_cached_span_descriptor_type(
	const struct nci_dma_cached_span_descriptor * const cached_span_descriptor)
{
	NOT_USED(cached_span_descriptor);
	IA_CSS_ASSERT(NULL != cached_span_descriptor);

	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_cached_span_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_location  = 0x%08x\n",
		*((uint32_t *)cached_span_descriptor->unit_location));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_column    = 0x%08x\n",
		*((uint16_t *)cached_span_descriptor->span_column));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_row       = 0x%08x\n",
		*((uint16_t *)cached_span_descriptor->span_row));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_width     = 0x%08x\n",
		*((uint16_t *)cached_span_descriptor->span_width));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_height    = 0x%08x\n",
		*((uint16_t *)cached_span_descriptor->span_height));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t span_mode      = 0x%08x\n",
		cached_span_descriptor->span_mode);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy	= 0x%08x\n",
		cached_span_descriptor->replacement_policy);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_cached_terminal_descriptor_type(
	const struct nci_dma_cached_terminal_descriptor * const cached_terminal_descriptor)
{
	NOT_USED(cached_terminal_descriptor);
	IA_CSS_ASSERT(NULL != cached_terminal_descriptor);

	IA_CSS_ASSERT(NULL != cached_terminal_descriptor);
	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_cached_terminal_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_origin  = 0x%08x\n",
		*((uint32_t *)cached_terminal_descriptor->region_origin));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_stride  = 0x%08x\n",
		*((uint32_t *)cached_terminal_descriptor->region_stride));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_width   = 0x%08x\n",
		*((uint16_t *)cached_terminal_descriptor->region_width));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t region_height  = 0x%08x\n",
		*((uint32_t *)cached_terminal_descriptor->region_height));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t cio_info_setup = 0x%08x\n",
		*((uint16_t *)cached_terminal_descriptor->cio_info_setup));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy	= 0x%08x\n",
		cached_terminal_descriptor->replacement_policy);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t element_setup	= 0x%08x\n",
		cached_terminal_descriptor->element_setup);
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_cached_unit_descriptor_type(
	const struct nci_dma_cached_unit_descriptor * const cached_unit_descriptor)
{
	NOT_USED(cached_unit_descriptor);
	IA_CSS_ASSERT(NULL != cached_unit_descriptor);

	IA_CSS_TRACE_0(NCI_DMA_DESCRIPTORS, DEBUG,
		"nci_dma_descriptors_print_cached_unit_descriptor:\n");
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_width  = 0x%08x\n",
		*((uint16_t *)cached_unit_descriptor->unit_width));
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t unit_height = 0x%08x\n",
		cached_unit_descriptor->unit_height);
	IA_CSS_TRACE_1(NCI_DMA_DESCRIPTORS, DEBUG, "\t replacement_policy = 0x%08x\n",
		cached_unit_descriptor->replacement_policy);
}

#endif /* __NCI_DMA_DESCRIPTORS_PRINT_COMMON_V4_H */
