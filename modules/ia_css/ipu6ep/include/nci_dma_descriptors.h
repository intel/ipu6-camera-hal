/**
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

#ifndef __NCI_DMA_DESCRIPTORS_H
#define __NCI_DMA_DESCRIPTORS_H

#include "type_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "nci_dma_descriptors_storage_class.h"
#include "nci_dma_descriptors_types_channel.h"
#include "nci_dma_descriptors_types_global.h"
#include "nci_dma_descriptors_types_master.h"
#include "nci_dma_descriptors_types_request.h"
#include "nci_dma_descriptors_types_span.h"
#include "nci_dma_descriptors_types_terminal.h"
#include "nci_dma_descriptors_types_unit.h"
#include "nci_dma_descriptors_dep.h"

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_non_cached_descriptor_base(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_channel_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_channel_descriptor * const public_channel_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_request_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id,
	const struct nci_dma_public_request_descriptor * const public_request_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_global_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id,
	const struct nci_dma_public_global_descriptor * const public_global_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_master_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id,
	const struct nci_dma_public_master_descriptor * const public_master_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_span_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_span_descriptor * const public_span_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_terminal_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_terminal_descriptor * const public_terminal_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_unit_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode,
	const struct nci_dma_public_unit_descriptor * const public_unit_descriptor,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_channel_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_descriptor_size(void);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_bank_dequeue_ack_section_size(void);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_global_descriptor_size(void);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_master_descriptor_size(void);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_span_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_terminal_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_unit_descriptor_size(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_channel_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_span_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_terminal_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_unit_descriptor_offset(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_global_descriptor_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_master_descriptor_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_descriptor_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_bank_dequeue_ack_section_address(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H unsigned int nci_dma_invalidate_instruction(
	const unsigned int dev_id,
	const enum nci_dma_descriptor_kind descriptor_kind,
	const unsigned int lower_id,
	const unsigned int upper_id);

#ifdef __INLINE_NCI_DMA_DESCRIPTORS__
#include "nci_dma_descriptors_impl.h"
#endif

#endif /* __NCI_DMA_DESCRIPTORS_H */
