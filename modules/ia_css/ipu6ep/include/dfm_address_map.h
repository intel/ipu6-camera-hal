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
/*
 * dfm_address_map.h
 *
 *  Created on: Nov 4, 2015
 *      Author: shalevmi
 */

#ifndef DFM_DFM_ADDRESS_MAP_H_
#define DFM_DFM_ADDRESS_MAP_H_

// Decoding of address's bit 12
#define DFM_SUB_DEVICE_ID_BIT_POS   			12
#define DFM_DP_SUB_DEVICE_ID 					0
#define DFM_SYS_REG_CMD_BANK_SUB_DEVICE_ID 		1
// Decoding of address's bit 11
#define DFM_DP_CFG_SELECT_BIT_POS				11
#define DFM_CMD_STATUS_REG_SUB_DEVICE_ID  		0
#define DFM_CFG_REG_SUB_DEVICE_ID  				1
// Decoding of address's bit 10
#define DFM_DP_STATUS_SELECT_BIT_POS			10
#define DFM_CMD_REG_SUB_DEVICE_ID  				0
#define DFM_STATUS_REG_SUB_DEVICE_ID  			1

//
#define DFM_DP_ID_SIZE							6
#define DFM_DP_OFFSET_SIZE						6
#define DFM_DP_ID_CFG_LSB_BIT_POS				5
#define DFM_DP_ID_STATUS_LSB_BIT_POS			4
#define DFM_DP_CFG_OFFSET_SIZE					5
#define DFM_DP_STATUS_OFFSET_SIZE				4

//
#define DFM_CMD_BANK_OFFSET_SIZE				12

// Derived values
#define DFM_DP_MAX_NUM_OF_REGISTERS_PER_DP  	((1 << DFM_DP_OFFSET_SIZE) >> 2)
#define DFM_DP_MAX_NUM_OF_DP  					(1 << DFM_DP_ID_SIZE)
#define DFM_DP_MAX_NUM_OF_DP_PAIRS 				(DFM_DP_MAX_NUM_OF_DP >> 1)
#define DFM_DP_IS_EMPTY(dp_id)					!((dp_id) >> (DFM_DP_ID_SIZE - 1))
#define DFM_TEST_ID_VALID_DP(dp_id, dp_pairs_num) (((dp_id) & 0x1F) < (dp_pairs_num))

// Device-port register's addresses within address-space of device-port
#define DFM_RATIO_CONV_EVENT_CMD_ADDR 			0x0
#define DFM_RATIO_CONV_SUSPEND_CMD_ADDR 		0x4
#define DFM_BUFF_CTRL_START_ADDR    			0x8
#define DFM_RATIO_CONV_START_ADDR    			0xC
//
#define DFM_RATIO_CONV_STATUS_0_ADDR    		0x0
#define DFM_RATIO_CONV_STATUS_1_ADDR    		0x4
#define DFM_BUFF_CTRL_STATUS_0_ADDR    			0x8
//
#define DFM_RATIO_CONV_CFG_0_ADDR    			0x0
#define DFM_RATIO_CONV_CFG_1_ADDR    			0x4
#define DFM_RATIO_CONV_SDF_ADDR    				0x8
#define DFM_GTHR_MULT_EN_MASK_ADDR 				0xC
#define DFM_BUFF_CTRL_DYNAMIC_VAL_ADDR    		0x10
#define DFM_BUFF_CTRL_BEGIN_SEQ_ADDR        	0x14
#define DFM_BUFF_CTRL_MIDDLE_SEQ_ADDR        	0x18
#define DFM_BUFF_CTRL_END_SEQ_ADDR        		0x1C

// System register addresses relative to system-registers address-space (512 registers)
#define DFM_SYS_REG_IRQ_ADDR					0x0
#define DFM_SYS_REG_IRQ_DATA_ADDR				0x4
#define DFM_SYS_REG_CFG_0_ADDR					0x8
// BCPR registers
#define DFM_SYS_REG_BCPR_BASE_ADDR				0x40
#define DFM_BCPR_MAX_NUM						16
#define DFM_NUM_OF_REGISTERS_PER_BCPR  			2
#define DFM_SYS_BCPR_REG_ADDR					0x0
#define DFM_SYS_BCPR_REG_VALID_ADDR				0x4

// Building DFM address from DP id and address
#define DFM_DP_BASE_ADDR                  		0x0000
#define MAKE_DP_CFG_ADDR(dp_id, dp_addr) 		((1 << DFM_DP_CFG_SELECT_BIT_POS) | ((dp_id) << DFM_DP_CFG_OFFSET_SIZE) | (dp_addr))
#define MAKE_DP_CMD_ADDR(dp_id, dp_addr) 		(((dp_id) << DFM_DP_STATUS_OFFSET_SIZE) | (dp_addr))
#define MAKE_DP_STATUS_ADDR(dp_id, dp_addr) 	((1 << DFM_DP_STATUS_SELECT_BIT_POS) | ((dp_id) << DFM_DP_STATUS_OFFSET_SIZE) | (dp_addr))

// Building BCPR system-register address
#define DFM_SYS_REG_BASE_ADDR                   0x1E00
#define DFM_BCPR_OFFSET_SIZE					0x3
#define DFM_BCPR_BIT_MASK						((1 << DFM_BCPR_OFFSET_SIZE) - 1)
#define MAKE_BCPR_ADDR(bcpr_id, bcpr_addr)		(DFM_SYS_REG_BASE_ADDR |(DFM_SYS_REG_BCPR_BASE_ADDR + (((bcpr_id) << DFM_BCPR_OFFSET_SIZE) | (bcpr_addr))))

// Building DFM system-register address
#define MAKE_SYS_REG_ADDR(sys_reg_addr)			(DFM_SYS_REG_BASE_ADDR | (sys_reg_addr))

// Building DFM bank address
#define DFM_CMD_BANK_BASE_ADDR                  0x1000
#define MAKE_CMD_BANK_ADDR(sys_reg_addr)		(DFM_CMD_BANK_BASE_ADDR | (sys_reg_addr))

#endif /* DFM_DFM_ADDRESS_MAP_H_ */
