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

#ifndef __NCI_DMA_DESCRIPTORS_PRINT_V4_H
#define __NCI_DMA_DESCRIPTORS_PRINT_V4_H

#include "storage_class.h"
#include "misc_support.h"
#include "assert_support.h"
#include "ia_css_trace.h"
#include "nci_dma_descriptors.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_properties_defs.h"
#include "nci_dma_descriptors_trace.h"
#include "nci_dma_descriptors_print_dep.h"

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_channel_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(channel_id);
	NOT_USED(bank_mode);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	IA_CSS_ASSERT(((NCI_DMA_BANK_MODE_CACHED == bank_mode) &&
		(ipu_device_dma_channels(dev_id) > channel_id)) ||
		((NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) &&
		(ipu_device_dma_channel_banks(dev_id) > channel_id)));
	IA_CSS_ASSERT(NULL != buffer);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_descriptors_print_non_cached_channel_descriptor(
			(const struct nci_dma_non_cached_channel_descriptor * const)buffer);
	} else {
		nci_dma_descriptors_print_cached_channel_descriptor(dev_id, channel_id, buffer);
	}
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_span_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(span_id);
	NOT_USED(bank_mode);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	IA_CSS_ASSERT(((NCI_DMA_BANK_MODE_CACHED == bank_mode) &&
		(ipu_device_dma_spans(dev_id) > span_id)) ||
		((NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) &&
		(ipu_device_dma_span_banks(dev_id) > span_id)));
	IA_CSS_ASSERT(NULL != buffer);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_descriptors_print_non_cached_span_descriptor(
			(const struct nci_dma_non_cached_span_descriptor * const)buffer);
	} else {
		nci_dma_descriptors_print_cached_span_descriptor(dev_id, span_id, buffer);
	}
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_terminal_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(terminal_id);
	NOT_USED(bank_mode);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	IA_CSS_ASSERT(((NCI_DMA_BANK_MODE_CACHED == bank_mode) &&
		(ipu_device_dma_terminals(dev_id) > terminal_id)) ||
		((NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) &&
		(ipu_device_dma_terminal_banks(dev_id) > terminal_id)));
	IA_CSS_ASSERT(NULL != buffer);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_descriptors_print_non_cached_terminal_descriptor(
			(const struct nci_dma_non_cached_terminal_descriptor * const)buffer);
	} else {
		nci_dma_descriptors_print_cached_terminal_descriptor(dev_id, terminal_id, buffer);
	}
}

STORAGE_CLASS_INLINE void nci_dma_descriptors_print_private_unit_descriptor(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode,
	const void * const buffer)
{
	NOT_USED(dev_id);
	NOT_USED(unit_id);
	NOT_USED(bank_mode);
	NOT_USED(buffer);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	IA_CSS_ASSERT(((NCI_DMA_BANK_MODE_CACHED == bank_mode) &&
		(ipu_device_dma_units(dev_id) > unit_id)) ||
		((NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) &&
		(ipu_device_dma_unit_banks(dev_id) > unit_id)));
	IA_CSS_ASSERT(NULL != buffer);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_descriptors_print_non_cached_unit_descriptor(
			(const struct nci_dma_non_cached_unit_descriptor * const)buffer);
	} else {
		nci_dma_descriptors_print_cached_unit_descriptor(dev_id, unit_id, buffer);
	}
}

#endif /* __NCI_DMA_DESCRIPTORS_PRINT_V4_H */
