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

#ifndef __DFM_DMA_CONFIG_PORT_ISP_H
#define __DFM_DMA_CONFIG_PORT_ISP_H

#ifndef PIPE_GENERATION
#include "dev_api_dfm_types.h"
#endif

#include "storage_class.h"

struct dfm_dma_seq_config {
	unsigned dma_res; /* DMA resource ID */
	unsigned chan_id;
	unsigned unit_id;
	unsigned terminal_id_A;
	unsigned terminal_id_B;
	unsigned span_id_A;
	unsigned span_id_B;
	unsigned req_id;
	unsigned macro;
	unsigned iter;
};

struct dfm_dma_seq_exception_config {
	struct dfm_dma_seq_config seq_config[DEV_API_DFM_NUM_SEQ];
};

struct dfm_buf_chase_config {
	unsigned enable;
	unsigned frame_index;
	unsigned bcpr_reg_alloc;
	unsigned iter_to_unit_ratio;
};

enum dfm_dma_direction{
	DFM_DMA_INPUT_DIRECTION = 0,
	DFM_DMA_OUTPUT_DIRECTION
};

STORAGE_CLASS_INLINE void dfm_dma_config_port(struct dev_api_dfm_stream *stream,
			 struct dev_api_dfm_port *port_config,
			 unsigned port,
			 unsigned gather_multi_mask,
			 unsigned is_active,
			 unsigned ignore_first_event,
			 unsigned num_buf,
			 struct dfm_dma_seq_exception_config *seq_except_config,
			 struct dfm_buf_chase_config *buf_chase,
			 unsigned command,
			unsigned threshold);
#ifndef PIPE_GENERATION
#include "dfm_dma_config_port.isp.c"
#endif
#endif /* __DFM_DMA_CONFIG_PORT_ISP_H */
