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
#ifndef _device_types_slave_ports_h_
#define _device_types_slave_ports_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all slave ports for devices of type bb_output_formatter */
typedef enum {
	DAI_BB_OUTPUT_FORMATTER_SL_CFG,
	NUM_DAI_BB_OUTPUT_FORMATTER_SLAVES = 1
} dai_bb_output_formatter_slaves_t;

/* enumerate for all slave ports for devices of type bbgdc4 */
typedef enum {
	DAI_BBGDC4_IN_CFG,
	DAI_BBGDC4_IN_DATA,
	NUM_DAI_BBGDC4_SLAVES = 2
} dai_bbgdc4_slaves_t;

/* enumerate for all slave ports for devices of type bbpolyphase_scaler */
typedef enum {
	DAI_BBPOLYPHASE_SCALER_SL_IN,
	NUM_DAI_BBPOLYPHASE_SCALER_SLAVES = 1
} dai_bbpolyphase_scaler_slaves_t;

/* enumerate for all slave ports for devices of type bbtnr */
typedef enum {
	DAI_BBTNR_C_SL,
	DAI_BBTNR_M_SL,
	NUM_DAI_BBTNR_SLAVES = 2
} dai_bbtnr_slaves_t;

/* enumerate for all slave ports for devices of type cell */
typedef enum {
	DAI_CELL_SL_STAT_IP,
	DAI_CELL_SL_DMEM_IP,
	NUM_DAI_CELL_SLAVES = 2
} dai_cell_slaves_t;

/* enumerate for all slave ports for devices of type dfm */
typedef enum {
	DAI_DFM_SL_X_C,
	NUM_DAI_DFM_SLAVES = 1
} dai_dfm_slaves_t;

/* enumerate for all slave ports for devices of type dvs_controller */
typedef enum {
	DAI_DVS_CONTROLLER_CRQ_IN,
	NUM_DAI_DVS_CONTROLLER_SLAVES = 1
} dai_dvs_controller_slaves_t;

/* enumerate for all slave ports for devices of type entry */
typedef enum {
	DAI_ENTRY_CIO_M_UPSTREAM,
	DAI_ENTRY_IS_NPK,
	DAI_ENTRY_MT_BUTT_NPK,
	NUM_DAI_ENTRY_SLAVES     = 3
} dai_entry_slaves_t;

/* enumerate for all slave ports for devices of type isa_ps_gp_regs_block */
typedef enum {
	DAI_ISA_PS_GP_REGS_BLOCK_SLV_IN,
	NUM_DAI_ISA_PS_GP_REGS_BLOCK_SLAVES = 1
} dai_isa_ps_gp_regs_block_slaves_t;

/* enumerate for all slave ports for devices of type isf_cio_switch_info */
typedef enum {
	DAI_ISF_CIO_SWITCH_INFO_SLAVE,
	NUM_DAI_ISF_CIO_SWITCH_INFO_SLAVES = 1
} dai_isf_cio_switch_info_slaves_t;

/* enumerate for all slave ports for devices of type isf_cio_switch_vc2_info */
typedef enum {
	DAI_ISF_CIO_SWITCH_VC2_INFO_SLAVE,
	NUM_DAI_ISF_CIO_SWITCH_VC2_INFO_SLAVES = 1
} dai_isf_cio_switch_vc2_info_slaves_t;

/* enumerate for all slave ports for devices of type lbff_infra_gp_regs_block */
typedef enum {
	DAI_LBFF_INFRA_GP_REGS_BLOCK_SLV_IN,
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_SLAVES = 1
} dai_lbff_infra_gp_regs_block_slaves_t;

/* enumerate for all slave ports for devices of type mbr */
typedef enum {
	DAI_MBR_CRQ_IN,
	NUM_DAI_MBR_SLAVES = 1
} dai_mbr_slaves_t;

/* enumerate for all slave ports for devices of type psa_1_gp_regs_block */
typedef enum {
	DAI_PSA_1_GP_REGS_BLOCK_SLV_IN,
	NUM_DAI_PSA_1_GP_REGS_BLOCK_SLAVES = 1
} dai_psa_1_gp_regs_block_slaves_t;

/* enumerate for all slave ports for devices of type psa_2_gp_regs_block */
typedef enum {
	DAI_PSA_2_GP_REGS_BLOCK_SLV_IN,
	NUM_DAI_PSA_2_GP_REGS_BLOCK_SLAVES = 1
} dai_psa_2_gp_regs_block_slaves_t;

/* enumerate for all slave ports for devices of type psa_3_gp_regs_block */
typedef enum {
	DAI_PSA_3_GP_REGS_BLOCK_SLV_IN,
	NUM_DAI_PSA_3_GP_REGS_BLOCK_SLAVES = 1
} dai_psa_3_gp_regs_block_slaves_t;

/* enumerate for all slave ports for devices of type tr_cio_switch_info */
typedef enum {
	DAI_TR_CIO_SWITCH_INFO_SLAVE,
	NUM_DAI_TR_CIO_SWITCH_INFO_SLAVES = 1
} dai_tr_cio_switch_info_slaves_t;

/* enumerate for all slave ports for devices of type vec_to_str_v3 */
typedef enum {
	DAI_VEC_TO_STR_V3_SL_C_CFG,
	NUM_DAI_VEC_TO_STR_V3_SLAVES = 1
} dai_vec_to_str_v3_slaves_t;

/* enumerate for all slave ports for devices of type wpt */
typedef enum {
	DAI_WPT_SLV_OBS,
	DAI_WPT_SLV_CFG,
	NUM_DAI_WPT_SLAVES = 2
} dai_wpt_slaves_t;

#endif /* _device_types_slave_ports_h_ */
