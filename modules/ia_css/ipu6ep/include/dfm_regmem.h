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
#ifndef _dfm_regmem_h_
#define _dfm_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type dfm */
typedef enum {
	DAI_DFM_MEM_DFM_CMD_BANK,
	NUM_DAI_DFM_MEMORY_ID    = 1
} dai_dfm_memory_id_t;

/* enumerate for all register bank types for devices of type dfm */
typedef enum {
	DAI_DFM_RB_DFM_DP_CMD_REGS,
	DAI_DFM_RB_DFM_DP_STATUS_REGS,
	DAI_DFM_RB_DFM_EMPTY_DP_CFG_REGS,
	DAI_DFM_RB_DFM_FULL_DP_CFG_REGS,
	DAI_DFM_RB_DFM_SYS_REGS,
	NUM_DAI_DFM_RB_TYPES             = 5
} dai_dfm_rb_types_t;

/* enumerate for all registers for register bank type dfm_dp_cmd_regs for devices of type dfm */
typedef enum {
	DAI_DFM_REG_DFM_DP_CMD_REGS_REG_DFM_DP_CMD_RATIO_CONV_EVENT_CMD_INFO   = 0,
	DAI_DFM_REG_DFM_DP_CMD_REGS_REG_DFM_DP_CMD_RATIO_CONV_SUSPEND_CMD_INFO = 4,
	DAI_DFM_REG_DFM_DP_CMD_REGS_REG_DFM_DP_CMD_BUFF_CTRL_START_INFO        = 8,
	DAI_DFM_REG_DFM_DP_CMD_REGS_REG_DFM_DP_CMD_RATIO_CONV_START_INFO       = 12,
	NUM_DAI_DFM_DFM_DP_CMD_REGS_REGISTERS                                  = 4
} dai_dfm_dfm_dp_cmd_regs_registers_t;

/* enumerate for all registers for register bank type dfm_dp_status_regs for devices of type dfm */
typedef enum {
	DAI_DFM_REG_DFM_DP_STATUS_REGS_REG_DFM_DP_STATUS_RATIO_CONV_STATUS_0_INFO = 0,
	DAI_DFM_REG_DFM_DP_STATUS_REGS_REG_DFM_DP_STATUS_RATIO_CONV_STATUS_1_INFO = 4,
	DAI_DFM_REG_DFM_DP_STATUS_REGS_REG_DFM_DP_STATUS_BUFF_CTRL_STATUS_0_INFO  = 8,
	DAI_DFM_REG_DFM_DP_STATUS_REGS_REG_DFM_DP_STATUS_BUFF_CTRL_STATUS_1_INFO  = 12,
	NUM_DAI_DFM_DFM_DP_STATUS_REGS_REGISTERS                                  = 4
} dai_dfm_dfm_dp_status_regs_registers_t;

/* enumerate for all registers for register bank type dfm_empty_dp_cfg_regs for devices of type dfm */
typedef enum {
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_RATIO_CONV_CFG_0_INFO      = 0,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_RATIO_CONV_CFG_1_INFO      = 4,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_RATIO_CONV_SDF_PR_CFG_INFO = 8,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_GTH_MULT_EN_MASK_INFO      = 12,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_DYNAMIC_VAL_INFO = 16,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_BEGIN_SEQ_INFO   = 20,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_MIDDLE_SEQ_INFO  = 24,
	DAI_DFM_REG_DFM_EMPTY_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_END_SEQ_INFO     = 28,
	NUM_DAI_DFM_DFM_EMPTY_DP_CFG_REGS_REGISTERS                                 = 8
} dai_dfm_dfm_empty_dp_cfg_regs_registers_t;

/* enumerate for all registers for register bank type dfm_full_dp_cfg_regs for devices of type dfm */
typedef enum {
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_RATIO_CONV_CFG_0_INFO      = 0,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_SDF_PR_CFG_INFO  = 4,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_RATIO_CONV_SDF_PR_CFG_INFO = 8,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_GTH_MULT_EN_MASK_INFO      = 12,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_DYNAMIC_VAL_INFO = 16,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_BEGIN_SEQ_INFO   = 20,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_MIDDLE_SEQ_INFO  = 24,
	DAI_DFM_REG_DFM_FULL_DP_CFG_REGS_REG_DFM_DP_CFG_BUFF_CTRL_END_SEQ_INFO     = 28,
	NUM_DAI_DFM_DFM_FULL_DP_CFG_REGS_REGISTERS                                 = 8
} dai_dfm_dfm_full_dp_cfg_regs_registers_t;

/* enumerate for all registers for register bank type dfm_sys_regs for devices of type dfm */
typedef enum {
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_IRQ_INFO                 = 0,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_IRQ_DATA_INFO            = 4,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_CFG_0_INFO               = 8,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_CFG_1_INFO               = 12,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_4_INFO  = 16,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_5_INFO  = 20,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_6_INFO  = 24,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_7_INFO  = 28,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_8_INFO  = 32,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_9_INFO  = 36,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_10_INFO = 40,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_11_INFO = 44,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_12_INFO = 48,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_13_INFO = 52,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_14_INFO = 56,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_ADDR_SPACE_SAVER_15_INFO = 60,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_0_INFO           = 64,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_0_VALID_INFO     = 68,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_1_INFO           = 72,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_1_VALID_INFO     = 76,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_2_INFO           = 80,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_2_VALID_INFO     = 84,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_3_INFO           = 88,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_3_VALID_INFO     = 92,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_4_INFO           = 96,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_4_VALID_INFO     = 100,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_5_INFO           = 104,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_5_VALID_INFO     = 108,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_6_INFO           = 112,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_6_VALID_INFO     = 116,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_7_INFO           = 120,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_7_VALID_INFO     = 124,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_8_INFO           = 128,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_8_VALID_INFO     = 132,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_9_INFO           = 136,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_9_VALID_INFO     = 140,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_10_INFO          = 144,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_10_VALID_INFO    = 148,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_11_INFO          = 152,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_11_VALID_INFO    = 156,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_12_INFO          = 160,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_12_VALID_INFO    = 164,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_13_INFO          = 168,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_13_VALID_INFO    = 172,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_14_INFO          = 176,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_14_VALID_INFO    = 180,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_15_INFO          = 184,
	DAI_DFM_REG_DFM_SYS_REGS_REG_SYS_REG_SDF_REG_15_VALID_INFO    = 188,
	NUM_DAI_DFM_DFM_SYS_REGS_REGISTERS                            = 48
} dai_dfm_dfm_sys_regs_registers_t;

/* enumerate for all registers for slave port sl_x_c for devices of type dfm */
typedef enum {
	NUM_DAI_DFM_SL_X_C_REGISTERS = 0
} dai_dfm_sl_x_c_registers_t;

#endif /* _dfm_regmem_h_ */
