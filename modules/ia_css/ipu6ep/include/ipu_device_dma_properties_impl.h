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

#ifndef __IPU_DEVICE_DMA_PROPERTIES_IMPL_H
#define __IPU_DEVICE_DMA_PROPERTIES_IMPL_H

/* Properties of the DMA devices in PSYS */
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_properties_defs.h" /* from the subsystem */
#include "ipu_device_dma_devices.h" /* from the subsystem */
#include "ipu_device_dma_properties_storage_class.h"
#include "ipu_device_dma_properties_impl_dep.h"
#include "type_support.h"

DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_nativecio_2dblock[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NATIVE_CIO2D_BLOCK)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_nativecio_raccept[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NATIVE_CIOR_ACCEPT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_nativecio_srmd[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NATIVE_CIOSRMD)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_request_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REQUEST_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_units[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_unit_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNIT_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_spans[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPANS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_terminals[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(TERMINALS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_terminal_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(TERMINAL_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_channels[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CHANNELS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_channel_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CHANNEL_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_macro_size[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_MACRO_SIZE)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_instructions_pending[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_INSTRUCTIONS_PENDING)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_operations_pending[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_OPERATIONS_PENDING)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_unit_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_UNIT_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_unit_height[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_UNIT_HEIGHT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_span_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_SPAN_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_span_height[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_SPAN_HEIGHT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_region_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_REGION_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_region_height[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_REGION_HEIGHT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_completed_count[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_COMPLETED_COUNT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_element_init_data_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ELEMENT_INIT_DATA_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_global_sets[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(GLOBAL_SETS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_linear_burst_size[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_LINEAR_BURST_SIZE)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_max_block_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_BLOCK_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_block_height[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_BLOCK_HEIGHT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_padding_amount[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_PADDING_AMOUNT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_instruction_queue_depth[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(INSTRUCTION_QUEUE_DEPTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_command_queue_depth[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(COMMAND_QUEUE_DEPTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_request_queue_depth[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REQUEST_QUEUE_DEPTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_max_sub_sampling_factor[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MAX_SUBSAMPLING_FACTOR)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_srmd_support[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(SRMD_SUPPORT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_burst_support[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(BURST_SUPPORT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint32_t ipu_device_dma_properties_max_region_stride[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(MAX_REGION_STRIDE)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_element_precisions[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(ELEMENT_PRECISIONS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_sub_sampling_factors[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(SUBSAMPLING_FACTORS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_info_width[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(INFO_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint32_t ipu_device_dma_properties_base_address[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(BASE_ADDRESS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint16_t ipu_device_dma_properties_data_master_data_width[NCI_DMA_NUM_DEVICES][IPU_DEVICE_DMA_MAX_NUM_PORTS] = {
	DMA_INSTANCES_PORT_PROPERTY(DATA_MASTER_DATA_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_control_master_data_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CONTROL_MASTER_DATA_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_control_slave_data_width[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CONTROL_SLAVE_DATA_WIDTH)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_bank_mode_register_available[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(BANK_MODE_REGISTER_AVAILABLE)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_region_stride_register_available[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REGION_STRIDE_REGISTER_AVAILABLE)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint32_t ipu_device_dma_properties_int_base_address[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(INT_BASE_ADDRESS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_master_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MASTER_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_global_banks[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(GLOBAL_BANKS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_datama_addr_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(DATAMA_ADDR_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_datamb_addr_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(DATAMB_ADDR_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_region_origin_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REGION_ORIGIN_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_region_width_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REGION_WIDTH_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_region_stride_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(REGION_STRIDE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_element_setup_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ELEMENT_SETUP_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_cio_info_setup_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CIO_INFO_SETUP_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_port_mode_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(PORT_MODE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_terminal_desc_32bit_words[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(TERMINAL_DESC_32BIT_WORDS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_x_coordinate_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(X_COORDINATE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_y_coordinate_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(Y_COORDINATE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_unit_location_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNIT_LOCATION_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_column_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_COLUMN_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_row_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_ROW_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_width_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_WIDTH_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_height_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_HEIGHT_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_mode_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_MODE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_span_desc_32bit_words[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPAN_DESC_32BIT_WORDS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_element_extend_mode_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ELEMENT_EXTEND_MODE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_padding_mode_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(PADDING_MODE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_sampling_setup_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SAMPLING_SETUP_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_global_set_id_bit[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(GLOBAL_SET_ID_BIT)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_ack_mode_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ACK_MODE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_ack_addr_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ACK_ADDR_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_ack_data_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(ACK_DATA_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_completed_count_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(COMPLETED_COUNT_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_channel_desc_32bit_words[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CHANNEL_DESC_32BIT_WORDS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_unit_width_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNIT_WIDTH_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_unit_height_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNIT_HEIGHT_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_unit_desc_32bit_words[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNIT_DESC_32BIT_WORDS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_group_id_idx[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(GROUP_ID_IDX)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_bank_id_idx[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(BANK_ID_IDX)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_macro_size_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(MACRO_SIZE_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_descriptor_id_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(DESCRIPTOR_ID_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_descriptor_id_bit_mask[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(DESCRIPTOR_ID_BIT_MASK)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_terminal_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(TERMINAL_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_spans_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(SPANS_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_channels_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(CHANNELS_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_units_bits[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(UNITS_BITS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_num_of_terminal_desc[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NUM_OF_TERMINAL_DESC)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_num_of_span_desc[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NUM_OF_SPAN_DESC)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_num_of_unit_desc[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NUM_OF_UNIT_DESC)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_num_of_physical_non_cached_channels[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NUM_OF_PHYSICAL_NON_CACHED_CHANNELS)
};
DMA_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_dma_properties_num_of_channel_desc[NCI_DMA_NUM_DEVICES] = {
	DMA_INSTANCES_PROPERTY(NUM_OF_CHANNEL_DESC)
};

#endif /* __IPU_DEVICE_DMA_PROPERTIES_IMPL_H */
