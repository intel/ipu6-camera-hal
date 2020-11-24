/**
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

#ifndef __IPU_DEVICE_DMA_TYPE_PROPERTIES_DEP_H
#define __IPU_DEVICE_DMA_TYPE_PROPERTIES_DEP_H

#include "storage_class.h"

/* Request Register */
enum ipu_device_dma_request_reg {
	IPU_DEVICE_DMA_REQUEST_REG_INSTRUCTION = 0x0,
	IPU_DEVICE_DMA_REQUEST_REG_DESC_ID_SETUP1,
	IPU_DEVICE_DMA_REQUEST_REG_DESC_ID_SETUP2,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_VALID,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_RESOURCED,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_ACCEPTED_ADDR,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_ACCEPTED_TOKEN,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_ACCEPTED_ACTIVE,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_QUEUE_READ_PTR,
	IPU_DEVICE_DMA_REQUEST_REG_REQUEST_QUEUE_DEPTH
};

#define N_IPU_DEVICE_DMA_REQUEST_REG (IPU_DEVICE_DMA_REQUEST_REG_REQUEST_QUEUE_DEPTH + 1)
/*
 * DMA can be configured to send a de-queue ack after moving
 * the instruction from register bank to instruction queue
 */
#define IPU_DEVICE_DMA_SUPPORTS_REQUEST_ACCEPTED_ACK (1)

#endif /* __IPU_DEVICE_DMA_TYPE_PROPERTIES_H */
