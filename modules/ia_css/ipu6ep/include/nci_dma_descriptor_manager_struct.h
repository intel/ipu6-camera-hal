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

#ifndef __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_H
#define __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_H

#include "type_support.h" /* for uint32_t ... */
#include "nci_dma_descriptor_manager_struct_dep.h" /* for ia_css_dma_cached_ext0_descriptor ... */
#include "ipu_device_dma_type_properties.h" /* for nci_dma_bank_mode ... */

struct ia_css_dma_descriptor_addresses {
	uint32_t channel_descriptor_offset;
	uint32_t span_descriptor_offset;
	uint32_t terminal_descriptor_offset;
	uint32_t unit_descriptor_offset;
	uint32_t cmem_descriptor_base_address;
};

union ia_css_dma_descriptor_manager {
	struct {
		uint32_t channel_descriptor_offset;
		uint32_t request_descriptor_offset;
		uint32_t global_descriptor_offset;
		uint32_t master_descriptor_offset;
		uint32_t span_descriptor_offset;
		uint32_t unit_descriptor_offset;
		uint32_t terminal_descriptor_offset;
		uint32_t cmem_descriptor_base_address;
	} values;
	uint32_t data[N_IPU_DEVICE_DMA_GROUP_ID + 1];
};

#endif /* __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_H */
