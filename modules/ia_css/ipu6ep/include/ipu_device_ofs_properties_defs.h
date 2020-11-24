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
/* Generated file - please do not edit. */

#ifndef _IPU_DEVICE_OFS_PROPERTIES_DEFS_H_
#define _IPU_DEVICE_OFS_PROPERTIES_DEFS_H_
#define IPU_DEVICE_OFS_OF_BASE_ADDR 0x32F9000
#define IPU_DEVICE_OFS_GP_REG_SIZE 4
#define IPU_DEVICE_OFS_DATABUS_BUFFER1_BASE_ADDRESS 0x32D4000
#define IPU_DEVICE_OFS_DATABUS_BUFFER0_0_BASE_ADDRESS 0x32B0000
#define IPU_DEVICE_OFS_DATABUS_BUFFER0_1_BASE_ADDRESS 0x32C0000
#define IPU_DEVICE_OFS_DATABUS_BUFFER1_FROM_DMA_BASE_ADDR _hrt_master_to_slave_address_ipu_dma_top_dma_hbtx_m0_to_ipu_ofs_top_buffer0_mem_ip0
#define IPU_DEVICE_OFS_DATABUS_INPUT_BASE_ADDRESS IPU_DEVICE_OFS_DATABUS_BUFFER0_BASE_ADDRESS
#define IPU_DEVICE_OFS_FABRIC_OFFSET _hrt_master_to_slave_address_ipu_ofs_top_eqc_mp0_to_ipu_bb_ff_fabric_top_ciopipe_cmd_mt_cio_fifo_sl
#define IPU_DEVICE_OFS_SC0_BASE_ADDR 0x32F8000
#define IPU_DEVICE_OFS_SC0_ELEMENTS_PER_VECTOR 0x10
#define IPU_DEVICE_OFS_SC0_NUM_POLYPHASE_TAPS 0x4
#define IPU_DEVICE_OFS_SC0_NUM_PHASES 0x20
#define IPU_DEVICE_OFS_SC0_INPUT_DATA_BPP 0x8
#define IPU_DEVICE_OFS_SC0_OUTPUT_DATA_BPP 0x8
#define IPU_DEVICE_OFS_SC0_INTER_DATA_BPP 0xA
#define IPU_DEVICE_OFS_SC0_MAX_BITS_PER_COEF 0x8
#define IPU_DEVICE_OFS_SC0_OF_TO_SCALER_REL_MSG_Q_ADDR 0x32F8000
#define IPU_DEVICE_OFS_SC0_SCALER_TO_OF_RDY_MSG_Q_ADDR 0x32F9000
#define IPU_DEVICE_OFS_SC1_BASE_ADDR 0x32F8800
#define IPU_DEVICE_OFS_SC1_ELEMENTS_PER_VECTOR 0x10
#define IPU_DEVICE_OFS_SC1_NUM_POLYPHASE_TAPS 0x4
#define IPU_DEVICE_OFS_SC1_NUM_PHASES 0x20
#define IPU_DEVICE_OFS_SC1_INPUT_DATA_BPP 0x8
#define IPU_DEVICE_OFS_SC1_OUTPUT_DATA_BPP 0x8
#define IPU_DEVICE_OFS_SC1_INTER_DATA_BPP 0xA
#define IPU_DEVICE_OFS_SC1_MAX_BITS_PER_COEF 0x8
#define IPU_DEVICE_OFS_SC1_OF_TO_SCALER_REL_MSG_Q_ADDR 0x32F8800
#define IPU_DEVICE_OFS_SC1_SCALER_TO_OF_RDY_MSG_Q_ADDR 0x32F9000
#define IPU_DEVICE_GP_OFS_OUT_CONV_BASE_ADDR 0x32FA800
#define IPU_DEVICE_GP_OFS_OUT_CONV_SOFT_RESET_ADDR (IPU_DEVICE_GP_OFS_OUT_CONV_BASE_ADDR + 0 * IPU_DEVICE_OFS_GP_REG_SIZE)
#define IPU_DEVICE_GP_OFS_OUT_CONV_BYPASS_ADDR (IPU_DEVICE_GP_OFS_OUT_CONV_BASE_ADDR + 1 * IPU_DEVICE_OFS_GP_REG_SIZE)
#endif /* _IPU_DEVICE_OFS_PROPERTIES_DEFS_H_ */

