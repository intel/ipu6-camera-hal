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

#ifndef __IPU_RESOURCES_SECTION_COUNT_IMPL_H
#define __IPU_RESOURCES_SECTION_COUNT_IMPL_H

#include "ipu_resources_section_count.h"
#include "ipu_resources_dep.h"
#include "vied_nci_psys_resource_model_private.h"
#include "misc_support.h"

/* Default number of sections in a logical DMA channel, i.e.,
 * channel, span, terminal and unit descriptors.
 */
#define DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS    4

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_progctrlinit_section_count(
	const vied_nci_dev_chn_ID_t		dev)
{
	assert(resource_model_dev_chn_2_nci_dma_dev[dev] < NCI_DMA_NUM_DEVICES);
	NOT_USED(dev);
	return DMA_NUMBER_OF_PROGCTRLINIT_SECTIONS;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dma_get_progctrlinit_connect_section_count(
	const vied_nci_dev_chn_ID_t		dev)
{
	assert(resource_model_dev_chn_2_nci_dma_dev[dev] < NCI_DMA_NUM_DEVICES);
	return resource_model_dev_chn_2_connect_section_count[dev];
}

#if HAS_DFM
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_section_count(
	const vied_nci_dev_dfm_id_t  dev)
{
	assert(resource_model_dfm_dev_2_nci_dfm_dev[dev] < IPU_DEVICE_DFM_NUM_DEVICES);
	NOT_USED(dev);
	return VIED_NCI_DFM_SECTION_ID_N;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_connect_section_count(
	const vied_nci_dev_dfm_id_t  dev)
{
	assert(resource_model_dfm_dev_2_nci_dfm_dev[dev] < IPU_DEVICE_DFM_NUM_DEVICES);
	NOT_USED(dev);
	return VIED_NCI_DFM_CONNECT_SECTION_ID_N;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_section_count_resource(
	dfm_port_resource_t dfm_resource)
{
	return (dfm_resource.value * ipu_resources_dfm_get_progctrlinit_section_count(dfm_resource.id));
}

#endif
#endif /* __IPU_RESOURCES_SECTION_COUNT_IMPL_H */
