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
#ifndef _psa_3_gp_regs_block_regmem_h_
#define _psa_3_gp_regs_block_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type psa_3_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_MEMORY_ID = 0
} dai_psa_3_gp_regs_block_memory_id_t;

/* enumerate for all register bank types for devices of type psa_3_gp_regs_block */
typedef enum {
	NUM_DAI_PSA_3_GP_REGS_BLOCK_RB_TYPES = 0
} dai_psa_3_gp_regs_block_rb_types_t;

/* enumerate for all registers for slave port slv_in for devices of type psa_3_gp_regs_block */
typedef enum {
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_ACKBUS_SRST_OUT       = NUM_DAI_PSA_3_GP_REGS_BLOCK_MEMORY_ID,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_VCSC_SRST_OUT,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_GLTM_SRST_OUT,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_XNR_SRST_OUT,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_VCSC_BP_MUX_SEL,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_VCSC_TO_GLTM_BLOCK_EN,
	DAI_PSA_3_GP_REGS_BLOCK_REG_PSA_GP_REGS_BNLM_TO_GLTM_BLOCK_EN,
	NUM_DAI_PSA_3_GP_REGS_BLOCK_SLV_IN_REGISTERS                  = 7
} dai_psa_3_gp_regs_block_slv_in_registers_t;

#endif /* _psa_3_gp_regs_block_regmem_h_ */
