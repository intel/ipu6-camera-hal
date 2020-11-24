/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2018 Intel Corporation.
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

#ifndef __IPU_DEVICE_DMA_PROPERTIES_FUNC_H
#define __IPU_DEVICE_DMA_PROPERTIES_FUNC_H

#include "assert_support.h"
#include "storage_class.h"
#include "misc_support.h"
#include "ipu_device_dma_devices.h"

#ifdef __EXTERN_DATA_DMA_PROPERTIES__
#include "ipu_device_dma_properties_decl.h" /* declaration only */
#else
#include "ipu_device_dma_properties_impl.h" /* definition */
#endif

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_num_devices(void)
{
	return NCI_DMA_NUM_DEVICES;
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_2dblock(const unsigned int dev_id)
{
	return ipu_device_dma_properties_nativecio_2dblock[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_raccept(const unsigned int dev_id)
{
	return ipu_device_dma_properties_nativecio_raccept[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_nativecio_srmd(const unsigned int dev_id)
{
	return ipu_device_dma_properties_nativecio_srmd[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_request_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_request_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_units(const unsigned int dev_id)
{
	return ipu_device_dma_properties_units[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_unit_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_unit_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_spans(const unsigned int dev_id)
{
	return ipu_device_dma_properties_spans[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_span_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_terminals(const unsigned int dev_id)
{
	return ipu_device_dma_properties_terminals[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_terminal_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_terminal_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_channels(const unsigned int dev_id)
{
	return ipu_device_dma_properties_channels[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_channel_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_channel_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_macro_size(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_macro_size[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_instructions_pending(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_instructions_pending[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_operations_pending(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_operations_pending[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_unit_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_unit_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_unit_height(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_unit_height[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_span_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_span_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_span_height(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_span_height[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_region_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_height(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_region_height[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_completed_count(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_completed_count[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_element_init_data_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_element_init_data_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_global_sets(const unsigned int dev_id)
{
	return ipu_device_dma_properties_global_sets[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_linear_burst_size(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_linear_burst_size[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_block_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_block_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_block_height(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_block_height[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_padding_amount(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_padding_amount[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_instruction_queue_depth(const unsigned int dev_id)
{
	return ipu_device_dma_properties_instruction_queue_depth[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_command_queue_depth(const unsigned int dev_id)
{
	return ipu_device_dma_properties_command_queue_depth[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_request_queue_depth(const unsigned int dev_id)
{
	return ipu_device_dma_properties_request_queue_depth[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_sub_sampling_factor(const unsigned int dev_id)
{
	return ipu_device_dma_properties_max_sub_sampling_factor[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_srmd_support(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_srmd_support[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_burst_support(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_burst_support[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_max_region_stride(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_max_region_stride[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_element_precisions(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_element_precisions[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_sub_sampling_factors(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_sub_sampling_factors[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_info_width(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_info_width[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_base_address(const unsigned int dev_id)
{
	return ipu_device_dma_properties_base_address[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_data_master_data_width(const unsigned int dev_id, const unsigned int master_bank)
{
	return ipu_device_dma_properties_data_master_data_width[dev_id][master_bank];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_control_master_data_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_control_master_data_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_control_slave_data_width(const unsigned int dev_id)
{
	return ipu_device_dma_properties_control_slave_data_width[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_bank_mode_register_available(const unsigned int dev_id)
{
	return ipu_device_dma_properties_bank_mode_register_available[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_region_stride_register_available(const unsigned int dev_id)
{
	return ipu_device_dma_properties_region_stride_register_available[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_int_base_address(const unsigned int dev_id)
{
	return ipu_device_dma_properties_int_base_address[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_master_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_master_banks[dev_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_dma_global_banks(const unsigned int dev_id)
{
	return ipu_device_dma_properties_global_banks[dev_id];
}

/* Terminal descriptor */

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_datama_addr_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_datama_addr_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_datamb_addr_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_datamb_addr_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_region_origin_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_region_origin_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_region_width_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_region_width_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_region_stride_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_region_stride_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_element_setup_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_element_setup_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_cio_info_setup_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_cio_info_setup_bits[dev_id];

}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_port_mode_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_port_mode_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_terminal_desc_32bit_words(const unsigned int dev_id)
{
	return ipu_device_dma_properties_terminal_desc_32bit_words[dev_id];
}

/* Span descriptor */

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_x_coordinate_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_x_coordinate_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_y_coordinate_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_y_coordinate_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_unit_location_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_unit_location_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_column_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_column_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_row_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_row_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_width_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_width_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_height_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_height_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_mode_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_mode_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_span_desc_32bit_words(const unsigned int dev_id)
{
	return ipu_device_dma_properties_span_desc_32bit_words[dev_id];
}

/* Channel descriptor */
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_element_extend_mode_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_element_extend_mode_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_padding_mode_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_padding_mode_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_sampling_setup_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_sampling_setup_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_global_set_id_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_global_set_id_bit[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_ack_mode_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_ack_mode_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_ack_addr_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_ack_addr_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_ack_data_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_ack_data_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_completed_count_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_completed_count_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_channel_desc_32bit_words(const unsigned int dev_id)
{
	return ipu_device_dma_properties_channel_desc_32bit_words[dev_id];
}

/* Unit descriptor */
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_unit_width_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_unit_width_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_unit_height_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_unit_height_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_unit_desc_32bit_words(const unsigned int dev_id)
{
	return ipu_device_dma_properties_unit_desc_32bit_words[dev_id];

}

/* Other properties */
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_group_id_idx(const unsigned int dev_id)
{
	return ipu_device_dma_properties_group_id_idx[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_bank_id_idx(const unsigned int dev_id)
{
	return ipu_device_dma_properties_bank_id_idx[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_macro_size_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_macro_size_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_descriptor_id_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_descriptor_id_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_descriptor_id_bit_mask(const unsigned int dev_id)
{
	return ipu_device_dma_properties_descriptor_id_bit_mask[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_terminal_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_terminal_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_spans_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_spans_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_channels_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_channels_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_units_bits(const unsigned int dev_id)
{
	return ipu_device_dma_properties_units_bits[dev_id];
}

STORAGE_CLASS_INLINE unsigned int ipu_device_dma_num_of_terminal_desc(const unsigned int dev_id)
{
	return ipu_device_dma_properties_num_of_terminal_desc[dev_id];
}
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_num_of_span_desc(const unsigned int dev_id)
{
	return ipu_device_dma_properties_num_of_span_desc[dev_id];
}
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_num_of_unit_desc(const unsigned int dev_id)
{
	return ipu_device_dma_properties_num_of_unit_desc[dev_id];
}
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_num_of_physical_non_cached_channels(const unsigned int dev_id)
{
	return ipu_device_dma_properties_num_of_physical_non_cached_channels[dev_id];
}
STORAGE_CLASS_INLINE unsigned int ipu_device_dma_num_of_channel_desc(const unsigned int dev_id)
{
	return ipu_device_dma_properties_num_of_channel_desc[dev_id];
}

#endif /* __IPU_DEVICE_DMA_PROPERTIES_FUNC_H */
