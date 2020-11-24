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

#ifndef __NCI_PIXEL_STR2MMIO_H
#define __NCI_PIXEL_STR2MMIO_H

#include "nci_pixel_str2mmio_local.h"
#include "nci_pixel_str2mmio_types.h"
#include "nci_pixel_str2mmio_storage_class.h"
#include "ipu_device_str2mmio_devices.h"

/*
 * NOTES/Limitations:
 * 1. This is a str2mmio NCI version that is specifically written
 * for pixel type str2mmio. It'd need to be extended if MIPI type
 * str2mmios are to be supported.
 * 2. This NCI version supports only str2mmio instances with a
 * singled SID. There are assertions in the code to make sure that
 * user doesn't try to configure a non-supported instance. It needs
 * to be extended if the others are to be supported.
 */

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_nof_sections(void);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_sizeof_section(unsigned int section_id);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_offsetof_section(
		unsigned int section_id);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_sizeof_blob(void);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_fill_config(
		enum ipu_device_str2mmio_id dev_id,
		const struct nci_pixel_str2mmio_cfg *cfg,
		void *buffer);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
void nci_pixel_str2mmio_print_config(
	void *buffer);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_command_reg_offset(void);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_value_for_store_command(void);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_H
uint32_t nci_pixel_str2mmio_get_value_for_sync_command(void);

#ifdef __INLINE_NCI_PIXEL_STR2MMIO__
#include "nci_pixel_str2mmio_impl.h"
#endif

#endif /* __NCI_PIXEL_STR2MMIO_H */
