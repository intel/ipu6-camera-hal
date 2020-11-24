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

#ifndef __NCI_CIO2STR_H
#define __NCI_CIO2STR_H

#include "nci_cio2str_types.h"
#include "nci_cio2str_storage_class.h"
#include "ipu_device_cio2str_devices.h"

NCI_CIO2STR_STORAGE_CLASS_H
uint32_t nci_cio2str_get_nof_sections(void);

NCI_CIO2STR_STORAGE_CLASS_H
uint32_t nci_cio2str_get_sizeof_section(unsigned int section_id);

NCI_CIO2STR_STORAGE_CLASS_H
uint32_t nci_cio2str_get_offsetof_section(
		unsigned int section_id);

NCI_CIO2STR_STORAGE_CLASS_H
uint32_t nci_cio2str_get_sizeof_blob(void);

NCI_CIO2STR_STORAGE_CLASS_H
uint32_t nci_cio2str_fill_config(
		enum ipu_device_cio2str_dev_id dev_id,
		const struct nci_cio2str_cfg *cfg,
		void *buffer);

NCI_CIO2STR_STORAGE_CLASS_H
void nci_cio2str_print_config(
	void *buffer);

#ifdef __INLINE_NCI_CIO2STR__
#include "nci_cio2str_impl.h"
#endif

#endif /* __NCI_CIO2STR_H */
