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

#ifndef __IPU_RESOURCES_DMA_ADDR_H
#define __IPU_RESOURCES_DMA_ADDR_H

#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "storage_class.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"

#define IPU_RESOURCES_DMA_REGION_ORIGIN_SIZE (sizeof(uint32_t))

/*!
 * Gets the address (SPC point of view) for a DMA descriptor section of the program control init terminal.
 * @param dev             Device channel ID from the resource model.
 * @param resource_offset Assigned resource value according to the resource model.
 * @param section_id      The type of descriptor to get the address for.
 * @return                CMEM bus address of DMA descriptor from point of view of SPC.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_descriptor_address(
	const vied_nci_dev_chn_ID_t	dev,
	const vied_nci_resource_size_t	resource_offset,
	const vied_nci_dma_section_id_t	section_id);

/*!
 * Gets the address (SPC point of view) for the DMA terminal descriptor region origin.
 * @param dev             Device channel ID from the resource model.
 * @param resource_offset Assigned resource value according to the resource model.
 * @param section_id      The descriptor to get the address for.
 * @return                CMEM bus address of DMA descriptor from point of view of SPC.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_region_origin_address(
	const vied_nci_dev_chn_ID_t             dev,
	const vied_nci_resource_size_t          resource_offset,
	const vied_nci_dma_connect_section_id_t section_id);

#include "ipu_resources_dma_addr_impl.h"

#endif /* __IPU_RESOURCES_DMA_ADDR_H */
