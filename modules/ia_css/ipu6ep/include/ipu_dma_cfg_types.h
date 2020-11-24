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

#ifndef __IPU_DMA_CFG_TYPES_H
#define __IPU_DMA_CFG_TYPES_H

/* WARNING: DON'T CHANGE THE ORDER OF THE FIELDS AS THEY REPRESENT THE REGISTER
 * LAYOUT.
 */

struct ipu_dma_cfg {
	uint32_t unit_desc_id;			/* start unit descriptor id */
	uint32_t channel_desc_id;		/* start channel descriptor id */
	uint32_t spanA_desc_id;			/* start Span A descriptor id */
	uint32_t spanB_desc_id;			/* start Span B descriptor id */
	uint32_t terminalA_desc_id;		/* start Terminal A descriptor id */
	uint32_t terminalB_desc_id;		/* start Terminal B descriptor id */
	uint32_t unit_desc_address;		/* Unit descriptor address in Transfer DMEM */
	uint32_t unit_desc_stride;		/* Stride for jumping from 1 unit descriptor to the next one */
	uint32_t spanA_desc_address;		/* Span A descriptor address in Transfer DMEM */
	uint32_t spanB_desc_address;		/* Span B descriptor address in Transfer DMEM */
	uint32_t span_desc_stride;		/* Stride for jumping from 1 span descriptor to the next one */
	uint32_t terminalA_desc_address;	/* Terminal A descriptor address in Transfer DMEM */
	uint32_t terminalB_desc_address;	/* Terminal B descriptor address in Transfer DMEM */
	uint32_t terminal_desc_stride;		/* Stride for jumping from 1 terminal descriptor to the next one */
	uint32_t DMA_req_queue_size;		/* DMA requestor Bank Queue depth */
};

struct ipu_dma_desc_id_cfg {
	uint32_t unit_desc_id;			/* start unit descriptor id */
	uint32_t channel_desc_id;		/* start channel descriptor id */
	uint32_t spanA_desc_id;			/* start Span A descriptor id */
	uint32_t spanB_desc_id;			/* start Span B descriptor id */
	uint32_t terminalA_desc_id;		/* start Terminal A descriptor id */
	uint32_t terminalB_desc_id;		/* start Terminal B descriptor id */
};

#endif /* __IPU_DMA_CFG_TYPES_H */
