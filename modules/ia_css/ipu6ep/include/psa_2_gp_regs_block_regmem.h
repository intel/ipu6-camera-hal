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
#ifndef _psa_2_gp_regs_block_regmem_h_
#define _psa_2_gp_regs_block_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type psa_2_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_MEMORY_ID = 0
} dai_psa_2_gp_regs_block_memory_id_t;

/* enumerate for all register bank types for devices of type psa_2_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_2_GP_REGS_BLOCK_RB_TYPES = 0
} dai_psa_2_gp_regs_block_rb_types_t;

/* enumerate for all registers for slave port slv_in for devices of type psa_2_gp_regs_block */
typedef enum {
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_ACKBUS_SRST_OUT            = NUM_DAI_PSA_2_GP_REGS_BLOCK_MEMORY_ID,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_ADVANCED_DEMOSAIC_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_VCA_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_YUV1_PROCESSING_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_DVS_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_GTC_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_GAMMA_STAR_SRST_OUT,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_STILLS_MODE_MUX_SEL,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_VEC2STR_RGB_4_DEMUX_SEL,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_STR2VEC_F_INPUT_MUX_SEL,
	DAI_PSA_2_GP_REGS_BLOCK_REG_PSA_GP_REGS_VEC2STR_5_DEMUX_SEL,
	NUM_DAI_PSA_2_GP_REGS_BLOCK_SLV_IN_REGISTERS                       = 11
} dai_psa_2_gp_regs_block_slv_in_registers_t;

#endif /* _psa_2_gp_regs_block_regmem_h_ */
