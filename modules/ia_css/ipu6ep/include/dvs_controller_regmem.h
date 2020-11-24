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
#ifndef _dvs_controller_regmem_h_
#define _dvs_controller_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type dvs_controller */
typedef enum {
	NUM_DAI_DVS_CONTROLLER_MEMORY_ID = 0
} dai_dvs_controller_memory_id_t;

/* enumerate for all register bank types for devices of type dvs_controller */
typedef enum {
	NUM_DAI_DVS_CONTROLLER_RB_TYPES = 0
} dai_dvs_controller_rb_types_t;

/* enumerate for all registers for slave port crq_in for devices of type dvs_controller */
typedef enum {
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_CMD_FIFO_TAIL_L0_INFO = NUM_DAI_DVS_CONTROLLER_MEMORY_ID,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L0_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_DATA_L0_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_CMD_FIFO_TAIL_L1_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L1_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_DATA_L1_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_CMD_FIFO_TAIL_L2_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_ADDR_L2_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_DATA_L2_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_ACK_FIFO_TAIL_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_GENERAL_CNRL_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_DVS_ACC_ACB_ADDR_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_STATUS0_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_STATUS1_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_STATUS2_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_HW_ASSR_CLEAR_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_HW_ASSR_MASK_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_HW_ASSR_STAT_INFO,
	DAI_DVS_CONTROLLER_REG_DVS_CONTROLLER_REG_DVS_CONTROLLER_HW_ASSR_RAW_STAT_INFO,
	NUM_DAI_DVS_CONTROLLER_CRQ_IN_REGISTERS                                        = 19
} dai_dvs_controller_crq_in_registers_t;

#endif /* _dvs_controller_regmem_h_ */
