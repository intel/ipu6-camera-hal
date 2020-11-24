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
#ifndef _vec_to_str_v3_regmem_h_
#define _vec_to_str_v3_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type vec_to_str_v3 */
typedef enum {
	NUM_DAI_VEC_TO_STR_V3_MEMORY_ID = 0
} dai_vec_to_str_v3_memory_id_t;

/* enumerate for all register bank types for devices of type vec_to_str_v3 */
typedef enum {
	NUM_DAI_VEC_TO_STR_V3_RB_TYPES = 0
} dai_vec_to_str_v3_rb_types_t;

/* enumerate for all registers for slave port sl_c_cfg for devices of type vec_to_str_v3 */
typedef enum {
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_CMD_FIFO_TAIL       = NUM_DAI_VEC_TO_STR_V3_MEMORY_ID,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_CMD_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_ACK_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_ACK_MSG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_ACK_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_FRAME_DIM,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STRM_DIM,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_IRQ_FALSE_CMD_VAL,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_AUTO_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STRIDE_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STTS_FRAME_LOC,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STTS_IO,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STTS_VEC_RD_BIDS,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STTS_CMD_FSM,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_1_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_2_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_3_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_4_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_EN,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_ST_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_END_ADDR,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_OFFSET_0,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_OFFSET_1,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_5_CFG_STRIDE,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_0_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_1_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_2_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_3_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_4_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_5_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_6_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_7_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_8_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_9_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_10_CFG,
	DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_COMP_11_CFG,
	NUM_DAI_VEC_TO_STR_V3_SL_C_CFG_REGISTERS                = 62
} dai_vec_to_str_v3_sl_c_cfg_registers_t;

#endif /* _vec_to_str_v3_regmem_h_ */
