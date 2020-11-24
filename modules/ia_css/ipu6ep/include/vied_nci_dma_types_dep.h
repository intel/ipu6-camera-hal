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

#ifndef __VIED_NCI_DMA_TYPES_DEP_H
#define __VIED_NCI_DMA_TYPES_DEP_H

#include "type_support.h"
#include "storage_class.h"

enum nci_dma_init_data {
	NCI_DMA_INIT_DATA_ZERO,
	NCI_DMA_INIT_DATA_ONE
};
#define N_NCI_DMA_INIT_DATA 0x2

enum nci_dma_extend_mode {
	NCI_DMA_EXTEND_MODE_ZERO,
	NCI_DMA_EXTEND_MODE_SIGN
};
#define N_NCI_DMA_EXTEND_MODE 0x2

enum nci_dma_sign_bit_toggle {
	NCI_DMA_SIGN_BIT_TOGGLE_ZERO,
	NCI_DMA_SIGN_BIT_TOGGLE_ONE
};
#define N_NCI_DMA_SIGN_BIT_TOGGLE 0x2

enum nci_dma_padding_mode {
	NCI_DMA_PADDING_MODE_CONSTANT,
	NCI_DMA_PADDING_MODE_CLONE,
	NCI_DMA_PADDING_MODE_MIRROR,
	NCI_DMA_PADDING_MODE_APPEND,
	NCI_DMA_PADDING_MODE_TRUNCATE
};
#define N_NCI_DMA_PADDING_MODE 0x5

enum nci_dma_constant_value {
	NCI_DMA_CONSTANT_VALUE_ZERO		=	NCI_DMA_INIT_DATA_ZERO << 2 |
							NCI_DMA_EXTEND_MODE_ZERO << 1 |
							NCI_DMA_SIGN_BIT_TOGGLE_ZERO,
	NCI_DMA_CONSTANT_VALUE_ONE		=	NCI_DMA_INIT_DATA_ONE << 2 |
							NCI_DMA_EXTEND_MODE_ZERO << 1 |
							NCI_DMA_SIGN_BIT_TOGGLE_ZERO,
	NCI_DMA_CONSTANT_VALUE_MAX_UNSIGNED	=	NCI_DMA_INIT_DATA_ONE << 2 |
							NCI_DMA_EXTEND_MODE_SIGN << 1 |
							NCI_DMA_SIGN_BIT_TOGGLE_ZERO,
	NCI_DMA_CONSTANT_VALUE_MAX_SIGNED	=	NCI_DMA_INIT_DATA_ONE << 2 |
							NCI_DMA_EXTEND_MODE_SIGN << 1 |
							NCI_DMA_SIGN_BIT_TOGGLE_ONE,
	NCI_DMA_CONSTANT_VALUE_MIN_SIGNED	=	NCI_DMA_INIT_DATA_ZERO << 2 |
							NCI_DMA_EXTEND_MODE_ZERO << 1 |
							NCI_DMA_SIGN_BIT_TOGGLE_ONE,
};

#define NCI_DMA_CHANNEL_ACK_ADDRESS_SHIFT 0
#define NCI_DMA_CHANNEL_ACK_DATA_SHIFT 0
#define NCI_DMA_CHANNEL_ELEMENT_INIT_DATA_SHIFT 5
#define NCI_DMA_CHANNEL_ELEMENT_SIGN_EXTEND_SHIFT 4
#define NCI_DMA_CHANNEL_ELEMENT_SIGN_TOGGLE_VALUE_SHIFT 3
#define NCI_DMA_CHANNEL_PADDING_MODE_SHIFT 0
#define NCI_DMA_CHANNEL_REPLACEMENT_POLICY_SHIFT 8
#define NCI_DMA_CHANNEL_GLOBAL_SET_ID_SHIFT 16
#define NCI_DMA_CHANNEL_ACK_MODE_SHIFT 24
#define NCI_DMA_CHANNEL_COMPLETED_COUNT_SHIFT 0

#define NCI_DMA_CHANNEL_ACK_ADDRESS_BITS 32
#define NCI_DMA_CHANNEL_ACK_DATA_BITS 32
#define NCI_DMA_CHANNEL_ELEMENT_INIT_DATA_BITS 1
#define NCI_DMA_CHANNEL_ELEMENT_SIGN_EXTEND_BITS 1
#define NCI_DMA_CHANNEL_ELEMENT_SIGN_TOGGLE_VALUE_BITS 1
#define NCI_DMA_CHANNEL_PADDING_MODE_BITS 3
#define NCI_DMA_CHANNEL_REPLACEMENT_POLICY_BITS 2
#define NCI_DMA_CHANNEL_GLOBAL_SET_ID_BITS 8
#define NCI_DMA_CHANNEL_ACK_MODE_BITS 8
#define NCI_DMA_CHANNEL_COMPLETED_COUNT_BITS 8

/* TODO: remove */
enum nci_dma_sampling_setup {
	NCI_DMA_SAMPLING_SETUP_FACTOR_1 = 0,
	NCI_DMA_SAMPLING_SETUP_FACTOR_2 = 0,
	NCI_DMA_SAMPLING_SETUP_FACTOR_4 = 0
};
#define N_NCI_DMA_SAMPLING_SETUP 0

#define NCI_DMA_SPAN_UNIT_LOCATION_SHIFT 0
#define NCI_DMA_SPAN_COLUMN_SHIFT 16
#define NCI_DMA_SPAN_ROW_SHIFT 0
#define NCI_DMA_SPAN_WIDTH_SHIFT 16
#define NCI_DMA_SPAN_HEIGHT_SHIFT 0
#define NCI_DMA_SPAN_MODE_SHIFT 0
#define NCI_DMA_SPAN_REPLACEMENT_POLICY_SHIFT 8

#define NCI_DMA_SPAN_UNIT_LOCATION_BITS 32
#define NCI_DMA_SPAN_COLUMN_BITS 16
#define NCI_DMA_SPAN_ROW_BITS 16
#define NCI_DMA_SPAN_WIDTH_BITS 16
#define NCI_DMA_SPAN_HEIGHT_BITS 16
#define NCI_DMA_SPAN_MODE_BITS 2
#define NCI_DMA_SPAN_REPLACEMENT_POLICY_BITS 2

#define NCI_DMA_TERMINAL_REGION_ORIGIN_SHIFT 0
#define NCI_DMA_TERMINAL_REGION_STRIDE_SHIFT 0
#define NCI_DMA_TERMINAL_REGION_WIDTH_SHIFT 0
#define NCI_DMA_TERMINAL_REGION_HEIGHT_SHIFT 16
#define NCI_DMA_TERMINAL_CIO_INFO_SETUP_SHIFT 0
#define NCI_DMA_TERMINAL_REPLACEMENT_POLICY_SHIFT 16
#define NCI_DMA_TERMINAL_ELEMENT_SETUP_SHIFT 24

#define NCI_DMA_TERMINAL_REGION_ORIGIN_BITS 32
#define NCI_DMA_TERMINAL_REGION_STRIDE_BITS 32
#define NCI_DMA_TERMINAL_REGION_WIDTH_BITS 16
#define NCI_DMA_TERMINAL_REGION_HEIGHT_BITS 16
#define NCI_DMA_TERMINAL_CIO_INFO_SETUP_BITS 16
#define NCI_DMA_TERMINAL_REPLACEMENT_POLICY_BITS 2
#define NCI_DMA_TERMINAL_ELEMENT_SETUP_BITS 4

#define NCI_DMA_UNIT_WIDTH_SHIFT 0
#define NCI_DMA_UNIT_HEIGHT_SHIFT 16
#define NCI_DMA_UNIT_REPLACEMENT_POLICY_SHIFT 24

#define NCI_DMA_UNIT_WIDTH_BITS 16
#define NCI_DMA_UNIT_HEIGHT_BITS 8
#define NCI_DMA_UNIT_REPLACEMENT_POLICY_BITS 2

#define NCI_DMA_MAX_ELEMENTS_FAST_1D (16*1024)
#define NCI_DMA_CHANNEL_ID_BITS_INSTRUCTION (8)
#define NCI_DMA_UNIT_WIDTH_BITS_INSTRUCTION (16)
#define NCI_DMA_EXECUTION_TYPE_BITS (3)
#define NCI_DMA_RESERVED_BITS_INSTRUCTION (8)
#define NCI_DMA_EXECUTION_TYPE_SLIM_NEXT 1
#define NCI_DMA_EXECUTION_TYPE_FAST_1D 3

struct nci_dma_channel_desc {
	uint32_t element_extend_mode;
	uint32_t element_init_data;
	uint32_t padding_mode;
	uint32_t sampling_setup;
	uint32_t global_set_id;
	uint32_t ack_mode;
	uint32_t ack_addr;
	uint32_t ack_data;
	uint32_t completed_count;
	uint32_t replacement_policy;
	uint32_t sign_bit_toggle_value;
};

struct nci_dma_span_desc {
	uint32_t unit_location;
	uint32_t span_row;
	uint32_t span_column;
	uint32_t span_width;
	uint32_t span_height;
	uint32_t span_mode;
	uint32_t replacement_policy;
	int32_t x_coordinate;
	int32_t y_coordinate;
};

struct nci_dma_terminal_desc {
	uint32_t region_origin;
	uint32_t region_width;
	uint32_t region_stride;
	uint32_t element_setup;
	uint32_t cio_info_setup;
	uint32_t port_mode;
	uint32_t region_height;
	uint32_t replacement_policy;
};

struct nci_dma_unit_desc {
	uint16_t unit_width;
	uint16_t unit_height;
	uint32_t replacement_policy;
};

#endif /* __VIED_NCI_DMA_TYPES_DEP_H */
