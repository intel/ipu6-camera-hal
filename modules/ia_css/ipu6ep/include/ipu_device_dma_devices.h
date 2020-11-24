/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_DMA_DEVICES_H
#define __IPU_DEVICE_DMA_DEVICES_H

/* DMA Devices
* Public IDs to be used by the NCI user.
*/
#ifndef USE_DAI
enum nci_dma_device_id {
	NCI_DMA_EXT0,
	NCI_DMA_EXT1R,
	NCI_DMA_EXT1W,
	NCI_DMA_INT,
	NCI_DMA_ISA
};
#define NCI_DMA_NUM_DEVICES (NCI_DMA_ISA + 1)
#else
#include "dai_ipu_top_devices.h"
enum nci_dma_device_id {
	NCI_DMA_EXT0 = DAI_IPU_TOP_DMA_TOP_DMA_LB,
	NCI_DMA_EXT1R = DAI_IPU_TOP_DMA_TOP_DMA_HBFRX,
	NCI_DMA_EXT1W = DAI_IPU_TOP_DMA_TOP_DMA_HBTX,
	NCI_DMA_INT = DAI_IPU_TOP_FABRIC_TOP_CF_DEV_CL_DMA_INTERNAL,
	NCI_DMA_ISA = DAI_IPU_TOP_ISL_CTRL_PS_TOP_DMA
};

/* TODO: Since DAI combines isys and psys DMA devices,
 * NCI_DMA_NUM_DEVICES is equal to total number of DMA instances (isys+psys)
 * here instead of only the DMA instances. This is temporary. Can be a
 * problem when NCI_DMA_NUM_DEVICES is used as an iteration counter. Needs to
 * change when migrating to DAI and FW integrates isys and psys under
 * 'ipu_system'
 */
#define NCI_DMA_NUM_DEVICES ((int)NUM_DAI_IPU_TOP_DMA4_INSTANCES)
#endif /*USE_DAI */
#endif /* __IPU_DEVICE_DMA_DEVICES_H */
