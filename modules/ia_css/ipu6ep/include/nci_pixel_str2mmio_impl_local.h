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

#ifndef __NCI_PIXEL_STR2MMIO_IMPL_LOCAL_H
#define __NCI_PIXEL_STR2MMIO_IMPL_LOCAL_H

#include "nci_pixel_str2mmio.h"
#include "nci_pixel_str2mmio_types.h"
#include "nci_pixel_str2mmio_private_types.h"
#include "assert_support.h"
#include "misc_support.h"
#include "ipu_device_str2mmio_properties.h" /* for ipu_device_str2mmio_get_type */

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_fill_config(
		enum ipu_device_str2mmio_id dev_id,
		const struct nci_pixel_str2mmio_cfg *cfg,
		void *buffer)
{
	uint8_t *buffer_u8;

	assert(cfg != NULL);
	assert(buffer != NULL);
	assert(dev_id < IPU_DEVICE_STR2MMIO_NUM_DEVS);

	assert(ipu_device_str2mmio_get_type(dev_id) == IPU_DEVICE_STR2MMIO_TYPE_PIXEL);

	buffer_u8 = (uint8_t *)buffer;
	buffer_u8 += nci_pixel_str2mmio_fill_section0(cfg, buffer);

	return nci_pixel_str2mmio_get_sizeof_blob();
}

#endif /* __NCI_PIXEL_STR2MMIO_IMPL_LOCAL_H */
