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

#ifndef __NCI_DVS_CONTROLLER_IMPL_LOCAL_H
#define __NCI_DVS_CONTROLLER_IMPL_LOCAL_H

#include "nci_dvs_controller.h"
#include "nci_dvs_controller_types.h"
#include "nci_dvs_controller_private_types.h"
#include "assert_support.h"
#include "misc_support.h"

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_fill_config(
		dai_ipu_top_dvs_controller_instances_t dev_id,
		const struct nci_dvs_controller_cfg *cfg,
		void *buffer)
{
	char *buffer_char = (char *)buffer;
	assert(cfg != NULL);
	assert(buffer_char != NULL);
	assert(dev_id < NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES);

	/* not used in host implementation */
	NOT_USED(dev_id);

	buffer_char += nci_dvs_controller_fill_section_ack_cfg(cfg, NCI_DVS_CONTROLLER_SECTION0, buffer_char);
	buffer_char += nci_dvs_controller_fill_section_ack_cfg(cfg, NCI_DVS_CONTROLLER_SECTION1, buffer_char);
	buffer_char += nci_dvs_controller_fill_section_ack_cfg(cfg, NCI_DVS_CONTROLLER_SECTION2, buffer_char);
	nci_dvs_controller_fill_section3(cfg, buffer_char);

	return nci_dvs_controller_get_sizeof_blob();
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_fill_config_per_level(
		dai_ipu_top_dvs_controller_instances_t dev_id,
		enum nci_dvs_controller_levels level,
		const struct nci_dvs_controller_cfg *cfg,
		void *buffer)
{
	char *buffer_char = (char *)buffer;
	enum nci_dvs_controller_sections sec_level_id = (enum nci_dvs_controller_sections) level;

	assert(cfg != NULL);
	assert(buffer_char != NULL);
	assert(dev_id < NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES);

	/* not used in host implementation */
	NOT_USED(dev_id);

	buffer_char += nci_dvs_controller_fill_section_ack_cfg(cfg, sec_level_id, buffer_char);
	nci_dvs_controller_fill_section3(cfg, buffer_char);

	return nci_dvs_controller_get_sizeof_blob_per_level();
}

#endif /* __NCI_DVS_CONTROLLER_IMPL_LOCAL_H */
