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

#ifndef __NCI_DMA_DESCRIPTORS_CHECKS_DEP_H
#define __NCI_DMA_DESCRIPTORS_CHECKS_DEP_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"
#include "misc_support.h"
#include "math_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "ipu_device_dma_type_properties.h"
#include "nci_dma_descriptors_types.h"
#include "nci_dma_descriptors_storage_class.h"
#include "nci_dma_descriptors_checks_common.h"

STORAGE_CLASS_INLINE void nci_dma_check_channel_structures(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_check_non_cached_channel_structure();
	} else {
		nci_dma_check_cached_channel_structure(dev_id);
	}
}

STORAGE_CLASS_INLINE void nci_dma_check_span_structures(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_check_non_cached_span_structure();
	} else {
		nci_dma_check_cached_span_structure(dev_id);
	}
}

STORAGE_CLASS_INLINE void nci_dma_check_terminal_structures(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_check_non_cached_terminal_structure();
	} else {
		nci_dma_check_cached_terminal_structure(dev_id);
	}
}

STORAGE_CLASS_INLINE void nci_dma_check_unit_structures(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_bank_mode bank_mode)
{
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	NOT_USED(dev_id);
	NOT_USED(bank_mode);

	if (NCI_DMA_BANK_MODE_NON_CACHED == bank_mode) {
		nci_dma_check_non_cached_unit_structure();
	} else {
		nci_dma_check_cached_unit_structure(dev_id);
	}
}

STORAGE_CLASS_INLINE void nci_dma_check_all_structures(void)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < NCI_DMA_NUM_DEVICES; dev_id++) {
		nci_dma_check_channel_structures(dev_id, NCI_DMA_BANK_MODE_NON_CACHED);
		nci_dma_check_span_structures(dev_id, NCI_DMA_BANK_MODE_NON_CACHED);
		nci_dma_check_terminal_structures(dev_id, NCI_DMA_BANK_MODE_NON_CACHED);
		nci_dma_check_unit_structures(dev_id, NCI_DMA_BANK_MODE_NON_CACHED);
		if (ipu_device_dma_bank_mode_register_available(dev_id)) {
			nci_dma_check_channel_structures(dev_id, NCI_DMA_BANK_MODE_CACHED);
			nci_dma_check_span_structures(dev_id, NCI_DMA_BANK_MODE_CACHED);
			nci_dma_check_terminal_structures(dev_id, NCI_DMA_BANK_MODE_CACHED);
			nci_dma_check_unit_structures(dev_id, NCI_DMA_BANK_MODE_CACHED);
		}
	}
}

#endif /* __NCI_DMA_DESCRIPTORS_CHECKS_DEP_H */
