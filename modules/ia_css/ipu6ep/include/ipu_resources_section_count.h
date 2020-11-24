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

#ifndef __IPU_RESOURCES_SECTION_COUNT_H
#define __IPU_RESOURCES_SECTION_COUNT_H

#include "storage_class.h"
#include "vied_nci_psys_resource_model.h"
#if HAS_DFM
#include "dfm_resource_types.h"
#endif

/*!
 * Returns the number of sections for a DMA resources.
 * @param dev            Device channel ID from the resource model.
 * @return               The number of section per DMA (channel) resource.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_progctrlinit_section_count(
	const vied_nci_dev_chn_ID_t		dev);

/*!
 * Returns the number of connect sections for a DMA resources.
 * @param dev            Device channel ID from the resource model.
 * @return               The number of section per DMA (channel) resource.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_progctrlinit_connect_section_count(
	const vied_nci_dev_chn_ID_t		dev);

#if HAS_DFM
/*!
 * Returns the number of sections for a single DFM resource (port).
 * @param dev            Device channel ID from the resource model.
 * @return               The number of section per DFM (channel) resource.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_section_count(
	const vied_nci_dev_dfm_id_t  dev);

/*!
 * Returns the number of connect sections for a single DFM resource (port).
 * @param dev            Device channel ID from the resource model.
 * @return               The number of connect section per DFM (channel) resource.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_connect_section_count(
	const vied_nci_dev_dfm_id_t  dev);
/*!
 * Returns the number of sections for a DFM resource (consecutive ports)
 * @param dfm_resource   DFM resource.
 * @return               The number of section per DMA (channel) resource.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_section_count_resource(
	dfm_port_resource_t dfm_resource);
#endif

#include "ipu_resources_section_count_impl.h"

#endif /* __IPU_RESOURCES_SECTION_COUNT_H */
