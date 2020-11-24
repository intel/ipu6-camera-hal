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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_CHANNEL_H
#define __NCI_DMA_DESCRIPTORS_TYPES_CHANNEL_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "math_support.h"

enum nci_dma_channel_rw_registers {
	NCI_DMA_CHANNEL_OFFSET_ELEMENT_EXTEND_MODE	= 0x0,
	NCI_DMA_CHANNEL_OFFSET_ELEMENT_INIT_DATA	= 0x1,
	NCI_DMA_CHANNEL_OFFSET_PADDING_MODE		= 0x2,
	NCI_DMA_CHANNEL_OFFSET_SAMPLING_SETUP		= 0x3,
	NCI_DMA_CHANNEL_OFFSET_GLOBAL_SET_ID		= 0x4,
	NCI_DMA_CHANNEL_OFFSET_ACK_MODE			= 0x5,
	NCI_DMA_CHANNEL_OFFSET_ACK_ADDRESS		= 0x6,
	NCI_DMA_CHANNEL_OFFSET_ACK_DATA			= 0x7,
	NCI_DMA_CHANNEL_OFFSET_EXTENDED_PAD		= 0x8,
	NCI_DMA_CHANNEL_OFFSET_COMPLETED_COUNTER	= 0x9,
	NCI_DMA_CHANNEL_OFFSET_REPLACEMENT_POLICY	= 0xA,
	NCI_DMA_CHANNEL_OFFSET_BANK_MODE		= 0xB,
};
#define N_NCI_DMA_CHANNEL_RW_REGISTERS (NCI_DMA_CHANNEL_OFFSET_BANK_MODE + 1)

enum nci_dma_channel_descriptor_fields {
	NCI_DMA_CHANNEL_FIELDS_NONE			= (0),
	NCI_DMA_CHANNEL_FIELD_ELEMENT_EXTEND_MODE	= (1 << NCI_DMA_CHANNEL_OFFSET_ELEMENT_EXTEND_MODE),
	NCI_DMA_CHANNEL_FIELD_ELEMENT_INIT_DATA		= (1 << NCI_DMA_CHANNEL_OFFSET_ELEMENT_INIT_DATA),
	NCI_DMA_CHANNEL_FIELD_PADDING_MODE		= (1 << NCI_DMA_CHANNEL_OFFSET_PADDING_MODE),
	NCI_DMA_CHANNEL_FIELD_SAMPLING_SETUP		= (1 << NCI_DMA_CHANNEL_OFFSET_SAMPLING_SETUP),
	NCI_DMA_CHANNEL_FIELD_GLOBAL_SET_ID		= (1 << NCI_DMA_CHANNEL_OFFSET_GLOBAL_SET_ID),
	NCI_DMA_CHANNEL_FIELD_ACK_MODE			= (1 << NCI_DMA_CHANNEL_OFFSET_ACK_MODE),
	NCI_DMA_CHANNEL_FIELD_ACK_ADDRESS		= (1 << NCI_DMA_CHANNEL_OFFSET_ACK_ADDRESS),
	NCI_DMA_CHANNEL_FIELD_ACK_DATA			= (1 << NCI_DMA_CHANNEL_OFFSET_ACK_DATA),
	NCI_DMA_CHANNEL_FIELD_EXTENDED_PAD		= (1 << NCI_DMA_CHANNEL_OFFSET_EXTENDED_PAD),
	NCI_DMA_CHANNEL_FIELD_COMPLETED_COUNTER		= (1 << NCI_DMA_CHANNEL_OFFSET_COMPLETED_COUNTER),
	NCI_DMA_CHANNEL_FIELD_REPLACEMENT_POLICY	= (1 << NCI_DMA_CHANNEL_OFFSET_REPLACEMENT_POLICY),
	NCI_DMA_CHANNEL_FIELD_BANK_MODE			= (1 << NCI_DMA_CHANNEL_OFFSET_BANK_MODE),
	NCI_DMA_CHANNEL_FIELDS_ALL			= (NCI_DMA_CHANNEL_FIELD_ELEMENT_EXTEND_MODE +
								NCI_DMA_CHANNEL_FIELD_ELEMENT_INIT_DATA	+
								NCI_DMA_CHANNEL_FIELD_PADDING_MODE	+
								NCI_DMA_CHANNEL_FIELD_SAMPLING_SETUP	+
								NCI_DMA_CHANNEL_FIELD_GLOBAL_SET_ID	+
								NCI_DMA_CHANNEL_FIELD_ACK_MODE		+
								NCI_DMA_CHANNEL_FIELD_ACK_ADDRESS	+
								NCI_DMA_CHANNEL_FIELD_ACK_DATA		+
								NCI_DMA_CHANNEL_FIELD_EXTENDED_PAD	+
								NCI_DMA_CHANNEL_FIELD_COMPLETED_COUNTER	+
								NCI_DMA_CHANNEL_FIELD_REPLACEMENT_POLICY+
								NCI_DMA_CHANNEL_FIELD_BANK_MODE),
};

#define NCI_DMA_CACHED_CHANNEL_DESCRIPTOR_WORD1		(NCI_DMA_CHANNEL_FIELD_ACK_ADDRESS)
#define NCI_DMA_CACHED_CHANNEL_DESCRIPTOR_WORD2		(NCI_DMA_CHANNEL_FIELD_ACK_DATA)
#define NCI_DMA_CACHED_CHANNEL_DESCRIPTOR_WORD3		(NCI_DMA_CHANNEL_FIELD_EXTENDED_PAD			| \
								NCI_DMA_CHANNEL_FIELD_REPLACEMENT_POLICY	| \
								NCI_DMA_CHANNEL_FIELD_GLOBAL_SET_ID		| \
								NCI_DMA_CHANNEL_FIELD_ACK_MODE)
#define NCI_DMA_CACHED_CHANNEL_DESCRIPTOR_WORD4		(NCI_DMA_CHANNEL_FIELD_COMPLETED_COUNTER)

struct nci_dma_public_channel_descriptor {
	uint32_t element_extend_mode;
	uint32_t element_init_data;
	uint32_t padding_mode;
	uint32_t sampling_setup;
	uint32_t global_set_id;
	uint32_t ack_mode;
	uint32_t ack_addr;
	uint32_t ack_data;
	uint32_t completed_counter;
	uint32_t replacement_policy;
	uint32_t sign_bit_toggle_value;
	uint32_t bank_mode;
};

struct nci_dma_non_cached_channel_descriptor {	/* register_name		register index	access (R/W)	*/
	uint32_t element_extend_mode;		/* element extend mode			0x0		R/W	*/
	uint32_t element_init_data;		/* element initialization data		0x1		R/W	*/
	uint32_t padding_mode;			/* padding mode				0x2		R/W	*/
	uint32_t sampling_setup;		/* sampling setup			0x3		R/W	*/
	uint32_t global_set_id;			/* global set ID			0x4		R/W	*/
	uint32_t ack_mode;			/* acknowledge mode			0x5		R/W	*/
	uint32_t ack_addr;			/* acknowledge address			0x6		R/W	*/
	uint32_t ack_data;			/* acknowledge data			0x7		R/W	*/
	uint32_t unused_8;			/* <unused>				0x8		<n/a>	*/
	uint32_t extended_pad;			/* extended_pad				0x9		R/W	*/
	uint32_t replacement_policy;		/* replacement_policy			0x8		R/W	*/
	uint32_t completed_counter;		/* completed counter			0xB		R/W	*/
	uint32_t pending_counter;		/* pending_counter			0xC		R	*/
	uint32_t lock_status;			/* lock_status				0xD		R	*/
	uint32_t dirty_status;			/* dirty_status				0xE		R	*/
	uint32_t bank_mode;			/* bank_mode				0xF		R/W	*/
};

struct nci_dma_cached_channel_descriptor {
	uint8_t ack_addr[4];
	uint8_t ack_data[4];
	uint8_t extended_pad;
	uint8_t replacement_policy;
	uint8_t global_set_id;
	uint8_t ack_mode;
	uint8_t completed_counter;
	uint8_t padding[3];
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_channel_descriptor(
	const struct nci_dma_public_channel_descriptor * const public_channel_descriptor,
	struct nci_dma_non_cached_channel_descriptor * const non_cached_channel_descriptor)
{
	IA_CSS_ASSERT(NULL != public_channel_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_channel_descriptor);

	/* element_extend_mode has meaning only in non-cached mode */
	non_cached_channel_descriptor->element_extend_mode = public_channel_descriptor->element_extend_mode;
	/* element_init_data has meaning only in non-cached mode */
	non_cached_channel_descriptor->element_init_data = public_channel_descriptor->element_init_data;
	/* padding_mode has meaning only in non-cached mode */
	non_cached_channel_descriptor->padding_mode = public_channel_descriptor->padding_mode;
	non_cached_channel_descriptor->sampling_setup = public_channel_descriptor->sampling_setup;
	non_cached_channel_descriptor->global_set_id = public_channel_descriptor->global_set_id;
	non_cached_channel_descriptor->ack_mode = public_channel_descriptor->ack_mode;
	non_cached_channel_descriptor->ack_addr = public_channel_descriptor->ack_addr;
	non_cached_channel_descriptor->ack_data = public_channel_descriptor->ack_data;
	non_cached_channel_descriptor->unused_8 = 0;
	/* extended_pad has meaning only in cached mode */
	non_cached_channel_descriptor->extended_pad = 0;
	non_cached_channel_descriptor->replacement_policy = public_channel_descriptor->replacement_policy;
	non_cached_channel_descriptor->completed_counter = public_channel_descriptor->completed_counter;
	non_cached_channel_descriptor->completed_counter = 0;
	non_cached_channel_descriptor->pending_counter = 0;
	non_cached_channel_descriptor->lock_status = 0;
	non_cached_channel_descriptor->dirty_status = 0;
	non_cached_channel_descriptor->bank_mode = public_channel_descriptor->bank_mode;
	/* sign_bit_toggle_value has meaning only for logical descriptors */

	return sizeof(*non_cached_channel_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_fill_cached_channel_descriptor(
	const struct nci_dma_public_channel_descriptor * const public_channel_descriptor,
	struct nci_dma_cached_channel_descriptor * const cached_channel_descriptor)
{
	IA_CSS_ASSERT(NULL != public_channel_descriptor);
	IA_CSS_ASSERT(NULL != cached_channel_descriptor);

	IA_CSS_ASSERT((1 << 1) > public_channel_descriptor->element_init_data);
	IA_CSS_ASSERT((1 << 1) > public_channel_descriptor->element_extend_mode);
	IA_CSS_ASSERT((1 << 1) > public_channel_descriptor->sign_bit_toggle_value);
	IA_CSS_ASSERT((1 << 3) > public_channel_descriptor->padding_mode);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->global_set_id)) >=
		public_channel_descriptor->global_set_id);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->ack_mode)) >=
		public_channel_descriptor->ack_mode);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->ack_addr)) >=
		public_channel_descriptor->ack_addr);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->ack_data)) >=
		public_channel_descriptor->ack_data);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->replacement_policy)) >=
		public_channel_descriptor->replacement_policy);
	IA_CSS_ASSERT(max_value_bytes(sizeof(cached_channel_descriptor->completed_counter)) >=
		public_channel_descriptor->completed_counter);

	/* Word 1 */
	*((uint32_t *)cached_channel_descriptor->ack_addr) = public_channel_descriptor->ack_addr;
	/* Word 2 */
	*((uint32_t *)cached_channel_descriptor->ack_data) = public_channel_descriptor->ack_data;
	/* Word 3 */
	cached_channel_descriptor->extended_pad =
		((public_channel_descriptor->element_init_data << 5) & 0x1) |
		((public_channel_descriptor->element_extend_mode << 4) & 0x1) |
		((public_channel_descriptor->sign_bit_toggle_value << 3) & 0x1) |
		((public_channel_descriptor->padding_mode) & 0x3);
	cached_channel_descriptor->replacement_policy = public_channel_descriptor->replacement_policy;
	cached_channel_descriptor->global_set_id = public_channel_descriptor->global_set_id;
	cached_channel_descriptor->ack_mode = public_channel_descriptor->ack_mode;
	/* Word 4 */
	cached_channel_descriptor->completed_counter = public_channel_descriptor->completed_counter;
	cached_channel_descriptor->padding[0] = 0;
	cached_channel_descriptor->padding[1] = 0;
	cached_channel_descriptor->padding[2] = 0;

	return sizeof(*cached_channel_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_CHANNEL_H */
