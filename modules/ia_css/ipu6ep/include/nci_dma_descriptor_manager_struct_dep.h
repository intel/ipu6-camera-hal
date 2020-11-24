/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2020 Intel Corporation.
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

#ifndef __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_DEP_H
#define __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_DEP_H

#include "nci_dma_descriptors.h" /* for nci_dma_descriptors_cached_channel_descriptor ... */
#include "ipu_device_dma_properties_defs.h" /* for IPU_DEVICE_DMA_ISA_CHANNELS */

/* Define Maximum Number of Channels Per DMA Device */
#define DMA_EXT0_NUM_LOGICAL_CHANNELS (30)
#define DMA_EXT1R_NUM_LOGICAL_CHANNELS (30)
/* NOTE: once FW code will support DMA with one unit descriptor #channels will be cut but half (36->18) */
#define DMA_EXT1W_NUM_LOGICAL_CHANNELS (43)

#define DMA_INT_NUM_LOGICAL_CHANNELS (8)

#define DMA_ISA_NUM_LOGICAL_CHANNELS (IPU_DEVICE_DMA_ISA_CHANNELS)

struct ia_css_dma_cached_ext0_descriptor {
	struct nci_dma_cached_terminal_descriptor ia_css_dma_cached_ext0_terminal_descriptor[2 * DMA_EXT0_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_span_descriptor ia_css_dma_cached_ext0_span_descriptor[2 * DMA_EXT0_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_channel_descriptor ia_css_dma_cached_ext0_channel_descriptor[DMA_EXT0_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_unit_descriptor ia_css_dma_cached_ext0_unit_descriptor[DMA_EXT0_NUM_LOGICAL_CHANNELS];
};

struct ia_css_dma_cached_ext1r_descriptor {
	struct nci_dma_cached_terminal_descriptor ia_css_dma_cached_ext1r_terminal_descriptor[2 * DMA_EXT1R_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_span_descriptor ia_css_dma_cached_ext1r_span_descriptor[2 * DMA_EXT1R_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_channel_descriptor ia_css_dma_cached_ext1r_channel_descriptor[DMA_EXT1R_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_unit_descriptor ia_css_dma_cached_ext1r_unit_descriptor[DMA_EXT1R_NUM_LOGICAL_CHANNELS];
};

struct ia_css_dma_cached_ext1w_descriptor {
	struct nci_dma_cached_terminal_descriptor ia_css_dma_cached_ext1w_terminal_descriptor[2 * DMA_EXT1W_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_span_descriptor ia_css_dma_cached_ext1w_span_descriptor[2 * DMA_EXT1W_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_channel_descriptor ia_css_dma_cached_ext1w_channel_descriptor[DMA_EXT1W_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_unit_descriptor ia_css_dma_cached_ext1w_unit_descriptor[DMA_EXT1W_NUM_LOGICAL_CHANNELS];
};

struct ia_css_dma_cached_int_descriptor {
	struct nci_dma_cached_terminal_descriptor ia_css_dma_cached_int_terminal_descriptor[2 * DMA_INT_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_span_descriptor ia_css_dma_cached_int_span_descriptor[2 * DMA_INT_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_channel_descriptor ia_css_dma_cached_int_channel_descriptor[DMA_INT_NUM_LOGICAL_CHANNELS];
	struct nci_dma_cached_unit_descriptor ia_css_dma_cached_int_unit_descriptor[DMA_INT_NUM_LOGICAL_CHANNELS];
};

struct ia_css_dma_cached_descriptor {
	struct ia_css_dma_cached_ext0_descriptor ia_css_dma_cached_ext0_descriptors;
	struct ia_css_dma_cached_ext1r_descriptor ia_css_dma_cached_ext1r_descriptors;
	struct ia_css_dma_cached_ext1w_descriptor ia_css_dma_cached_ext1w_descriptors;
	struct ia_css_dma_cached_int_descriptor ia_css_dma_cached_int_descriptors;
};

#endif /* __NCI_DMA_DESCRIPTOR_MANAGER_STRUCT_DEP_H */
