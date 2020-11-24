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

#ifndef MBR_DEFINES_H_
#define MBR_DEFINES_H_

/* GDC RF buffer allocation (GDC HAS section 4.1) */
#define GDC_RF_LUMA_BUF_SIZE_VECS                   578
#define GDC_RF_CHROMA_BUF_SIZE_VECS                 222

#define MAX_NUMBER_OF_INTERNAL_DMA_CHANNELS         (3)

#define GDC_MBR_MAX_BATCH_SIZE                      (8)
#define GDC_MBR_NUMBER_OF_INTERNAL_DMA_CHANNELS     (MAX_NUMBER_OF_INTERNAL_DMA_CHANNELS)

#define TNR_MBR_MAX_BATCH_SIZE                      (5)
#define TNR_MBR_NUMBER_OF_INTERNAL_DMA_CHANNELS     (MAX_NUMBER_OF_INTERNAL_DMA_CHANNELS)

/* maximum of GDC */
#define MAX_BATCH_SIZE                              MAX(GDC_MBR_MAX_BATCH_SIZE, TNR_MBR_MAX_BATCH_SIZE)

#define CHROMA_SUBSAMPLE_SHIFT_X 1
#define CHROMA_SUBSAMPLE_SHIFT_Y 1

#endif /* MBR_DEFINES_H_ */
