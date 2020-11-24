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

#ifndef __VIED_NCI_DMA_GLOBAL_DEFS_H
#define __VIED_NCI_DMA_GLOBAL_DEFS_H

#define NCI_DMA_MAX_MASTER_BANKS	2
#define NCI_DMA_MAX_NUM_PORTS	2
#define NCI_DMA_MAX_GLOBAL_SETS	4

/* PS address width is always 32bits */
#define NCI_DMA_CTRLM_DATA_BITS	0x20
#define NCI_DMA_CTRLM_ADDR_BITS	0x20
#define NCI_DMA_DATAMB_ADDR_BITS	0x20
#define NCI_DMA_DATAMA_ADDR_BITS	0x20

#define NCI_DMA_PORT_MODE_BITS			1
#define NCI_DMA_SPAN_MODE_BITS			2
#define NCI_DMA_DESCRIPTOR_KIND_BITS		2
#define NCI_DMA_ELEMENT_EXTEND_MODE_BITS 1
#define NCI_DMA_ACK_ADDR_BITS	(NCI_DMA_CTRLM_ADDR_BITS)
#define NCI_DMA_ACK_DATA_BITS	(NCI_DMA_CTRLM_DATA_BITS)
#define NCI_DMA_PADDING_MODE_BITS		3

#define NCI_DMA_CTRLS_DATA_BYTES	4
#define NCI_DMA_CTRLM_DATA_BYTES	4

#define NCI_DMA_MAX_DESC_WORDS	4

#define NCI_DMA_EXECUTION_INVAL_MODIFIER_BITS	0x6
#define NCI_DMA_EXECUTION_COMMAND_BITS		0x4
#define NCI_DMA_EXECUTION_FORMAT_BITS		0x1
#define NCI_DMA_REQUEST_EXECUTION_FORMAT		0x0
#define NCI_DMA_REQUEST_INVALIDATION_FORMAT	0x1

#define NCI_DMA_REQUEST_FORMAT_BIT			0x0
#define NCI_DMA_REQUEST_TRANSFER_DIRECTION_BIT	0x3

#define NCI_DMA_SPAN_ADDRESSING_MODE_BIT		0x0
#define NCI_DMA_SPAN_SPAN_ORDER_BIT		0x1

#define NCI_DMA_MIXED_MAX_NO_OF_CACHED_DESCRIPTORS 6

/* These macros are set to MAX values that a 32 bit register can take.
 * The NCI implementation will set the global registers to the maximum
 * allowed value per instance (available as Device specific properties)
 * if these values are passed during the device configuraiton.
 * Else registers will be set with the configuration passed by the user.
 */
#define NCI_DMA_USE_DEFAULT_BLOCK_HEIGHT		0xFFFFFFFF
#define NCI_DMA_USE_DEFAULT_1D_BLOCK_WIDTH		0xFFFFFFFF
#define NCI_DMA_USE_DEFAULT_2D_BLOCK_WIDTH		0xFFFFFFFF

#endif /* __VIED_NCI_DMA_GLOBAL_DEFS_H */
