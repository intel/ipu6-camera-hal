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

#ifndef __NCI_DMA_DESCRIPTOR_MANAGER_H
#define __NCI_DMA_DESCRIPTOR_MANAGER_H

#include "ipu_device_dma_devices.h" /* for NCI_DMA_NUM_DEVICES ... */
#include "nci_dma_descriptor_manager_storage_class.h" /* for STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H ... */
#include "ipu_device_dma_type_properties.h" /* for nci_dma_bank_mode ... */
#include "nci_dma_descriptor_manager_struct.h" /* for ia_css_dma_descriptor_addresses */

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_num_chan(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H enum nci_dma_bank_mode nci_dma_descriptor_manager_bank_mode(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H void nci_dma_descriptor_manager_offsets(
	struct ia_css_dma_descriptor_addresses * const descriptor_addresses,
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_channel_offset(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_span_offset(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_terminal_offset(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_unit_offset(
	const enum nci_dma_device_id dev_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTOR_MANAGER_H unsigned int nci_dma_descriptor_manager_cmem_base(
	const enum nci_dma_device_id dev_id);

#ifdef __INLINE_NCI_DMA_DESCRIPTOR_MANAGER__
#include "nci_dma_descriptor_manager_impl.h"
#endif

#endif /* __NCI_DMA_DESCRIPTOR_MANAGER_H */
