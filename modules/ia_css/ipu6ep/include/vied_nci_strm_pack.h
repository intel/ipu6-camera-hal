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

#ifndef __VIED_NCI_STRM_PACK_H
#define __VIED_NCI_STRM_PACK_H

#include "vied_nci_strm_pack_storage_class.h"
#include "ipu_device_strm_pack_devices.h"
#include "vied_nci_strm_pack_types.h"

NCI_STRM_PACK_STORAGE_CLASS_H  uint32_t nci_strm_pack_config(
	const nci_strm_pack_control_config_t *public_cfg,
	void *buffer);

NCI_STRM_PACK_STORAGE_CLASS_H uint8_t nci_strm_pack_get_numberof_sections(void);

NCI_STRM_PACK_STORAGE_CLASS_H uint32_t nci_strm_pack_get_sizeof_sections(void);

NCI_STRM_PACK_STORAGE_CLASS_H
uint32_t nci_strm_pack_get_offsetof_section(unsigned int sec);

NCI_STRM_PACK_STORAGE_CLASS_H
uint32_t nci_strm_pack_get_sizeof_section(unsigned int sec);

NCI_STRM_PACK_STORAGE_CLASS_C
uint32_t nci_strm_pack_get_sizeof_sections(void);

#include "vied_nci_strm_pack_local.h"

#ifdef __INLINE_NCI_STRM_PACK__
#include "vied_nci_strm_pack_impl.h"
#endif /*__INLINE_NCI_STRM_PACK__*/

#endif /* __VIED_NCI_STRM_PACK_H */
