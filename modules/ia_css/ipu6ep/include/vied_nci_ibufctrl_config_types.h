/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __VIED_NCI_IBUFCTRL_CONFIG_TYPES_H
#define __VIED_NCI_IBUFCTRL_CONFIG_TYPES_H

#include "vied_nci_ibufctrl_configuration.h"
#include "vied_nci_ibufctrl_config_types_dep.h"

typedef struct {
	unsigned int sidpid;
	unsigned int ack_addr;		/* byte addressable in v4 onwards */
	unsigned int req_addr;		/* byte addressable in v4 onwards */
	unsigned int channel_addr;	/* byte addressable in v4 onwards */
	unsigned int units_p_line_in;
	unsigned int units_out_p_in;
	unsigned int last_units_out;
	unsigned int height;
	unsigned char wait_for_other_feeder;
} vied_nci_ibufctrl_feeder_cfg_s;

typedef struct {
	unsigned int frame_check_id;
	unsigned int enable;
	vied_nci_ibufctrl_check_mode_s mode;
	unsigned int sid_id;
	unsigned int trigger_offset;
	unsigned int trigger_repeat;
	unsigned int addr;		/* byte addressable in v4 onwards */
	unsigned int token;
} vied_nci_ibufctrl_frame_check_cfg_s;

typedef struct {
	unsigned int ack_addr;
	unsigned int s2m_ack;
	unsigned int snd_buf_ack;
	unsigned int snd_buf_cmd_addr;
	unsigned int sidpid;
	unsigned int dest_ack[IBUFCTRL_MAX_DESTS_PER_SID];
} vied_nci_ibufctrl_proc_cmd_s;

typedef struct {
	unsigned char nr_dests;
	unsigned char has_2nd_buff;
	unsigned char s2v_support;
	vied_nci_ibufctrl_proc_cmd_s cmd;
	vied_nci_ibufctrl_proc_cfg_s cfg;
} vied_nci_ibufctrl_proc_s;

typedef struct {
	unsigned int iwake_addr;	/* byte addressable in v4 onwards */
	unsigned int error_irq_en;
	unsigned int secure_touch_handling;
	unsigned int gda_en;
	unsigned int gda_addr;		/* byte addressable in v4 onwards */
	unsigned int enable_clk_gates;
	vied_nci_ibufctrl_proc_s proc[IBUFCTRL_MAX_SIDS];
	vied_nci_ibufctrl_dest_cfg_s dest_cfg[IBUFCTRL_MAX_DESTS];
	vied_nci_ibufctrl_frame_check_cfg_s frame_check_cfg[IBUFCTRL_MAX_IRQS];
	vied_nci_ibufctrl_feeder_cfg_s feeder_cfg[IBUFCTRL_MAX_FEEDERS];
} vied_nci_ibufctrl_ibuf_s;

#endif /* __VIED_NCI_IBUFCTRL_CONFIG_TYPES_H */
