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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_SPAN_H
#define __NCI_DMA_DESCRIPTORS_TYPES_SPAN_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "math_support.h"

enum nci_dma_span_rw_registers {
	NCI_DMA_SPAN_OFFSET_UNIT_LOCATION	= 0x0,
	NCI_DMA_SPAN_OFFSET_SPAN_COLUMN		= 0x1,
	NCI_DMA_SPAN_OFFSET_SPAN_ROW		= 0x2,
	NCI_DMA_SPAN_OFFSET_SPAN_WIDTH		= 0x3,
	NCI_DMA_SPAN_OFFSET_SPAN_HEIGHT		= 0x4,
	NCI_DMA_SPAN_OFFSET_SPAN_MODE		= 0x5,
	NCI_DMA_SPAN_OFFSET_REPLACEMENT_POLICY	= 0x6,
	NCI_DMA_SPAN_OFFSET_BANK_MODE		= 0x7,
};
#define N_NCI_DMA_SPAN_RW_REGISTERS (NCI_DMA_SPAN_FIELD_BANK_MODE + 1)

enum nci_dma_span_descriptor_fields {
	NCI_DMA_SPAN_FIELDS_NONE		= (0),
	NCI_DMA_SPAN_FIELD_UNIT_LOCATION	= (1 << NCI_DMA_SPAN_OFFSET_UNIT_LOCATION),
	NCI_DMA_SPAN_FIELD_SPAN_COLUMN		= (1 << NCI_DMA_SPAN_OFFSET_SPAN_COLUMN),
	NCI_DMA_SPAN_FIELD_SPAN_ROW		= (1 << NCI_DMA_SPAN_OFFSET_SPAN_ROW),
	NCI_DMA_SPAN_FIELD_SPAN_WIDTH		= (1 << NCI_DMA_SPAN_OFFSET_SPAN_WIDTH),
	NCI_DMA_SPAN_FIELD_SPAN_HEIGHT		= (1 << NCI_DMA_SPAN_OFFSET_SPAN_HEIGHT),
	NCI_DMA_SPAN_FIELD_SPAN_MODE		= (1 << NCI_DMA_SPAN_OFFSET_SPAN_MODE),
	NCI_DMA_SPAN_FIELD_REPLACEMENT_POLICY	= (1 << NCI_DMA_SPAN_OFFSET_REPLACEMENT_POLICY),
	NCI_DMA_SPAN_FIELD_BANK_MODE		= (1 << NCI_DMA_SPAN_OFFSET_BANK_MODE),
	NCI_DMA_SPAN_FIELDS_ALL			= (NCI_DMA_SPAN_FIELD_UNIT_LOCATION		+
							NCI_DMA_SPAN_FIELD_SPAN_COLUMN		+
							NCI_DMA_SPAN_FIELD_SPAN_ROW		+
							NCI_DMA_SPAN_FIELD_SPAN_WIDTH		+
							NCI_DMA_SPAN_FIELD_SPAN_HEIGHT		+
							NCI_DMA_SPAN_FIELD_SPAN_MODE		+
							NCI_DMA_SPAN_FIELD_REPLACEMENT_POLICY	+
							NCI_DMA_SPAN_FIELD_BANK_MODE),
};

#define NCI_DMA_CACHED_SPAN_DESCRIPTOR_WORD1	(NCI_DMA_SPAN_FIELD_UNIT_LOCATION)
#define NCI_DMA_CACHED_SPAN_DESCRIPTOR_WORD2	(NCI_DMA_SPAN_FIELD_SPAN_COLUMN		| \
							NCI_DMA_SPAN_FIELD_SPAN_ROW)
#define NCI_DMA_CACHED_SPAN_DESCRIPTOR_WORD3	(NCI_DMA_SPAN_FIELD_SPAN_WIDTH		| \
							NCI_DMA_SPAN_FIELD_SPAN_HEIGHT)
#define NCI_DMA_CACHED_SPAN_DESCRIPTOR_WORD4	(NCI_DMA_SPAN_FIELD_SPAN_MODE		| \
							NCI_DMA_SPAN_FIELD_REPLACEMENT_POLICY)

struct nci_dma_public_span_descriptor {
	uint32_t unit_location;
	uint32_t span_row;
	uint32_t span_column;
	uint32_t span_width;
	uint32_t span_height;
	uint32_t span_mode;
	uint32_t bank_mode;
	uint32_t replacement_policy;
};

struct nci_dma_non_cached_span_descriptor {	/* register_name	register index	access (R/W)	*/
	uint32_t unit_location;			/* unit location		0x0		R/W	*/
	uint32_t span_row;			/* span column			0x1		R/W	*/
	uint32_t span_column;			/* span row			0x2		R/W	*/
	uint32_t span_width;			/* span width			0x3		R/W	*/
	uint32_t span_height;			/* span height			0x4		R/W	*/
	uint32_t span_mode;			/* span mode			0x5		R/W	*/
	uint32_t unused_6;			/* <unused>			0x6		<n/a>	*/
	uint32_t unused_7;			/* <unused>			0x7		<n/a>	*/
	uint32_t unused_8;			/* <unused>			0x8		<n/a>	*/
	uint32_t unused_9;			/* <unused>			0x9		<n/a>	*/
	uint32_t replacement_policy;		/* replacement_policy		0xA		R/W	*/
	uint32_t unused_B;			/* <unused>			0xB		<n/a>	*/
	uint32_t pending_counter;		/* pending_counter		0xC		R	*/
	uint32_t lock_status;			/* lock_status			0xD		R	*/
	uint32_t dirty_status;			/* dirty_status			0xE		R	*/
	uint32_t bank_mode;			/* bank_mode			0xF		R/W	*/
};

struct nci_dma_cached_span_descriptor {
	uint8_t unit_location[4];
	uint8_t span_column[2];
	uint8_t span_row[2];
	uint8_t span_width[2];
	uint8_t span_height[2];
	uint8_t span_mode;
	uint8_t replacement_policy;
	uint8_t padding[2];
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_span_descriptor(
	const struct nci_dma_public_span_descriptor * const public_span_descriptor,
	struct nci_dma_non_cached_span_descriptor * const non_cached_span_descriptor)
{
	IA_CSS_ASSERT(NULL != public_span_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_span_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_span_descriptor);

	non_cached_span_descriptor->unit_location = public_span_descriptor->unit_location;
	non_cached_span_descriptor->span_row = public_span_descriptor->span_row;
	non_cached_span_descriptor->span_column = public_span_descriptor->span_column;
	non_cached_span_descriptor->span_width = public_span_descriptor->span_width;
	non_cached_span_descriptor->span_height = public_span_descriptor->span_height;
	non_cached_span_descriptor->span_mode = public_span_descriptor->span_mode;
	non_cached_span_descriptor->unused_6 = 0;
	non_cached_span_descriptor->unused_7 = 0;
	non_cached_span_descriptor->unused_8 = 0;
	non_cached_span_descriptor->unused_9 = 0;
	non_cached_span_descriptor->replacement_policy = public_span_descriptor->replacement_policy;
	non_cached_span_descriptor->unused_B = 0;
	non_cached_span_descriptor->pending_counter = 0;
	non_cached_span_descriptor->lock_status = 0;
	non_cached_span_descriptor->dirty_status = 0;
	non_cached_span_descriptor->bank_mode = public_span_descriptor->bank_mode;

	return sizeof(*non_cached_span_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_fill_cached_span_descriptor(
	const struct nci_dma_public_span_descriptor * const public_span_descriptor,
	struct nci_dma_cached_span_descriptor * const cached_span_descriptor)
{
	IA_CSS_ASSERT(NULL != public_span_descriptor);
	IA_CSS_ASSERT(NULL != cached_span_descriptor);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->unit_location)) >=
		public_span_descriptor->unit_location);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->span_row)) >=
		public_span_descriptor->span_row);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->span_column)) >=
		public_span_descriptor->span_column);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->span_width)) >=
		public_span_descriptor->span_width);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->span_height)) >=
		public_span_descriptor->span_height);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->span_mode)) >=
		public_span_descriptor->span_mode);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_span_descriptor->replacement_policy)) >=
		public_span_descriptor->replacement_policy);

	*((uint32_t *)(cached_span_descriptor->unit_location)) = public_span_descriptor->unit_location;
	*((uint16_t *)(cached_span_descriptor->span_column)) = public_span_descriptor->span_column;
	*((uint16_t *)(cached_span_descriptor->span_row)) = public_span_descriptor->span_row;
	*((uint16_t *)(cached_span_descriptor->span_width)) = public_span_descriptor->span_width;
	*((uint16_t *)(cached_span_descriptor->span_height)) = public_span_descriptor->span_height;
	cached_span_descriptor->span_mode = public_span_descriptor->span_mode;
	cached_span_descriptor->replacement_policy = public_span_descriptor->replacement_policy;
	cached_span_descriptor->padding[0] = 0;
	cached_span_descriptor->padding[1] = 0;

	return sizeof(*cached_span_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_SPAN_H */
