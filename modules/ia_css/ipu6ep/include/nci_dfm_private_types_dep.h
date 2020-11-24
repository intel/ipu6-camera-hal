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

#ifndef NCI_DFM_PRIVATE_TYPES_DEP_H_
#define NCI_DFM_PRIVATE_TYPES_DEP_H_

#include "type_support.h"

struct nci_dfm_private_dev_port_config {
	uint32_t ratio_conv_cfg_0;
	uint32_t ratio_conv_cfg_1;
	uint32_t ratio_conv_sdf_pr_cfg_0;
	uint32_t gthr_mult_en_mask;
	uint32_t buff_ctrl_dynamic_val;
	uint32_t buff_ctrl_begin_seq;
	uint32_t buff_ctrl_middle_seq;
	uint32_t buff_ctrl_end_seq;
};

#endif /* NCI_DFM_PRIVATE_TYPES_DEP_H_ */
