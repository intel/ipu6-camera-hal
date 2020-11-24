/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2018 Intel Corporation.
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

#include "nci_dfm_descriptors.h"
#include "nci_dfm_trace.h"
#include "assert_support.h"
#include "misc_support.h"
#include "ipu_device_subsystem_ids.h"
#include "ipu_device_dfm_port_properties.h"
#include "ipu_nci_dfm_struct.h"
#include "ipu_nci_dfm.h"
#include "nci_dfm_reg_pack_unpack.h"
#include "nci_dfm_private_types.h"

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_fill_section1(const struct nci_dfm_port_config *dfm_config,
		void *buffer)
{
	struct nci_dfm_private_dev_port_config *output = (struct nci_dfm_private_dev_port_config *)buffer;

	assert(NULL != dfm_config);
	assert(NULL != buffer);

	output->ratio_conv_cfg_0 = pack_reg_dfm_dp_ratio_conv_cfg_0(dfm_config->rc_cfg.threshold,
			dfm_config->rc_cfg.out_ratio,
			dfm_config->rc_cfg.ignore_first_event);
	output->ratio_conv_cfg_1 = pack_reg_dfm_dp_ratio_conv_cfg_1(dfm_config->rc_cnt.in_event,
			dfm_config->rc_cnt.iter);
	output->ratio_conv_sdf_pr_cfg_0 = pack_reg_dfm_dp_ratio_conv_sdf_pr_cfg_0(
			dfm_config->bcpr_config.buff_chasing_en,
			dfm_config->bcpr_config.frame_index,
			dfm_config->bcpr_config.bcpr_reg_alloc,
			dfm_config->bcpr_config.iter_to_unit_ratio);
	output->gthr_mult_en_mask = pack_reg_dfm_dp_gth_mult_en_mask(dfm_config->gthr_multi_mask);
	output->buff_ctrl_dynamic_val = pack_reg_dfm_dp_buff_ctrl_dynamic_val(
					dfm_config->bc_dynamic_val.wrap,
					dfm_config->bc_dynamic_val.inc,
					dfm_config->bc_dynamic_val.init);
	output->buff_ctrl_begin_seq = pack_reg_dfm_dp_buff_ctrl_begin_seq(
			dfm_config->bc_iter.begin_iter,
			dfm_config->bc_seq_type.dfm_seq_type_begin_type,
			dfm_config->bc_seq_type.dfm_seq_type_init_type);
	output->buff_ctrl_middle_seq = pack_reg_dfm_dp_buff_ctrl_middle_seq(
			dfm_config->bc_iter.middle_iter,
			dfm_config->bc_seq_type.dfm_seq_type_middle_type);
	output->buff_ctrl_end_seq = pack_reg_dfm_dp_buff_ctrl_end_seq(
			dfm_config->bc_iter.end_iter,
			dfm_config->bc_seq_type.dfm_seq_type_end_type);

	return ipu_nci_dfm_port_get_section1_size();
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
void ipu_nci_dfm_port_print_section1(const void *buffer)
{
/* Prevent unnecessary workload when no pretty print is required */
#if (NCI_DFM_TRACE_CONFIG == NCI_DFM_TRACE_LOG_LEVEL_DEBUG)
	struct nci_dfm_private_dev_port_config *data;
	uint32_t unpacked_value_32[4] = { 0 };
	uint16_t unpacked_value_16;
	uint8_t  unpacked_value_8[4] = { 0 };

	assert(NULL != buffer);
	data = (struct nci_dfm_private_dev_port_config *)buffer;

	unpack_reg_dfm_dp_ratio_conv_cfg_0(
			&unpacked_value_8[0] /*threshold*/,
			&unpacked_value_8[1] /*out_ratio*/,
			&unpacked_value_8[2] /*ignore_first_event*/,
			data->ratio_conv_cfg_0);
	IA_CSS_TRACE_0(NCI_DFM, DEBUG,
		"ipu_nci_dfm_port_print_section1:\n");
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_0.threshold               = 0x%08x\n",
			unpacked_value_8[0]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_0.out_ratio               = 0x%08x\n",
			unpacked_value_8[1]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_0.ignore_first_event      = 0x%08x\n",
			unpacked_value_8[3]);
			IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_0.ratio_conv_event_cmd      = 0x%08x\n",
				       unpacked_value_8[3]);
	unpack_reg_dfm_dp_ratio_conv_cfg_1(
			&unpacked_value_8[0] /*priming_val*/,
			&unpacked_value_32[0] /*iter*/,
			data->ratio_conv_cfg_1);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_1.priming_val                = 0x%08x\n",
			unpacked_value_8[0]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t rc_cfg_1.iter                    = 0x%08x\n",
			unpacked_value_32[0]);

	unpack_reg_dfm_dp_ratio_conv_sdf_pr_cfg_0(
			&unpacked_value_8[0] /*en*/,
			&unpacked_value_8[1] /*frame_index*/,
			&unpacked_value_8[2] /*sdf_reg_alloc*/,
			&unpacked_value_8[3] /*iter2unit*/,
			data->ratio_conv_sdf_pr_cfg_0);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bcpr_config.buff_chasing_en    = 0x%08x\n",
			unpacked_value_8[0]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bcpr_config.frame_index        = 0x%08x\n",
			unpacked_value_8[1]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bcpr_config.bcpr_reg_alloc     = 0x%08x\n",
			unpacked_value_8[2]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bcpr_config.iter_to_unit_ratio = 0x%08x\n",
			unpacked_value_8[3]);

	unpack_reg_dfm_dp_gth_mult_en_mask(
			&unpacked_value_32[0] /*gthr_multi_mask*/,
			data->gthr_mult_en_mask);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t gthr_multi_mask                = 0x%08x\n",
			unpacked_value_32[0]);

	unpack_reg_dfm_dp_buff_ctrl_begin_seq(
			&unpacked_value_16 , /*begin_val*/
			&unpacked_value_32[1],/*begin_type*/
			&unpacked_value_32[0],/*begin_init_type*/
			data->buff_ctrl_begin_seq);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_iter.begin_iter             = 0x%08x\n",
			unpacked_value_16);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_seq_type.dfm_seq_type_begin_type  = 0x%08x\n",
				       unpacked_value_32[1]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_seq_type.dfm_seq_type_init_type   = 0x%08x\n",
						      unpacked_value_32[0]);

	unpack_reg_dfm_dp_buff_ctrl_middle_seq(
			&unpacked_value_16 /*middle_val*/,
			&unpacked_value_32[2] /*middle_type*/,
			data->buff_ctrl_middle_seq);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_iter.middle_iter            = 0x%08x\n",
			unpacked_value_16);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_seq_type.dfm_seq_type_middle_type = 0x%08x\n",
				       unpacked_value_32[2]);

	unpack_reg_dfm_dp_buff_ctrl_end_seq(
			&unpacked_value_16 /*end_val*/,
			&unpacked_value_32[2] /*middle_type*/,
			data->buff_ctrl_end_seq);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_iter.end_iter               = 0x%08x\n",
				       unpacked_value_8[1]);

	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_seq_type.dfm_seq_type_end_type    = 0x%08x\n",
			unpacked_value_32[3]);

	unpack_reg_dfm_dp_buff_ctrl_dynamic_val(
			&unpacked_value_8[0] /*wrap*/,
			&unpacked_value_8[1] /*inc*/,
			&unpacked_value_8[2] /*init*/,
			data->buff_ctrl_dynamic_val);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_dynamic_val.wrap            = 0x%08x\n",
			unpacked_value_8[0]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_dynamic_val.inc             = 0x%08x\n",
			unpacked_value_8[1]);
	IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t bc_dynamic_val.init            = 0x%08x\n",
			unpacked_value_8[2]);
#else
	NOT_USED(buffer);
#endif
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_section1_offset(uint32_t port_num)
{
	assert(port_num < DFM_DP_MAX_NUM_OF_DP);
	/* This is where the conf blob begins. We don't want to write to the cmd regs.*/
	/* Modify the regs corresponding to the port.*/
	return MAKE_DP_CFG_ADDR(port_num, DFM_RATIO_CONV_CFG_0_ADDR);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_section1_size(void)
{
	return sizeof(struct nci_dfm_private_dev_port_config);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_section0_offset(uint32_t dev_id, uint32_t port_num)
{
	unsigned p;
	uint32_t offset = 0;

	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(port_num < DFM_DP_MAX_NUM_OF_DP);

	/* This iteration finds out the offset in the cmd bank for the given
	 * port. It goes through all the ports and keeps adding offset depending
	 * upon the capability of each port till it reaches the concerned port.
	 */
		/* get no. of cmds port supports for each seq and ack and add
		 * offsets for those commands
		 */
	for (p = 0; p < port_num; p++) {
		/* get no. of cmds port supports for each seq and ack and add
		* offsets for those commands
		*/
		offset += ipu_device_dfm_port_properties_get_cmdbank_config_size(dev_id, p);
	}
	assert(offset < (1 << DFM_CMD_BANK_OFFSET_SIZE));
	offset += DFM_CMD_BANK_BASE_ADDR;
	return offset;
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_rc_buf_chasing_offset(uint32_t port_num)
{
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));
	return MAKE_DP_CFG_ADDR(port_num, DFM_RATIO_CONV_SDF_ADDR);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_gather_multicast_offset(uint32_t port_num)
{
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));
	return MAKE_DP_CFG_ADDR(port_num, DFM_GTHR_MULT_EN_MASK_ADDR);
}
