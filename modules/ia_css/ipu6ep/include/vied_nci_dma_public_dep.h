/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2017 Intel Corporation.
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

/*! \file */

#ifndef __VIED_NCI_DMA_PUBLIC_DEP_H
#define __VIED_NCI_DMA_PUBLIC_DEP_H

#include "vied_nci_dma_storage_class.h"
#include "vied_nci_dma_types.h"
#include "ipu_device_dma_devices.h"

/*
 * DMA NCI Interfaces
 */

/*
 * Start a slim next DMA transfer on channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_slim_next(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const uint32_t macro_size,
	const enum nci_dma_command command);

/*
* Start a fast 1D DMA transfer on channel \p chan.
*/
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_fast_1d(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const uint32_t unit_width,
	const enum nci_dma_command command,
	const uint32_t address_a,
	const uint32_t address_b);

STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_next_shared(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const uint32_t n);

STORAGE_CLASS_NCI_DMA_H void nci_dma_request_bank_polling_configure(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_requestor_id requestor_id,
	unsigned int addr_value);
#endif /* __VIED_NCI_DMA_PUBLIC_DEP_H */
