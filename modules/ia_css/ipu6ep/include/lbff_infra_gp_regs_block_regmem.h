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
#ifndef _lbff_infra_gp_regs_block_regmem_h_
#define _lbff_infra_gp_regs_block_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type lbff_infra_gp_regs_block */
typedef enum {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_MEMORY_ID = 0
} dai_lbff_infra_gp_regs_block_memory_id_t;

/* enumerate for all register bank types for devices of type lbff_infra_gp_regs_block */
typedef enum {
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_RB_TYPES = 0
} dai_lbff_infra_gp_regs_block_rb_types_t;

/* enumerate for all registers for slave port slv_in for devices of type lbff_infra_gp_regs_block */
typedef enum {
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_FTA_SRST_OUT                 = NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_MEMORY_ID,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_C_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_D_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_E_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_F_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_G_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_S2V_H_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_V2S_1_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_V2S_4_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_V2S_5_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_DFM_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_W2N_ISA_STRM_ORIG_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_W2N_ISA_STRM_SCALED_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_DVS_CTRL_SRST_OUT,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_V2S_1_OUTPUT_MUX_SEL,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_C_FRAME_SIZE,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_D_FRAME_SIZE,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_E_FRAME_SIZE,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_F_FRAME_SIZE,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_G_FRAME_SIZE,
	DAI_LBFF_INFRA_GP_REGS_BLOCK_REG_LBFF_INFRA_GP_REGS_STR2VEC_H_FRAME_SIZE,
	NUM_DAI_LBFF_INFRA_GP_REGS_BLOCK_SLV_IN_REGISTERS                                = 21
} dai_lbff_infra_gp_regs_block_slv_in_registers_t;

#endif /* _lbff_infra_gp_regs_block_regmem_h_ */
