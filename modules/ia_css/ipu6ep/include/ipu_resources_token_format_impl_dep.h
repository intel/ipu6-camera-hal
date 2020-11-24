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

#ifndef __IPU_RESOURCES_TOKEN_FORMAT_IMPL_DEP_H
#define __IPU_RESOURCES_TOKEN_FORMAT_IMPL_DEP_H

#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"
#include "misc_support.h"
#include "assert_support.h"
#include "ipu_resources_dfm.h"

STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_encode_process_id(
		vied_nci_device_type_id_t dev_type,
		uint8_t dev_instance,
		uint8_t dev_channel)
{
	ia_css_process_id_t process_id;

	if (VIED_NCI_DEVICE_ID_LB_ACB == dev_type) {
		/* for accelerators, encode dev instance to SID */
		/* device channel is not applicable for accelerators */
		assert(dev_channel == 0);
		assert(dev_instance <= ((1<<N_BITS_DEVICE_CHANNEL)-1));
		process_id = ENCODE_PROCESS_ID_ACB(dev_type, dev_instance);
	} else {
		/* for all other device type, device channel is encoded in SID */
		assert(dev_instance <= ((1<<N_BITS_DEVICE_INSTANCE)-1));
		assert(dev_channel <= ((1<<N_BITS_DEVICE_CHANNEL)-1));
		process_id = ENCODE_PROCESS_ID_NON_ACB(dev_type, dev_instance, dev_channel);
	}
	return process_id;
}

STORAGE_CLASS_INLINE
void ipu_resources_decode_process_id(
			ia_css_process_id_t process_id,
			vied_nci_device_type_id_t *dev_type,
			uint8_t *dev_instance,
			uint8_t *dev_channel)
{
	vied_nci_device_type_id_t device_type;
	uint8_t device_inst, device_ch;

	device_type = (vied_nci_device_type_id_t)((process_id >> DEVICE_TYPE_START_OFFSET) & DEVICE_TYPE_MASK);
	assert(device_type < VIED_NCI_DEVICE_NUM);

	if  (VIED_NCI_DEVICE_ID_LB_ACB == device_type) {
		/* for accelerators, dev instance is encoded in SID */
		/* device channel is not applicable for accelerators */
		device_inst = process_id & (DEVICE_INSTANCE_MASK_ACB);
		device_ch = 0;
	} else {
		/* for all other device type, device channel is encoded in SID */
		device_ch = process_id & (DEVICE_CHANNEL_MASK_NON_ACB);
		device_inst = (process_id >> DEVICE_INSTANCE_START_OFFSET_NON_ACB) & (DEVICE_INSTANCE_MASK_NON_ACB);
	}

	*dev_type = device_type;
	*dev_instance = device_inst;
	*dev_channel = device_ch;
}

#endif /* __IPU_RESOURCES_TOKEN_FORMAT_IMPL_DEP_H */
