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

#ifndef __NCI_DMA_DESCRIPTORS_DEP_H
#define __NCI_DMA_DESCRIPTORS_DEP_H

#include "type_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_type_properties.h"
#include "nci_dma_descriptors_storage_class.h"
#include "nci_dma_descriptors_types_request.h"

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_fill_request_bank_dequeue_ack(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_bank_id,
	const struct nci_dma_public_request_bank_dequeue_ack_config * const public_request_reg_bank_config,
	void * const buffer);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_bank_dequeue_ack_size(void);

STORAGE_CLASS_NCI_DMA_DESCRIPTORS_H int nci_dma_get_request_bank_dequeue_ack_num_sections(void);

#endif /* __NCI_DMA_DESCRIPTORS_DEP_H */
