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

#ifndef __DMA_NCI_IO_HOST_H
#define __DMA_NCI_IO_HOST_H

#include "storage_class.h"
#include "dma_nci_io.shared.h"
#include "vied_nci_dma_types.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "dev_api_dfm_types.h"
#include "resource_types.h"
#include "buf_blk.host.h"
#include "ia_css_common_io_types.h"
#include "ipu_resources_dma_term.h"
#include "dma_nci_io_dep.host.h"

STORAGE_CLASS_INLINE void dma_cfg_set_channel_desc_cfg(
		struct nci_dma_public_channel_descriptor *channel_desc,
		uint32_t ack_addr,
		uint32_t ack_data,
		uint32_t cache_policy,
		enum nci_dma_bank_mode bank_mode,
		enum nci_dma_global_set dma_global_set);

STORAGE_CLASS_INLINE void dma_cfg_set_terminal_unit_desc_cfg(
		struct nci_dma_public_terminal_descriptor terminal_desc[N_NCI_DMA_PORT],
		struct nci_dma_public_unit_descriptor *unit_desc,
		const struct local_dma_term_desc_cfg *here,
		const struct local_dma_term_desc_cfg *there,
		uint32_t unit_width,
		uint32_t unit_height,
		uint32_t cache_policy);

STORAGE_CLASS_INLINE void dma_cfg_set_span_desc_cfg(
		struct nci_dma_public_span_descriptor span_desc[N_NCI_DMA_PORT],
		const struct local_dma_span_desc_cfg *here,
		const struct local_dma_span_desc_cfg *there,
		uint32_t cache_policy,
		enum nci_dma_bank_mode bank_mode);

STORAGE_CLASS_INLINE void dma_nci_dma_channel_configure_ext(p_terminal_config ddr_config,
		buf_blk_t buf,
		dma_resource_t resource,
		struct ipu_resources_dma_descriptor_config *terminal_config,
		uint32_t num_buf,
		uint32_t stream_id,
		uint32_t ack_addr,
		uint32_t ack_data,
		enum nci_dma_command command,
		dma_nci_data_layout_property_t layout_prop);

STORAGE_CLASS_INLINE void dma_nci_isldma_channel_configure(p_terminal_config ddr_config,
		buf_blk_t buf,
		uint32_t cio2str_stride,
		dma_resource_t resource,
		struct  ipu_resources_dma_descriptor_config *terminal_config,
		uint32_t num_buf,
		uint32_t stream_id,
		uint32_t ack_addr,
		uint32_t ack_data,
		dma_nci_data_layout_property_t layout_prop,
		uint32_t cio2str_fifo_size,
		uint32_t cio2str_databus_addr,
		uint32_t lines_per_pixel);

STORAGE_CLASS_INLINE void dma_nci_fill_dma_channel_configure_ext(p_terminal_config ddr_config,
		dma_resource_t resource,
		struct  ipu_resources_dma_descriptor_config *terminal_config,
		uint32_t stream_id,
		uint32_t ack_addr,
		uint32_t ack_data,
		uint32_t num_of_lines,
		dma_nci_data_layout_property_t layout_prop);

#ifndef PIPE_GENERATION
#include "dma_nci_io.host.c"
#include "dma_nci_io_dep.host.c"
#endif

#endif /* __DMA_NCI_IO_ISP_H */
