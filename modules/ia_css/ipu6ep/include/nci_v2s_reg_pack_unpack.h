/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2017 Intel Corporation.
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
#ifndef __NCI_V2S_REG_PACK_UNPACK_H
#define __NCI_V2S_REG_PACK_UNPACK_H

#include "type_support.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_ack_cfg(uint16_t ack_k_vec, uint8_t ack_eol_en);
STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_ack_msg(uint32_t message, uint8_t cmd, uint8_t pid, uint8_t sid);
STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_frame_dim(uint16_t f_width, uint16_t f_height);
STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_cmd_fifo_tail(uint8_t opcode, uint16_t reserved, uint16_t args);
STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_strm_dim(uint8_t h_incr, uint8_t v_incr);
STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_comp_cfg(uint8_t en, uint8_t src_vec, uint8_t start_idx, uint8_t offset_idx);

STORAGE_CLASS_INLINE void unpack_reg_v2s_ack_cfg(uint16_t *ack_k_vec, uint8_t *ack_eol_en, uint32_t in_data);
STORAGE_CLASS_INLINE void unpack_reg_v2s_ack_msg(uint32_t *message, uint8_t *cmd,
		uint8_t *pid, uint8_t *sid, uint32_t in_data);
STORAGE_CLASS_INLINE void unpack_reg_v2s_frame_dim(uint16_t *f_width, uint16_t *f_height, uint32_t in_data);

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_ack_cfg(uint16_t ack_k_vec, uint8_t ack_eol_en)
{
	return ((ack_k_vec & 0xFFFF) | ((ack_eol_en & 0x1) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_v2s_ack_cfg(uint16_t *ack_k_vec, uint8_t *ack_eol_en, uint32_t in_data)
{
	*ack_k_vec = in_data & 0xFFFF;
	*ack_eol_en = (in_data >> 16) & 0x1;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_ack_msg(uint32_t message, uint8_t cmd, uint8_t pid, uint8_t sid)
{
	return ((message & 0x3FFFF) | ((cmd & 0x3) << 18) | ((pid & 0x3F) << 20) | ((sid & 0x3F) << 26));
}

STORAGE_CLASS_INLINE void unpack_reg_v2s_ack_msg(uint32_t *message, uint8_t *cmd,
		uint8_t *pid, uint8_t *sid, uint32_t in_data)
{
	*message = in_data & 0x3FFFF;
	*cmd = (in_data >> 18) & 0x3;
	*pid = (in_data >> 20) & 0x3F;
	*sid = (in_data >> 26) & 0x3F;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_frame_dim(uint16_t f_width, uint16_t f_height)
{
	return ((f_width & 0xFFFF) | ((f_height & 0xFFFF) << 16));
}

STORAGE_CLASS_INLINE void unpack_reg_v2s_frame_dim(uint16_t *f_width, uint16_t *f_height, uint32_t in_data)
{
	*f_width = in_data & 0xFFFF;
	*f_height = (in_data >> 16) & 0xFFFF;
}

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_cmd_fifo_tail(uint8_t opcode, uint16_t reserved, uint16_t args)
{
	return ((opcode & 0x1F) | ((reserved & 0x7FF) << 5) | ((args & 0xFFFF) << 16));
}

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_strm_dim(uint8_t h_incr, uint8_t v_incr)
{
	return ((h_incr & 0xFF) | ((v_incr & 0xFF) << 8));
}

STORAGE_CLASS_INLINE uint32_t pack_reg_v2s_comp_cfg(uint8_t en, uint8_t src_vec, uint8_t start_idx, uint8_t offset_idx)
{
	return ((en & 0x1) | ((src_vec & 0xF) << 4) | ((start_idx & 0x1) << 8) | ((offset_idx & 0x7) << 12));
}

#endif /* __NCI_V2S_REG_PACK_UNPACK_H */
