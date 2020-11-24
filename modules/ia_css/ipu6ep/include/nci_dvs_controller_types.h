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
#ifndef __NCI_DVS_CONTROLLER_TYPES_H
#define __NCI_DVS_CONTROLLER_TYPES_H

#include "type_support.h"

struct nci_dvs_controller_cfg {
	uint32_t ack_data_level0;
	uint32_t ack_addr_level0;
	uint32_t ack_data_level1;
	uint32_t ack_addr_level1;
	uint32_t ack_data_level2;
	uint32_t ack_addr_level2;
	uint32_t dvs_type;
	uint32_t acb_addr;
};

enum nci_dvs_controller_levels {
	NCI_DVS_CONTROLLER_LEVEL0,
	NCI_DVS_CONTROLLER_LEVEL1,
	NCI_DVS_CONTROLLER_LEVEL2,
	NCI_DVS_CONTROLLER_NOF_LEVELS
};

enum nci_dvs_controller_sections {
	NCI_DVS_CONTROLLER_SECTION0,
	NCI_DVS_CONTROLLER_SECTION1,
	NCI_DVS_CONTROLLER_SECTION2,
	NCI_DVS_CONTROLLER_SECTION3,
	NCI_DVS_CONTROLLER_NOF_SECTIONS
};

#define NCI_DVS_CONTROLLER_NOF_SECTIONS_PER_LEVEL (NCI_DVS_CONTROLLER_NOF_SECTIONS - NCI_DVS_CONTROLLER_NOF_LEVELS + 1)

#endif /* __NCI_DVS_CONTROLLER_TYPES_H */
