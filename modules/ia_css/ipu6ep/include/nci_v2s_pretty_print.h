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

#ifndef __NCI_V2S_PRETTY_PRINT_H
#define __NCI_V2S_PRETTY_PRINT_H

#include "nci_v2s.h"
#include "nci_v2s_trace.h"

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_print_section0(
	void *buffer);

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_print_section1(
	void *buffer);

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_print_section2(
	void *buffer);

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_print_section3(
	void *buffer);

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_print_section0(
	void *buffer)
{
	struct nci_v2s_private_cfg_section0 *priv = (struct nci_v2s_private_cfg_section0 *) buffer;
	uint16_t ack_cfg_ack_k_vec;
	uint8_t ack_cfg_ack_eol_en;
	uint32_t ack_msg_message;
	uint8_t ack_msg_cmd;
	uint8_t ack_msg_pid;
	uint8_t ack_msg_sid;
	uint16_t frame_dim_width;
	uint16_t frame_dim_height;

	assert(buffer != NULL);

	unpack_reg_v2s_ack_cfg(
				&ack_cfg_ack_k_vec,
				&ack_cfg_ack_eol_en,
				priv->ack_cfg);
	unpack_reg_v2s_ack_msg(
				&ack_msg_message,
				&ack_msg_cmd,
				&ack_msg_pid,
				&ack_msg_sid,
				priv->ack_msg);
	unpack_reg_v2s_frame_dim(
				&frame_dim_width,
				&frame_dim_height,
				priv->frame_dim);

	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "cmd_cfg_cmd_eol_end_en = %x\n", priv->cmd_cfg);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_cfg_ack_k_vec      = %x\n", ack_cfg_ack_k_vec);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_cfg_ack_eol_en     = %x\n", ack_cfg_ack_eol_en);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_msg_message        = %x\n", ack_msg_message);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_msg_cmd            = %x\n", ack_msg_cmd);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_msg_pid            = %x\n", ack_msg_pid);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_msg_sid            = %x\n", ack_msg_sid);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "ack_addr               = %x\n", priv->ack_addr);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "frame_dim_width        = %x\n", frame_dim_width);
	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "frame_dim_height       = %x\n", frame_dim_height);
}

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_print_section1(
	void *buffer)
{
	struct nci_v2s_private_cfg_section1 *priv = (struct nci_v2s_private_cfg_section1 *) buffer;

	NOT_USED(priv); /* if trace level < DEBUG the traces are removed by the preprocessor */

	assert(buffer != NULL);

	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "stride_cfg_en = %x\n", priv->stride_cfg);
}

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_print_section2(
	void *buffer)
{
	struct nci_v2s_private_cfg_section2 *priv = (struct nci_v2s_private_cfg_section2 *) buffer;
	int i = 0;

	assert(buffer != NULL);
	NOT_USED(priv); /* if trace level < DEBUG the traces are removed by the preprocessor */

	for (i = 0; i < V2S_NUM_BUFFERS; i++) {
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "Buffer configuration for buffer %x\n", i);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "buffer_en = %x\n", priv->buf_cfg[i].buffer_en);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "st_addr   = %x\n", priv->buf_cfg[i].st_addr);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "end_addr  = %x\n", priv->buf_cfg[i].end_addr);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "offset_0  = %x\n", priv->buf_cfg[i].offset_0);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "offset_1  = %x\n", priv->buf_cfg[i].offset_1);
		IA_CSS_TRACE_1(NCI_V2S, DEBUG, "stride    = %x\n", priv->buf_cfg[i].stride);
	}

}

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_print_section3(
	void *buffer)
{
	struct nci_v2s_private_cfg_section3 *priv = (struct nci_v2s_private_cfg_section3 *) buffer;

	NOT_USED(priv); /* if trace level < DEBUG the traces are removed by the preprocessor */

	assert(buffer != NULL);

	IA_CSS_TRACE_1(NCI_V2S, DEBUG, "auto_cfg = %x\n", priv->auto_cfg);
}

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_print_config(
	void *buffer)
{
	char *buffer_char = (char *)buffer;
	uint32_t size_sec0 = nci_v2s_get_sizeof_section(NCI_V2S_SECTION0);
	uint32_t size_sec1 = nci_v2s_get_sizeof_section(NCI_V2S_SECTION1);
	uint32_t size_sec2 = nci_v2s_get_sizeof_section(NCI_V2S_SECTION2);

	assert(buffer_char != NULL);

	IA_CSS_TRACE_0(NCI_V2S, DEBUG, "=== NCI_V2S configuration ===\n");

	nci_v2s_print_section0(buffer_char);
	buffer_char += size_sec0;
	nci_v2s_print_section1(buffer_char);
	buffer_char += size_sec1;
	nci_v2s_print_section2(buffer_char);
	buffer_char += size_sec2;
	nci_v2s_print_section3(buffer_char);
}

#endif /* __NCI_V2S_PRETTY_PRINT_H */
