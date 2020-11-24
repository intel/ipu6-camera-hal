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

#ifndef __VIED_NCI_ACB_IMPL_H
#define __VIED_NCI_ACB_IMPL_H

#include "assert_support.h"
#include "vied_nci_acb.h"
#include "vied_nci_acb_trace.h"              /* for NCI_ACB_TRACE_METHOD, NCI_ACB_TRACE_LEVEL_VERBOSE      */
#include "vied_nci_acb_private_types.h"
#include "vied_nci_acb_storage_class.h"

NCI_ACB_STORAGE_CLASS_C
uint8_t nci_acb_get_numberof_sections(void)
{
	return NCI_ACB_NUM_SECTIONS;
}

NCI_ACB_STORAGE_CLASS_C
uint32_t nci_acb_get_sizeof_section(nci_acb_section_t sec)
{
	uint32_t size = 0;

	assert(sec < NCI_ACB_NUM_SECTIONS);

	if (sec == NCI_ACB_SECTION0) {
		size = sizeof(struct nci_acb_private_cfg);
	} else if (sec == NCI_ACB_SECTION1) {
		size = sizeof(struct nci_acb_private_ack_cfg);
	} else {
		IA_CSS_TRACE_0(NCI_ACB, ERROR, "nci_acb_get_sizeof_section(): invalid section id\n");
		assert(0);
	}
	return size;
}

NCI_ACB_STORAGE_CLASS_C
uint32_t nci_acb_get_offsetof_section(nci_acb_section_t sec)
{
	uint32_t offset = 0;

	assert(sec < NCI_ACB_NUM_SECTIONS);

	if (sec == NCI_ACB_SECTION0) {
		offset = IPU_DEVICE_ACB_GA_ACB_BASE_CTRL_ADDR;
	} else if (sec == NCI_ACB_SECTION1) {
		offset = IPU_DEVICE_ACB_ACK_CMD_ADDR;
	} else {
		IA_CSS_TRACE_0(NCI_ACB, ERROR, "nci_acb_get_offsetof_section(): invalid section id\n");
		assert(0);
	}

	return offset;
}

NCI_ACB_STORAGE_CLASS_C uint32_t nci_acb_get_sizeof_sections(void)
{
	return (nci_acb_get_sizeof_section(NCI_ACB_SECTION0) + nci_acb_get_sizeof_section(NCI_ACB_SECTION1));
}

NCI_ACB_STORAGE_CLASS_C uint32_t nci_acb_build_process_cmd_token(
	const nci_acb_process_cmd_t cmd
)
{
	return ((cmd.num_lines & 0xffff) << 16) |
		((cmd.config_set & 0xf) << 12) |
		((cmd.ctrl_id & 0x7) << 8) |
		(IPU_DEVICE_ACB_PROC_N_LINES_CMD_ID & 0x1f);
}

/* host or cell specific implementations */
#include "vied_nci_acb_impl_local.h"

#endif /* __VIED_NCI_ACB_IMPL_H */
