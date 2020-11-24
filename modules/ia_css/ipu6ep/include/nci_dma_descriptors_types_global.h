/*
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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_GLOBAL_H
#define __NCI_DMA_DESCRIPTORS_TYPES_GLOBAL_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"

enum nci_dma_global_rw_registers {
	NCI_DMA_GLOBAL_OFFSET_ERROR			= 0x0,
	NCI_DMA_GLOBAL_OFFSET_UNIT_DESCRIPTOR_BASE	= 0x1,
	NCI_DMA_GLOBAL_OFFSET_SPAN_DESCRIPTOR_BASE	= 0x2,
	NCI_DMA_GLOBAL_OFFSET_TERMINAL_DESCRIPTOR_BASE	= 0x3,
	NCI_DMA_GLOBAL_OFFSET_CHANNEL_DESCRIPTOR_BASE	= 0x4,
	NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_HEIGHT	= 0x5,
	NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_0_1D	= 0x6,
	NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_0_2D	= 0x7,
	NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_1_1D	= 0x8,
	NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_1_2D	= 0x9,
};
#define N_NCI_DMA_GLOBAL_RW_REGISTERS (NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_1_2D + 1)

enum nci_dma_global_descriptor_fields {
	NCI_DMA_GLOBAL_FIELDS_NONE			= (0),
	NCI_DMA_GLOBAL_FIELD_ERROR			= (1 << NCI_DMA_GLOBAL_OFFSET_ERROR),
	NCI_DMA_GLOBAL_FIELD_UNIT_DESCRIPTOR_BASE	= (1 << NCI_DMA_GLOBAL_OFFSET_UNIT_DESCRIPTOR_BASE),
	NCI_DMA_GLOBAL_FIELD_SPAN_DESCRIPTOR_BASE	= (1 << NCI_DMA_GLOBAL_OFFSET_SPAN_DESCRIPTOR_BASE),
	NCI_DMA_GLOBAL_FIELD_TERMINAL_DESCRIPTOR_BASE	= (1 << NCI_DMA_GLOBAL_OFFSET_TERMINAL_DESCRIPTOR_BASE),
	NCI_DMA_GLOBAL_FIELD_CHANNEL_DESCRIPTOR_BASE	= (1 << NCI_DMA_GLOBAL_OFFSET_CHANNEL_DESCRIPTOR_BASE),
	NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_HEIGHT	= (1 << NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_HEIGHT),
	NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_0_1D	= (1 << NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_0_1D),
	NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_0_2D	= (1 << NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_0_2D),
	NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_1_1D	= (1 << NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_1_1D),
	NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_1_2D	= (1 << NCI_DMA_GLOBAL_OFFSET_MAX_PHYSICAL_BLOCK_W_1_2D),
	NCI_DMA_GLOBAL_FIELDS_ALL			= (NCI_DMA_GLOBAL_FIELD_ERROR				+
								NCI_DMA_GLOBAL_FIELD_UNIT_DESCRIPTOR_BASE	+
								NCI_DMA_GLOBAL_FIELD_SPAN_DESCRIPTOR_BASE	+
								NCI_DMA_GLOBAL_FIELD_TERMINAL_DESCRIPTOR_BASE	+
								NCI_DMA_GLOBAL_FIELD_CHANNEL_DESCRIPTOR_BASE	+
								NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_HEIGHT	+
								NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_0_1D	+
								NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_0_2D	+
								NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_1_1D	+
								NCI_DMA_GLOBAL_FIELD_MAX_PHYSICAL_BLOCK_W_1_2D),
};

#define NCI_DMA_MAX_GLOBAL_SETS 4

struct nci_dma_global_physical_block_width {
	uint32_t max_1d_block_width;
	uint32_t max_2d_block_width;
};

struct nci_dma_public_global_descriptor {
	uint32_t unit_descriptor_base_address;
	uint32_t span_descriptor_base_address;
	uint32_t terminal_descriptor_base_address;
	uint32_t channel_descriptor_base_address;
	uint32_t maximum_physical_block_height;
	struct nci_dma_global_physical_block_width maximum_physical_block_width[NCI_DMA_MAX_GLOBAL_SETS];
};

/*
* Non cached span descriptor for:
* - DMA_FW (read the Note below)
* - DMA_EXT0
* - DMA_EXT1R
* - DMA_EXT1W
* - DMA_INT
* - DMA_ISA
*
* Note: There should be 2 different types of non_cached_global_descriptor since DMA_FW does not have
*       max_1d_block_width_1/max_2d_block_width_1.
*       Since the address map has no other register after that I decided to keep one structure to
	simplify the FW and write to non-existing registers in the DMA_FW case.
	The write should be simply discarded by the HW.
*/
struct nci_dma_non_cached_global_descriptor {		/* register_name		register index	access (R/W)*/
	uint32_t error;					/* error				0x0		R/W */
	uint32_t idle;					/* idle					0x1		R   */
	uint32_t unit_descriptor_base_address;		/* unit_descriptor_base_address		0x2		R/W */
	uint32_t span_descriptor_base_address;		/* span_descriptor_base_address		0x3		R/W */
	uint32_t terminal_descriptor_base_address;	/* terminal_descriptor_base_address	0x4		R/W */
	uint32_t channel_descriptor_base_address;	/* channel_descriptor_base_address	0x5		R/W */
	uint32_t maximum_physical_block_height;		/* maximum_physical_block_height	0x6		R/W */
	uint32_t max_1d_block_width_0;			/* max_1d_block_width(set 0)		0x7		R/W */
	uint32_t max_2d_block_width_0;			/* max_2d_block_width(set 0)		0x8		R/W */
	uint32_t max_1d_block_width_1;			/* max_1d_block_width(set 1)		0x9		R/W */
	uint32_t max_2d_block_width_1;			/* max_2d_block_width(set 1)		0xA		R/W */
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_global_descriptor(
	const struct nci_dma_public_global_descriptor * const public_global_descriptor,
	struct nci_dma_non_cached_global_descriptor * const non_cached_global_descriptor)
{
	IA_CSS_ASSERT(NULL != public_global_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_global_descriptor);

	non_cached_global_descriptor->error = 0;
	non_cached_global_descriptor->idle = 0;
	non_cached_global_descriptor->unit_descriptor_base_address =
		public_global_descriptor->unit_descriptor_base_address;
	non_cached_global_descriptor->span_descriptor_base_address =
		public_global_descriptor->span_descriptor_base_address;
	non_cached_global_descriptor->terminal_descriptor_base_address =
		public_global_descriptor->terminal_descriptor_base_address;
	non_cached_global_descriptor->channel_descriptor_base_address =
		public_global_descriptor->channel_descriptor_base_address;
	non_cached_global_descriptor->maximum_physical_block_height =
		public_global_descriptor->maximum_physical_block_height;
	non_cached_global_descriptor->max_1d_block_width_0 =
		public_global_descriptor->maximum_physical_block_width[0].max_1d_block_width;
	non_cached_global_descriptor->max_2d_block_width_0 =
		public_global_descriptor->maximum_physical_block_width[0].max_2d_block_width;
	non_cached_global_descriptor->max_1d_block_width_1 =
		public_global_descriptor->maximum_physical_block_width[1].max_1d_block_width;
	non_cached_global_descriptor->max_2d_block_width_1 =
		public_global_descriptor->maximum_physical_block_width[1].max_2d_block_width;

	return sizeof(*non_cached_global_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_GLOBAL_H */
