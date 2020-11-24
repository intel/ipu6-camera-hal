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

#ifndef __NCI_DMA_DESCRIPTORS_TYPES_MASTER_H
#define __NCI_DMA_DESCRIPTORS_TYPES_MASTER_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"

enum nci_dma_master_rw_registers {
	NCI_DMA_MASTER_OFFSET_SRMD_SUPPORT			= 0x0,
	NCI_DMA_MASTER_OFFSET_BURST_SUPPORT			= 0x1,
	NCI_DMA_MASTER_OFFSET_MAXIMUM_PHYSICAL_STRIDE_SHIFT	= 0x2,
};
#define N_NCI_DMA_MASTER_RW_REGISTERS (NCI_DMA_MASTER_OFFSET_MAXIMUM_PHYSICAL_STRIDE_SHIFT + 1)

enum nci_dma_master_descriptor_fields {
	NCI_DMA_MASTER_FIELDS_NONE			= (0),
	NCI_DMA_MASTER_FIELD_SRMD_SUPPORT		= (1 << NCI_DMA_MASTER_OFFSET_SRMD_SUPPORT),
	NCI_DMA_MASTER_FIELD_BURST_SUPPORT		= (1 << NCI_DMA_MASTER_OFFSET_BURST_SUPPORT),
	NCI_DMA_MASTER_FIELD_MAXIMUM_PHYSICAL_STRIDE	= (1 << NCI_DMA_MASTER_OFFSET_MAXIMUM_PHYSICAL_STRIDE_SHIFT),
	NCI_DMA_MASTER_FIELDS_ALL			= (NCI_DMA_MASTER_FIELD_SRMD_SUPPORT			+
								NCI_DMA_MASTER_FIELD_BURST_SUPPORT		+
								NCI_DMA_MASTER_FIELD_MAXIMUM_PHYSICAL_STRIDE),
};

struct nci_dma_public_master_descriptor {
	uint32_t srmd_support;
	uint32_t burst_support;
	uint32_t maximum_physical_stride;
};

struct nci_dma_non_cached_master_descriptor {	/* register_name		register index	access (R/W)	*/
	uint32_t srmd_support;			/* srmd_support				0x0		R/W	*/
	uint32_t unused_1;			/* <unused>				0x1		<n/a>	*/
	uint32_t burst_support;			/* burst_support			0x2		R/W	*/
	uint32_t maximum_physical_stride;	/* maximum_physical_stride		0x3		R/W	*/
};

STORAGE_CLASS_INLINE int nci_dma_fill_non_cached_master_descriptor(
	const struct nci_dma_public_master_descriptor * const public_master_descriptor,
	struct nci_dma_non_cached_master_descriptor * const non_cached_master_descriptor)
{
	IA_CSS_ASSERT(NULL != public_master_descriptor);
	IA_CSS_ASSERT(NULL != non_cached_master_descriptor);

	non_cached_master_descriptor->srmd_support = public_master_descriptor->srmd_support;
	non_cached_master_descriptor->unused_1 = 0;
	non_cached_master_descriptor->burst_support = public_master_descriptor->burst_support;
	non_cached_master_descriptor->maximum_physical_stride = public_master_descriptor->maximum_physical_stride;

	return sizeof(*non_cached_master_descriptor);
}

#endif /* __NCI_DMA_DESCRIPTORS_TYPES_MASTER_H */
