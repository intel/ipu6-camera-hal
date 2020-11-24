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

#ifndef NCI_DFM_PRIVATE_TYPES_H_
#define NCI_DFM_PRIVATE_TYPES_H_

#include "type_support.h"
#include "nci_dfm_private_types_dep.h"

#define NCI_DFM_DESC_SEQ_MAX_NUM_CMDS	(3) /* Max number of commands per sequence */

struct nci_dfm_private_cmdbank_config {
	uint32_t begin_addr;
	uint32_t begin_cmd[NCI_DFM_DESC_SEQ_MAX_NUM_CMDS];
	uint32_t middle_addr;
	uint32_t middle_cmd[NCI_DFM_DESC_SEQ_MAX_NUM_CMDS];
	uint32_t end_addr;
	uint32_t end_cmd[NCI_DFM_DESC_SEQ_MAX_NUM_CMDS];
	uint32_t ack_done_addr;
	uint32_t ack_done_cmd;
};

#endif /* NCI_DFM_PRIVATE_TYPES_H_ */
