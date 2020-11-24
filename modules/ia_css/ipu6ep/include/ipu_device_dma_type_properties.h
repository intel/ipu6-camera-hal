/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __IPU_DEVICE_DMA_TYPE_PROPERTIES_H
#define __IPU_DEVICE_DMA_TYPE_PROPERTIES_H

#include "storage_class.h"
#include "ipu_device_dma_type_properties_dep.h"

#define IPU_DEVICE_DMA_MAX_NUM_PORTS   2
#define IPU_DEVICE_DMA_MAX_GLOBAL_SETS 2

enum ipu_device_dma_group_id {
	IPU_DEVICE_DMA_CHANNEL_GROUP_ID = 0,
	IPU_DEVICE_DMA_REQUEST_GROUP_ID,
	IPU_DEVICE_DMA_GLOBAL_GROUP_ID,
	IPU_DEVICE_DMA_MASTER_GROUP_ID,
	IPU_DEVICE_DMA_SPAN_GROUP_ID,
	IPU_DEVICE_DMA_UNIT_GROUP_ID,
	IPU_DEVICE_DMA_TERMINAL_GROUP_ID
};
#define N_IPU_DEVICE_DMA_GROUP_ID 0x7

#define IPU_DEVICE_DMA_RESERVED_CHANNEL_BANKS_NONE	0

/* Global Register */
enum ipu_device_dma_global_reg {
	IPU_DEVICE_DMA_GLOBAL_REG_ERROR = 0x0,
	IPU_DEVICE_DMA_GLOBAL_REG_IDLE,
	IPU_DEVICE_DMA_GLOBAL_REG_UNIT_BASE_ADDR,
	IPU_DEVICE_DMA_GLOBAL_REG_SPAN_BASE_ADDR,
	IPU_DEVICE_DMA_GLOBAL_REG_TERMINAL_BASE_ADDR,
	IPU_DEVICE_DMA_GLOBAL_REG_CHANNEL_BASE_ADDR,
	IPU_DEVICE_DMA_GLOBAL_REG_MAX_BLOCK_HEIGHT,
	/* IPU_DEVICE_DMA_GLOBAL_REG_BLOCK_WIDTH_1DBURST_GLOBAL_SET_ID_1, */
	/* IPU_DEVICE_DMA_GLOBAL_REG_BLOCK_WIDTH_1DBURST_GLOBAL_SET_ID_1, */
	/* ... */
	/* IPU_DEVICE_DMA_GLOBAL_REG_BLOCK_WIDTH_1DBURST_GLOBAL_SET_ID_GLOBAL_SETS, */
	/* IPU_DEVICE_DMA_GLOBAL_REG_BLOCK_WIDTH_1DBURST_GLOBAL_SET_ID_GLOBAL_SETS, */
};
#define IPU_DEVICE_DMA_REG_MAX_BLOCK_WIDTH_1DBURST(global_set_id) \
	(IPU_DEVICE_DMA_GLOBAL_REG_MAX_BLOCK_HEIGHT + 1 + (2 * (global_set_id)))
#define IPU_DEVICE_DMA_REG_MAX_BLOCK_WIDTH_2DBURST(global_set_id) \
	(IPU_DEVICE_DMA_REG_MAX_BLOCK_WIDTH_1DBURST((global_set_id)) + 1)

#define N_IPU_DEVICE_DMA_GLOBAL_REG(dev_id) \
	(IPU_DEVICE_DMA_GLOBAL_REG_MAX_BLOCK_HEIGHT + 1 + (2 * (ipu_device_dma_global_sets((dev_id)))))

/* Master Register */
enum ipu_device_dma_master_reg {
	IPU_DEVICE_DMA_MASTER_REG_SRMD_SUPPORT = 0x0,
	IPU_DEVICE_DMA_MASTER_REG_BURST_SUPPORT = 0x2,
	IPU_DEVICE_DMA_MASTER_REG_MAX_PHYSICAL_STRIDE,
};
#define N_IPU_DEVICE_DMA_MASTER_REG 0x4

/* Unit Register */
enum ipu_device_dma_unit_reg {
	/* Descriptor information */
	IPU_DEVICE_DMA_UNIT_REG_UNIT_WIDTH = 0x0,
	IPU_DEVICE_DMA_UNIT_REG_UNIT_HEIGHT,
	/* Bank Status */
	IPU_DEVICE_DMA_UNIT_REG_PENDING_COUNTER = 0xC,
	IPU_DEVICE_DMA_UNIT_REG_LOCK_STATUS,
	/* Bank Control */
	IPU_DEVICE_DMA_UNIT_REG_BANK_MODE = 0xF,
};
#define N_IPU_DEVICE_DMA_UNIT_REG 0x10

/* Span Register */
enum ipu_device_dma_span_reg {
	/* Descriptor information */
	IPU_DEVICE_DMA_SPAN_REG_UNIT_LOCATION = 0x0,
	IPU_DEVICE_DMA_SPAN_REG_SPAN_COLUMN,
	IPU_DEVICE_DMA_SPAN_REG_SPAN_ROW,
	IPU_DEVICE_DMA_SPAN_REG_SPAN_WIDTH,
	IPU_DEVICE_DMA_SPAN_REG_SPAN_HEIGHT,
	IPU_DEVICE_DMA_SPAN_REG_SPAN_MODE,
	/* Bank Status */
	IPU_DEVICE_DMA_SPAN_REG_PENDING_COUNTER = 0xC,
	IPU_DEVICE_DMA_SPAN_REG_LOCK_STATUS,
	IPU_DEVICE_DMA_SPAN_REG_DIRTY_STATUS,
	/* Bank Control */
	IPU_DEVICE_DMA_SPAN_REG_BANK_MODE,
};
#define N_IPU_DEVICE_DMA_SPAN_REG 0x10

/* Terminal Register */
enum ipu_device_dma_terminal_reg {
	/* Descriptor information */
	IPU_DEVICE_DMA_TERMINAL_REG_REGION_ORIGIN = 0x0,
	IPU_DEVICE_DMA_TERMINAL_REG_REGION_WIDTH,
	IPU_DEVICE_DMA_TERMINAL_REG_REGION_STRIDE,
	IPU_DEVICE_DMA_TERMINAL_REG_ELEMENT_SETUP,
	IPU_DEVICE_DMA_TERMINAL_REG_CIO_INFO,
	IPU_DEVICE_DMA_TERMINAL_REG_PORT_MODE,
	IPU_DEVICE_DMA_TERMINAL_REG_REGION_HEIGHT,
	/* Bank Status */
	IPU_DEVICE_DMA_TERMINAL_REG_PENDING_COUNTER = 0xC,
	IPU_DEVICE_DMA_TERMINAL_REG_LOCK_STATUS,
	/* Bank Control */
	IPU_DEVICE_DMA_TERMINAL_REG_BANK_MODE = 0xF,
};
#define N_IPU_DEVICE_DMA_TERMINAL_REG 0x10

/* Channel Register */
enum ipu_device_dma_channel_reg {
	/* Descriptor information */
	IPU_DEVICE_DMA_CHANNEL_REG_ELEMENT_EXTEND_MODE = 0x0,
	IPU_DEVICE_DMA_CHANNEL_REG_ELEMENT_INIT_DATA,
	IPU_DEVICE_DMA_CHANNEL_REG_PADDING_MODE,
	IPU_DEVICE_DMA_CHANNEL_REG_SAMPLING_SETUP,
	IPU_DEVICE_DMA_CHANNEL_REG_GLOBAL_SET_ID,
	IPU_DEVICE_DMA_CHANNEL_REG_ACK_MODE,
	IPU_DEVICE_DMA_CHANNEL_REG_ACK_ADDRESS,
	IPU_DEVICE_DMA_CHANNEL_REG_ACK_DATA,
	/* Bank Status */
	IPU_DEVICE_DMA_CHANNEL_REG_COMPLETED_COUNTER = 0xB,
	IPU_DEVICE_DMA_CHANNEL_REG_PENDING_COUNTER,
	IPU_DEVICE_DMA_CHANNEL_REG_LOCK_STATUS,
	IPU_DEVICE_DMA_CHANNEL_REG_DIRTY_STATUS,
	/* Bank Control */
	IPU_DEVICE_DMA_CHANNEL_REG_BANK_MODE,
};
#define N_IPU_DEVICE_DMA_CHANNEL_REG 0x10

enum nci_dma_port {
	NCI_DMA_PORT_A = 0,
	NCI_DMA_PORT_B = 1,
};
#define N_NCI_DMA_PORT (NCI_DMA_PORT_B + 1)

enum nci_dma_bank_mode {
	NCI_DMA_BANK_MODE_NON_CACHED,
	NCI_DMA_BANK_MODE_CACHED,
};
#define N_NCI_DMA_BANK_MODE 0x2

enum nci_dma_command {
	NCI_DMA_COMMAND_FILL_B = 0x0,
	NCI_DMA_COMMAND_FILL_B_ACK = 0x1,
	NCI_DMA_COMMAND_FILL_B_FLUSH_ACK = 0x3,
	NCI_DMA_COMMAND_FILL_A = 0x4,
	NCI_DMA_COMMAND_FILL_A_ACK = 0x5,
	NCI_DMA_COMMAND_FILL_A_FLUSH_ACK = 0x7,
	NCI_DMA_COMMAND_MOVE_AB = 0x8,
	NCI_DMA_COMMAND_MOVE_AB_ACK = 0x9,
	NCI_DMA_COMMAND_MOVE_AB_FLUSH_ACK = 0xb,
	NCI_DMA_COMMAND_MOVE_BA = 0xc,
	NCI_DMA_COMMAND_MOVE_BA_ACK = 0xd,
	NCI_DMA_COMMAND_MOVE_BA_FLUSH_ACK = 0xf
};
#define N_NCI_DMA_COMMAND 0x10

#define NCI_DMA_DESCRIPTOR_KIND_BITS            2
#define NCI_DMA_EXECUTION_FORMAT_BITS           0x1
#define NCI_DMA_REQUEST_INVALIDATION_FORMAT     0x1

enum nci_dma_descriptor_kind {
	NCI_DMA_DESCRIPTOR_KIND_CHANNEL,
	NCI_DMA_DESCRIPTOR_KIND_TERMINAL,
	NCI_DMA_DESCRIPTOR_KIND_UNIT,
	NCI_DMA_DESCRIPTOR_KIND_SPAN
};
#define N_NCI_DMA_DESCRIPTOR_KIND (NCI_DMA_DESCRIPTOR_KIND_SPAN + 1)

/* Ensure nof format types is not exceeding the nof format bitmap bits */
#if (N_NCI_DMA_DESCRIPTOR_KIND > (1 << NCI_DMA_DESCRIPTOR_KIND_BITS))
#error "Number of defined DMA descriptor kinds is exceeding the number of DMA descriptor kind bits"
#endif

enum nci_dma_requestor_id {
	NCI_DMA_REQUESTOR_ID_0 = 0x0,
	NCI_DMA_REQUESTOR_ID_1,
	NCI_DMA_REQUESTOR_ID_2,
	NCI_DMA_REQUESTOR_ID_3,
	NCI_DMA_REQUESTOR_ID_4,
	NCI_DMA_REQUESTOR_ID_5,
	NCI_DMA_REQUESTOR_ID_6,
	NCI_DMA_REQUESTOR_ID_7,
	NCI_DMA_REQUESTOR_ID_8,
	NCI_DMA_REQUESTOR_ID_9,
	NCI_DMA_REQUESTOR_ID_A,
	NCI_DMA_REQUESTOR_ID_B,
	NCI_DMA_REQUESTOR_ID_C,
	NCI_DMA_REQUESTOR_ID_D,
	NCI_DMA_REQUESTOR_ID_E,
	NCI_DMA_REQUESTOR_ID_F,
};
#define NCI_DMA_PSYS_NUM_REQUESTOR_ID 0x08
#define NCI_DMA_ISYS_NUM_REQUESTOR_ID 0x10

enum nci_dma_caching_policy {
	NCI_DMA_CACHING_POLICY_NORMAL = 0x0, /* decriptors will be reused by next operation, not guanranteed */
	NCI_DMA_CACHING_POLICY_PIN = 0x1,    /* descriptors should be kept in cache (the channel will be locked) */
	NCI_DMA_CACHING_POLICY_USE_AND_INVALIDATE = 0x2 /* Use the descriptor once and invalidate without write back */
};

#define IPU_DEVICE_DMA_REG_REG0	0x0 /* A dummy register for memory store/load */

#define IPU_DEVICE_DMA_WORD_POSITION_BYTE_1  8
#define IPU_DEVICE_DMA_WORD_POSITION_BYTE_2 16
#define IPU_DEVICE_DMA_WORD_POSITION_BYTE_3 24

#endif /* __IPU_DEVICE_DMA_TYPE_PROPERTIES_H */
