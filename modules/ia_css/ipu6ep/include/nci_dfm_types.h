/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __NCI_DFM_TYPES_H
#define __NCI_DFM_TYPES_H

#include "ipu_nci_dfm_struct.h"

#define NCI_DFM_SEQ_TYPE_NUM            (DFM_SEQ_TYPE_FF_ACC + 1)
#define NCI_DFM_MAX_NUM_SEQ             (DFM_SEQ_TYPE_IDX_END + 1)
#define NCI_DFM_MAX_NUM_INIT_SEQ_TYPE   (DFM_SEQ_INIT_TYPE_S2MMIO + 1)

struct nci_dfm_ratio_conv_cfg{
	uint8_t threshold;
	uint8_t out_ratio;
	uint8_t ignore_first_event;
};

struct nci_dfm_ratio_conv_cfg_1{
	uint8_t  priming_val;
	uint32_t iter;
};

struct nci_dfm_ratio_conv_cnt {
	uint8_t  in_event;
	uint32_t iter;
};

struct nci_dfm_buff_ctrl_iter {
	uint8_t  init_en;
	uint16_t begin_iter;
	uint16_t middle_iter;
	uint8_t  end_iter;
};

struct nci_dfm_buff_ctrl_begin_seq {
	uint16_t begin_iter;
	uint8_t begin_type;
	uint8_t begin_init_type;
};

struct nci_dfm_buff_ctrl_seq {
	uint16_t iter;
	uint8_t type;
};

struct nci_dfm_buff_ctrl_dynamic_val {
	uint8_t wrap;
	uint8_t inc;
	uint8_t init;
};

/* Buffer Chasing Progress Reporting */
struct nci_dfm_bcpr_config {
	uint8_t buff_chasing_en;
	uint8_t frame_index;
	uint8_t bcpr_reg_alloc; /* BPCR register to sync with. Maximum of 16 Registers available per DFM device. Valid values [0, 15]*/
	uint8_t iter_to_unit_ratio;
};

struct nci_dfm_port_config {
	struct nci_dfm_ratio_conv_cfg        rc_cfg;
	struct nci_dfm_ratio_conv_cnt        rc_cnt;
	struct nci_dfm_buff_ctrl_iter        bc_iter;
	struct nci_dfm_bcpr_config           bcpr_config;
	ipu_nci_dfm_seq_type_t               bc_seq_type;
	struct nci_dfm_buff_ctrl_dynamic_val bc_dynamic_val;
	uint32_t                             gthr_multi_mask;
};

/* Public structs for command bank configuration. */
struct nci_dfm_sequence {
	uint32_t agent_address;
	uint32_t size;
	uint32_t *cmd;
};
struct nci_dfm_public_cmdbank_config {
	uint32_t port_num;
	struct nci_dfm_sequence begin;
	struct nci_dfm_sequence middle;
	struct nci_dfm_sequence end;
	uint32_t ack_done_addr;
	uint32_t ack_done_cmd;
};

#endif /* __NCI_DFM_TYPES_H */
