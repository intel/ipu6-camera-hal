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

#ifndef __VIED_NCI_ACB_PRIVATE_TYPES_H
#define __VIED_NCI_ACB_PRIVATE_TYPES_H

struct nci_acb_private_cfg {
	uint32_t base_ctrl;
	uint16_t frame_width;
	uint16_t frame_height;
	uint32_t scalefactor; /* both scale mult [3:0] and scale nf [7:4] are encoded into this variable*/
};

struct nci_acb_private_ack_cfg {
	uint32_t ack_data; /* msg[18:0] cmd[19] pid[25:20] sid[31:26] */
	uint32_t ack_addr;
};

#endif /* __VIED_NCI_ACB_PRIVATE_TYPES_H */
