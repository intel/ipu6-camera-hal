/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __VIED_NCI_S2V_DEFS_H
#define __VIED_NCI_S2V_DEFS_H

#include "type_support.h"

/*
 * The StreamToVec device writes each of the color component vectors to a circular buffer in the VMEM/BAMEM
 * which is defined by a start address, and end address, offset and stride.
 * For instance for a Bayer type VecToStr there is a circular buffer defined for Gr, R, B, and Gb.
 * The offset is used to increment the address from one vector write to the following one.
 * The stride is used to increment the address when an end of line is reached.
 */

#define NCI_N_S2V_BUFFERS		6
#define NCI_S2V_CMD_REG_BIT_SHIFT	16

/* one pixel component per pixel in bayer */
#define NOF_PIXEL_COMPS_PER_PIXEL_BAYER	(1)

/************* Public Config *************/
/*
 * S2V has 2 sections with some space in between
 * Section types are private to nci and defined as
 * - nci_s2v_private_cfg     // section0
 * - nci_s2v_private_ack_cfg // section1
 */
enum nci_s2v_section_id {
	NCI_S2V_SECTION0 = 0,
	NCI_S2V_SECTION1,
	NCI_S2V_NUM_SECTIONS
};

struct nci_s2v_buf_s {
	uint32_t start_address;
	uint32_t end_address;
	uint32_t offset;
	uint32_t offset1;
	uint32_t stride;
};

struct nci_s2v_cfg {
	uint32_t ack_vec_nr;
	uint32_t pxlcmp_per_line;
	uint32_t lines_per_frame;
	uint32_t interleave_enable;
	uint32_t yuv420_enable;
	uint32_t bayer_mode_enable;
	uint32_t dev_null_enable;
	uint32_t rpwc_enable;
	uint32_t number_of_buffers;
	struct nci_s2v_buf_s *buffers;
	uint32_t buf_addr_byte_aligned; /** Unused in v2 */
	uint32_t stride_on_eof_cmd_enable; /** Unused in v2 */
	/* When set to 1, the frame width is defined by this register.
	* When set to 0, the frame width is defined by pxlcmp_line register.
	*/
	uint32_t frame_width_en; /** Only used in v4 */
	/* Frame width (in pixels).*/
	uint32_t frame_width; /** Only used in v4 */
	/*
	* uv_interleaved_en - Enable interleaving of UV components of the YUV420 stream
	* on one single vector. When this bit is set to 0 (regular operation mode) then
	* U and V components are sent on separate vectors to separate buffers.
	* When this bit is set to 1 (UV interleaving is on)
	* UV components are sent on the U vector in an interleaved fashion,
	* while the V vector is unused and not sent.
	*/
	uint32_t uv_interleaved_en; /** Only used in v4 */
};

struct nci_s2v_ack_cfg{
	uint32_t address;
	uint32_t ack_sid;
	uint32_t ack_pid;
	uint32_t ack_msg;
};
/********* End of Public Config **********/

#endif /* __VIED_NCI_S2V_DEFS_H */
