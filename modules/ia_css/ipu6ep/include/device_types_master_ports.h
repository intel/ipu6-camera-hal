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
#ifndef _device_types_master_ports_h_
#define _device_types_master_ports_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all master ports for devices of type bb_output_formatter */
typedef enum {
	DAI_BB_OUTPUT_FORMATTER_MT_VEC,
	DAI_BB_OUTPUT_FORMATTER_MT_ACK,
	DAI_BB_OUTPUT_FORMATTER_MT_VEC_512,
	DAI_BB_OUTPUT_FORMATTER_MT_VEC_OUT_512,
	NUM_DAI_BB_OUTPUT_FORMATTER_MASTERS    = 4
} dai_bb_output_formatter_masters_t;

/* enumerate for all master ports for devices of type bbgdc4 */
typedef enum {
	DAI_BBGDC4_OUT_ACK,
	DAI_BBGDC4_OUT_DATA,
	NUM_DAI_BBGDC4_MASTERS = 2
} dai_bbgdc4_masters_t;

/* enumerate for all master ports for devices of type bbpolyphase_scaler */
typedef enum {
	DAI_BBPOLYPHASE_SCALER_MT_CTRL,
	DAI_BBPOLYPHASE_SCALER_MT_VEC,
	DAI_BBPOLYPHASE_SCALER_MT_INT,
	DAI_BBPOLYPHASE_SCALER_MT_CONTEXT,
	NUM_DAI_BBPOLYPHASE_SCALER_MASTERS = 4
} dai_bbpolyphase_scaler_masters_t;

/* enumerate for all master ports for devices of type bbtnr */
typedef enum {
	DAI_BBTNR_C_MT,
	DAI_BBTNR_M_MT,
	NUM_DAI_BBTNR_MASTERS = 2
} dai_bbtnr_masters_t;

/* enumerate for all master ports for devices of type cell */
typedef enum {
	DAI_CELL_IMT_OP,
	DAI_CELL_QMT_OP,
	DAI_CELL_CMT_OP,
	DAI_CELL_XMT_OP,
	NUM_DAI_CELL_MASTERS = 4
} dai_cell_masters_t;

/* enumerate for all master ports for devices of type dfm */
typedef enum {
	DAI_DFM_MT_X_C,
	NUM_DAI_DFM_MASTERS = 1
} dai_dfm_masters_t;

/* enumerate for all master ports for devices of type dvs_controller */
typedef enum {
	DAI_DVS_CONTROLLER_CRQ_OUT,
	NUM_DAI_DVS_CONTROLLER_MASTERS = 1
} dai_dvs_controller_masters_t;

/* enumerate for all master ports for devices of type entry */
typedef enum {
	DAI_ENTRY_CIO_S_DOWNSTREAM,
	NUM_DAI_ENTRY_MASTERS      = 1
} dai_entry_masters_t;

/* enumerate for all master ports for devices of type isa_ps_gp_regs_block */
typedef enum {
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_MASTERS = 0
} dai_isa_ps_gp_regs_block_masters_t;

/* enumerate for all master ports for devices of type isf_cio_switch_info */
typedef enum {
	DAI_ISF_CIO_SWITCH_INFO_M_A,
	DAI_ISF_CIO_SWITCH_INFO_M_B,
	NUM_DAI_ISF_CIO_SWITCH_INFO_MASTERS = 2
} dai_isf_cio_switch_info_masters_t;

/* enumerate for all master ports for devices of type isf_cio_switch_vc2_info */
typedef enum {
	DAI_ISF_CIO_SWITCH_VC2_INFO_M_A,
	DAI_ISF_CIO_SWITCH_VC2_INFO_M_B,
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_MASTERS = 2
} dai_isf_cio_switch_vc2_info_masters_t;

/* enumerate for all master ports for devices of type lbff_infra_gp_regs_block */
typedef enum {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_MASTERS = 0
} dai_lbff_infra_gp_regs_block_masters_t;

/* enumerate for all master ports for devices of type mbr */
typedef enum {
	DAI_MBR_MS_DMA,
	NUM_DAI_MBR_MASTERS = 1
} dai_mbr_masters_t;

/* enumerate for all master ports for devices of type psa_1_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_1_GP_REGS_BLOCK_MASTERS = 0
} dai_psa_1_gp_regs_block_masters_t;

/* enumerate for all master ports for devices of type psa_2_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_MASTERS = 0
} dai_psa_2_gp_regs_block_masters_t;

/* enumerate for all master ports for devices of type psa_3_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_MASTERS = 0
} dai_psa_3_gp_regs_block_masters_t;

/* enumerate for all master ports for devices of type tr_cio_switch_info */
typedef enum {
	DAI_TR_CIO_SWITCH_INFO_M_A,
	DAI_TR_CIO_SWITCH_INFO_M_B,
	NUM_DAI_TR_CIO_SWITCH_INFO_MASTERS = 2
} dai_tr_cio_switch_info_masters_t;

/* enumerate for all master ports for devices of type vec_to_str_v3 */
typedef enum {
	DAI_VEC_TO_STR_V3_MT_V_VEC,
	DAI_VEC_TO_STR_V3_MT_C_ACK,
	NUM_DAI_VEC_TO_STR_V3_MASTERS = 2
} dai_vec_to_str_v3_masters_t;

/* enumerate for all master ports for devices of type wpt */
typedef enum {
	DAI_WPT_MST_OBS,
	DAI_WPT_MST_TRC,
	NUM_DAI_WPT_MASTERS = 2
} dai_wpt_masters_t;

typedef enum {
	INVALID_MASTER_PORT = 57005 /* = 0xDEAD */,
	INVALID_PORT        = 57005 /* = 0xDEAD */,
	INVALID_REGISTER    = 57005 /* = 0xDEAD */,
	INVALID_MEMORY      = 57005 /* = 0xDEAD */
} dai_invalid_t;

#endif /* _device_types_master_ports_h_ */
