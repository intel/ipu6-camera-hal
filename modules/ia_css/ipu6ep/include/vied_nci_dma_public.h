/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2018 Intel Corporation.
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

#ifndef __VIED_NCI_DMA_PUBLIC_H
#define __VIED_NCI_DMA_PUBLIC_H

#include "vied_nci_dma_storage_class.h"
#include "vied_nci_dma_types.h"
#include "ipu_device_dma_devices.h"
#include "vied_nci_dma_public_dep.h"

/*
 * DMA NCI Interfaces
 */

/*
 * Open the DMA device with device id \p dev_id.
 */
STORAGE_CLASS_NCI_DMA_H struct nci_dma_dev *nci_dma_open(
	const enum nci_dma_device_id dev_id);

/*
 * Close the DMA device \p dev.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_close(
	const enum nci_dma_device_id dev_id);

/*
 * Configure the DMA device \p dev.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_configure(
	const enum nci_dma_device_id dev_id,
	const struct nci_dma_dev_config * const dev_config);

/*
 * Configure the DMA channel \p chan, with configuration \p config.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_chan_configure(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const struct nci_dma_terminal_config * const terminal_config,
	const enum nci_dma_command_type command_type);

/*
 * Configure the DMA cached terminal.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_cached_terminal_configure(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const struct nci_dma_terminal_config * const terminal_config,
	const enum nci_dma_command_type command_type);

/*
 * Precompute the DMA request descriptor for \p chan for DMA transfers.
 * Primarily to test the mixed physical and cached descriptor functionality
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_chan_compute_and_update_req_desc_mixed(
	const struct nci_dma_mixed_chan * const dma_mixed_chan,
	const uint32_t cached_chan_index);

/*
 * memcpy move the DMA channel \p chan, with memcpy \p config.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_memcpy_move(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const struct nci_dma_memcpy_config * const memcpy_config);

/*
 * memcpy fill the DMA channel \p chan, with memcpy \p config.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_memcpy_fill(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_terminal term,
	const enum nci_dma_command command,
	const struct nci_dma_memcpy_config * const memcpy_config);

/*
 * memcpy 2d (unaligned) the DMA channel \p chan, with memcpy \p config.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_memcpy_2d(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const struct nci_dma_memcpy_2d_config * const memcpy_2d_config);

/*
 * memcpy 1d (coordinate-based) the DMA channel \p chan, with memcpy \p config.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_memcpy_1d(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const struct nci_dma_memcpy_1d_config * const memcpy_1d_config);

/*
 * Open the DMA channel \p chan_id, of DMA device \p dev_id.
 */
STORAGE_CLASS_NCI_DMA_H struct nci_dma_chan *nci_dma_chan_open(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const struct nci_dma_transfer_config * const transfer_config);

/*
 * Open the DMA cached terminal \p chan_id, of DMA device \p dev_id.
 */
STORAGE_CLASS_NCI_DMA_H struct nci_dma_chan *nci_dma_cached_terminal_open(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const enum nci_dma_requestor_id requestor_id);

/*
 * Start the initial DMA transfer on channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_chan_start(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id);

/*
 * Start the subsequent DMA transfers on channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H int nci_dma_chan_next(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const uint32_t n);

STORAGE_CLASS_NCI_DMA_C int nci_dma_chan_next_shared(
	const enum nci_dma_device_id dev_id,
	const enum nci_dma_requestor_id requestor_id,
	const struct nci_dma_request_desc * const request_descr,
	const enum nci_dma_command command,
	const uint32_t n);

/*
 * Stop DMA transfer on DMA channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_chan_stop(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id);

/*
 * Close DMA channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_chan_close(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id);

/*
 * Get resource assigned to a channel \p chan.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_get_chan_res(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	struct nci_dma_chan_res * const res,
	const enum nci_dma_resource_type res_type);

/*
 * Open a physical channel and re-arrange spans for the cached ones.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_mixed_chan_config(
	struct nci_dma_mixed_chan * const dma_mixed_chan,
	const struct nci_dma_transfer_config * const transfer_config,
	const struct nci_dma_terminal_config * const terminal_config);

/*
 * Reconfiguring ack data and ack address for a channel.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_reconfig_ack_info(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	const uint32_t ack_addr,
	const uint32_t ack_data);

/*
 * Configure partial terminal desc.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_terminal_partial_conf(
	const enum nci_dma_device_id cached_dev_id,
	const uint32_t cached_chan_id,
	const enum nci_dma_device_id physical_dev_id,
	const uint32_t physical_chan_id,
	const struct nci_dma_partial_terminal_desc_config * const partial_terminal_desc_config);

/*
 * Reconfiguring cio_info for a terminal (for enabling / disabling) compression on a per frame basis
 */
STORAGE_CLASS_NCI_DMA_C void nci_dma_terminal_reconfig_cio_info(
	const enum nci_dma_device_id dev_id,
	const uint32_t terminal_id,
	const uint32_t cio_info_setup);

/*
 * Configure partial physical terminal desc.
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_physical_terminal_partial_conf(
	const enum nci_dma_device_id physical_dev_id,
	const uint32_t physical_chan_id,
	const nci_dma_partial_terminal_desc_config_t partial_terminal_desc_config);

/*
 * Get DMA descriptor's address associated with physical channel
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_get_chan_resources_addr(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id,
	struct nci_dma_chan_res_addr * const dma_desc_info);

/*
 * Get DMA descriptor's address associated with mixed mode channel
 */
STORAGE_CLASS_NCI_DMA_H void nci_dma_get_mixed_chan_resources_addr(
	const struct nci_dma_mixed_chan * const dma_mixed_chan,
	struct nci_dma_chan_res_addr * const dma_desc_info);

/*
 * Compute descriptor ID setup1 register value of request descriptor
 * for a mixed mode channel
 * @param[in]	dma_mixed_chan	Mixed Mode DMA Channel Handler
 * @param[in]	chan_index	Index of cached channel in the mixed mode
 * channel handler for which descriptor ID setup1 has to be retrieved
 * @return	Descriptor ID setup1 register of the request descriptor
 */
STORAGE_CLASS_NCI_DMA_H uint32_t nci_dma_mixed_chan_compute_setup1(
	const struct nci_dma_mixed_chan * const dma_mixed_chan,
	const int chan_index);

/*
 * Compute descriptor ID setup2 register value of request descriptor
 * for a mixed mode channel
 * @param[in]	dma_mixed_chan	Mixed Mode DMA Channel Handler
 * @return	Descriptor ID setup2 register of the request descriptor
 */
STORAGE_CLASS_NCI_DMA_H uint32_t nci_dma_mixed_chan_compute_setup2(
	const struct nci_dma_mixed_chan * const dma_mixed_chan);

/*
 * Compute descriptor ID setup1 register value of request descriptor
 * for a channel
 * @param[in]	chan	DMA Channel Handler
 * @return	Descriptor ID setup1 register of the request descriptor
 */
STORAGE_CLASS_NCI_DMA_H uint32_t nci_dma_chan_compute_setup1(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id);

/*
 * Compute descriptor ID setup2 register value of the request descriptor
 * for a channel
 * @param[in]	chan	DMA Channel Handler
 * @return	Descriptor ID setup2 register of the request descriptor
 */
STORAGE_CLASS_NCI_DMA_H uint32_t nci_dma_chan_compute_setup2(
	const enum nci_dma_device_id dev_id,
	const uint32_t chan_id);

#endif /* __VIED_NCI_DMA_PUBLIC_H */
