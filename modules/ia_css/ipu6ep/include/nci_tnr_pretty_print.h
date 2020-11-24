/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __NCI_TNR_PRETTY_PRINT_H
#define __NCI_TNR_PRETTY_PRINT_H

#include "nci_tnr.h"
#include "nci_tnr_trace.h"

/* Forward declarations of pretty print config functions */
STORAGE_CLASS_INLINE
void nci_tnr_print_part_1(
	struct nci_tnr_cfg *cfg);

STORAGE_CLASS_INLINE
void nci_tnr_print_part_2(
	struct nci_tnr_cfg *cfg);

STORAGE_CLASS_INLINE
void nci_tnr_print_part_3(
	struct nci_tnr_cfg *cfg);

STORAGE_CLASS_INLINE
void nci_tnr_print_part_1(struct nci_tnr_cfg *cfg)
{
	/* Input buffer Ratio Converter */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_ratio_converter = %x\n", cfg->ip_buff_ratio_converter);

	/* Input buffer */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg cmd_addr = %x\n", cfg->ip_buff_cfg.buff_cfg.cmd_addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg token = %x\n", cfg->ip_buff_cfg.buff_cfg.token);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg num_buf = %x\n", cfg->ip_buff_cfg.buff_cfg.num_buf);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_y addr = %x\n", cfg->ip_buff_cfg.buff_y.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_y stride = %x\n", cfg->ip_buff_cfg.buff_y.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_y line_stride = %x\n", cfg->ip_buff_cfg.buff_y.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_u addr = %x\n", cfg->ip_buff_cfg.buff_u.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_u stride = %x\n", cfg->ip_buff_cfg.buff_u.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_u line_stride = %x\n", cfg->ip_buff_cfg.buff_u.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_buff_cfg buff_v_addr = %x\n", cfg->ip_buff_cfg.buff_v_addr);

	/* Output buffer */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg cmd_addr = %x\n", cfg->op_buff_cfg.buff_cfg.cmd_addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg token = %x\n",    cfg->op_buff_cfg.buff_cfg.token);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg num_buf = %x\n",  cfg->op_buff_cfg.buff_cfg.num_buf);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_y addr = %x\n",   cfg->op_buff_cfg.buff_y.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_y stride = %x\n", cfg->op_buff_cfg.buff_y.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_y line_stride = %x\n", cfg->op_buff_cfg.buff_y.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_u addr = %x\n", cfg->op_buff_cfg.buff_u.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_u stride = %x\n", cfg->op_buff_cfg.buff_u.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_u line_stride = %x\n", cfg->op_buff_cfg.buff_u.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_buff_cfg buff_v_addr = %x\n", cfg->op_buff_cfg.buff_v_addr);

	/* Top-left/bottom-lieft tetragon point address */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_mbr_TL_grid_start_addr = %x\n", cfg->ip_ref_mbr_TL_grid_start_addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_mbr_BL_grid_start_addr = %x\n", cfg->ip_ref_mbr_BL_grid_start_addr);

	/* MBR command address */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_mbr_cmd_addr = %x\n", cfg->ip_ref_mbr_cmd_addr);

	(void) cfg;
}

STORAGE_CLASS_INLINE
void nci_tnr_print_part_2(struct nci_tnr_cfg *cfg)
{
	/* Input reference buffer */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg num_buf = %x\n", cfg->ip_ref_buf_cfg.num_buf);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg y addr = %x\n", cfg->ip_ref_buf_cfg.y.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg y stride = %x\n", cfg->ip_ref_buf_cfg.y.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg y line_stride = %x\n", cfg->ip_ref_buf_cfg.y.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg uv addr = %x\n", cfg->ip_ref_buf_cfg.uv.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg uv stride = %x\n", cfg->ip_ref_buf_cfg.uv.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg uv line_stride = %x\n", cfg->ip_ref_buf_cfg.uv.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg recsim addr = %x\n", cfg->ip_ref_buf_cfg.recsim.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg recsim stride = %x\n", cfg->ip_ref_buf_cfg.recsim.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "ip_ref_buf_cfg recsim line_stride = %x\n", cfg->ip_ref_buf_cfg.recsim.line_stride);

	/* DMA parameters */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "dma_channel_descr_id = %x\n", cfg->dma_channel_descr_id);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "dma_req_queue_size = %x\n", cfg->dma_req_queue_size);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "cmd_addr = %x\n", cfg->cmd_addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "cmd_slim_next = %x\n", cfg->cmd_slim_next);

	/* Output reference buffer */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg num_buf = %x\n", cfg->op_ref_buf_cfg.num_buf);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg y addr = %x\n", cfg->op_ref_buf_cfg.y.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg y stride = %x\n", cfg->op_ref_buf_cfg.y.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg y line_stride = %x\n", cfg->op_ref_buf_cfg.y.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg uv addr = %x\n", cfg->op_ref_buf_cfg.uv.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg uv stride = %x\n", cfg->op_ref_buf_cfg.uv.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg uv line_stride = %x\n", cfg->op_ref_buf_cfg.uv.line_stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg recsim addr = %x\n", cfg->op_ref_buf_cfg.recsim.addr);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg recsim stride = %x\n", cfg->op_ref_buf_cfg.recsim.stride);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "op_ref_buf_cfg recsim line_stride = %x\n", cfg->op_ref_buf_cfg.recsim.line_stride);

	(void) cfg;
}

STORAGE_CLASS_INLINE
void nci_tnr_print_part_3(struct nci_tnr_cfg *cfg)
{
	/* RPWC */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "read_post_write_check = %x\n", cfg->read_post_write_check);

	/* Dimension related register */
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "fragment ack_address = %x\n", cfg->fragment.ack_address);
	IA_CSS_TRACE_1(NCI_TNR, DEBUG, "fragment ack_data = %x\n", cfg->fragment.ack_data);

	(void) cfg;

}

NCI_TNR_STORAGE_CLASS_C
void nci_tnr_print_config(
	void *buffer)
{
	assert(buffer != NULL);

	IA_CSS_TRACE_0(NCI_TNR, DEBUG, "nci_tnr_print_config\n");
	struct nci_tnr_cfg *cfg =(struct nci_tnr_cfg *)buffer;

	nci_tnr_print_part_1(cfg);
	nci_tnr_print_part_2(cfg);
	nci_tnr_print_part_3(cfg);

	(void) buffer;
}

#endif /* __NCI_TNR_PRETTY_PRINT_H */
