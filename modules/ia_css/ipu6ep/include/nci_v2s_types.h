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
#ifndef __NCI_V2S_TYPES_H
#define __NCI_V2S_TYPES_H

#include "type_support.h"

#define V2S_NUM_BUFFERS 6
#define V2S_NUM_COMPONENTS 12

enum nci_v2s_format {
	V2S_FORMAT_BAYER_4PPC,
	V2S_FORMAT_BAYER_INTERLEAVED_4PPC,
	V2S_FORMAT_YUV420_4PPC,
	V2S_FORMAT_RGB_4PPC,
	V2S_FORMAT_BAYER_ON_YUV420_4PPC,
	NUM_V2S_FORMAT = 5
};

struct nci_v2s_cmd {
	uint8_t cmd_fifo_tail_opcode;
	uint16_t cmd_fifo_tail_args;
};

struct nci_v2s_strm_dim {
	uint8_t strm_dim_h_incr;
	uint8_t strm_dim_v_incr;
};

struct nci_v2s_buf_cfg {
	uint32_t buffer_en;
	uint32_t st_addr;
	uint32_t end_addr;
	uint32_t offset_0;
	uint32_t offset_1;
	uint32_t stride;
};

struct nci_v2s_component_cfg {
	uint8_t en;
	uint8_t src_vec;
	uint8_t start_idx;
	uint8_t offset_idx;
};

struct nci_v2s_ack_cfg {
	uint16_t ack_cfg_ack_k_vec;
	uint8_t	ack_cfg_ack_eol_en;
	uint32_t ack_msg_message;
	uint8_t	ack_msg_cmd;
	uint8_t	ack_msg_pid;
	uint8_t	ack_msg_sid;
	uint32_t ack_addr;
};

struct nci_v2s_dev_cfg {
	uint8_t	stride_cfg_en;
	uint8_t	cmd_cfg_cmd_eol_end_en;
	uint16_t frame_dim_width;
	uint16_t frame_dim_height;
};

/* public */
struct nci_v2s_cfg {
	enum nci_v2s_format format;
	struct nci_v2s_dev_cfg dev_cfg;
	struct nci_v2s_ack_cfg ack_cfg;
	struct nci_v2s_buf_cfg buf_cfg[V2S_NUM_BUFFERS];
};

/* HSD 1304805686: Because of bug in BAYER, All patterns are manually configured
 * So section 3 is Removed as its NO longer used.
*/
enum nci_v2s_sections {
	NCI_V2S_SECTION0,
	NCI_V2S_SECTION1,
	NCI_V2S_SECTION2,
	NCI_V2S_NOF_SECTIONS
};

#endif /* __NCI_V2S_TYPES_H */
