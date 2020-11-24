/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2020 Intel Corporation.
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

#ifndef __IA_CSS_TPROXY_GLOBAL_H
#define __IA_CSS_TPROXY_GLOBAL_H

#include "vied_nci_dma_types.h"
#include "ia_css_tproxy_global_dep.h"

#define IA_CSS_N_TPROXY_EVENTS		64

#define IA_CSS_TPROXY_CTRL_DATA_BITS	32
#define IA_CSS_TPROXY_CTRL_DATA_BYTES	4
#define IA_CSS_TPROXY_CTRL_DATA_BYTES_MASK	0x0003

#define IA_CSS_TPROXY_UNIT_CORRECTION 1
#define IA_CSS_TPROXY_SPAN_CORRECTION 1
#define IA_CSS_TPROXY_REGION_WIDTH_CORRECTION 1

#if (HAS_DEC400 == 1)
#define IA_CSS_TPROXY_MAX_DEC400_FLUSHES_PER_FRAME 5 /* 3 for PSA out streams, 2 for TNR out streams */
#endif

typedef uint32_t ia_css_tproxy_chan_id_t;

typedef uint32_t ia_css_tproxy_chan_handle_t;

typedef struct nci_dma_terminal_desc ia_css_tproxy_terminal_desc_t;
typedef struct nci_dma_span_desc ia_css_tproxy_span_desc_t;
typedef struct nci_dma_unit_desc ia_css_tproxy_unit_desc_t;

/* TODO: A dummy cricular buffer structure */
struct ia_css_tproxy_buf_info {
	uint32_t tail_addr;
	uint8_t size;
	uint8_t tail;
	uint8_t crop;
	uint8_t padding;
};

struct ia_css_tproxy_transfer_config {
	struct nci_dma_channel_desc channel;
	struct ia_css_tproxy_buf_info buf_info;
	uint32_t dpcm_channel_config;
	enum ia_css_tproxy_dma_requestor_cell_id requestor_id;
};

enum ia_css_tproxy_terminal {
	IA_CSS_TPROXY_TERMINAL_A = NCI_DMA_TERMINAL_A,
	IA_CSS_TPROXY_TERMINAL_B = NCI_DMA_TERMINAL_B,
	IA_CSS_N_TPROXY_TERMINALS
};

enum ia_css_tproxy_extend_mode {
	IA_CSS_TPROXY_EXTEND_MODE_ZERO = NCI_DMA_EXTEND_MODE_ZERO,
	IA_CSS_TPROXY_EXTEND_MODE_SIGN = NCI_DMA_EXTEND_MODE_SIGN
};

enum ia_css_tproxy_padding_mode {
	IA_CSS_TPROXY_PADDING_CONSTANT = NCI_DMA_PADDING_MODE_CONSTANT,
	IA_CSS_TPROXY_PADDING_CLONE = NCI_DMA_PADDING_MODE_CLONE,
	IA_CSS_TPROXY_PADDING_MIRROR = NCI_DMA_PADDING_MODE_MIRROR,
	IA_CSS_TPROXY_PADDING_APPEND = NCI_DMA_PADDING_MODE_APPEND,
	IA_CSS_TPROXY_PADDING_TRUNCATE = NCI_DMA_PADDING_MODE_TRUNCATE
};

enum ia_css_tproxy_span_order {
	IA_CSS_TPROXY_SPAN_ORDER_ROW_FIRST = NCI_DMA_SPAN_ORDER_ROW_FIRST,
	IA_CSS_TPROXY_SPAN_ORDER_COL_FIRST = NCI_DMA_SPAN_ORDER_COLUMN_FIRST
};

enum ia_css_tproxy_addressing_mode {
	IA_CSS_TPROXY_ADDRESSING_BYTE_ADDRESS_BASED =
		NCI_DMA_ADDRESSING_MODE_BYTE_ADDRESS_BASED,
	IA_CSS_TPROXY_ADDRESSING_COORDINATE_ADDRESS_BASED =
		NCI_DMA_ADDRESSING_MODE_COORDINATE_BASED
};

enum ia_css_tproxy_ack_mode {
	IA_CSS_TPROXY_ACK_MODE_PASSIVE,
	IA_CSS_TPROXY_ACK_MODE_ACTIVE,
	IA_CSS_TPROXY_ACK_MODE_NO_ACK
};

enum ia_css_tproxy_transfer_command {
	IA_CSS_TPROXY_COMMAND_FILL_B = NCI_DMA_COMMAND_FILL_B,
	IA_CSS_TPROXY_COMMAND_FILL_B_ACK = NCI_DMA_COMMAND_FILL_B_ACK,
	IA_CSS_TPROXY_COMMAND_FILL_B_FLUSH_ACK = NCI_DMA_COMMAND_FILL_B_FLUSH_ACK,
	IA_CSS_TPROXY_COMMAND_FILL_A = NCI_DMA_COMMAND_FILL_A,
	IA_CSS_TPROXY_COMMAND_FILL_A_ACK = NCI_DMA_COMMAND_FILL_A_ACK,
	IA_CSS_TPROXY_COMMAND_FILL_A_FLUSH_ACK = NCI_DMA_COMMAND_FILL_A_FLUSH_ACK,
	IA_CSS_TPROXY_COMMAND_MOVE_AB = NCI_DMA_COMMAND_MOVE_AB,
	IA_CSS_TPROXY_COMMAND_MOVE_AB_ACK = NCI_DMA_COMMAND_MOVE_AB_ACK,
	IA_CSS_TPROXY_COMMAND_MOVE_AB_FLUSH_ACK = NCI_DMA_COMMAND_MOVE_AB_FLUSH_ACK,
	IA_CSS_TPROXY_COMMAND_MOVE_BA = NCI_DMA_COMMAND_MOVE_BA,
	IA_CSS_TPROXY_COMMAND_MOVE_BA_ACK = NCI_DMA_COMMAND_MOVE_BA_ACK,
	IA_CSS_TPROXY_COMMAND_MOVE_BA_FLUSH_ACK = NCI_DMA_COMMAND_MOVE_BA_FLUSH_ACK
};

struct ia_css_tproxy_terminal_config {
	struct nci_dma_terminal_desc terminal[IA_CSS_N_TPROXY_TERMINALS];
	struct nci_dma_span_desc span[IA_CSS_N_TPROXY_TERMINALS];
	struct nci_dma_unit_desc unit;
	uint32_t dpcm_buffer_config;
	uint32_t command;
};

struct ia_css_tproxy_memcpy_config {
	struct nci_dma_memcpy_config native_config;
	uint32_t command;
};

struct ia_css_tproxy_memcpy_2d_config {
	struct nci_dma_memcpy_2d_config native_config;
	uint32_t command;
};

#if (HAS_DMA_FAST_1D_INT == 1)
struct ia_css_tproxy_memcpy_1d_config {
	/* unit width */
	uint32_t unit_width;
	/* command */
	uint32_t command;
	/* addresses */
	uint32_t address_a;
	uint32_t address_b;
};
#else
struct ia_css_tproxy_memcpy_1d_config {
	/* partial terminal */
	uint32_t region_origin[NCI_DMA_MAX_NUM_PORTS];
	uint32_t region_width[NCI_DMA_MAX_NUM_PORTS];
	/* partial span */
	uint32_t x_coordinate[NCI_DMA_MAX_NUM_PORTS];
	/* partial unit */
	struct nci_dma_unit_desc unit_desc;
	/* command */
	uint32_t command;
};
#endif

enum ia_css_tproxy_range_otf_index {
	OPERATION_RANGE_OTF_MAIN_OFFLINE = MAX_NUM_EXEC_PROCESS_PER_PROC_GRP,
	OPERATION_RANGE_OTF_MAIN_BUF_CHASING,
	OPERATION_RANGE_OTF_MAIN_ONLINE,
	OPERATION_RANGE_OTF_PDAF_OFFLINE,
	OPERATION_RANGE_OTF_PDAF_BUF_CHASING,
	OPERATION_RANGE_OTF_PDAF_ONLINE,
	IA_CSS_TPROXY_OTF_MAX_RANGE_SIZE
};

/*
 * TPROXY init-time improvment
 */
#define MAX_PRIORITY_TYPES 2
#define OPERATION_RANGE_SIZE_MASK 0x7FFF /* Use in order to set/get operation range size */
#define OPERATION_RANGE_IS_FINAL_MASK 0x8000 /* Use in order to set/get operation range is_final */
#define OPERATION_RANGE_DEFAULT_IDX 0 /* In not pipeline case, all transfers are assigned to entry 0 at the operation_range queue */
#define MAX_OPERATION_RANGE_SIZE IA_CSS_TPROXY_OTF_MAX_RANGE_SIZE
#define OPERATION_RANGE_SYNC_ENTRY_IDX (MAX_NUM_EXEC_PROCESS_PER_PROC_GRP - 1) /* SYNC range entry index */

typedef enum ia_css_tproxy_operation_q_services {
	IA_CSS_TPROXY_PG_0_CONFIGURATION,
	IA_CSS_TPROXY_PG_0_CACHED_OUT,
	IA_CSS_TPROXY_PG_1_CONFIGURATION,
	IA_CSS_TPROXY_PG_1_CACHED_OUT,
	IA_CSS_TPROXY_OPERATION_Q_NUM
} tproxy_q_service;

typedef enum ia_css_tproxy_operation_q_types {
	IA_CSS_TPROXY_MANUAL_XMEM_TO_CMEM,
	IA_CSS_TPROXY_DMA_XMEM_TO_CMEM,
	IA_CSS_TPROXY_MANUAL_CMEM_TO_XMEM,
	IA_CSS_TPROXY_DMA_CMEM_TO_XMEM,
	IA_CSS_TPROXY_FILL_IMMEDIATE_TO_CMEM,
	IA_CSS_TPROXY_BITWISE_OR_CMEM_WITH_IMMEDIATE, /* read from CMEM, modify and write to CMEM a new value */
	IA_CSS_TPROXY_MANUAL_CMEM_TO_CMEM,
	IA_CSS_TPROXY_SYNC,
	IA_CSS_TPROXY_SYNC_AND_ACK,
	IA_CSS_TPROXY_COPY_FROM_IMR,
#if HAS_VP
	IA_CSS_TPROXY_LOAD_IMAGE,
#endif
	IA_CSS_TPROXY_OPERATION_Q_TYPES_NUM
} tproxy_operation_q_types;

typedef union payload_data_union {
	uint32_t single_value;
	uint32_t cmem_address;
	uint32_t external_address;
	uint32_t ack_data; /* for sync ack */
} payload_data;

typedef struct ia_css_tproxy_operation_q_open_s {
	uint32_t operation_q_ptr;
} operation_q_open_t;

typedef struct ia_css_tproxy_operation_q_config_s {
	uint32_t priority;
} operation_q_config_t;

typedef struct ia_css_tproxy_operation_q_start_s {
	uint32_t operation_id;
} operation_q_start_t;

typedef struct ia_css_tproxy_operation_q_entry_s {
	uint32_t internal_address;
	payload_data payload;
	tproxy_operation_q_types operation_type;
	uint32_t size; /* TODO 16bit */
} tproxy_operation_q_t;

typedef struct ia_css_tproxy_operation_q_range_s {
	uint16_t start_index;
	uint16_t size_and_is_final; /* 15bit belong to size and the last one be for is_final */
} tproxy_operation_q_range_t;

struct ia_css_tproxy_operation_queue {
	uint16_t total_operations_count;
	uint16_t max_num_of_operations;
	bool is_q_final;
	tproxy_operation_q_t *operation_entries;
	tproxy_operation_q_range_t operation_range[MAX_OPERATION_RANGE_SIZE];
};

#endif /* __IA_CSS_TPROXY_GLOBAL_H */
