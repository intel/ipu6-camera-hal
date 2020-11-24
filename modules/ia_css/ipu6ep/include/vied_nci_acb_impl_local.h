/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __VIED_NCI_ACB_IMPL_LOCAL_H
#define __VIED_NCI_ACB_IMPL_LOCAL_H

#include "vied_nci_acb.h"
#include "vied_nci_acb_types.h"              /* for vied_nci_acb_config_t, vied_nci_acb_route_t, IPU_DEVICE_ACB_G... */
#include "vied_nci_acb_trace.h"              /* for NCI_ACB_TRACE_METHOD, NCI_ACB_TRACE_LEVEL_VERBOSE      */

NCI_ACB_STORAGE_CLASS_C
uint32_t nci_acb_fill_section0(
	const nci_acb_control_config_t *public_cfg,
	void *buffer)
{
	struct nci_acb_private_cfg *private_cfg = (struct nci_acb_private_cfg *) buffer;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	private_cfg->base_ctrl	= (((public_cfg->base_ctrl.fork_acb_output & 0x1) << 16) |
				((public_cfg->base_ctrl.ignore_line_num & 0x1) << 2) |
				((public_cfg->base_ctrl.out_select & 0x1) << 1) |
				(public_cfg->base_ctrl.in_select & 0x1));
	private_cfg->frame_width	= public_cfg->frame_width;
	private_cfg->frame_height	= public_cfg->frame_height;
	private_cfg->scalefactor	= (public_cfg->scale_nf << 4) | (public_cfg->scale_mult);

	return sizeof(struct nci_acb_private_cfg);
}

NCI_ACB_STORAGE_CLASS_C
uint32_t nci_acb_fill_section1(
	const nci_acb_ack_config_t *public_cfg,
	void *buffer)
{
	struct nci_acb_private_ack_cfg *private_cfg = (struct nci_acb_private_ack_cfg *) buffer;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	private_cfg->ack_addr		= public_cfg->event_queue_address;
	private_cfg->ack_data		= public_cfg->ack_data;

	return sizeof(struct nci_acb_private_ack_cfg);
}

NCI_ACB_STORAGE_CLASS_C uint32_t nci_acb_config(
	const enum ipu_device_acb_id acb_dev_id,
	const nci_acb_config_t *acb_cfg,
	void *buffer)
{
	uint32_t size_sec0 = 0;
	uint32_t size_sec1 = 0;

	assert(acb_dev_id < IPU_DEVICE_ACB_NUM_ACB);
	assert(acb_cfg != NULL);
	assert(buffer != NULL);

	size_sec0 = nci_acb_fill_section0(&acb_cfg->ctrl_cfg, buffer);
	buffer = (uint8_t *) buffer + size_sec0;
	size_sec1 = nci_acb_fill_section1(&acb_cfg->ack_cfg, buffer);

	return (size_sec0 + size_sec1);
}

NCI_ACB_STORAGE_CLASS_C
void nci_acb_print_section0(void *buffer)
{
	struct nci_acb_private_cfg *private_cfg = (struct nci_acb_private_cfg *)buffer;

	IA_CSS_TRACE_4(NCI_ACB, INFO, "{fork_acb_output = %d ignore_line_num = %d out_select = %d in_select = %d }\n",
		(private_cfg->base_ctrl >> 16) & 0x1,
		(private_cfg->base_ctrl >> 2) & 0x1,
		(private_cfg->base_ctrl >> 1) & 0x1,
		(private_cfg->base_ctrl) & 0x1);
	IA_CSS_TRACE_4(NCI_ACB, INFO, "{frame_height = %d frame_width = %d scale_nf = %d scale_mult = %d }\n",
		private_cfg->frame_height,
		private_cfg->frame_width,
		private_cfg->scalefactor >> 4 & 0xf,
		private_cfg->scalefactor & 0xf);
}

NCI_ACB_STORAGE_CLASS_C
void nci_acb_print_section1(void *buffer)
{
	struct nci_acb_private_ack_cfg *private_ack_cfg = (struct nci_acb_private_ack_cfg *)buffer;

	IA_CSS_TRACE_4(NCI_ACB, INFO, "{ack_addr = 0x%x ack_sid = %d ack_pid = %d ack_msg = 0x%x }\n",
		private_ack_cfg->ack_addr,
		(private_ack_cfg->ack_data >> 26) & 0x3f,
		(private_ack_cfg->ack_data >> 20) & 0x3f,
		(private_ack_cfg->ack_data & 0x3ffff));
}

NCI_ACB_STORAGE_CLASS_C
void nci_acb_print_sections(void *buffer)
{
	assert(buffer != NULL);
	nci_acb_print_section0(buffer);
	buffer = (uint8_t *)buffer + nci_acb_get_sizeof_section(NCI_ACB_SECTION0);
	nci_acb_print_section1(buffer);
}

#endif /*__VIED_NCI_ACB_IMPL_LOCAL_H */
