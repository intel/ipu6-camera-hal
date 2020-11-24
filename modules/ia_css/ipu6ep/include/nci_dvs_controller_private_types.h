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

#ifndef __NCI_DVS_CONTROLLER_PRIVATE_TYPES_H
#define __NCI_DVS_CONTROLLER_PRIVATE_TYPES_H

#include "nci_dvs_controller_types.h"
#include "type_support.h"

struct nci_dvs_controller_private_section_ack_cfg {
	uint32_t ack_addr;
	uint32_t ack_data;
};

struct nci_dvs_controller_private_section3 {
	uint32_t dvs_type;
	uint32_t acb_addr;
};

#endif /* __NCI_DVS_CONTROLLER_PRIVATE_TYPES_H */
