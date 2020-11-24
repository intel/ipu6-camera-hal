/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2018 - 2020 Intel Corporation.
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

#ifndef __ISA_SHARED_DEFS_DEPH
#define __ISA_SHARED_DEFS_DEPH

/* Defines for output buffers - TRANSFER VMEM */
/* Bayer full resolution */
#define OUTPUT_BAYER_NUMBUF			(4)

/* Video YUV Planar */
#define VIDEO_FULL_Q_OUT_NUMBUF		OUTPUT_BAYER_NUMBUF

#define GET_MIN_WIDTH		(256)
#define GET_MIN_HEIGHT		(128)
#define GET_MAX_WIDTH		(8192)
#define GET_MAX_HEIGHT		(8192)

#define GET_PAF_MIN_WIDTH	GET_MIN_WIDTH
#define GET_PAF_MIN_HEIGHT	GET_MIN_HEIGHT
#define GET_PAF_MAX_WIDTH	GET_MAX_WIDTH
#define GET_PAF_MAX_HEIGHT	GET_MAX_HEIGHT

#if HAS_FF_DOL_DOL
#define GET_SHORT_MIN_WIDTH	GET_MIN_WIDTH
#define GET_SHORT_MIN_HEIGHT	GET_MIN_HEIGHT
#define GET_SHORT_MAX_WIDTH	GET_MAX_WIDTH
#define GET_SHORT_MAX_HEIGHT	GET_MAX_HEIGHT

#define GET_MEDIUM_MIN_WIDTH	GET_MIN_WIDTH
#define GET_MEDIUM_MIN_HEIGHT	GET_MIN_HEIGHT
#define GET_MEDIUM_MAX_WIDTH	GET_MAX_WIDTH
#define GET_MEDIUM_MAX_HEIGHT	GET_MAX_HEIGHT
#endif

#define GET_HIGH_MIN_WIDTH	GET_MIN_WIDTH
#define GET_HIGH_MIN_HEIGHT	GET_MIN_HEIGHT
#define GET_HIGH_MAX_WIDTH	GET_MAX_WIDTH
#define GET_HIGH_MAX_HEIGHT	GET_MAX_HEIGHT

#define PUT_BAYER_MIN_WIDTH	(256)
#define PUT_BAYER_MIN_HEIGHT	(128)
#define PUT_BAYER_MAX_WIDTH	(8192)
#define PUT_BAYER_MAX_HEIGHT	(8192)

#if HAS_FF_DOL_DOL
/* DOL */
#define INPUT_BLOCK_WIDTH_DOL	(1152)
#define IPFD_BLOCK_WIDTH_DOL	(CEIL_DIV(INPUT_BLOCK_WIDTH_DOL, S2V_VECTOR_SIZE))
#define IPFD_BLOCK_HEIGHT_DOL	(2)
#define IPFD_BUFSIZE_DOL	(IPFD_BLOCK_WIDTH_DOL * IPFD_BLOCK_HEIGHT_DOL)
#define IPFD_NUMBUF_DOL		(2)
#define INPUT_QUEUE_SIZE_DOL	(IPFD_BUFSIZE_DOL * S2V_VECTOR_SIZE)
#endif

#define NUM_LINES_INTERLEAVED_IR	(1)
#define DMA_ONE_CHANNEL			(1)

#define DMA_OUT_SIS_DMA_NOF_CHANS			DMA_ONE_CHANNEL
#define DMA_OUT_IR_DMA_NOF_CHANS			DMA_ONE_CHANNEL

#define DMA_OUT_SIS_DMA_ID_API_MODEL		VIED_NCI_DEV_CHN_DMA_EXT1_WRITE_ID
#define DMA_OUT_SIS_DMA_ID_ISP_MODEL		vied_nci_dev_chn_dma_ext1_write
#define DMA_OUT_IR_DMA_ID_API_MODEL		VIED_NCI_DEV_CHN_DMA_EXT1_WRITE_ID
#define DMA_OUT_IR_DMA_ID_ISP_MODEL		vied_nci_dev_chn_dma_ext1_write

#define DFM_DMA_IR_OUTPUT_DMA_RESOURCE_ID			vied_nci_dev_chn_dma_ext1_write
#define DFM_DMA_IR_OUTPUT_NUM_LINES_INTERLEAVED			NUM_LINES_INTERLEAVED_IR

#define DMA_NCI_BAYER_SIS_OUTPUT_DMA_RESOURCE_ID		vied_nci_dev_chn_dma_ext1_write
#define DMA_NCI_BAYER_SIS_OUTPUT_NUM_LINES_INTERLEAVED		NUM_LINES_INTERLEAVED

#define DMA_NCI_IR_OUTPUT_DMA_RESOURCE_ID			vied_nci_dev_chn_dma_ext1_write
#define DMA_NCI_IR_OUTPUT_NUM_LINES_INTERLEAVED			NUM_LINES_INTERLEAVED_IR

/* SIS A/B bayer interleaved */
#define OUTPUT_SIS_BAYER_BLOCK_WIDTH	(1024 * BYTES_PER_ELEMENT / VECTOR_SIZE_IN_BYTES)
#define OUTPUT_SIS_BAYER_BLOCK_HEIGHT	(2)
#define OUTPUT_SIS_BAYER_BUFSIZE	(OUTPUT_SIS_BAYER_BLOCK_WIDTH * OUTPUT_SIS_BAYER_BLOCK_HEIGHT)
#define OUTPUT_SIS_BAYER_NUMBUF		(2)
#define OUTPUT_SIS_BUF_MEM_SIZE		(OUTPUT_SIS_BAYER_BUFSIZE * OUTPUT_SIS_BAYER_NUMBUF * VECTOR_SIZE_IN_BYTES)

/* IR/MD output */
#define OUTPUT_IR_BAYER_BLOCK_WIDTH	(1024 * BYTES_PER_ELEMENT / VECTOR_SIZE_IN_BYTES)
#define OUTPUT_IR_BLOCK_HEIGHT		(1)
#define OUTPUT_IR_BUFSIZE		(OUTPUT_IR_BAYER_BLOCK_WIDTH * OUTPUT_IR_BLOCK_HEIGHT)
#define OUTPUT_IR_NUMBUF		(2)
#define OUTPUT_IR_BUF_MEM_SIZE		(OUTPUT_IR_BUFSIZE * OUTPUT_IR_NUMBUF * VECTOR_SIZE_IN_BYTES)

#endif /* __ISA_SHRED_DEFS_DEPH */
