/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_DFM_DEVICES_H
#define __IPU_DEVICE_DFM_DEVICES_H

#include "dai_ipu_top_devices.h"

enum ipu_device_dfm_id {
	IPU_DEVICE_DFM_BB          = DAI_IPU_TOP_BB_FF_FABRIC_TOP_I_DFM,
	IPU_DEVICE_DFM_ISL         = DAI_IPU_TOP_ISL_CTRL_PS_TOP_I_ISL_PS_DFM,
	IPU_DEVICE_DFM_LB          = DAI_IPU_TOP_LBFF_INFRA_TOP_LBFF_INFRA_I_LBFF_DFM,
	IPU_DEVICE_DFM_NUM_DEVICES = NUM_DAI_IPU_TOP_DFM_INSTANCES
};

#endif /* __IPU_DEVICE_DFM_DEVICES_H */
