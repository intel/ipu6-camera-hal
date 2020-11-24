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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_TERMINAL_H
#define __NCI_DMA_DESCRIPTORS_TYPES_TERMINAL_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "math_support.h"

enum nci_dma_terminal_rw_registers {
	NCI_DMA_TERMINAL_OFFSET_REGION_ORIGIN		= 0x0,
	NCI_DMA_TERMINAL_OFFSET_REGION_STRIDE		= 0x1,
	NCI_DMA_TERMINAL_OFFSET_REGION_WIDTH		= 0x2,
	NCI_DMA_TERMINAL_OFFSET_ELEMENT_SETUP		= 0x3,
	NCI_DMA_TERMINAL_OFFSET_CIO_INFO_SETUP		= 0x4,
	NCI_DMA_TERMINAL_OFFSET_PORT_MODE		= 0x5,
	NCI_DMA_TERMINAL_OFFSET_REGION_HEIGHT		= 0x6,
	NCI_DMA_TERMINAL_OFFSET_REPLACEMENT_POLICY	= 0x7,
	NCI_DMA_TERMINAL_OFFSET_BANK_MODE		= 0x8,
};
#define N_NCI_DMA_TERMINAL_RW_REGISTERS (NCI_DMA_TERMINAL_OFFSET_BANK_MODE + 1)

enum nci_dma_terminal_descriptor_fields {
	NCI_DMA_TERMINAL_FIELDS_NONE = (0),
	NCI_DMA_TERMINAL_FIELD_REGION_ORIGIN		= (1 << NCI_DMA_TERMINAL_OFFSET_REGION_ORIGIN),
	NCI_DMA_TERMINAL_FIELD_REGION_STRIDE		= (1 << NCI_DMA_TERMINAL_OFFSET_REGION_STRIDE),
	NCI_DMA_TERMINAL_FIELD_REGION_WIDTH		= (1 << NCI_DMA_TERMINAL_OFFSET_REGION_WIDTH),
	NCI_DMA_TERMINAL_FIELD_REGION_HEIGHT		= (1 << NCI_DMA_TERMINAL_OFFSET_REGION_HEIGHT),
	NCI_DMA_TERMINAL_FIELD_ELEMENT_SETUP		= (1 << NCI_DMA_TERMINAL_OFFSET_ELEMENT_SETUP),
	NCI_DMA_TERMINAL_FIELD_CIO_INFO_SETUP		= (1 << NCI_DMA_TERMINAL_OFFSET_CIO_INFO_SETUP),
	NCI_DMA_TERMINAL_FIELD_PORT_MODE		= (1 << NCI_DMA_TERMINAL_OFFSET_PORT_MODE),
	NCI_DMA_TERMINAL_FIELD_REPLACEMENT_POLICY	= (1 << NCI_DMA_TERMINAL_OFFSET_REPLACEMENT_POLICY),
	NCI_DMA_TERMINAL_FIELD_BANK_MODE		= (1 << NCI_DMA_TERMINAL_OFFSET_BANK_MODE),
	NCI_DMA_TERMINAL_FIELDS_ALL			= (NCI_DMA_TERMINAL_FIELD_REGION_ORIGIN			+
								NCI_DMA_TERMINAL_FIELD_REGION_STRIDE		+
								NCI_DMA_TERMINAL_FIELD_REGION_WIDTH		+
								NCI_DMA_TERMINAL_FIELD_REGION_HEIGHT		+
								NCI_DMA_TERMINAL_FIELD_ELEMENT_SETUP		+
								NCI_DMA_TERMINAL_FIELD_CIO_INFO_SETUP		+
								NCI_DMA_TERMINAL_FIELD_PORT_MODE		+
								NCI_DMA_TERMINAL_FIELD_REPLACEMENT_POLICY	+
								NCI_DMA_TERMINAL_FIELD_BANK_MODE),
};

#define NCI_DMA_CACHED_TERMINAL_DESCRIPTOR_WORD1	(NCI_DMA_TERMINAL_FIELD_REGION_ORIGIN)
#define NCI_DMA_CACHED_TERMINAL_DESCRIPTOR_WORD2	(NCI_DMA_TERMINAL_FIELD_REGION_STRIDE)
#define NCI_DMA_CACHED_TERMINAL_DESCRIPTOR_WORD3	(NCI_DMA_TERMINAL_FIELD_REGION_WIDTH			| \
								NCI_DMA_TERMINAL_FIELD_REGION_HEIGHT)
#define NCI_DMA_CACHED_TERMINAL_DESCRIPTOR_WORD4	(NCI_DMA_TERMINAL_FIELD_CIO_INFO_SETUP			| \
								NCI_DMA_TERMINAL_FIELD_REPLACEMENT_POLICY	| \
								NCI_DMA_TERMINAL_FIELD_ELEMENT_SETUP)

struct nci_dma_public_terminal_descriptor {
	uint32_t region_origin;
	uint32_t region_stride;
	uint32_t region_width;
	uint32_t region_height;
	uint32_t cio_info_setup;
	uint32_t replacement_policy;
	uint32_t element_setup;
	uint32_t port_mode;
	uint32_t bank_mode;
};

struct nci_dma_non_cached_terminal_descriptor {	/* register_name	register index	access (R/W)	*/
	uint32_t region_origin;			/* region origin		0x0		R/W	*/
	uint32_t region_width;			/* region width			0x1		R/W	*/
	uint32_t region_stride;			/* region stride		0x2		R/W	*/
	uint32_t element_setup;			/* element setup		0x3		R/W	*/
	uint32_t cio_info_setup;		/* CIO info			0x4		R/W	*/
	uint32_t port_mode;			/* port mode			0x5		R/W	*/
	uint32_t region_height;			/* region_height		0x6		R/W	*/
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

struct nci_dma_cached_terminal_descriptor {
	uint8_t region_origin[4];
	uint8_t region_stride[4];
	uint8_t region_width[2];
	uint8_t region_height[2];
	uint8_t cio_info_setup[2];
	uint8_t replacement_policy;
	uint8_t element_setup;
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_terminal_descriptor(
	const struct nci_dma_public_terminal_descriptor * const public_terminal_descriptor,
	struct nci_dma_non_cached_terminal_descriptor * const non_cached_terminal_descriptor)
{
	IA_CSS_ASSERT(NULL != public_terminal_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_terminal_descriptor);

	non_cached_terminal_descriptor->region_origin = public_terminal_descriptor->region_origin;
	non_cached_terminal_descriptor->region_width = public_terminal_descriptor->region_width;
	non_cached_terminal_descriptor->region_stride = public_terminal_descriptor->region_stride;
	non_cached_terminal_descriptor->element_setup = public_terminal_descriptor->element_setup;
	non_cached_terminal_descriptor->cio_info_setup = public_terminal_descriptor->cio_info_setup;
	non_cached_terminal_descriptor->port_mode = public_terminal_descriptor->port_mode;
	non_cached_terminal_descriptor->region_height = public_terminal_descriptor->region_height;
	non_cached_terminal_descriptor->unused_7 = 0;
	non_cached_terminal_descriptor->unused_8 = 0;
	non_cached_terminal_descriptor->unused_9 = 0;
	non_cached_terminal_descriptor->replacement_policy = public_terminal_descriptor->replacement_policy;
	non_cached_terminal_descriptor->unused_B = 0;
	non_cached_terminal_descriptor->pending_counter = 0;
	non_cached_terminal_descriptor->lock_status = 0;
	non_cached_terminal_descriptor->unused_E = 0;
	non_cached_terminal_descriptor->bank_mode = public_terminal_descriptor->bank_mode;

	return sizeof(*non_cached_terminal_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_fill_cached_terminal_descriptor(
	const struct nci_dma_public_terminal_descriptor * const public_terminal_descriptor,
	struct nci_dma_cached_terminal_descriptor * const cached_terminal_descriptor)
{
	IA_CSS_ASSERT(NULL != public_terminal_descriptor);
	IA_CSS_ASSERT(NULL != cached_terminal_descriptor);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->region_origin)) >=
		public_terminal_descriptor->region_origin);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->region_width)) >=
		public_terminal_descriptor->region_width);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->region_height)) >=
		public_terminal_descriptor->region_height);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->cio_info_setup)) >=
		public_terminal_descriptor->cio_info_setup);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->replacement_policy)) >=
		public_terminal_descriptor->replacement_policy);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_terminal_descriptor->element_setup)) >=
		public_terminal_descriptor->element_setup);

	*((uint32_t *)cached_terminal_descriptor->region_origin) = public_terminal_descriptor->region_origin;
	*((uint32_t *)cached_terminal_descriptor->region_stride) = (public_terminal_descriptor->region_stride);
	*((uint16_t *)cached_terminal_descriptor->region_width) = public_terminal_descriptor->region_width;
	*((uint16_t *)cached_terminal_descriptor->region_height) = public_terminal_descriptor->region_height;
	*((uint16_t *)cached_terminal_descriptor->cio_info_setup) = public_terminal_descriptor->cio_info_setup;
	cached_terminal_descriptor->replacement_policy = public_terminal_descriptor->replacement_policy;
	cached_terminal_descriptor->element_setup = public_terminal_descriptor->element_setup;

	return sizeof(*cached_terminal_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_TERMINAL_H */
