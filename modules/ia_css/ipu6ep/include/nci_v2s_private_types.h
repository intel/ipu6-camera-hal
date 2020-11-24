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

#ifndef __NCI_V2S_PRIVATE_TYPES_H
#define __NCI_V2S_PRIVATE_TYPES_H

#include "nci_v2s_types.h"
#include "type_support.h"

struct nci_v2s_private_cfg_section0 {
	uint32_t cmd_cfg;
	uint32_t ack_cfg;
	uint32_t ack_msg;
	uint32_t ack_addr;
	uint32_t frame_dim;
	struct nci_v2s_strm_dim strm_dim;
};

struct nci_v2s_private_cfg_section1 {
	uint32_t stride_cfg;
};

struct nci_v2s_private_cfg_section2 {
	struct nci_v2s_buf_cfg buf_cfg[V2S_NUM_BUFFERS];
	uint32_t comp_cfg[V2S_NUM_COMPONENTS];
};

/* NOTE: Auto CFG is the last section so that when this section
 * is written into it overrides manual configuration done by
 * the previous sections
 */
struct nci_v2s_private_cfg_section3 {
	uint32_t auto_cfg;
};

#endif /* __NCI_V2S_PRIVATE_TYPES_H */
