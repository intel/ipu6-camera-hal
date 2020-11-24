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

#ifndef __NCI_DMA_DESCRIPTORS_CHECKS_V4_H
#define __NCI_DMA_DESCRIPTORS_CHECKS_V4_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "nci_dma_descriptors_types.h"
#include "nci_dma_descriptors_checks_dep.h"

STORAGE_CLASS_INLINE void nci_dma_check_non_cached_channel_structure(void)
{
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->element_extend_mode);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->element_init_data);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->padding_mode);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->sampling_setup);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->global_set_id);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_mode);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_addr);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_data);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->unused_8);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->extended_pad);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->completed_counter);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->pending_counter);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->lock_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->dirty_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->bank_mode);
	COMPILATION_ERROR_IF(sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->element_extend_mode +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->element_init_data +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->padding_mode +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->sampling_setup +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->global_set_id +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_mode +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_addr +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->ack_data +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->unused_8 +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->extended_pad +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->completed_counter +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->pending_counter +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->lock_status +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->dirty_status +
		sizeof((struct nci_dma_non_cached_channel_descriptor *)0)->bank_mode !=
		sizeof(struct nci_dma_non_cached_channel_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_non_cached_channel_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_cached_channel_structure(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(BYTES(ipu_device_dma_ack_addr_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_addr);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_ack_data_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_data);
	IA_CSS_ASSERT(1 == sizeof((struct nci_dma_cached_channel_descriptor *)0)->extended_pad);
	IA_CSS_ASSERT(1 == sizeof((struct nci_dma_cached_channel_descriptor *)0)->replacement_policy);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_global_set_id_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->global_set_id);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_ack_mode_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_mode);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_completed_count_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->completed_counter);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_addr +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_data +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->extended_pad +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->global_set_id +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->ack_mode +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->completed_counter +
		sizeof((struct nci_dma_cached_channel_descriptor *)0)->padding !=
		sizeof(struct nci_dma_cached_channel_descriptor));
	COMPILATION_ERROR_IF(0 !=
		(sizeof(struct nci_dma_cached_channel_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_non_cached_span_structure(void)
{
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unit_location);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_row);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_column);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_width);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_height);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_mode);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_6);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_7);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_8);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_9);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_B);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->pending_counter);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->lock_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->dirty_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->bank_mode);
	COMPILATION_ERROR_IF(sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unit_location +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_row +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_column +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_width +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_height +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->span_mode +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_6 +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_7 +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_8 +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_9 +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->unused_B +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->pending_counter +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->lock_status +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->dirty_status +
		sizeof((struct nci_dma_non_cached_span_descriptor *)0)->bank_mode !=
		sizeof(struct nci_dma_non_cached_span_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_non_cached_span_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_cached_span_structure(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(BYTES(ipu_device_dma_unit_location_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->unit_location);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_span_column_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_column);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_span_row_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_row);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_span_width_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_width);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_span_height_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_height);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_span_mode_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_mode);
	IA_CSS_ASSERT(1 == sizeof((struct nci_dma_cached_span_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_cached_span_descriptor *)0)->unit_location +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_column +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_row +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_width +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_height +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->span_mode +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_cached_span_descriptor *)0)->padding !=
		sizeof(struct nci_dma_cached_span_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_cached_span_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_non_cached_terminal_structure(void)
{
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
			sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_origin);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_width);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_stride);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->element_setup);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->cio_info_setup);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->port_mode);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_height);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_7);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_8);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_9);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_B);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->pending_counter);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->lock_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_E);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->bank_mode);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_origin +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_width +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_stride +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->element_setup +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->cio_info_setup +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->port_mode +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->region_height +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_7 +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_8 +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_9 +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_B +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->pending_counter +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->lock_status +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->unused_E +
		sizeof((struct nci_dma_non_cached_terminal_descriptor *)0)->bank_mode !=
		sizeof(struct nci_dma_non_cached_terminal_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_non_cached_terminal_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_cached_terminal_structure(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(BYTES(ipu_device_dma_region_origin_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_origin);
	/* Region stride in using the lower 24 bits out of 32 */
	IA_CSS_ASSERT(BYTES(ipu_device_dma_region_stride_bits(dev_id)) + 1 ==
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_stride);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_region_width_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_width);
	IA_CSS_ASSERT(2 == sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_height);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_cio_info_setup_bits(dev_id)) <=
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->cio_info_setup);
	IA_CSS_ASSERT(1 == sizeof((struct nci_dma_cached_terminal_descriptor *)0)->replacement_policy);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_element_setup_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->element_setup);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_origin +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_stride +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_width +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->region_height +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->cio_info_setup +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_cached_terminal_descriptor *)0)->element_setup !=
		sizeof(struct nci_dma_cached_terminal_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_cached_terminal_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_non_cached_unit_structure(void)
{
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unit_width);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unit_height);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_2);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_3);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_4);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_5);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_6);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_7);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_8);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_9);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_B);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->pending_counter);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->lock_status);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_E);
	COMPILATION_ERROR_IF(sizeof(uint32_t) !=
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->bank_mode);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unit_width +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unit_height +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_2 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_3 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_4 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_5 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_6 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_7 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_8 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_9 +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->replacement_policy +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_B +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->pending_counter +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->lock_status +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->unused_E +
		sizeof((struct nci_dma_non_cached_unit_descriptor *)0)->bank_mode !=
		sizeof(struct nci_dma_non_cached_unit_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_non_cached_unit_descriptor) % 4));
}

STORAGE_CLASS_INLINE void nci_dma_check_cached_unit_structure(
	const enum nci_dma_device_id dev_id)
{
	IA_CSS_ASSERT(BYTES(ipu_device_dma_unit_width_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_unit_descriptor *)0)->unit_width);
	IA_CSS_ASSERT(BYTES(ipu_device_dma_unit_height_bits(dev_id)) ==
		sizeof((struct nci_dma_cached_unit_descriptor *)0)->unit_height);
	IA_CSS_ASSERT(1 == sizeof((struct nci_dma_cached_unit_descriptor *)0)->replacement_policy);
	COMPILATION_ERROR_IF(
		sizeof((struct nci_dma_cached_unit_descriptor *)0)->unit_width +
		sizeof((struct nci_dma_cached_unit_descriptor *)0)->unit_height +
		sizeof((struct nci_dma_cached_unit_descriptor *)0)->replacement_policy !=
		sizeof(struct nci_dma_cached_unit_descriptor));
	COMPILATION_ERROR_IF(0 != (sizeof(struct nci_dma_cached_unit_descriptor) % 4));
}

#endif /* __NCI_DMA_DESCRIPTORS_CHECKS_V4_H */
