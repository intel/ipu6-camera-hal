/**
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

#ifndef __VIED_NCI_S2V_PRIVATE_TYPES_H
#define __VIED_NCI_S2V_PRIVATE_TYPES_H

#include "vied_nci_s2v_types.h"

/************ Private Config *************/
/* Refer to ipu_system_reg_addr_map.txt from the SDK */
struct nci_s2v_private_cfg {
	uint32_t ack_k_vec;
	uint32_t pxlcmp_line;
	uint32_t lines_frame;
	uint32_t yuv420_en;
	uint32_t interleave_en;
	uint32_t dev_null_enable;
	uint32_t unused_0;					/* Read-only register : irq_false_cmd_val	*/
	struct nci_s2v_buf_s buff_cfg[NCI_N_S2V_BUFFERS];
	uint32_t unused_1[NCI_N_S2V_BUFFERS];			/* Read-only registers: buff_<N>_sts_cur_addr	*/
	uint32_t unused_2;					/* Read-only register : pxls_cur_line		*/
	uint32_t unused_3;					/* Read-only register : lines_done		*/
	uint32_t unused_4;					/* Read-only register : io_status		*/
	uint32_t unused_5;					/* Read-only register : ack_status		*/
	uint32_t unused_6;					/* Read-only register : main_status		*/
	uint32_t unused_7;					/* Read-only register : tracker_status		*/
	uint32_t unused_8;					/* Skipped (used as crun(?) in simulator)	*/
	uint32_t ack_rpwc_en;
	uint32_t buf_addr_byte_aligned;
	uint32_t frame_width_en;
	uint32_t frame_width;
	uint32_t stride_on_eof_cmd_en;
	uint32_t bayer_mode_en;
	uint32_t uv_interleaved_en;
};

struct nci_s2v_private_ack_cfg {
	uint32_t ack_cmd_info;		/* SID, PID, MSG packed into one 32-bit register */
	uint32_t address;
};
/******** End of Private Config **********/

#endif /* __VIED_NCI_S2V_PRIVATE_TYPES_H */
