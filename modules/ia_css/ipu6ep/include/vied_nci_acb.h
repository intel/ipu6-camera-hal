/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2017 Intel Corporation.
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

#ifndef __VIED_NCI_ACB_H
#define __VIED_NCI_ACB_H

#include "vied_nci_acb_storage_class.h"
#include "ipu_device_acb_devices.h"
#include "vied_nci_acb_types.h"

NCI_ACB_STORAGE_CLASS_H  uint32_t nci_acb_config(
	const enum ipu_device_acb_id acb_dev_id,
	const nci_acb_config_t *acb_cfg,
	void *buffer);

NCI_ACB_STORAGE_CLASS_H uint8_t nci_acb_get_numberof_sections(void);

NCI_ACB_STORAGE_CLASS_H uint32_t nci_acb_get_sizeof_sections(void);

NCI_ACB_STORAGE_CLASS_H
uint32_t nci_acb_get_offsetof_section(nci_acb_section_t sec);

NCI_ACB_STORAGE_CLASS_H
uint32_t nci_acb_get_sizeof_section(nci_acb_section_t sec);

NCI_ACB_STORAGE_CLASS_H uint32_t nci_acb_build_process_cmd_token(const nci_acb_process_cmd_t cmd);

/* interface specific to cell or host */
#include "vied_nci_acb_local.h"

#ifdef __INLINE_NCI_ACB__
#include "vied_nci_acb_impl.h"
#endif /*__INLINE_NCI_ACB__*/

#endif /* __VIED_NCI_ACB_H */
