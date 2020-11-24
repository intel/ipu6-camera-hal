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

#ifndef __IPU_RESOURCES_DMA_H
#define __IPU_RESOURCES_DMA_H

#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "storage_class.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"

/*!
 * Returns the NCI DMA device ID given a vied_nci_dev_chn_ID_t value (from resource_model).
 */
STORAGE_CLASS_INLINE enum nci_dma_device_id ipu_resources_dma_get_nci_dev_id(
	const vied_nci_dev_chn_ID_t dev);

/*!
 * Returns the NCI DMA channel ID given a vied_nci_dev_chn_ID_t
 *   value and vied_nci_resource_id_t (from resource_model).
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_nci_chan_id(
	const vied_nci_dev_chn_ID_t dev,
	enum nci_dma_bank_mode bank_mode,
	vied_nci_resource_size_t resource_offset);

/*!
 * Gets the size for a consecutive set of DMA descriptors.
 * @param dev            Device channel ID from the resource model.
 * @param resource_size  Number of assigned resources.
 * @param section_id     The type of descriptor to get the address for.
 * @return               Size in bytes.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_descriptor_size(
	const vied_nci_dev_chn_ID_t	dev,
	const vied_nci_resource_size_t	resource_size,
	const vied_nci_dma_section_id_t	section_id);

/*!
 * Gets the total payload size for a set of DMA channels.
 * @param dev            Device channel ID from the resource model.
 * @param resource_size  Number of assigned resources.
 * @return               Size in bytes.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_payload_size(
	const vied_nci_dev_chn_ID_t		dev,
	const vied_nci_resource_size_t		resource_size);

/*!
 * This function checks if a DMA section_id is a managed resource (by SW)
 *
 * @param dev            Device channel ID from the resource model.
 * @param section_id     Section identifier in section descriptor.
 *
 * @return               True if the section_id  is a dma bank (resource)
 *                       that is managed by SW, False otherwise
 */
STORAGE_CLASS_INLINE bool ipu_resources_dma_section_is_managed_resource(
		const vied_nci_dev_chn_ID_t dev,
		const vied_nci_dma_section_id_t section_id);

#include "ipu_resources_dma_impl.h"

#endif /* __IPU_RESOURCES_DMA_H */
