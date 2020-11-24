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

#ifndef __IPU_RESOURCES_DMA_PRIVATE_H
#define __IPU_RESOURCES_DMA_PRIVATE_H

#include "storage_class.h"
#include "vied_nci_psys_resource_model.h"

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_channel_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_unit_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_terminal_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_span_descriptor_size(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	const vied_nci_resource_size_t resource_size);

#if IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_request_bank_dequeue_ack_section_size(void);
#endif

#include "ipu_resources_dma_impl.h"

#endif /* __IPU_RESOURCES_DMA_PRIVATE_H */
