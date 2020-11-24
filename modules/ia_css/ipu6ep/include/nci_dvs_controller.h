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

#ifndef __NCI_DVS_CONTROLLER_H
#define __NCI_DVS_CONTROLLER_H

#include "nci_dvs_controller_types.h"
#include "nci_dvs_controller_storage_class.h"
#include "dai_ipu_top_devices.h"

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_nof_sections(void);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_nof_sections_per_level(void);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_sizeof_section(enum nci_dvs_controller_sections section_id);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_offsetof_section(
		enum nci_dvs_controller_sections section_id);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_sizeof_blob(void);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_get_sizeof_blob_per_level(void);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_fill_config(
		dai_ipu_top_dvs_controller_instances_t dev_id,
		const struct nci_dvs_controller_cfg *cfg,
		void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_fill_config_per_level(
		dai_ipu_top_dvs_controller_instances_t dev_id,
		enum nci_dvs_controller_levels level,
		const struct nci_dvs_controller_cfg *cfg,
		void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
void nci_dvs_controller_print_config(
	void *buffer);

#ifdef __INLINE_NCI_DVS_CONTROLLER__
#include "nci_dvs_controller_impl.h"
#endif

#endif /* __NCI_DVS_CONTROLLER_H */
