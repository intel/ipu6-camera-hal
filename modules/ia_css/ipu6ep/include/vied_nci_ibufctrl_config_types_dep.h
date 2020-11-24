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

#ifndef __VIED_NCI_IBUFCTRL_CONFIG_TYPES_DEP_H
#define __VIED_NCI_IBUFCTRL_CONFIG_TYPES_DEP_H

#include "vied_nci_ibufctrl_configuration.h"

typedef union {
	struct {
		unsigned int check_type	: 1;
		unsigned int rep_en	: 1;
		unsigned int send_eof	: 1;
		unsigned int inc_unit	: 1;
		unsigned int scale	: 1;
		unsigned int inc_frame	: 1;
		unsigned int others	: 26;
	} bits;
	unsigned int word;
} vied_nci_ibufctrl_check_mode_s;

typedef union {
	struct {
		unsigned int is_feeder         : 1;
		unsigned int config_dma        : 1;
		unsigned int iwake_en          : 1;
		unsigned int disable_spana_rst : 1;
		unsigned int disable_spanb_rst : 1;
		unsigned int others            : 27;
	} bits;
	unsigned int word;
} vied_nci_ibufctrl_dest_mode_s;

typedef struct {
	unsigned int dest_id;
	unsigned int feed_addr;		/* byte addressable in v4 onwards */
	unsigned int req_addr;		/* byte addressable in v4 onwards */
	unsigned int channel_addr;	/* byte addressable in v4 onwards */
	unsigned int span_a_addr;	/* byte addressable in v4 onwards */
	unsigned int span_b_addr;	/* byte addressable in v4 onwards */
	unsigned int terminal_b_addr;	/* byte addressable in v4 onwards */
	vied_nci_ibufctrl_dest_mode_s dest_mode;
	unsigned int st_addr;		/* byte addressable */
	unsigned int num_items;
	unsigned int iwake_threshold;	/* in units */
	unsigned int sync_dests;
	unsigned int dma_flush_cntr;
} vied_nci_ibufctrl_dest_cfg_s;

typedef struct {
	unsigned int feed_addr;
	vied_nci_ibufctrl_dest_mode_s dest_mode;
} vied_nci_ibufctrl_dest_otf_cfg_s;

typedef struct {
	unsigned int str2mmio_addr;	/* byte addressable in v4 onwards */
	unsigned char store_cmd;
	unsigned int units_p_line;
	unsigned int lines_p_frame;
	unsigned int unit_size;		/* in bytes in v4 onwards */
	unsigned int units_p_ibuf;
	unsigned int camera_selection;
	unsigned int buffer_st_addr;	/* byte addressable (absolute) in v4 onwards */
	unsigned char s2v_not_str2mmio;
	unsigned int s2v_n_vec_p_unit;
	unsigned int invert_gda_en;
	vied_nci_ibufctrl_dest_cfg_s * dest_cfg[IBUFCTRL_MAX_DESTS_PER_SID];
	unsigned char dest_en[IBUFCTRL_MAX_DESTS_PER_SID];
} vied_nci_ibufctrl_proc_cfg_s;

typedef struct {
	unsigned int units_p_line;
	unsigned int unit_size; /* TODO: should only be needed for GDA, can it be removed? */
	unsigned int units_p_ibuf;
} vied_nci_ibufctrl_proc_otf_cfg_s;

#endif /* __VIED_NCI_IBUFCTRL_CONFIG_TYPES_DEP_H */
