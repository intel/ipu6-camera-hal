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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_UNIT_H
#define __NCI_DMA_DESCRIPTORS_TYPES_UNIT_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "math_support.h"

enum nci_dma_unit_rw_registers {
	NCI_DMA_UNIT_OFFSET_UNIT_WIDTH		= 0x0,
	NCI_DMA_UNIT_OFFSET_UNIT_HEIGHT		= 0x1,
	NCI_DMA_UNIT_OFFSET_REPLACEMENT_POLICY	= 0x2,
	NCI_DMA_UNIT_OFFSET_BANK_MODE		= 0x3,
};
#define N_NCI_DMA_UNIT_RW_REGISTERS (NCI_DMA_UNIT_OFFSET_BANK_MODE + 1)

enum nci_dma_unit_descriptor_fields {
	NCI_DMA_UNIT_FIELDS_NONE		= (0),
	NCI_DMA_UNIT_FIELD_UNIT_WIDTH		= (1 << NCI_DMA_UNIT_OFFSET_UNIT_WIDTH),
	NCI_DMA_UNIT_FIELD_UNIT_HEIGHT		= (1 << NCI_DMA_UNIT_OFFSET_UNIT_HEIGHT),
	NCI_DMA_UNIT_FIELD_REPLACEMENT_POLICY	= (1 << NCI_DMA_UNIT_OFFSET_REPLACEMENT_POLICY),
	NCI_DMA_UNIT_FIELD_BANK_MODE		= (1 << NCI_DMA_UNIT_OFFSET_BANK_MODE),
	NCI_DMA_UNIT_FIELDS_ALL			= (NCI_DMA_UNIT_FIELD_UNIT_WIDTH	+
							NCI_DMA_UNIT_FIELD_BANK_MODE	+
							NCI_DMA_UNIT_FIELD_UNIT_HEIGHT),
};

#define NCI_DMA_CACHED_UNIT_DESCRIPTOR_WORD1	(NCI_DMA_UNIT_FIELD_UNIT_WIDTH		| \
							NCI_DMA_UNIT_FIELD_UNIT_HEIGHT	| \
							NCI_DMA_UNIT_FIELD_REPLACEMENT_POLICY)

struct nci_dma_public_unit_descriptor {
	uint32_t unit_width;
	uint32_t unit_height;
	uint32_t replacement_policy;
	uint32_t bank_mode;
};

struct nci_dma_non_cached_unit_descriptor {	/* register_name	register index	access (R/W)	*/
	uint32_t unit_width;			/* unit width			0x0		R/W	*/
	uint32_t unit_height;			/* unit height			0x1		R/W	*/
	uint32_t unused_2;			/* <unused>			0x2		<n/a>	*/
	uint32_t unused_3;			/* <unused>			0x3		<n/a>	*/
	uint32_t unused_4;			/* <unused>			0x4		<n/a>	*/
	uint32_t unused_5;			/* <unused>			0x5		<n/a>	*/
	uint32_t unused_6;			/* <unused>			0x6		<n/a>	*/
	uint32_t unused_7;			/* <unused>			0x7		<n/a>	*/
	uint32_t unused_8;			/* <unused>			0x8		<n/a>	*/
	uint32_t unused_9;			/* <unused>			0x9		<n/a>	*/
	uint32_t replacement_policy;		/* replacement_policy		0xA		R/W	*/
	uint32_t unused_B;			/* <unused>			0xB		<n/a>	*/
	uint32_t pending_counter;		/* pending_counter		0xC		R	*/
	uint32_t lock_status;			/* lock_status			0xD		R	*/
	uint32_t unused_E;			/* <unused>			0xE		<n/a>	*/
	uint32_t bank_mode;			/* bank_mode			0xF		R/W	*/
};

struct nci_dma_cached_unit_descriptor {
	uint8_t unit_width[2];
	uint8_t unit_height;
	uint8_t replacement_policy;
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_unit_descriptor(
	const struct nci_dma_public_unit_descriptor * const public_unit_descriptor,
	struct nci_dma_non_cached_unit_descriptor * const non_cached_unit_descriptor)
{
	IA_CSS_ASSERT(NULL != public_unit_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_unit_descriptor);

	non_cached_unit_descriptor->unit_width = public_unit_descriptor->unit_width;
	non_cached_unit_descriptor->unit_height = public_unit_descriptor->unit_height;
	non_cached_unit_descriptor->unused_2 = 0;
	non_cached_unit_descriptor->unused_3 = 0;
	non_cached_unit_descriptor->unused_4 = 0;
	non_cached_unit_descriptor->unused_5 = 0;
	non_cached_unit_descriptor->unused_6 = 0;
	non_cached_unit_descriptor->unused_7 = 0;
	non_cached_unit_descriptor->unused_8 = 0;
	non_cached_unit_descriptor->unused_9 = 0;
	non_cached_unit_descriptor->replacement_policy = public_unit_descriptor->replacement_policy;
	non_cached_unit_descriptor->unused_B = 0;
	non_cached_unit_descriptor->pending_counter = 0;
	non_cached_unit_descriptor->lock_status = 0;
	non_cached_unit_descriptor->unused_E = 0;
	non_cached_unit_descriptor->bank_mode = public_unit_descriptor->bank_mode;

	return sizeof(*non_cached_unit_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_fill_cached_unit_descriptor(
	const struct nci_dma_public_unit_descriptor * const public_unit_descriptor,
	struct nci_dma_cached_unit_descriptor * const cached_unit_descriptor)
{
	IA_CSS_ASSERT(NULL != public_unit_descriptor);
	IA_CSS_ASSERT(NULL != cached_unit_descriptor);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_unit_descriptor->unit_width)) >=
		public_unit_descriptor->unit_width);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_unit_descriptor->unit_height)) >=
		public_unit_descriptor->unit_height);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_unit_descriptor->replacement_policy)) >=
		public_unit_descriptor->replacement_policy);

	*((uint16_t *)cached_unit_descriptor->unit_width) = public_unit_descriptor->unit_width;
	cached_unit_descriptor->unit_height = public_unit_descriptor->unit_height;
	cached_unit_descriptor->replacement_policy = public_unit_descriptor->replacement_policy;

	return sizeof(*cached_unit_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_UNIT_H */
