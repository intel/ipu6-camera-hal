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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_REQUEST_H
#define __NCI_DMA_DESCRIPTORS_TYPES_REQUEST_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"

enum nci_dma_request_rw_registers {
	NCI_DMA_REQUEST_OFFSET_INSTRUCTION			= 0x0,
	NCI_DMA_REQUEST_OFFSET_DESC_ID_SETUP1			= 0x1,
	NCI_DMA_REQUEST_OFFSET_DESC_ID_SETUP2			= 0x2,
	NCI_DMA_REQUEST_OFFSET_REQUEST_VALID			= 0x3,
	NCI_DMA_REQUEST_OFFSET_REQUEST_RESOURCES		= 0x4,
	NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_ADDR		= 0x5,
	NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_TOKEN_SIDPID	= 0x6,
	NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_ACTIVE		= 0x7,
};
#define N_NCI_DMA_REQUEST_RW_REGISTERS (NCI_DMA_REQUEST_OFFSET_REQUEST_QUEUE_DEPTH + 1)

enum nci_dma_request_descriptor_fields {
	NCI_DMA_REQUEST_FIELDS_NONE		= (0),
	NCI_DMA_REQUEST_FIELD_INSTRUCTION	= (1 << NCI_DMA_REQUEST_OFFSET_INSTRUCTION),
	NCI_DMA_REQUEST_FIELD_DESC_ID_SETUP1	= (1 << NCI_DMA_REQUEST_OFFSET_DESC_ID_SETUP1),
	NCI_DMA_REQUEST_FIELD_DESC_ID_SETUP2	= (1 << NCI_DMA_REQUEST_OFFSET_DESC_ID_SETUP2),
	NCI_DMA_REQUEST_FIELD_REQUEST_VALID	= (1 << NCI_DMA_REQUEST_OFFSET_REQUEST_VALID),
	NCI_DMA_REQUEST_FIELD_REQUEST_RESOURCES	= (1 << NCI_DMA_REQUEST_OFFSET_REQUEST_RESOURCES),
	NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_ADDR		= (1 << NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_ADDR),
	NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_TOKEN_SIDPID	= (1 << NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_TOKEN_SIDPID),
	NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_ACTIVE		= (1 << NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_ACTIVE),
	NCI_DMA_REQUEST_FIELDS_ALL		= (NCI_DMA_REQUEST_FIELD_INSTRUCTION		+
							NCI_DMA_REQUEST_FIELD_DESC_ID_SETUP1	+
							NCI_DMA_REQUEST_FIELD_DESC_ID_SETUP2	+
							NCI_DMA_REQUEST_FIELD_REQUEST_VALID	+
							NCI_DMA_REQUEST_FIELD_REQUEST_RESOURCES +
							NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_ADDR +
							NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_TOKEN_SIDPID +
							NCI_DMA_REQUEST_FIELD_REQUEST_ACCEPTED_ACTIVE)
};

#define	NCI_DMA_REG_ALIGN				(4)
#define NCI_DMA_REQUEST_BANK_REQ_ACC_ACK_ADDR_OFFSET	(NCI_DMA_REQUEST_OFFSET_REQUEST_ACCEPTED_ADDR * NCI_DMA_REG_ALIGN)

/* Private struct - section 0 */
struct nci_dma_non_cached_request_descriptor {		/* register_name		register index	access (R/W)	*/
	uint32_t instruction;				/* instruction				0x0		R/W	*/
	uint32_t descriptor_id_setup_1;			/* descriptor_id_setup_1		0x1		R/W	*/
	uint32_t descriptor_id_setup_2;			/* descriptor_id_setup_2		0x2		R/W	*/
	uint32_t request_valid;				/* request_valid			0x3		R	*/
	uint32_t request_resourced;			/* request_resourced			0x4		R	*/
};

/* Private struct - section 1 */
struct nci_dma_non_cached_request_dequeue_ack {		/* register index	access (R/W)	*/
	uint32_t request_accepted_addr;			/* 0x5			R/W		*/
	uint32_t request_accepted_token;		/* 0x6			R/W		*/
	uint32_t request_accepted_active;		/* 0x7			R/W		*/
};

/* Private struct - register layout of Request register bank */
struct nci_dma_non_cached_request_bank {
	struct nci_dma_non_cached_request_descriptor	request_instruction_config;
	struct nci_dma_non_cached_request_dequeue_ack	request_accepted_ack_config;
};

/* Public struct - section 0 */
struct nci_dma_public_request_descriptor {
	uint32_t instruction;
	uint32_t descriptor_id_setup_1;
	uint32_t descriptor_id_setup_2;
};

/* Public struct - section 1 */
struct nci_dma_public_request_bank_dequeue_ack_config {
	uint8_t enable_instr_dequeue_ack; 	/* Enable sending a dequeue ack token when instruction
	 	 	 	 	 	 * is accepted and moved to the instruction queue
	 	 	 	 	 	 */
	uint32_t instr_dequeue_ack_addr; 	/* Address to which the dequeue ack has to be sent */
	uint32_t instr_dequeue_ack_sid_pid; 	/* SID PID of the dequeue ack token*/
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_request_descriptor(
	const struct nci_dma_public_request_descriptor * const public_request_descriptor,
	struct nci_dma_non_cached_request_descriptor * const non_cached_request_descriptor)
{
	IA_CSS_ASSERT(NULL != public_request_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_request_descriptor);

	non_cached_request_descriptor->instruction = public_request_descriptor->instruction;
	non_cached_request_descriptor->descriptor_id_setup_1 = public_request_descriptor->descriptor_id_setup_1;
	non_cached_request_descriptor->descriptor_id_setup_2 = public_request_descriptor->descriptor_id_setup_2;
	non_cached_request_descriptor->request_valid = 0;
	non_cached_request_descriptor->request_resourced = 0;

	return sizeof(*non_cached_request_descriptor);
}

STORAGE_CLASS_INLINE int nci_dma_fill_request_bank_dequeue_ack_config(
	const struct nci_dma_public_request_bank_dequeue_ack_config * const public_request_reg_bank,
	struct nci_dma_non_cached_request_dequeue_ack * const request_dequeue_ack_cfg)
{
	IA_CSS_ASSERT(NULL != public_request_reg_bank);
	IA_CSS_ASSERT(NULL != request_dequeue_ack_cfg);

	request_dequeue_ack_cfg->request_accepted_active = public_request_reg_bank->enable_instr_dequeue_ack;
	request_dequeue_ack_cfg->request_accepted_addr = public_request_reg_bank->instr_dequeue_ack_addr;
	request_dequeue_ack_cfg->request_accepted_token = public_request_reg_bank->instr_dequeue_ack_sid_pid;

	return sizeof(*request_dequeue_ack_cfg);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_REQUEST_H */
