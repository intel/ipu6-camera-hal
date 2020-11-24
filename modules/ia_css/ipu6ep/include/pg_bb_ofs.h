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

#ifndef __PG_BB_OFS_H
#define __PG_BB_OFS_H

#include "isp_global.h"
#include "ia_css_vmem_resources.h"
#include "ofs_dma_config_def.h"

#define TEST_FRAGMENT_WIDTH             (128)
#define TEST_BLOCK_WIDTH                (TEST_FRAGMENT_WIDTH / ISP_NWAY)
#define TEST_INPUT_BLOCK_HEIGHT         (32)
#define TEST_OUTPUT_BLOCK_HEIGHT        (32)

/* Queue Defines */
#define Q_BUF_MEM                                   (buf_mem_ofs_vmem)
#define Q_BUF_START_ADDR                            (0)
#define Q_NUMBUF                                    (3)
#define Q_WIDTH_Y                                   (TEST_BLOCK_WIDTH)
#define Q_HEIGHT_Y                                  (TEST_INPUT_BLOCK_HEIGHT)
#define Q_SIZE_Y                                    (Q_WIDTH_Y * Q_HEIGHT_Y)

#define CHROMA_SUBSAMPLE_SHIFT                      (1)
#define Q_WIDTH_U                                   (Q_WIDTH_Y >> CHROMA_SUBSAMPLE_SHIFT)
#define Q_HEIGHT_U                                  (Q_HEIGHT_Y >> CHROMA_SUBSAMPLE_SHIFT)
#define Q_SIZE_U                                    (Q_WIDTH_U * Q_HEIGHT_U)
#define Q_WIDTH_V                                   (Q_WIDTH_Y >> CHROMA_SUBSAMPLE_SHIFT)
#define Q_HEIGHT_V                                  (Q_HEIGHT_Y >> CHROMA_SUBSAMPLE_SHIFT)
#define Q_SIZE_V                                    (Q_WIDTH_V * Q_HEIGHT_V)
#define Q_SIZE                                      (Q_SIZE_Y + Q_SIZE_U + Q_SIZE_V)
/* * * * * * * * * * * * * * * * * * DFM CONNECTIONS * * * * * * * * * * * * * * * * * * */
#define DFM_PROD_DMA_YUV_INPUT_PORT_DEV_ID           (IPU_DEVICE_DFM_BB)
/* For Producer DMA, Empty port selected.
 * At input this port is selected for DMA producer sequence which has 3 seq,
 * 2 cmds tokens capability. For reference, please see section 8 MAS of DFM.
 */
#define DFM_PROD_DMA_YUV_INPUT_PORT_NUM_VPLESS       (0)
#define DFM_PROD_DMA_INPUT_ACK_PORT_NUM_VPLESS       (3)

#define DFM_CONS_DMA_YUV_OUTPUT_PORT_NUM_VPLESS      (33)
#define DFM_CONS_DMA_OUTPUT_ACK_PORT_NUM_VPLESS      (36)

/* For Gather and Multicast, Each bit represent a master port to which Buffer
 * controller is connected */

/* connected to only 1 master full port (relative num) i.e. 7
 *  Thus, mask ->
 *         1 0 0 0 0 0 0 0 = x80
 * In above each bit position represents port num.
 */
#define DFM_PROD_DMA_YUV_INPUT_PORT_GM_MASK          DFM_GET_GATHER_MULTICAST_MASK(DFM_PROD_DMA_YUV_INPUT_PORT_NUM_VPLESS)

#define DFM_CONS_DMA_YUV_OUTPUT_PORT_GM_MASK         DFM_GET_GATHER_MULTICAST_MASK(DFM_CONS_DMA_YUV_OUTPUT_PORT_NUM_VPLESS)

#define DFM_DMA_YUV_INPUT_DMA_RESOURCE_ID            vied_nci_dev_chn_dma_ext1_read
#define DMA_NCI_YUV_INPUT_TRANSFER_DMA_RESOURCE_ID   vied_nci_dev_chn_dma_ext1_read
#define DFM_DMA_YUV_OUTPUT_DMA_RESOURCE_ID           vied_nci_dev_chn_dma_ext1_write
#define DMA_NCI_YUV_OUTPUT_TRANSFER_DMA_RESOURCE_ID  vied_nci_dev_chn_dma_ext1_write

#define DFM_PROD_DEVICE_ID                           VIED_NCI_DEV_DFM_BB_EMPTY_PORT_ID
#define DFM_CONS_DEVICE_ID                           VIED_NCI_DEV_DFM_BB_FULL_PORT_ID

/* DMA configuration */
#define MAIN_OUTPUT_PIN_DMA_RESOURCE_ID vied_nci_dev_chn_dma_ext1_write
#define MAIN_OUTPUT_PIN_NUM_DMA_CHANNELS OFS_DMA_MAX_NUM_DMA_CHANNELS

#define DISPLAY_OUTPUT_PIN_DMA_RESOURCE_ID vied_nci_dev_chn_dma_ext1_write
#define DISPLAY_OUTPUT_PIN_NUM_DMA_CHANNELS OFS_DMA_MAX_NUM_DMA_CHANNELS

#define PP_OUTPUT_PIN_DMA_RESOURCE_ID vied_nci_dev_chn_dma_ext1_write
#define PP_OUTPUT_PIN_NUM_DMA_CHANNELS OFS_DMA_MAX_NUM_DMA_CHANNELS

/* OFS output queue properties */
#define Q_OUT_BUF_MEM                VMEM_SECTION_BBPS_OFA_OUTPUT_MEM_ID
#define Q_OUT_BUF_START_ADDR         (VMEM_SECTION_BBPS_OFA_OUTPUT_OFFSET * ISP_VEC_BYTES)
#define Q_OUT_BUF_SIZE_VECS          (VMEM_SECTION_BBPS_OFA_OUTPUT_SIZE)
#define Q_OUT_BUF_SIZE_BYTES         (VMEM_SECTION_BBPS_OFA_OUTPUT_SIZE * ISP_VEC_BYTES)

#endif /* __PG_BB_OFS_H */
