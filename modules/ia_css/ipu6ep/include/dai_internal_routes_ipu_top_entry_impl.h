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
#ifndef _dai_internal_routes_ipu_top_entry_impl_h_
#define _dai_internal_routes_ipu_top_entry_impl_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_access_types.h>
#include "dai_ipu_top_device_regmem.h"
#include "dai_ipu_top_devices.h"
#include "device_types_master_ports.h"
#include "device_types_slave_ports.h"
/* Routing per slave port of of for view point of device entry */
static const dai_internal_route_t dai_ipu_top_ofs_top_of_internal_route[NUM_DAI_BB_OUTPUT_FORMATTER_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x32F9000, 0x0, dai_ipu_top_ofs_top_of_sl_cfg_regmem_addresses}
};

/* Handles to the internal routes for devices of type bb_output_formatter */
static const dai_internal_route_t * const dai_ipu_top_type_bb_output_formatter_internal_route[NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES] = {
	dai_ipu_top_ofs_top_of_internal_route
};

/* Routing per slave port of gdc0 for view point of device entry */
static const dai_internal_route_t dai_ipu_top_gdc_top_gdc0_internal_route[NUM_DAI_BBGDC4_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3280000, 0x0, dai_ipu_top_gdc_top_gdc0_in_cfg_regmem_addresses},
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0}
};

/* Handles to the internal routes for devices of type bbgdc4 */
static const dai_internal_route_t * const dai_ipu_top_type_bbgdc4_internal_route[NUM_DAI_IPU_TOP_BBGDC4_INSTANCES] = {
	dai_ipu_top_gdc_top_gdc0_internal_route
};

/* Routing per slave port of scaler for view point of device entry */
static const dai_internal_route_t dai_ipu_top_ofs_top_scaler0_scaler_internal_route[NUM_DAI_BBPOLYPHASE_SCALER_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x32F8000, 0x0, dai_ipu_top_ofs_top_scaler0_scaler_sl_in_regmem_addresses}
};

/* Routing per slave port of scaler for view point of device entry */
static const dai_internal_route_t dai_ipu_top_ofs_top_scaler1_scaler_internal_route[NUM_DAI_BBPOLYPHASE_SCALER_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x32F8800, 0x0, dai_ipu_top_ofs_top_scaler1_scaler_sl_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type bbpolyphase_scaler */
static const dai_internal_route_t * const dai_ipu_top_type_bbpolyphase_scaler_internal_route[NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES] = {
	dai_ipu_top_ofs_top_scaler0_scaler_internal_route,
	dai_ipu_top_ofs_top_scaler1_scaler_internal_route
};

/* Routing per slave port of i_tnr6 for view point of device entry */
static const dai_internal_route_t dai_ipu_top_tnr_top_i_tnr6_internal_route[NUM_DAI_BBTNR_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3300000, 0x0, dai_ipu_top_tnr_top_i_tnr6_c_sl_regmem_addresses},
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0}
};

/* Handles to the internal routes for devices of type bbtnr */
static const dai_internal_route_t * const dai_ipu_top_type_bbtnr_internal_route[NUM_DAI_IPU_TOP_BBTNR_INSTANCES] = {
	dai_ipu_top_tnr_top_i_tnr6_internal_route
};

/* Routing per slave port of sp for view point of device entry */
static const dai_internal_route_t dai_ipu_top_sp_control_tile_ps_sp_internal_route[NUM_DAI_CELL_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x118000, 0x0, dai_ipu_top_sp_control_tile_ps_sp_sl_stat_ip_regmem_addresses},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x100000, 0x0, dai_ipu_top_sp_control_tile_ps_sp_sl_dmem_ip_regmem_addresses}
};

/* Handles to the internal routes for devices of type cell */
static const dai_internal_route_t * const dai_ipu_top_type_cell_internal_route[NUM_DAI_IPU_TOP_CELL_INSTANCES] = {
	dai_ipu_top_sp_control_tile_ps_sp_internal_route
};

/* Routing per slave port of i_dfm for view point of device entry */
static const dai_internal_route_t dai_ipu_top_bb_ff_fabric_top_i_dfm_internal_route[NUM_DAI_DFM_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3350000, dai_ipu_top_bb_ff_fabric_top_i_dfm_registerbanktypes, dai_ipu_top_bb_ff_fabric_top_i_dfm_sl_x_c_regmem_addresses}
};

/* Routing per slave port of i_isl_ps_dfm for view point of device entry */
static const dai_internal_route_t dai_ipu_top_isl_ctrl_ps_top_i_isl_ps_dfm_internal_route[NUM_DAI_DFM_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x302C000, dai_ipu_top_isl_ctrl_ps_top_i_isl_ps_dfm_registerbanktypes, dai_ipu_top_isl_ctrl_ps_top_i_isl_ps_dfm_sl_x_c_regmem_addresses}
};

/* Routing per slave port of i_lbff_dfm for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_i_lbff_dfm_internal_route[NUM_DAI_DFM_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3213000, dai_ipu_top_lbff_infra_top_lbff_infra_i_lbff_dfm_registerbanktypes, dai_ipu_top_lbff_infra_top_lbff_infra_i_lbff_dfm_sl_x_c_regmem_addresses}
};

/* Handles to the internal routes for devices of type dfm */
static const dai_internal_route_t * const dai_ipu_top_type_dfm_internal_route[NUM_DAI_IPU_TOP_DFM_INSTANCES] = {
	dai_ipu_top_bb_ff_fabric_top_i_dfm_internal_route,
	dai_ipu_top_isl_ctrl_ps_top_i_isl_ps_dfm_internal_route,
	dai_ipu_top_lbff_infra_top_lbff_infra_i_lbff_dfm_internal_route
};

/* Routing per slave port of dvs_controller for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_dvs_controller_internal_route[NUM_DAI_DVS_CONTROLLER_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3218000, 0x0, dai_ipu_top_lbff_infra_top_lbff_infra_dvs_controller_crq_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type dvs_controller */
static const dai_internal_route_t * const dai_ipu_top_type_dvs_controller_internal_route[NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES] = {
	dai_ipu_top_lbff_infra_top_lbff_infra_dvs_controller_internal_route
};

/* Handles to the internal routes for devices of type entry */
static const dai_internal_route_t * const dai_ipu_top_type_entry_internal_route[NUM_DAI_IPU_TOP_ENTRY_INSTANCES] = {
	0x0 /* this is device entry itself */
};

/* Routing per slave port of isa_ps_gp_reg for view point of device entry */
static const dai_internal_route_t dai_ipu_top_isa_ps_top_isa_ps_cluster_isa_ps_gp_reg_internal_route[NUM_DAI_ISA_PS_GP_REGS_BLOCK_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3049000, 0x0, dai_ipu_top_isa_ps_top_isa_ps_cluster_isa_ps_gp_reg_slv_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type isa_ps_gp_regs_block */
static const dai_internal_route_t * const dai_ipu_top_type_isa_ps_gp_regs_block_internal_route[NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES] = {
	dai_ipu_top_isa_ps_top_isa_ps_cluster_isa_ps_gp_reg_internal_route
};

/* Routing per slave port of cioswitch_cmp for view point of device entry */
static const dai_internal_route_t dai_ipu_top_isf_cmpr_top_cioswitch_cmp_internal_route[NUM_DAI_ISF_CIO_SWITCH_INFO_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0}
};

/* Handles to the internal routes for devices of type isf_cio_switch_info */
static const dai_internal_route_t * const dai_ipu_top_type_isf_cio_switch_info_internal_route[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES] = {
	dai_ipu_top_isf_cmpr_top_cioswitch_cmp_internal_route /* device cioswitch_cmp is not seen by entry */
};

/* Routing per slave port of cio_switch_i for view point of device entry */
static const dai_internal_route_t dai_ipu_top_isf_cdc_mmu_top_mmu_at_system_cio_switch_i_internal_route[NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0}
};

/* Handles to the internal routes for devices of type isf_cio_switch_vc2_info */
static const dai_internal_route_t * const dai_ipu_top_type_isf_cio_switch_vc2_info_internal_route[NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES] = {
	dai_ipu_top_isf_cdc_mmu_top_mmu_at_system_cio_switch_i_internal_route /* device cio_switch_i is not seen by entry */
};

/* Routing per slave port of gpregs for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_gpregs_internal_route[NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3216000, 0x0, dai_ipu_top_lbff_infra_top_lbff_infra_gpregs_slv_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type lbff_infra_gp_regs_block */
static const dai_internal_route_t * const dai_ipu_top_type_lbff_infra_gp_regs_block_internal_route[NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES] = {
	dai_ipu_top_lbff_infra_top_lbff_infra_gpregs_internal_route
};

/* Routing per slave port of gdc_mbr for view point of device entry */
static const dai_internal_route_t dai_ipu_top_gdc_top_gdc_mbr_internal_route[NUM_DAI_MBR_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3284000, 0x0, dai_ipu_top_gdc_top_gdc_mbr_crq_in_regmem_addresses}
};

/* Routing per slave port of tnr_mbr for view point of device entry */
static const dai_internal_route_t dai_ipu_top_tnr_top_tnr_mbr_internal_route[NUM_DAI_MBR_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3304000, 0x0, dai_ipu_top_tnr_top_tnr_mbr_crq_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type mbr */
static const dai_internal_route_t * const dai_ipu_top_type_mbr_internal_route[NUM_DAI_IPU_TOP_MBR_INSTANCES] = {
	dai_ipu_top_gdc_top_gdc_mbr_internal_route,
	dai_ipu_top_tnr_top_tnr_mbr_internal_route
};

/* Routing per slave port of acc_gp_reg for view point of device entry */
static const dai_internal_route_t dai_ipu_top_psa_1_top_psa_1_ff_cluster_acc_gp_reg_internal_route[NUM_DAI_PSA_1_GP_REGS_BLOCK_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3220000, 0x0, dai_ipu_top_psa_1_top_psa_1_ff_cluster_acc_gp_reg_slv_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type psa_1_gp_regs_block */
static const dai_internal_route_t * const dai_ipu_top_type_psa_1_gp_regs_block_internal_route[NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES] = {
	dai_ipu_top_psa_1_top_psa_1_ff_cluster_acc_gp_reg_internal_route
};

/* Routing per slave port of acc_gp_reg for view point of device entry */
static const dai_internal_route_t dai_ipu_top_psa_2_top_psa_2_ff_cluster_acc_gp_reg_internal_route[NUM_DAI_PSA_2_GP_REGS_BLOCK_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x322E000, 0x0, dai_ipu_top_psa_2_top_psa_2_ff_cluster_acc_gp_reg_slv_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type psa_2_gp_regs_block */
static const dai_internal_route_t * const dai_ipu_top_type_psa_2_gp_regs_block_internal_route[NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES] = {
	dai_ipu_top_psa_2_top_psa_2_ff_cluster_acc_gp_reg_internal_route
};

/* Routing per slave port of acc_gp_reg for view point of device entry */
static const dai_internal_route_t dai_ipu_top_psa_3_top_psa_3_ff_cluster_acc_gp_reg_internal_route[NUM_DAI_PSA_3_GP_REGS_BLOCK_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3240000, 0x0, dai_ipu_top_psa_3_top_psa_3_ff_cluster_acc_gp_reg_slv_in_regmem_addresses}
};

/* Handles to the internal routes for devices of type psa_3_gp_regs_block */
static const dai_internal_route_t * const dai_ipu_top_type_psa_3_gp_regs_block_internal_route[NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES] = {
	dai_ipu_top_psa_3_top_psa_3_ff_cluster_acc_gp_reg_internal_route
};

/* Routing per slave port of cio_switch_i for view point of device entry */
static const dai_internal_route_t dai_ipu_top_cdc_mmu_top_mmu_at_system_cio_switch_i_internal_route[NUM_DAI_TR_CIO_SWITCH_INFO_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0}
};

/* Handles to the internal routes for devices of type tr_cio_switch_info */
static const dai_internal_route_t * const dai_ipu_top_type_tr_cio_switch_info_internal_route[NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES] = {
	dai_ipu_top_cdc_mmu_top_mmu_at_system_cio_switch_i_internal_route /* device cio_switch_i is not seen by entry */
};

/* Routing per slave port of vec2str for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_1_vec2str_internal_route[NUM_DAI_VEC_TO_STR_V3_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3210000, 0x0, dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_1_vec2str_sl_c_cfg_regmem_addresses}
};

/* Routing per slave port of vec2str for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_4_vec2str_internal_route[NUM_DAI_VEC_TO_STR_V3_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3210100, 0x0, dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_4_vec2str_sl_c_cfg_regmem_addresses}
};

/* Routing per slave port of vec2str for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_5_vec2str_internal_route[NUM_DAI_VEC_TO_STR_V3_SLAVES] = {
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x3210200, 0x0, dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_5_vec2str_sl_c_cfg_regmem_addresses}
};

/* Handles to the internal routes for devices of type vec_to_str_v3 */
static const dai_internal_route_t * const dai_ipu_top_type_vec_to_str_v3_internal_route[NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES] = {
	dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_1_vec2str_internal_route,
	dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_4_vec2str_internal_route,
	dai_ipu_top_lbff_infra_top_lbff_infra_vec2str_5_vec2str_internal_route
};

/* Routing per slave port of wpt_dmem for view point of device entry */
static const dai_internal_route_t dai_ipu_top_dma_top_wpt_dmem_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x198000, dai_ipu_top_dma_top_wpt_dmem_registerbanktypes, dai_ipu_top_dma_top_wpt_dmem_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_sl_fabric_c for view point of device entry */
static const dai_internal_route_t dai_ipu_top_dma_top_wpt_sl_fabric_c_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x199000, dai_ipu_top_dma_top_wpt_sl_fabric_c_registerbanktypes, dai_ipu_top_dma_top_wpt_sl_fabric_c_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_dmai for view point of device entry */
static const dai_internal_route_t dai_ipu_top_fabric_top_wpt_dmai_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x175000, dai_ipu_top_fabric_top_wpt_dmai_registerbanktypes, dai_ipu_top_fabric_top_wpt_dmai_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_sl_tf for view point of device entry */
static const dai_internal_route_t dai_ipu_top_fabric_top_wpt_sl_tf_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x173000, dai_ipu_top_fabric_top_wpt_sl_tf_registerbanktypes, dai_ipu_top_fabric_top_wpt_sl_tf_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_spc for view point of device entry */
static const dai_internal_route_t dai_ipu_top_fabric_top_wpt_spc_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x170000, dai_ipu_top_fabric_top_wpt_spc_registerbanktypes, dai_ipu_top_fabric_top_wpt_spc_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_spp0 for view point of device entry */
static const dai_internal_route_t dai_ipu_top_fabric_top_wpt_spp0_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x171000, dai_ipu_top_fabric_top_wpt_spp0_registerbanktypes, dai_ipu_top_fabric_top_wpt_spp0_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_spp1 for view point of device entry */
static const dai_internal_route_t dai_ipu_top_fabric_top_wpt_spp1_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x172000, dai_ipu_top_fabric_top_wpt_spp1_registerbanktypes, dai_ipu_top_fabric_top_wpt_spp1_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_dfmmt for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmmt_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x321B000, dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmmt_registerbanktypes, dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmmt_slv_cfg_regmem_addresses}
};

/* Routing per slave port of wpt_dfmsl for view point of device entry */
static const dai_internal_route_t dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmsl_internal_route[NUM_DAI_WPT_SLAVES] = {
	{INVALID_MASTER_PORT, INVALID_ADDRESS, 0x0, 0x0},
	{DAI_ENTRY_CIO_S_DOWNSTREAM, 0x321A000, dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmsl_registerbanktypes, dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmsl_slv_cfg_regmem_addresses}
};

/* Handles to the internal routes for devices of type wpt */
static const dai_internal_route_t * const dai_ipu_top_type_wpt_internal_route[NUM_DAI_IPU_TOP_WPT_INSTANCES] = {
	dai_ipu_top_dma_top_wpt_dmem_internal_route,
	dai_ipu_top_dma_top_wpt_sl_fabric_c_internal_route,
	dai_ipu_top_fabric_top_wpt_dmai_internal_route,
	dai_ipu_top_fabric_top_wpt_sl_tf_internal_route,
	dai_ipu_top_fabric_top_wpt_spc_internal_route,
	dai_ipu_top_fabric_top_wpt_spp0_internal_route,
	dai_ipu_top_fabric_top_wpt_spp1_internal_route,
	dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmmt_internal_route,
	dai_ipu_top_lbff_infra_top_lbff_infra_wpt_dfmsl_internal_route
};

#endif /* _dai_internal_routes_ipu_top_entry_impl_h_ */
