/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2017 Intel Corporation.
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

#ifndef VIED_NCI_ACB_ROUTE_TYPE_H_
#define VIED_NCI_ACB_ROUTE_TYPE_H_

#include "type_support.h"

typedef enum {
	NCI_ACB_PORT_ISP = 0,
	NCI_ACB_PORT_ACC = 1,
	NCI_ACB_PORT_INVALID = 0xFF
} nci_acb_port_t;

typedef struct {
	/* 0 = ISP, 1 = Acc */
	nci_acb_port_t in_select;
	/* 0 = ISP, 1 = Acc */
	nci_acb_port_t out_select;
	/* When set, Ack will be sent only when Eof arrives */
	uint32_t ignore_line_num;
	/* Fork adapter to enable streaming to both output
	 * (next acb out and isp out)
	 */
	uint32_t fork_acb_output;
} nci_acb_route_t;

#endif /* VIED_NCI_ACB_ROUTE_TYPE_H_ */
