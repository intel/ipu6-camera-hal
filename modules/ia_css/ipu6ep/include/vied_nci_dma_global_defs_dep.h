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

#ifndef __VIED_NCI_DMA_GLOBAL_DEFS_DEP_H
#define __VIED_NCI_DMA_GLOBAL_DEFS_DEP_H

/**
  Precision Bits supported by DMA PSYS EXT0
	8bit, 10bit, 12bit, 16bit
*/
enum nci_dma_element_setup_ext0 {
	NCI_DMA_EXT0_ELEMENT_PRECISION_8BIT,
	NCI_DMA_EXT0_ELEMENT_PRECISION_10BIT,
	NCI_DMA_EXT0_ELEMENT_PRECISION_12BIT,
	NCI_DMA_EXT0_ELEMENT_PRECISION_16BIT,
	NCI_DMA_NUM_EXT0_ELEMENT_PRECISION
};

/**
  Precision Bits supported by DMA PSYS EXT1
	8bit, 10bit, 12bit, 16bit
*/
enum nci_dma_element_setup_ext1 {
	NCI_DMA_EXT1_ELEMENT_PRECISION_8BIT,
	NCI_DMA_EXT1_ELEMENT_PRECISION_10BIT,
	NCI_DMA_EXT1_ELEMENT_PRECISION_12BIT,
	NCI_DMA_EXT1_ELEMENT_PRECISION_16BIT,
	NCI_DMA_NUM_EXT1_ELEMENT_PRECISION
};

/**
 *  Precision Bits supported by DMA PSYS INTERNAL
 * 8bit, 16bit
*/
enum nci_dma_element_setup_internal {
	NCI_DMA_INTERNAL_ELEMENT_PRECISION_8BIT,
	NCI_DMA_INTERNAL_ELEMENT_PRECISION_16BIT,
	NCI_DMA_NUM_INTERNAL_ELEMENT_PRECISION
};

#define MAX_NUM_PRECISIONS 4
#define MAX_NUM_SUBSAMPLING_FACTORS 1

#endif /* __VIED_NCI_DMA_GLOBAL_DEFS_DEP_H */
