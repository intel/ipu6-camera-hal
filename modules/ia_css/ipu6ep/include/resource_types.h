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

#ifndef RESOURCE_TYPES_H_
#define RESOURCE_TYPES_H_

#include "storage_class.h" /* __register */
#include "system_global.h" /* resource_id */
#include "math_support.h"  /* MAX */

/* Resource type.
 * To be replaced by mem_resource_t after refactoring resource_id_t.
 */
typedef __register struct {
	resource_type_t type;
	resource_id_t   id;
	unsigned        value; /* offset */
	unsigned	size;
} resource_t;

typedef __register struct {
	resource_type_t type;
	buf_mem_t       id;
	unsigned        value;
} mem_resource_t;

typedef __register struct {
	resource_type_t type;
	resource_id_t   id;
	unsigned        value;
} dma_chan_resource_t;

/*dma resources*/
typedef __register struct {
	resource_id_t id;
	uint32_t dma_channel;
	uint16_t local_idx;
	uint16_t size;
} dma_resource_t;

enum yuv_io_dma_channels {
	YUV_IO_DMA_CHANNEL_Y,
	YUV_IO_DMA_CHANNEL_U,
	YUV_IO_DMA_CHANNEL_V
};
#define YUV_IO_DMA_MAX_PLANES 3

enum bayer_io_dma_channels {
	BAYER_IO_DMA_CHANNEL_R,
	BAYER_IO_DMA_CHANNEL_GR,
	BAYER_IO_DMA_CHANNEL_GB,
	BAYER_IO_DMA_CHANNEL_B
};
#define BAYER_IO_DMA_MAX_PLANES 4

#define IO_DMA_MAX_PLANES MAX(YUV_IO_DMA_MAX_PLANES, BAYER_IO_DMA_MAX_PLANES)

#ifdef PIPE_GENERATION
/* alloc_resource expects the size in number of smallest allocatable units of the specified resource.
   In case of dma channels this is a single channel, in case of vmem this is vectors, in case of dmem
   this is words.
   The function will return a struct that contains the resource type and id, and the
   offset of the requested resource. This offset is also in number of smallest allocatable units.
   These functions are only used in .pipe files (only prototypes are required by genpipe).
   */
resource_t alloc_resource(resource_type_t type, resource_id_t id, unsigned size);
mem_resource_t alloc_mem_resource(resource_type_t type, buf_mem_t id, unsigned size);
dma_chan_resource_t alloc_dma_resource(resource_type_t type, resource_id_t id, unsigned size);
#endif

#endif /* RESOURCE_TYPES_H_ */
