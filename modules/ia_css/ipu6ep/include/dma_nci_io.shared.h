/*
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

#ifndef __DMA_NCI_IO_SHARED_H
#define __DMA_NCI_IO_SHARED_H

#include "vied_nci_dma_types.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "dev_api_dfm_types.h"
#include "dma_resource.isp.h"

struct local_dma_term_desc_cfg {
	uint32_t loc;
	uint32_t bpe;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t info_bits;
};

struct local_dma_span_desc_cfg {
	uint32_t span_width;
	uint32_t span_height;
	uint32_t x_coord;
	uint32_t y_coord;
};

typedef __register struct {
	bool is_packed;
	unsigned hor_interleave_factor;
	unsigned ver_interleave_factor;
	unsigned hor_subsampling_factor;
	unsigned ver_subsampling_factor;
} dma_nci_data_layout_property_t;

STORAGE_CLASS_INLINE uint32_t
dma_nci_get_fragment_offset(uint32_t fragment_row, uint32_t fragment_col, uint32_t stride, uint32_t bpe);

STORAGE_CLASS_INLINE enum nci_dma_device_id dma_resource_to_dev_id(unsigned res_id);

STORAGE_CLASS_INLINE enum dev_api_dfm_agent dma_resource_to_dfm_agent(unsigned res_id);

STORAGE_CLASS_INLINE uint32_t dma_nci_get_info_bits(dma_resource_t resource,
					     const uint32_t stream_id);

STORAGE_CLASS_INLINE void dma_cfg_get_virtual_desc_id(
					const enum nci_dma_device_id dev_id,
					const uint32_t resource_offset,
					const enum nci_dma_bank_mode bank_mode,
					uint32_t *cached_chan_id,
					uint32_t *span_id_A,
					uint32_t *span_id_B,
					uint32_t *terminal_id_A,
					uint32_t *terminal_id_B,
					uint32_t *unit_id);

#ifndef PIPE_GENERATION
#include "dma_nci_io.shared.c"
#endif

#endif /* __DMA_NCI_IO_SHARED_H */
