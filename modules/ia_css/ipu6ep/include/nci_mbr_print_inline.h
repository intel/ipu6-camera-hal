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

#ifndef __NCI_MBR_PRINT_INLINE_H
#define __NCI_MBR_PRINT_INLINE_H

#include "nci_mbr_trace.h"
#include "nci_mbr_print.h"

NCI_MBR_STORAGE_CLASS_C void
nci_mbr_print_global0_cfg(const struct nci_mbr_global0_cfg *cfg)
{
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t activate_command                   = 0x%08X\n", cfg->activate_command);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t fragment_ack_addr                  = 0x%08X\n", cfg->fragment_ack.ack_address);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t fragment_ack_data                  = 0x%08X\n", cfg->fragment_ack.ack_data);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t general_cntl_register              = 0x%08X\n", cfg->general_cntl_register);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t gdc_stride_reg_offset              = 0x%08X\n", cfg->gdc_stride_reg_offset);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t gdc_tnr_cmd_addr                   = 0x%08X\n", cfg->gdc_tnr_cmd_addr);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t frame_init                         = 0x%08X\n", cfg->frame_init);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t tetragon_array_stride              = 0x%08x\n", cfg->tetragon_array_stride);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_lb_channel_id                  = %10d\n", cfg->dma_lb_channel_id);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_lb_req_queue_size              = %10d\n", cfg->dma_lb_req_queue_size);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_lb_req_queue_addr              = 0x%08x\n", cfg->dma_lb_req_queue_addr);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_lb_inst_format                 = 0x%08x\n", cfg->dma_lb_inst_format);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t ext_dma_inst_format                = 0x%08x\n", cfg->ext_dma_inst_format);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t int_dma_inst_format                = 0x%08x\n", cfg->int_dma_inst_format);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t start_blockID_X                    = %10d\n", cfg->start_blockID_X);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t start_blockID_Y                    = %10d\n", cfg->start_blockID_Y);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t output_block_width                 = %10d\n", cfg->op_block_dim.width);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t output_block_height                = %10d\n", cfg->op_block_dim.height);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t output_fragment_width              = %10d\n", cfg->op_fragment_dim.width);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t output_fragment_width              = %10d\n", cfg->op_fragment_dim.height);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_hb_req_queue_size              = %10d\n", cfg->dma_hb_req_queue_size);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t dma_int_req_queue_size             = %10d\n", cfg->dma_int_req_queue_size);

	(void) cfg;
}

NCI_MBR_STORAGE_CLASS_C void
nci_mbr_print_global1_cfg(const struct nci_mbr_global1_cfg *cfg)
{
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t input_image_width                  = %10d\n", cfg->input_image.width);
	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "\t input_image_height                 = %10d\n", cfg->input_image.height);

	(void) cfg;
}

NCI_MBR_STORAGE_CLASS_C void
nci_mbr_print_channel_cfg(const struct nci_mbr_channel_cfg *cfg, int ch)
{
	cfg = &cfg[ch];

	IA_CSS_TRACE_1(NCI_MBR, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_unit_desc_id           = %10d\n", ch, cfg->dma_hb.unit_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_channel_desc_id        = %10d\n", ch, cfg->dma_hb.channel_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_spanA_desc_id          = %10d\n", ch, cfg->dma_hb.spanA_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_spanB_desc_id          = %10d\n", ch, cfg->dma_hb.spanB_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_terminalA_desc_id      = %10d\n", ch, cfg->dma_hb.terminalA_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_hb_terminalB_desc_id      = %10d\n", ch, cfg->dma_hb.terminalB_desc_id);

	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_unit_desc_addr            = 0x%08x\n", ch, cfg->dma_unit_desc_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_span_desc_addr            = 0x%08x\n", ch, cfg->dma_span_desc_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_terminal_desc_addr        = 0x%08x\n", ch, cfg->dma_terminal_desc_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_terminal_desc_buf1_addr   = 0x%08x\n", ch, cfg->dma_terminal_desc_buf1_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_instruction_req_addr      = 0x%08x\n", ch, cfg->dma_instruction_req_addr);

	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_unit_desc_id          = %10d\n", ch, cfg->dma_int.unit_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_channel_desc_id       = %10d\n", ch, cfg->dma_int.channel_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_spanA_desc_id         = %10d\n", ch, cfg->dma_int.spanA_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_spanB_desc_id         = %10d\n", ch, cfg->dma_int.spanB_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_terminalA_desc_id     = %10d\n", ch, cfg->dma_int.terminalA_desc_id);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_terminalB_desc_id     = %10d\n", ch, cfg->dma_int.terminalB_desc_id);

	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_ch_ack_mode_addr          = 0x%08x\n", ch, cfg->dma_ch_ack_mode_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_unit_width_addr       = 0x%08x\n", ch, cfg->dma_int_unit_width_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_unit_height_addr      = 0x%08x\n", ch, cfg->dma_int_unit_height_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_unit_loc_src_addr     = 0x%08x\n", ch, cfg->dma_int_unit_loc_src_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_unit_loc_dst_addr     = 0x%08x\n", ch, cfg->dma_int_unit_loc_dst_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_terminal_a_descr_addr = 0x%08x\n", ch, cfg->dma_int_terminal_a_descr_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d dma_int_terminal_b_descr_addr = 0x%08x\n", ch, cfg->dma_int_terminal_b_descr_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d int_dma_instruction_req_addr  = 0x%08x\n", ch, cfg->int_dma_instruction_req_addr);

	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d ip_num_buf                    = 0x%08x\n", ch, cfg->ip_buf.num_buf);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d ip_buf_addr                   = 0x%08x\n", ch, cfg->ip_buf.buf.addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d ip_buf_line_stride            = 0x%08x\n", ch, cfg->ip_buf.buf.line_stride);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d ip_buf_stride                 = 0x%08x\n", ch, cfg->ip_buf.buf.stride);

	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d data_ack_addr                 = 0x%08x\n", ch, cfg->data_ack_addr);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d data_ack_message              = 0x%08x\n", ch, cfg->data_ack_message);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d top_left_offset               = %10d\n", ch, cfg->top_left_offset);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d cmd                           = 0x%08x\n", ch, cfg->cmd);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d block_erosion                 = %10d\n", ch, cfg->block_erosion);
	IA_CSS_TRACE_2(NCI_MBR, DEBUG, "\t ch%d coord_ratio                   = 0x%08x\n", ch, cfg->coord_ratio);

	(void) cfg;
	(void) ch;
}

#endif /* __NCI_MBR_PRINT_INLINE_H */
