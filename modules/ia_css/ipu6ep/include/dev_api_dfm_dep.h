/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2020 Intel Corporation.
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

#ifndef __DEV_API_DFM_DEP_H
#define __DEV_API_DFM_DEP_H

/* Because of bug #H1805148309 we copied, modified and renamed the ipu_device_dfm_init_type_t enum
* of the IPU6 DFM NCI (from SDK).
*/
typedef enum {
	_DEV_API_IPU6_DFM_SEQ_INIT_NONE = 0,
	_DEV_API_IPU6_DFM_SEQ_INIT_TYPE_V2S_S2V = 1,
	_DEV_API_IPU6_DFM_SEQ_INIT_TYPE_S2MMIO = 2
} _dev_api_ipu6_dfm_init_type_t;

STORAGE_CLASS_INLINE unsigned dev_api_get_init_seq(enum dev_api_dfm_agent agent)
{
	unsigned init_seq = _DEV_API_IPU6_DFM_SEQ_INIT_NONE;

	if ((agent == DEV_API_DFM_AGENT_V2S)
		|| (agent == DEV_API_DFM_AGENT_S2V_ISL_BAYER)
		|| (agent == DEV_API_DFM_AGENT_S2V_ISL_YUV)
		|| (agent == DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED0)
#if HAS_FF_R2I_DS_B
		|| (agent == DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED1)
#endif
		|| (agent == DEV_API_DFM_AGENT_S2V_PSA_BAYER)
		|| (agent == DEV_API_DFM_AGENT_V2S_PSA_BAYER)
		|| (agent == DEV_API_DFM_AGENT_GAMMASTAR)
		|| (agent == DEV_API_DFM_AGENT_S2V_PSA_YUV)
		|| (agent == DEV_API_DFM_AGENT_LSC)
		|| (agent == DEV_API_DFM_AGENT_3A_AWB)
		|| (agent == DEV_API_DFM_AGENT_DVS_STATS)
		|| (agent == DEV_API_DFM_AGENT_3A_AF)
		|| (agent == DEV_API_DFM_AGENT_PDAF)) {
		init_seq = _DEV_API_IPU6_DFM_SEQ_INIT_TYPE_V2S_S2V;
	}

	if ((agent == DEV_API_DFM_AGENT_STR2MMIO_IS_0)
		|| (agent == DEV_API_DFM_AGENT_STR2MMIO_IS_1)
		|| (agent == DEV_API_DFM_AGENT_STR2MMIO_PS_0)
		|| (agent == DEV_API_DFM_AGENT_STR2MMIO_PS_1)
		|| (agent == DEV_API_DFM_AGENT_STR2MMIO_PS_IR_DEPTH)) {
		init_seq = _DEV_API_IPU6_DFM_SEQ_INIT_TYPE_S2MMIO;
	}
	return init_seq;
}

/* TODO: the below function needs cleanup. The complete address should be passed to device API*/
STORAGE_CLASS_INLINE uint32_t get_agent_address(unsigned dev_id, struct dev_api_dfm_agent_property *agent)
{
	uint32_t address;

	switch (agent->type) {
	case DEV_API_DFM_AGENT_SPC_EQ:
		address = LB_DFM_TO_SPC_EVQ_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_SPP0_EQ:
		address = LB_DFM_TO_SPP0_EVQ_ADDRESS + agent->offset;
		break;
#if HAS_ISP0
	case DEV_API_DFM_AGENT_VP_EQ:
		address = LB_DFM_TO_VP0_EVQ_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_VP_DMEM:
		address = agent->offset + dev_api_dfm_get_fabric_offset(dev_id);
		break;
#endif
	case DEV_API_DFM_AGENT_SPP0_DMEM:
		address = agent->offset + dev_api_dfm_get_fabric_offset(dev_id);
		break;
	case DEV_API_DFM_AGENT_DMA_LB:
	case DEV_API_DFM_AGENT_DMA_HBTX:
	case DEV_API_DFM_AGENT_DMA_HBFRX:
		address = agent->offset + dev_api_dfm_get_fabric_offset(dev_id);
		break;
	case DEV_API_DFM_AGENT_DMA_ISL:
		address = agent->offset + LBFF_DFM_TO_ISL_DMA_BASE_ADDRESS;
		break;
	case DEV_API_DFM_AGENT_S2V_ISL_BAYER:
		address = LBFF_DFM_TO_ISL_PS_S2V_BAYER_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_V2S_PSA_BAYER:
		address = LBFF_DFM_TO_PSA_V2S_1_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_STR2MMIO_PS_0:
		address = LBFF_DFM_TO_ISL_PS_SIS_STR2MMIO_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_STR2MMIO_PS_1:
		address = LBFF_DFM_TO_ISL_PS_SIS_B_STR2MMIO_BASE_ADDRESS  + agent->offset;
		break;
	case DEV_API_DFM_AGENT_STR2MMIO_PS_IR_DEPTH:
		address = LBFF_DFM_TO_ISL_PS_IR_DEPTH_STR2MMIO_BASE_ADDRESS  + agent->offset;
		break;
	case DEV_API_DFM_AGENT_STR2MMIO_IS_0:
		address = ISL_PS_DFM_TO_ISYS_STR2MMIO_IS_0_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_STR2MMIO_IS_1:
		address = ISL_PS_DFM_TO_ISYS_STR2MMIO_IS_1_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_S2V_ISL_YUV:
		address = ISL_PS_DFM_TO_ISL_PS_S2V_YUV_FULL_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED0:
		address = ISL_PS_DFM_TO_ISL_PS_S2V_YUV_SCALED0_BASE_ADDRESS + agent->offset;
		break;
#if HAS_FF_R2I_DS_B
	case DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED1:
		address = ISL_PS_DFM_TO_ISL_PS_S2V_YUV_SCALED1_BASE_ADDRESS + agent->offset;
		break;
#endif
	case DEV_API_DFM_AGENT_S2V_PSA_YUV:
		address = LBFF_DFM_TO_PSA_S2V_YUV_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_3A_AF:
		address = LBFF_DFM_TO_3A_AF_ACB_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_3A_AWB:
		address = LBFF_DFM_TO_3A_AWB_ACB_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_LSC:
		address = LBFF_DFM_TO_LSC_ACB_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_PDAF:
		address = LBFF_DFM_TO_PAF_ACB_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_GAMMASTAR:
		address = LBFF_DFM_TO_GAMMASTAR_ACB_BASE_ADDRESS + agent->offset;
		break;
	case DEV_API_DFM_AGENT_MBR:
	case DEV_API_DFM_AGENT_IBUF:
	case DEV_API_DFM_AGENT_OFS:
	case DEV_API_DFM_AGENT_GENERIC:
	case DEV_API_DFM_AGENT_GENERIC_CMD2:
	case DEV_API_DFM_AGENT_DFM_PORT:
		address = agent->offset;
		break;
	case DEV_API_DFM_AGENT_DVS_STATS:
		address = LBFF_DFM_TO_DVS_CTRLR_ACB_BASE_ADDRESS + agent->offset;
		break;
	default:
		address = 0;
		assert(0);
	}

	return address;
}

DEV_API_DFM_STORAGE_CLASS_C
uint32_t dev_api_dfm_get_fabric_offset(enum ipu_device_dfm_id dev_id)
{
	if (dev_id == IPU_DEVICE_DFM_LB) {
		return IPU_DEVICE_LINE_DFM_FABRIC_OFFSET;
	} else {
		return IPU_DEVICE_BLOCK_DFM_FABRIC_OFFSET;
	}
}

STORAGE_CLASS_INLINE ipu_device_dfm_seq_type dev_api_get_seq_type(enum dev_api_dfm_agent agent)
{
	ipu_device_dfm_seq_type seq_type;

	switch (agent) {
	case DEV_API_DFM_AGENT_DMA_LB:
	case DEV_API_DFM_AGENT_DMA_HBTX:
	case DEV_API_DFM_AGENT_DMA_HBFRX:
	case DEV_API_DFM_AGENT_DMA_ISL:
		seq_type = DFM_SEQ_TYPE_DMA_POLLING;
		break;
	case DEV_API_DFM_AGENT_VP_EQ:
	case DEV_API_DFM_AGENT_SPC_EQ:
	case DEV_API_DFM_AGENT_SPP0_EQ:
	case DEV_API_DFM_AGENT_SPP0_DMEM:
	case DEV_API_DFM_AGENT_DFM_PORT:
	case DEV_API_DFM_AGENT_S2V_ISL_BAYER:
	case DEV_API_DFM_AGENT_S2V_ISL_YUV:
	case DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED0:
#if HAS_FF_R2I_DS_B
	case DEV_API_DFM_AGENT_S2V_ISL_YUV_SCALED1:
#endif
	case DEV_API_DFM_AGENT_STR2MMIO_IS_0:
	case DEV_API_DFM_AGENT_STR2MMIO_IS_1:
	case DEV_API_DFM_AGENT_STR2MMIO_PS_0:
	case DEV_API_DFM_AGENT_STR2MMIO_PS_1:
	case DEV_API_DFM_AGENT_STR2MMIO_PS_IR_DEPTH:
	case DEV_API_DFM_AGENT_S2V_PSA_YUV:
	case DEV_API_DFM_AGENT_V2S_PSA_BAYER:
	case DEV_API_DFM_AGENT_MBR:
	case DEV_API_DFM_AGENT_IBUF:
	case DEV_API_DFM_AGENT_GENERIC:
		seq_type = DFM_SEQ_TYPE_GENERIC_CMD;
		break;
	case DEV_API_DFM_AGENT_3A_AF:
	case DEV_API_DFM_AGENT_3A_AWB:
	case DEV_API_DFM_AGENT_DVS_STATS:
	case DEV_API_DFM_AGENT_GAMMASTAR:
	case DEV_API_DFM_AGENT_LSC:
	case DEV_API_DFM_AGENT_PDAF:
		seq_type = DFM_SEQ_TYPE_FF_ACC;
		break;
	case DEV_API_DFM_AGENT_VP_DMEM:
		seq_type = DFM_SEQ_TYPE_BUFF_IDX_UPDATE;
		break;
	case DEV_API_DFM_AGENT_OFS:
		seq_type = DFM_SEQ_TYPE_OFS_SEQ;
		break;
	case DEV_API_DFM_AGENT_GENERIC_CMD2:
		seq_type = DFM_SEQ_TYPE_GENERIC_CMD2;
		break;
	default:
		assert(0);
		seq_type = DFM_SEQ_TYPE_GENERIC_CMD;
	}
	return seq_type;
}
#endif
