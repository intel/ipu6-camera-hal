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
#ifndef __ACB_CONTROL_INIT_H
#define __ACB_CONTROL_INIT_H

#include "ia_css_psys_process_group.h"
#include "ia_css_psys_terminal_private_types.h"
#include "vied_nci_psys_resource_model_private.h"
#include "ipu_device_acb_devices.h"
#include "common_io.host.h"
#include "vied_nci_acb_types.h"
#include "pg_control_init_support.h"

int
acb_payload_fill(
		vied_nci_lb_acb_id_t acb_id,
		const ia_css_process_group_t *process_group,
		load_section_desc_list_t load_sec_desc_list,
		uint32_t ack_addr,
		uint32_t ack_data,
		void *payload_address,
		s_resolution_t res,
		nci_acb_port_t in_select,
		nci_acb_port_t out_select);

int
acb_get_payload_size(void);

int
acb_fill_load_section_descriptor(
		vied_nci_lb_acb_id_t acb_res_id,
		ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc);

unsigned int
acb_get_numberof_load_sections(void);

void
acb_set_desc_sid_pid(
		ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
		uint8_t dev_instance);

#endif /* __ACB_CONTROL_INIT_H */
