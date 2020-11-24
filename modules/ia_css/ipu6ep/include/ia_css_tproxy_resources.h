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

#ifndef __V6_IA_CSS_TPROXY_RESOURCES_H
#define __V6_IA_CSS_TPROXY_RESOURCES_H

#include "nci_dma_descriptor_manager_struct_dep.h" /* for DMA_EXT0_NUM_LOGICAL_CHANNELS ... */

/* Offset to each Device Base Channel */
#define DMA_EXT0_BASE_CHANNEL_OFFSET  (0)
#define DMA_EXT1R_BASE_CHANNEL_OFFSET (DMA_EXT0_BASE_CHANNEL_OFFSET + DMA_EXT0_NUM_LOGICAL_CHANNELS)
#define DMA_EXT1W_BASE_CHANNEL_OFFSET (DMA_EXT1R_BASE_CHANNEL_OFFSET + DMA_EXT1R_NUM_LOGICAL_CHANNELS)
#define DMA_INT_BASE_CHANNEL_OFFSET   (DMA_EXT1W_BASE_CHANNEL_OFFSET + DMA_EXT1W_NUM_LOGICAL_CHANNELS)
#define DMA_ISA_BASE_CHANNEL_OFFSET   (DMA_INT_BASE_CHANNEL_OFFSET + DMA_INT_NUM_LOGICAL_CHANNELS)

/* Channel ID - Absolute Base for Each Device Type */
#define DMA_EXT0_CHANNEL_BASE  (DMA_EXT0_BASE_CHANNEL_OFFSET)
#define DMA_EXT1R_CHANNEL_BASE (DMA_EXT1R_BASE_CHANNEL_OFFSET)
#define DMA_EXT1W_CHANNEL_BASE (DMA_EXT1W_BASE_CHANNEL_OFFSET)
#define DMA_INT_CHANNEL_BASE   (DMA_INT_BASE_CHANNEL_OFFSET)
#define DMA_ISA_CHANNEL_BASE   (DMA_ISA_BASE_CHANNEL_OFFSET)

#define IA_CSS_N_TPROXY_CHAN (DMA_EXT0_NUM_LOGICAL_CHANNELS + \
				DMA_EXT1R_NUM_LOGICAL_CHANNELS +\
				DMA_EXT1W_NUM_LOGICAL_CHANNELS +\
				DMA_INT_NUM_LOGICAL_CHANNELS +\
				DMA_ISA_NUM_LOGICAL_CHANNELS)

#endif /* __V6_IA_CSS_TPROXY_RESOURCES_H */
