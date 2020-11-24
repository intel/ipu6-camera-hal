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
#ifndef _device_types_h_
#define _device_types_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all device types */
typedef enum {
	DAI_TYPE_BB_OUTPUT_FORMATTER,
	DAI_TYPE_BBGDC4,
	DAI_TYPE_BBPOLYPHASE_SCALER,
	DAI_TYPE_BBTNR,
	DAI_TYPE_CELL,
	DAI_TYPE_DFM,
	DAI_TYPE_DVS_CONTROLLER,
	DAI_TYPE_ENTRY,
	DAI_TYPE_ISA_PS_GP_REGS_BLOCK,
	DAI_TYPE_ISF_CIO_SWITCH_INFO,
	DAI_TYPE_ISF_CIO_SWITCH_VC2_INFO,
	DAI_TYPE_LBFF_INFRA_GP_REGS_BLOCK,
	DAI_TYPE_MBR,
	DAI_TYPE_PSA_1_GP_REGS_BLOCK,
	DAI_TYPE_PSA_2_GP_REGS_BLOCK,
	DAI_TYPE_PSA_3_GP_REGS_BLOCK,
	DAI_TYPE_TR_CIO_SWITCH_INFO,
	DAI_TYPE_VEC_TO_STR_V3,
	DAI_TYPE_WPT,
	NUM_DAI_DEVICE_TYPES              = 19
} dai_device_types_t;

#endif /* _device_types_h_ */
