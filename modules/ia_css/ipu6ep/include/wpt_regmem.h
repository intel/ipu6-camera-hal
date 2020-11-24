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
#ifndef _wpt_regmem_h_
#define _wpt_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type wpt */
typedef enum {
	NUM_DAI_WPT_MEMORY_ID = 0
} dai_wpt_memory_id_t;

/* enumerate for all register bank types for devices of type wpt */
typedef enum {
	DAI_WPT_RB_ADDRESSES_FIX,
	DAI_WPT_RB_ADDRESSES_IDX,
	NUM_DAI_WPT_RB_TYPES     = 2
} dai_wpt_rb_types_t;

/* enumerate for all registers for register bank type addresses_fix for devices of type wpt */
typedef enum {
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_HEADER     = 0,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_ADDRESS    = 4,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_NPK_EN     = 8,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_DDR_EN     = 12,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_LPKT_CLEAR = 16,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_LPKT       = 20,
	DAI_WPT_REG_ADDRESSES_FIX_WPT_TRACE_LPKT_EN    = 24,
	NUM_DAI_WPT_ADDRESSES_FIX_REGISTERS            = 7
} dai_wpt_addresses_fix_registers_t;

/* enumerate for all registers for register bank type addresses_idx for devices of type wpt */
typedef enum {
	DAI_WPT_REG_ADDRESSES_IDX_WPT_CONFIG     = 0,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_LOWER_ADDR = 4,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_UPPER_ADDR = 8,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_WDATA_VAL  = 12,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_WDATA_MASK = 16,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_HIT_CNTR   = 20,
	DAI_WPT_REG_ADDRESSES_IDX_WPT_ENABLE     = 24,
	NUM_DAI_WPT_ADDRESSES_IDX_REGISTERS      = 7
} dai_wpt_addresses_idx_registers_t;

/* enumerate for all registers for slave port slv_obs for devices of type wpt */
typedef enum {
	NUM_DAI_WPT_SLV_OBS_REGISTERS = 0
} dai_wpt_slv_obs_registers_t;

/* enumerate for all registers for slave port slv_cfg for devices of type wpt */
typedef enum {
	NUM_DAI_WPT_SLV_CFG_REGISTERS = 0
} dai_wpt_slv_cfg_registers_t;

#endif /* _wpt_regmem_h_ */
