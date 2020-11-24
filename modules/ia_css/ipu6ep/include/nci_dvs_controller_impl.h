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

#ifndef __NCI_DVS_CONTROLLER_IMPL_H
#define __NCI_DVS_CONTROLLER_IMPL_H

#include "nci_dvs_controller.h"
#include "nci_dvs_controller_types.h"
#include "ia_css_trace.h"
#include "nci_dvs_controller_trace.h"
#include "nci_dvs_controller_private_types.h"
#include "vec_to_str_v3_regmem.h"
#include "dai_ipu_top_devices.h"
#include "dvs_controller_regmem.h"
#include "assert_support.h"
#include "misc_support.h"

#define _DVS_CONTROLLER_REG_ALIGN 4
#define _DVS_CONTROLLER_PROC_N_VEC_OPCODE	0x01

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_fill_section_ack_cfg(
	const struct nci_dvs_controller_cfg *cfg,
	enum nci_dvs_controller_sections section_id,
	void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
uint32_t nci_dvs_controller_fill_section3(
	const struct nci_dvs_controller_cfg *cfg,
	void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_nof_sections(void)
{
	return NCI_DVS_CONTROLLER_NOF_SECTIONS;
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_nof_sections_per_level(void)
{
	return NCI_DVS_CONTROLLER_NOF_SECTIONS_PER_LEVEL;
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_sizeof_section(enum nci_dvs_controller_sections section_id)
{
	switch (section_id) {
	case NCI_DVS_CONTROLLER_SECTION0:
	     return sizeof(struct nci_dvs_controller_private_section_ack_cfg);
	case NCI_DVS_CONTROLLER_SECTION1:
	     return sizeof(struct nci_dvs_controller_private_section_ack_cfg);
	case NCI_DVS_CONTROLLER_SECTION2:
		return sizeof(struct nci_dvs_controller_private_section_ack_cfg);
	case NCI_DVS_CONTROLLER_SECTION3:
		return sizeof(struct nci_dvs_controller_private_section3);
	default:
		IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, ERROR, "nci_dvs_controller_get_sizeof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_sizeof_blob(void)
{
	return nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION0) +
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION1) +
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION2) +
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION3);
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_sizeof_blob_per_level(void)
{
	assert(nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION0) ==
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION1));
	assert(nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION0) ==
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION2));

	return nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION0) +
		nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION3);
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_get_offsetof_section(
		enum nci_dvs_controller_sections section_id)
{
	switch (section_id) {
	case NCI_DVS_CONTROLLER_SECTION0:
		return _DVS_CONTROLLER_REG_ALIGN * DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L0_INFO;
	case NCI_DVS_CONTROLLER_SECTION1:
		return _DVS_CONTROLLER_REG_ALIGN * DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L1_INFO;
	case NCI_DVS_CONTROLLER_SECTION2:
		return _DVS_CONTROLLER_REG_ALIGN * DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L2_INFO;
	case NCI_DVS_CONTROLLER_SECTION3:
		return _DVS_CONTROLLER_REG_ALIGN * DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_GENERAL_CNRL_INFO;
	default:
		IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, ERROR, "nci_dvs_controller_get_offsetof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_fill_section_ack_cfg(
	const struct nci_dvs_controller_cfg *cfg,
	enum nci_dvs_controller_sections section_id,
	void *buffer)
{
	struct nci_dvs_controller_private_section_ack_cfg *priv = (struct nci_dvs_controller_private_section_ack_cfg *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	switch (section_id) {
	case NCI_DVS_CONTROLLER_SECTION0:
		priv->ack_data = cfg->ack_data_level0;
		priv->ack_addr = cfg->ack_addr_level0;
		break;
	case NCI_DVS_CONTROLLER_SECTION1:
		priv->ack_data = cfg->ack_data_level1;
		priv->ack_addr = cfg->ack_addr_level1;
		break;
	case NCI_DVS_CONTROLLER_SECTION2:
		priv->ack_data = cfg->ack_data_level2;
		priv->ack_addr = cfg->ack_addr_level2;
		break;
	case NCI_DVS_CONTROLLER_SECTION3:
	default:
		IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, ERROR, "nci_dvs_controller_fill_section_ack_cfg: Invalid section id\n");
		assert(0);
		return 0;
	}
	return nci_dvs_controller_get_sizeof_section(section_id);
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
uint32_t nci_dvs_controller_fill_section3(
	const struct nci_dvs_controller_cfg *cfg,
	void *buffer)
{
	struct nci_dvs_controller_private_section3 *priv = (struct nci_dvs_controller_private_section3 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	priv->dvs_type = cfg->dvs_type;
	priv->acb_addr = cfg->acb_addr;

	return nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION3);
}

#include "nci_dvs_controller_impl_local.h"
#include "nci_dvs_controller_pretty_print.h"

#endif /* __NCI_DVS_CONTROLLER_IMPL_H */
