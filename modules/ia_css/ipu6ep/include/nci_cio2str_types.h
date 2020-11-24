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
#ifndef __NCI_CIO2STR_TYPES_H
#define __NCI_CIO2STR_TYPES_H

#include "type_support.h"

struct nci_cio2str_port_config {
	uint32_t sl_addr_mask;
	uint32_t sl_addr_cmpv;
};

/* public */
struct nci_cio2str_cfg {
	struct nci_cio2str_port_config port0;
	struct nci_cio2str_port_config port1;
};

enum nci_cio2str_sections {
	NCI_CIO2STR_SECTION0,
	NCI_CIO2STR_SECTION1,
	NCI_CIO2STR_NOF_SECTIONS
};

#endif /* __NCI_CIO2STR_TYPES_H */
