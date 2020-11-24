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
#ifndef _dai_ipu_top_devices_h_
#define _dai_ipu_top_devices_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all devices of type bb_output_formatter */
typedef enum {
	DAI_IPU_TOP_OFS_TOP_OF,
	NUM_DAI_IPU_TOP_BB_OUTPUT_FORMATTER_INSTANCES = 1
} dai_ipu_top_bb_output_formatter_instances_t;

/* enumerate for all devices of type bbgdc4 */
typedef enum {
	DAI_IPU_TOP_GDC_TOP_GDC0,
	NUM_DAI_IPU_TOP_BBGDC4_INSTANCES = 1
} dai_ipu_top_bbgdc4_instances_t;

/* enumerate for all devices of type bbpolyphase_scaler */
typedef enum {
	DAI_IPU_TOP_OFS_TOP_SCALER0_SCALER,
	DAI_IPU_TOP_OFS_TOP_SCALER1_SCALER,
	NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES = 2
} dai_ipu_top_bbpolyphase_scaler_instances_t;

/* enumerate for all devices of type bbtnr */
typedef enum {
	DAI_IPU_TOP_TNR_TOP_I_TNR6,
	NUM_DAI_IPU_TOP_BBTNR_INSTANCES = 1
} dai_ipu_top_bbtnr_instances_t;

/* enumerate for all devices of type cell */
typedef enum {
	DAI_IPU_TOP_SP_CONTROL_TILE_PS_SP,
	NUM_DAI_IPU_TOP_CELL_INSTANCES    = 1
} dai_ipu_top_cell_instances_t;

/* enumerate for all devices of type cfchop_cio_switch */
typedef enum {
	NUM_DAI_IPU_TOP_CFCHOP_CIO_SWITCH_INSTANCES = 0
} dai_ipu_top_cfchop_cio_switch_instances_t;

/* enumerate for all devices of type cfchop_cio_switch_info */
typedef enum {
	NUM_DAI_IPU_TOP_CFCHOP_CIO_SWITCH_INFO_INSTANCES = 0
} dai_ipu_top_cfchop_cio_switch_info_instances_t;

/* enumerate for all devices of type dfm */
typedef enum {
	DAI_IPU_TOP_BB_FF_FABRIC_TOP_I_DFM,
	DAI_IPU_TOP_ISL_CTRL_PS_TOP_I_ISL_PS_DFM,
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_I_LBFF_DFM,
	NUM_DAI_IPU_TOP_DFM_INSTANCES                    = 3
} dai_ipu_top_dfm_instances_t;

/* enumerate for all devices of type dvs_controller */
typedef enum {
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_DVS_CONTROLLER,
	NUM_DAI_IPU_TOP_DVS_CONTROLLER_INSTANCES             = 1
} dai_ipu_top_dvs_controller_instances_t;

/* enumerate for all devices of type entry */
typedef enum {
	DAI_IPU_TOP_ENTRY,
	NUM_DAI_IPU_TOP_ENTRY_INSTANCES = 1
} dai_ipu_top_entry_instances_t;

/* enumerate for all devices of type isa_ps_gp_regs_block */
typedef enum {
	DAI_IPU_TOP_ISA_PS_TOP_ISA_PS_CLUSTER_ISA_PS_GP_REG,
	NUM_DAI_IPU_TOP_ISA_PS_GP_REGS_BLOCK_INSTANCES      = 1
} dai_ipu_top_isa_ps_gp_regs_block_instances_t;

/* enumerate for all devices of type isf_cio_switch_info */
typedef enum {
	DAI_IPU_TOP_ISF_CMPR_TOP_CIOSWITCH_CMP,
	NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_INFO_INSTANCES = 1
} dai_ipu_top_isf_cio_switch_info_instances_t;

/* enumerate for all devices of type isf_cio_switch_vc2_info */
typedef enum {
	DAI_IPU_TOP_ISF_CDC_MMU_TOP_MMU_AT_SYSTEM_CIO_SWITCH_I,
	NUM_DAI_IPU_TOP_ISF_CIO_SWITCH_VC2_INFO_INSTANCES      = 1
} dai_ipu_top_isf_cio_switch_vc2_info_instances_t;

/* enumerate for all devices of type lbff_infra_gp_regs_block */
typedef enum {
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_GPREGS,
	NUM_DAI_IPU_TOP_LBFF_INFRA_GP_REGS_BLOCK_INSTANCES = 1
} dai_ipu_top_lbff_infra_gp_regs_block_instances_t;

/* enumerate for all devices of type mbr */
typedef enum {
	DAI_IPU_TOP_GDC_TOP_GDC_MBR,
	DAI_IPU_TOP_TNR_TOP_TNR_MBR,
	NUM_DAI_IPU_TOP_MBR_INSTANCES = 2
} dai_ipu_top_mbr_instances_t;

/* enumerate for all devices of type psa_1_gp_regs_block */
typedef enum {
	DAI_IPU_TOP_PSA_1_TOP_PSA_1_FF_CLUSTER_ACC_GP_REG,
	NUM_DAI_IPU_TOP_PSA_1_GP_REGS_BLOCK_INSTANCES     = 1
} dai_ipu_top_psa_1_gp_regs_block_instances_t;

/* enumerate for all devices of type psa_2_gp_regs_block */
typedef enum {
	DAI_IPU_TOP_PSA_2_TOP_PSA_2_FF_CLUSTER_ACC_GP_REG,
	NUM_DAI_IPU_TOP_PSA_2_GP_REGS_BLOCK_INSTANCES     = 1
} dai_ipu_top_psa_2_gp_regs_block_instances_t;

/* enumerate for all devices of type psa_3_gp_regs_block */
typedef enum {
	DAI_IPU_TOP_PSA_3_TOP_PSA_3_FF_CLUSTER_ACC_GP_REG,
	NUM_DAI_IPU_TOP_PSA_3_GP_REGS_BLOCK_INSTANCES     = 1
} dai_ipu_top_psa_3_gp_regs_block_instances_t;

/* enumerate for all devices of type spcl_cio_switch */
typedef enum {
	NUM_DAI_IPU_TOP_SPCL_CIO_SWITCH_INSTANCES = 0
} dai_ipu_top_spcl_cio_switch_instances_t;

/* enumerate for all devices of type spcl_cio_switch_info */
typedef enum {
	NUM_DAI_IPU_TOP_SPCL_CIO_SWITCH_INFO_INSTANCES = 0
} dai_ipu_top_spcl_cio_switch_info_instances_t;

/* enumerate for all devices of type tr_cio_switch_cmp_info */
typedef enum {
	NUM_DAI_IPU_TOP_TR_CIO_SWITCH_CMP_INFO_INSTANCES = 0
} dai_ipu_top_tr_cio_switch_cmp_info_instances_t;

/* enumerate for all devices of type tr_cio_switch_info */
typedef enum {
	DAI_IPU_TOP_CDC_MMU_TOP_MMU_AT_SYSTEM_CIO_SWITCH_I,
	NUM_DAI_IPU_TOP_TR_CIO_SWITCH_INFO_INSTANCES       = 1
} dai_ipu_top_tr_cio_switch_info_instances_t;

/* enumerate for all devices of type vec_to_str_v3 */
typedef enum {
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_VEC2STR_1_VEC2STR,
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_VEC2STR_4_VEC2STR,
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_VEC2STR_5_VEC2STR,
	NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES                 = 3
} dai_ipu_top_vec_to_str_v3_instances_t;

/* enumerate for all devices of type wpt */
typedef enum {
	DAI_IPU_TOP_DMA_TOP_WPT_DMEM,
	DAI_IPU_TOP_DMA_TOP_WPT_SL_FABRIC_C,
	DAI_IPU_TOP_FABRIC_TOP_WPT_DMAI,
	DAI_IPU_TOP_FABRIC_TOP_WPT_SL_TF,
	DAI_IPU_TOP_FABRIC_TOP_WPT_SPC,
	DAI_IPU_TOP_FABRIC_TOP_WPT_SPP0,
	DAI_IPU_TOP_FABRIC_TOP_WPT_SPP1,
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_WPT_DFMMT,
	DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_WPT_DFMSL,
	NUM_DAI_IPU_TOP_WPT_INSTANCES                   = 9
} dai_ipu_top_wpt_instances_t;

#endif /* _dai_ipu_top_devices_h_ */
