/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2017 Intel Corporation.
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
#ifndef _dai_ipu_top_device_properties_h_
#define _dai_ipu_top_device_properties_h_

/* Generated with DAI generator 1.3.0 */

#include "bb_output_formatter_regmem.h"
#include "bbgdc4_regmem.h"
#include "bbpolyphase_scaler_regmem.h"
#include "bbtnr_regmem.h"
#include "cell_regmem.h"
#include "dai_ipu_top_devices.h"
#include "device_types.h"
#include "device_types_master_ports.h"
#include "device_types_slave_ports.h"
#include "dfm_regmem.h"
#include "dvs_controller_regmem.h"
#include "entry_regmem.h"
#include "isa_ps_gp_regs_block_regmem.h"
#include "isf_cio_switch_info_regmem.h"
#include "isf_cio_switch_vc2_info_regmem.h"
#include "lbff_infra_gp_regs_block_regmem.h"
#include "mbr_regmem.h"
#include "psa_1_gp_regs_block_regmem.h"
#include "psa_2_gp_regs_block_regmem.h"
#include "psa_3_gp_regs_block_regmem.h"
#include "tr_cio_switch_info_regmem.h"
#include "vec_to_str_v3_regmem.h"
#include "wpt_regmem.h"
/* array for number of master ports for devices of type bb_output_formatter */
static const unsigned int dai_ipu_top_bb_output_formatter_num_master_ports[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	NUM_DAI_BB_OUTPUT_FORMATTER_MASTERS
};

/* array for number of slave ports for devices of type bb_output_formatter */
static const unsigned int dai_ipu_top_bb_output_formatter_num_slave_ports[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	NUM_DAI_BB_OUTPUT_FORMATTER_SLAVES
};

/* array for number of memories for devices of type bb_output_formatter */
static const unsigned int dai_ipu_top_bb_output_formatter_num_memories[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	NUM_DAI_BB_OUTPUT_FORMATTER_MEMORY_ID
};

/* array for number of master ports for devices of type bbgdc4 */
static const unsigned int dai_ipu_top_bbgdc4_num_master_ports[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	NUM_DAI_BBGDC4_MASTERS
};

/* array for number of slave ports for devices of type bbgdc4 */
static const unsigned int dai_ipu_top_bbgdc4_num_slave_ports[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	NUM_DAI_BBGDC4_SLAVES
};

/* array for number of memories for devices of type bbgdc4 */
static const unsigned int dai_ipu_top_bbgdc4_num_memories[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	NUM_DAI_BBGDC4_MEMORY_ID
};

/* array for number of master ports for devices of type bbpolyphase_scaler */
static const unsigned int dai_ipu_top_bbpolyphase_scaler_num_master_ports[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	NUM_DAI_BBPOLYPHASE_SCALER_MASTERS,
	NUM_DAI_BBPOLYPHASE_SCALER_MASTERS
};

/* array for number of slave ports for devices of type bbpolyphase_scaler */
static const unsigned int dai_ipu_top_bbpolyphase_scaler_num_slave_ports[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	NUM_DAI_BBPOLYPHASE_SCALER_SLAVES,
	NUM_DAI_BBPOLYPHASE_SCALER_SLAVES
};

/* array for number of memories for devices of type bbpolyphase_scaler */
static const unsigned int dai_ipu_top_bbpolyphase_scaler_num_memories[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	NUM_DAI_BBPOLYPHASE_SCALER_MEMORY_ID,
	NUM_DAI_BBPOLYPHASE_SCALER_MEMORY_ID
};

/* array for number of master ports for devices of type bbtnr */
static const unsigned int dai_ipu_top_bbtnr_num_master_ports[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	NUM_DAI_BBTNR_MASTERS
};

/* array for number of slave ports for devices of type bbtnr */
static const unsigned int dai_ipu_top_bbtnr_num_slave_ports[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	NUM_DAI_BBTNR_SLAVES
};

/* array for number of memories for devices of type bbtnr */
static const unsigned int dai_ipu_top_bbtnr_num_memories[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	NUM_DAI_BBTNR_MEMORY_ID
};

/* array for number of master ports for devices of type cell */
static const unsigned int dai_ipu_top_cell_num_master_ports[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	NUM_DAI_CELL_MASTERS
};

/* array for number of slave ports for devices of type cell */
static const unsigned int dai_ipu_top_cell_num_slave_ports[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	NUM_DAI_CELL_SLAVES
};

/* array for number of memories for devices of type cell */
static const unsigned int dai_ipu_top_cell_num_memories[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	NUM_DAI_CELL_MEMORY_ID
};

/* array for number of registers per register bank for devices of type dfm */
static const unsigned int dai_dfm_num_registers[NUM_DAI_DFM_RB_TYPES] = {
	NUM_DAI_DFM_DFM_DP_CMD_REGS_REGISTERS,
	NUM_DAI_DFM_DFM_DP_STATUS_REGS_REGISTERS,
	NUM_DAI_DFM_DFM_EMPTY_DP_CFG_REGS_REGISTERS,
	NUM_DAI_DFM_DFM_FULL_DP_CFG_REGS_REGISTERS,
	NUM_DAI_DFM_DFM_SYS_REGS_REGISTERS
};

/* array for number of master ports for devices of type dfm */
static const unsigned int dai_ipu_top_dfm_num_master_ports[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	NUM_DAI_DFM_MASTERS,
	NUM_DAI_DFM_MASTERS,
	NUM_DAI_DFM_MASTERS
};

/* array for number of slave ports for devices of type dfm */
static const unsigned int dai_ipu_top_dfm_num_slave_ports[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	NUM_DAI_DFM_SLAVES,
	NUM_DAI_DFM_SLAVES,
	NUM_DAI_DFM_SLAVES
};

/* array for number of memories for devices of type dfm */
static const unsigned int dai_ipu_top_dfm_num_memories[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	NUM_DAI_DFM_MEMORY_ID,
	NUM_DAI_DFM_MEMORY_ID,
	NUM_DAI_DFM_MEMORY_ID
};

/* array for number of master ports for devices of type dvs_controller */
static const unsigned int dai_ipu_top_dvs_controller_num_master_ports[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	NUM_DAI_DVS_CONTROLLER_MASTERS
};

/* array for number of slave ports for devices of type dvs_controller */
static const unsigned int dai_ipu_top_dvs_controller_num_slave_ports[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	NUM_DAI_DVS_CONTROLLER_SLAVES
};

/* array for number of memories for devices of type dvs_controller */
static const unsigned int dai_ipu_top_dvs_controller_num_memories[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	NUM_DAI_DVS_CONTROLLER_MEMORY_ID
};

/* array for number of master ports for devices of type entry */
static const unsigned int dai_ipu_top_entry_num_master_ports[NUM_DAI_IPU_TOP_ENTRY_INSTANCES] = {
	NUM_DAI_ENTRY_MASTERS
};

/* array for number of slave ports for devices of type entry */
static const unsigned int dai_ipu_top_entry_num_slave_ports[NUM_DAI_IPU_TOP_ENTRY_INSTANCES] = {
	NUM_DAI_ENTRY_SLAVES
};

/* array for number of memories for devices of type entry */
static const unsigned int dai_ipu_top_entry_num_memories[NUM_DAI_IPU_TOP_ENTRY_INSTANCES] = {
	NUM_DAI_ENTRY_MEMORY_ID
};

/* array for number of master ports for devices of type isa_ps_gp_regs_block */
static const unsigned int dai_ipu_top_isa_ps_gp_regs_block_num_master_ports[NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_MASTERS
};

/* array for number of slave ports for devices of type isa_ps_gp_regs_block */
static const unsigned int dai_ipu_top_isa_ps_gp_regs_block_num_slave_ports[NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_SLAVES
};

/* array for number of memories for devices of type isa_ps_gp_regs_block */
static const unsigned int dai_ipu_top_isa_ps_gp_regs_block_num_memories[NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_MEMORY_ID
};

/* array for number of master ports for devices of type isf_cio_switch_info */
static const unsigned int dai_ipu_top_isf_cio_switch_info_num_master_ports[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_INFO_MASTERS
};

/* array for number of slave ports for devices of type isf_cio_switch_info */
static const unsigned int dai_ipu_top_isf_cio_switch_info_num_slave_ports[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_INFO_SLAVES
};

/* array for number of memories for devices of type isf_cio_switch_info */
static const unsigned int dai_ipu_top_isf_cio_switch_info_num_memories[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_INFO_MEMORY_ID
};

/* array for number of master ports for devices of type isf_cio_switch_vc2_info */
static const unsigned int dai_ipu_top_isf_cio_switch_vc2_info_num_master_ports[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_MASTERS
};

/* array for number of slave ports for devices of type isf_cio_switch_vc2_info */
static const unsigned int dai_ipu_top_isf_cio_switch_vc2_info_num_slave_ports[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_SLAVES
};

/* array for number of memories for devices of type isf_cio_switch_vc2_info */
static const unsigned int dai_ipu_top_isf_cio_switch_vc2_info_num_memories[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES] = {
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_MEMORY_ID
};

/* array for number of master ports for devices of type lbff_infra_gp_regs_block */
static const unsigned int dai_ipu_top_lbff_infra_gp_regs_block_num_master_ports[NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_MASTERS
};

/* array for number of slave ports for devices of type lbff_infra_gp_regs_block */
static const unsigned int dai_ipu_top_lbff_infra_gp_regs_block_num_slave_ports[NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_SLAVES
};

/* array for number of memories for devices of type lbff_infra_gp_regs_block */
static const unsigned int dai_ipu_top_lbff_infra_gp_regs_block_num_memories[NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_MEMORY_ID
};

/* array for number of master ports for devices of type mbr */
static const unsigned int dai_ipu_top_mbr_num_master_ports[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	NUM_DAI_MBR_MASTERS,
	NUM_DAI_MBR_MASTERS
};

/* array for number of slave ports for devices of type mbr */
static const unsigned int dai_ipu_top_mbr_num_slave_ports[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	NUM_DAI_MBR_SLAVES,
	NUM_DAI_MBR_SLAVES
};

/* array for number of memories for devices of type mbr */
static const unsigned int dai_ipu_top_mbr_num_memories[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	NUM_DAI_MBR_MEMORY_ID,
	NUM_DAI_MBR_MEMORY_ID
};

/* array for number of master ports for devices of type psa_1_gp_regs_block */
static const unsigned int dai_ipu_top_psa_1_gp_regs_block_num_master_ports[NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_1_GP_REGS_BLOCK_MASTERS
};

/* array for number of slave ports for devices of type psa_1_gp_regs_block */
static const unsigned int dai_ipu_top_psa_1_gp_regs_block_num_slave_ports[NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_1_GP_REGS_BLOCK_SLAVES
};

/* array for number of memories for devices of type psa_1_gp_regs_block */
static const unsigned int dai_ipu_top_psa_1_gp_regs_block_num_memories[NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_1_GP_REGS_BLOCK_MEMORY_ID
};

/* array for number of master ports for devices of type psa_2_gp_regs_block */
static const unsigned int dai_ipu_top_psa_2_gp_regs_block_num_master_ports[NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_MASTERS
};

/* array for number of slave ports for devices of type psa_2_gp_regs_block */
static const unsigned int dai_ipu_top_psa_2_gp_regs_block_num_slave_ports[NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_SLAVES
};

/* array for number of memories for devices of type psa_2_gp_regs_block */
static const unsigned int dai_ipu_top_psa_2_gp_regs_block_num_memories[NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_MEMORY_ID
};

/* array for number of master ports for devices of type psa_3_gp_regs_block */
static const unsigned int dai_ipu_top_psa_3_gp_regs_block_num_master_ports[NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_MASTERS
};

/* array for number of slave ports for devices of type psa_3_gp_regs_block */
static const unsigned int dai_ipu_top_psa_3_gp_regs_block_num_slave_ports[NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_SLAVES
};

/* array for number of memories for devices of type psa_3_gp_regs_block */
static const unsigned int dai_ipu_top_psa_3_gp_regs_block_num_memories[NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES] = {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_MEMORY_ID
};

/* array for number of master ports for devices of type tr_cio_switch_info */
static const unsigned int dai_ipu_top_tr_cio_switch_info_num_master_ports[NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_TR_CIO_SWITCH_INFO_MASTERS
};

/* array for number of slave ports for devices of type tr_cio_switch_info */
static const unsigned int dai_ipu_top_tr_cio_switch_info_num_slave_ports[NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_TR_CIO_SWITCH_INFO_SLAVES
};

/* array for number of memories for devices of type tr_cio_switch_info */
static const unsigned int dai_ipu_top_tr_cio_switch_info_num_memories[NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES] = {
	NUM_DAI_TR_CIO_SWITCH_INFO_MEMORY_ID
};

/* array for number of master ports for devices of type vec_to_str_v3 */
static const unsigned int dai_ipu_top_vec_to_str_v3_num_master_ports[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	NUM_DAI_VEC_TO_STR_V3_MASTERS,
	NUM_DAI_VEC_TO_STR_V3_MASTERS,
	NUM_DAI_VEC_TO_STR_V3_MASTERS
};

/* array for number of slave ports for devices of type vec_to_str_v3 */
static const unsigned int dai_ipu_top_vec_to_str_v3_num_slave_ports[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	NUM_DAI_VEC_TO_STR_V3_SLAVES,
	NUM_DAI_VEC_TO_STR_V3_SLAVES,
	NUM_DAI_VEC_TO_STR_V3_SLAVES
};

/* array for number of memories for devices of type vec_to_str_v3 */
static const unsigned int dai_ipu_top_vec_to_str_v3_num_memories[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	NUM_DAI_VEC_TO_STR_V3_MEMORY_ID,
	NUM_DAI_VEC_TO_STR_V3_MEMORY_ID,
	NUM_DAI_VEC_TO_STR_V3_MEMORY_ID
};

/* array for number of registers per register bank for devices of type wpt */
static const unsigned int dai_wpt_num_registers[NUM_DAI_WPT_RB_TYPES] = {
	NUM_DAI_WPT_ADDRESSES_FIX_REGISTERS,
	NUM_DAI_WPT_ADDRESSES_IDX_REGISTERS
};

/* array for number of master ports for devices of type wpt */
static const unsigned int dai_ipu_top_wpt_num_master_ports[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS,
	NUM_DAI_WPT_MASTERS
};

/* array for number of slave ports for devices of type wpt */
static const unsigned int dai_ipu_top_wpt_num_slave_ports[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES,
	NUM_DAI_WPT_SLAVES
};

/* array for number of memories for devices of type wpt */
static const unsigned int dai_ipu_top_wpt_num_memories[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID,
	NUM_DAI_WPT_MEMORY_ID
};

/* array for number of instances per device type */
static const unsigned int dai_ipu_top_num_instances[NUM_DAI_DEVICE_TYPES] = {
	NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES,
	NUM_DAI_IPU_TOP_BBGDC4_INSTANCES,
	NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES,
	NUM_DAI_IPU_TOP_BBTNR_INSTANCES,
	NUM_DAI_IPU_TOP_CELL_INSTANCES,
	NUM_DAI_IPU_TOP_DFM_INSTANCES,
	NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES,
	NUM_DAI_IPU_TOP_ENTRY_INSTANCES,
	NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES,
	NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES,
	NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES,
	NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES,
	NUM_DAI_IPU_TOP_MBR_INSTANCES,
	NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES,
	NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES,
	NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES,
	NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES,
	NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES,
	NUM_DAI_IPU_TOP_WPT_INSTANCES
};

/* array for number of master ports per device type */
static const unsigned int * const dai_ipu_top_num_master_ports[NUM_DAI_DEVICE_TYPES] = {
	dai_ipu_top_bb_output_formatter_num_master_ports,
	dai_ipu_top_bbgdc4_num_master_ports,
	dai_ipu_top_bbpolyphase_scaler_num_master_ports,
	dai_ipu_top_bbtnr_num_master_ports,
	dai_ipu_top_cell_num_master_ports,
	dai_ipu_top_dfm_num_master_ports,
	dai_ipu_top_dvs_controller_num_master_ports,
	dai_ipu_top_entry_num_master_ports,
	dai_ipu_top_isa_ps_gp_regs_block_num_master_ports,
	dai_ipu_top_isf_cio_switch_info_num_master_ports,
	dai_ipu_top_isf_cio_switch_vc2_info_num_master_ports,
	dai_ipu_top_lbff_infra_gp_regs_block_num_master_ports,
	dai_ipu_top_mbr_num_master_ports,
	dai_ipu_top_psa_1_gp_regs_block_num_master_ports,
	dai_ipu_top_psa_2_gp_regs_block_num_master_ports,
	dai_ipu_top_psa_3_gp_regs_block_num_master_ports,
	dai_ipu_top_tr_cio_switch_info_num_master_ports,
	dai_ipu_top_vec_to_str_v3_num_master_ports,
	dai_ipu_top_wpt_num_master_ports
};

/* array for number of slave ports per device type */
static const unsigned int * const dai_ipu_top_num_slave_ports[NUM_DAI_DEVICE_TYPES] = {
	dai_ipu_top_bb_output_formatter_num_slave_ports,
	dai_ipu_top_bbgdc4_num_slave_ports,
	dai_ipu_top_bbpolyphase_scaler_num_slave_ports,
	dai_ipu_top_bbtnr_num_slave_ports,
	dai_ipu_top_cell_num_slave_ports,
	dai_ipu_top_dfm_num_slave_ports,
	dai_ipu_top_dvs_controller_num_slave_ports,
	dai_ipu_top_entry_num_slave_ports,
	dai_ipu_top_isa_ps_gp_regs_block_num_slave_ports,
	dai_ipu_top_isf_cio_switch_info_num_slave_ports,
	dai_ipu_top_isf_cio_switch_vc2_info_num_slave_ports,
	dai_ipu_top_lbff_infra_gp_regs_block_num_slave_ports,
	dai_ipu_top_mbr_num_slave_ports,
	dai_ipu_top_psa_1_gp_regs_block_num_slave_ports,
	dai_ipu_top_psa_2_gp_regs_block_num_slave_ports,
	dai_ipu_top_psa_3_gp_regs_block_num_slave_ports,
	dai_ipu_top_tr_cio_switch_info_num_slave_ports,
	dai_ipu_top_vec_to_str_v3_num_slave_ports,
	dai_ipu_top_wpt_num_slave_ports
};

/* array for number of memories per device type */
static const unsigned int * const dai_ipu_top_num_memories_ports[NUM_DAI_DEVICE_TYPES] = {
	dai_ipu_top_bb_output_formatter_num_memories,
	dai_ipu_top_bbgdc4_num_memories,
	dai_ipu_top_bbpolyphase_scaler_num_memories,
	dai_ipu_top_bbtnr_num_memories,
	dai_ipu_top_cell_num_memories,
	dai_ipu_top_dfm_num_memories,
	dai_ipu_top_dvs_controller_num_memories,
	dai_ipu_top_entry_num_memories,
	dai_ipu_top_isa_ps_gp_regs_block_num_memories,
	dai_ipu_top_isf_cio_switch_info_num_memories,
	dai_ipu_top_isf_cio_switch_vc2_info_num_memories,
	dai_ipu_top_lbff_infra_gp_regs_block_num_memories,
	dai_ipu_top_mbr_num_memories,
	dai_ipu_top_psa_1_gp_regs_block_num_memories,
	dai_ipu_top_psa_2_gp_regs_block_num_memories,
	dai_ipu_top_psa_3_gp_regs_block_num_memories,
	dai_ipu_top_tr_cio_switch_info_num_memories,
	dai_ipu_top_vec_to_str_v3_num_memories,
	dai_ipu_top_wpt_num_memories
};

/* array for number of register banks per device type */
static const unsigned int dai_ipu_top_num_registersbanks[NUM_DAI_DEVICE_TYPES] = {
	NUM_DAI_BB_OUTPUT_FORMATTER_RB_TYPES,
	NUM_DAI_BBGDC4_RB_TYPES,
	NUM_DAI_BBPOLYPHASE_SCALER_RB_TYPES,
	NUM_DAI_BBTNR_RB_TYPES,
	NUM_DAI_CELL_RB_TYPES,
	NUM_DAI_DFM_RB_TYPES,
	NUM_DAI_DVS_CONTROLLER_RB_TYPES,
	NUM_DAI_ENTRY_RB_TYPES,
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_RB_TYPES,
	NUM_DAI_ISF_CIO_SWITCH_INFO_RB_TYPES,
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_RB_TYPES,
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_RB_TYPES,
	NUM_DAI_MBR_RB_TYPES,
	NUM_DAI_PSA_1_GP_REGS_BLOCK_RB_TYPES,
	NUM_DAI_PSA_2_GP_REGS_BLOCK_RB_TYPES,
	NUM_DAI_PSA_3_GP_REGS_BLOCK_RB_TYPES,
	NUM_DAI_TR_CIO_SWITCH_INFO_RB_TYPES,
	NUM_DAI_VEC_TO_STR_V3_RB_TYPES,
	NUM_DAI_WPT_RB_TYPES
};

/* array for number of registers per device type */
static const unsigned int * const dai_ipu_top_num_registers[NUM_DAI_DEVICE_TYPES] = {
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	dai_dfm_num_registers,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	dai_wpt_num_registers
};

#endif /* _dai_ipu_top_device_properties_h_ */
