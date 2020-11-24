/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __NCI_DFM_REG_PACK_UNPACK_H
#define __NCI_DFM_REG_PACK_UNPACK_H

#include "type_support.h"
#include "assert_support.h"
#include "storage_class.h"

#define DFM_DP_RATIO_CONV_EVENT_CMD_ALL_SIZE			32
#define DFM_DP_RATIO_CONV_SUSPEND_CMD_ALL_SIZE			32
#define DFM_DP_RATIO_CONV_CFG_THRESH_HOLD_SIZE			8
#define DFM_DP_RATIO_CONV_CFG_OUT_RATIO_SIZE			8
#define DFM_DP_RATIO_CONV_CFG_IGNORE_FIRST_EVENT_SIZE			1
#define DFM_DP_RATIO_CONV_EVENT_COMMAND_TYPE			1
#define DFM_DP_RATIO_CONV_CFG_1_PRIMING_VAL_SIZE			8
#define DFM_DP_RATIO_CONV_CFG_1_ITER_SIZE			24
#define DFM_DP_RATIO_CONV_STATUS_DRAINING_SIZE			8
#define DFM_DP_RATIO_CONV_STATUS_OUT_EVENT_SIZE			8
#define DFM_DP_RATIO_CONV_STATUS_UNIT_CNT_SIZE			16
#define DFM_DP_RATIO_CONV_EN_ALL_SIZE			1
#define DFM_DP_RATIO_CONV_BUFFER_CHASING_SDF_TYPE_SIZE			2
#define DFM_DP_RATIO_CONV_BUFFER_CHASING_FRAME_INDX_SIZE			8
#define DFM_DP_RATIO_CONV_BUFFER_CHASING_SDF_ALLOC_SIZE			4
#define DFM_DP_RATIO_CONV_BUFFER_CHASING_ITER2UNIT_SIZE			8
#define DFM_DP_GTH_MULT_EN_MASK_ALL_SIZE			32
#define DFM_DP_BUFF_CTRL_BEGIN_SEQ_VAL_SIZE			16
#define DFM_DP_BUFF_CTRL_MIDDLE_SEQ_VAL_SIZE			16
#define DFM_DP_BUFF_CTRL_END_SEQ_VAL_SIZE			8
#define DFM_DP_BUFF_CTRL_SEQ_TYPE_INIT_TYPE_SIZE			2
#define DFM_DP_BUFF_CTRL_SEQ_TYPE_BEGIN_TYPE_SIZE			3
#define DFM_DP_BUFF_CTRL_SEQ_TYPE_MIDDLE_TYPE_SIZE			3
#define DFM_DP_BUFF_CTRL_SEQ_TYPE_END_TYPE_SIZE			3
#define DFM_DP_BUFF_CTRL_DYNAMIC_VAL_WRAP_SIZE			8
#define DFM_DP_BUFF_CTRL_DYNAMIC_VAL_INC_SIZE			8
#define DFM_DP_BUFF_CTRL_DYNAMIC_VAL_INIT_SIZE			8
#define DFM_DP_BUFF_CTRL_EN_ALL_SIZE			1
#define DFM_SYS_REG_IRQ_ADDR_SIZE			16
#define DFM_SYS_REG_IRQ_CODE_SIZE			15
#define DFM_SYS_REG_IRQ_VALID_SIZE			1
#define DFM_SYS_REG_IRQ_DATA_ALL_SIZE			32
#define DFM_BCPR_REG_UNIT_INDX_SIZE			16
#define DFM_BCPR_REG_FRAME_INDX_SIZE			8
#define DFM_BCPR_REG_VALID_ALL_SIZE			1

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_ratio_conv_event_cmd(uint32_t all)
{
	return all;
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_event_cmd(uint32_t *all, uint32_t in_data)
{
	*all = in_data;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_ratio_conv_suspend_cmd(uint32_t all)
{
	return all;
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_suspend_cmd(uint32_t *all, uint32_t in_data)
{
	*all = in_data;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_ratio_conv_cfg_0(uint8_t thresh_hold, uint8_t out_ratio,
uint8_t ignore_first_event)
{
	assert(!(thresh_hold & ~0xFF) && "thresh_hold value exceeds 8 bits");
	assert(!(out_ratio & ~0xFF) && "out_ratio value exceeds 8 bits");
	assert(!(ignore_first_event & ~0x1) && "ignore_first_event value exceeds 1 bits");
	return ((thresh_hold & 0xFF) | ((out_ratio & 0xFF) << 8) | ((ignore_first_event & 0x1) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_cfg_0(uint8_t *thresh_hold, uint8_t *out_ratio,
uint8_t *ignore_first_event, uint32_t in_data)
{
	*thresh_hold = in_data & 0xFF;
	*out_ratio = (in_data >> 8) & 0xFF;
	*ignore_first_event = (in_data >> 16) & 0x1;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_ratio_conv_cfg_1(uint8_t priming_val, uint32_t iter)
{
	assert(!(priming_val & ~0xFF) && "priming_val value exceeds 8 bits");
	assert(!(iter & ~0xFFFFFF) && "iter value exceeds 24 bits");
	return ((( priming_val & 0xFFFFFF) << 24)| iter );
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_cfg_1(uint8_t *priming_val, uint32_t *iter, uint32_t in_data)
{
	*priming_val = (in_data >> 24) & 0xFF;
	*iter = in_data & 0xFFFFFF;
}

/* ratio_conv_cnt_status is a RO register hence only unpacking function is required */
STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_cnt_status(uint8_t *draining, uint8_t *out_event, uint16_t *unit_cnt,
uint32_t in_data)
{
	*draining = in_data & 0xFF;
	*out_event = (in_data >> 8) & 0xFF;
	*unit_cnt = (in_data >> 16) & 0xFFFF;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_ratio_conv_sdf_pr_cfg_0(uint8_t en, uint8_t frame_indx, uint8_t bcpr_alloc,
uint8_t iter2unit)
{
	assert(!(en & ~0x3) && "en value exceeds 2 bits");
	assert(!(frame_indx & ~0xFF) && "frame_indx value exceeds 8 bits");
	assert(!(bcpr_alloc & ~0xF) && "bcpr_alloc value exceeds 4 bits");
	assert(!(iter2unit & ~0xFF) && "iter2unit value exceeds 8 bits");
	return ((en & 0x3) | ((frame_indx & 0xFF) << 8) | ((bcpr_alloc & 0xF) << 16) | ((iter2unit & 0xFF) << 24));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_ratio_conv_sdf_pr_cfg_0(uint8_t *en, uint8_t *frame_indx, uint8_t *sdf_alloc,
uint8_t *iter2unit, uint32_t in_data)
{
	*en = in_data & 0x3;
	*frame_indx = (in_data >> 8) & 0xFF;
	*sdf_alloc = (in_data >> 16) & 0xF;
	*iter2unit = (in_data >> 24) & 0xFF;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_gth_mult_en_mask(uint32_t all)
{
	return all;
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_gth_mult_en_mask(uint32_t *all, uint32_t in_data)
{
	*all = in_data;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_buff_ctrl_begin_seq(uint16_t iter, uint8_t begin_type, uint8_t init_type)
{
	assert(!(iter & ~0xFFFF) && "iter exceeds 16 bits");
	assert(!(begin_type & ~0x7) && "begin_type value exceeds 3 bits");
	assert(!(init_type & ~0x3) && "init_type value exceeds 2 bits");
	return (iter | ((begin_type & 0x7) << 16) | ((init_type & 0x3) << 24));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_buff_ctrl_begin_seq(uint16_t *iter,
ipu_device_dfm_seq_type *begin_type, ipu_device_dfm_init_type_t *init_type, uint32_t in_data)
{
	*iter = in_data & 0xFFFF;
	*begin_type = (in_data >> 16) & 0x7;
	*init_type = (in_data >> 24) & 0x3;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_buff_ctrl_middle_seq(uint16_t iter, uint8_t middle_type)
{
	assert(!(iter & ~0xFFFF) && "iter exceeds 16 bits");
	assert(!(middle_type & ~0x7) && "middle_type value exceeds 3 bits");
	return (iter | ((middle_type & 0x7) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_buff_ctrl_middle_seq(uint16_t *iter,
ipu_device_dfm_seq_type *middle_type, uint32_t in_data)
{
	*iter = in_data & 0xFFFF;
	*middle_type = (in_data >> 16) & 0x7;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_buff_ctrl_end_seq(uint16_t iter, uint8_t end_type)
{
	assert(!(iter & ~0xFF) && "iter exceeds 16 bits");
	assert(!(end_type & ~0x7) && "end_type value exceeds 3 bits");
	return (iter | ((end_type & 0x7) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_buff_ctrl_end_seq(uint16_t *iter,
ipu_device_dfm_seq_type *end_type, uint32_t in_data)
{
	*iter = in_data & 0xFFFF;
	*end_type = (in_data >> 16) & 0x7;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_dp_buff_ctrl_dynamic_val(uint8_t wrap, uint8_t inc, uint8_t init)
{
	assert(!(wrap & ~0xFF) && "wrap value exceeds 8 bits");
	assert(!(inc & ~0xFF) && "inc value exceeds 8 bits");
	assert(!(init & ~0xFF) && "init value exceeds 8 bits");
	return ((wrap & 0xFF) | ((inc & 0xFF) << 8) | ((init & 0xFF) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_dp_buff_ctrl_dynamic_val(uint8_t *wrap, uint8_t *inc, uint8_t *init, uint32_t in_data)
{
	*wrap = in_data & 0xFF;
	*inc = (in_data >> 8) & 0xFF;
	*init = (in_data >> 16) & 0xFF;
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_sys_reg_irq(uint16_t *addr, uint16_t *code, uint8_t *valid, uint32_t in_data)
{
	*addr = in_data & 0xFFFF;
	*code = (in_data >> 16) & 0x7FFF;
	*valid = (in_data >> 31) & 0x1;
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_sys_reg_irq_data(uint32_t *all, uint32_t in_data)
{
	*all = in_data;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_sdf_reg(uint16_t unit_indx, uint8_t frame_indx)
{
	assert(!(unit_indx & ~0xFFFF) && "unit_indx value exceeds 16 bits");
	assert(!(frame_indx & ~0xFF) && "frame_indx value exceeds 8 bits");
	return ((unit_indx & 0xFFFF) | ((frame_indx & 0xFF) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_sdf_reg(uint16_t *unit_indx, uint8_t *frame_indx, uint32_t in_data)
{
	*unit_indx = in_data & 0xFFFF;
	*frame_indx = (in_data >> 16) & 0xFF;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_dfm_sdf_reg_valid(uint8_t all)
{
	assert(!(all & ~0x1) && "all value exceeds 1 bits");
	return ((all & 0x1));
}

STORAGE_CLASS_INLINE void unpack_reg_dfm_sdf_reg_valid(uint8_t *all, uint32_t in_data)
{
	*all = in_data & 0x1;
}

#endif /*__NCI_DFM_REG_PACK_UNPACK_H*/
