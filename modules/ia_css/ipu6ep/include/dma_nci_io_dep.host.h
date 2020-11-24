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

#ifndef DMA_NCI_IO_DEP_HOST_H_
#define DMA_NCI_IO_DEP_HOST_H_

#define DMA_NCI_IO_SUPPORT_CROPPING_WITH_SINGLE_CHANNEL 1

STORAGE_CLASS_INLINE void dma_cfg_set_unit_desc_cfg(
		struct nci_dma_public_unit_descriptor *unit_desc,
		uint32_t unit_width,
		uint32_t unit_height,
		uint32_t cache_policy,
		enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_INLINE void dma_cfg_set_single_terminal_desc_cfg(
		struct nci_dma_public_terminal_descriptor *terminal_desc,
		const struct local_dma_term_desc_cfg *term_desc_cfg,
		uint32_t cache_policy,
		enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_INLINE void dma_cfg_set_single_span_desc_cfg(
		struct nci_dma_public_span_descriptor *span_desc,
		const struct local_dma_span_desc_cfg *span_desc_cfg,
		uint32_t cache_policy,
		enum nci_dma_bank_mode bank_mode);

#endif /* DMA_NCI_IO_DEP_HOST_H_ */
