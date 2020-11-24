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

#ifndef __NCI_GDC_PRINT_INLINE_H
#define __NCI_GDC_PRINT_INLINE_H

#include "nci_gdc_trace.h"
#include "nci_gdc_print.h"

NCI_GDC_STORAGE_CLASS_C void
nci_gdc_print_global_cfg(const struct gdc_global_cfg *cfg)
{
	IA_CSS_TRACE_1(NCI_GDC, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_1(NCI_GDC, DEBUG, "\t input_image_width           = %10d\n", cfg->input_image.width);
	IA_CSS_TRACE_1(NCI_GDC, DEBUG, "\t input_image_height          = %10d\n", cfg->input_image.height);

	(void) cfg;

}

NCI_GDC_STORAGE_CLASS_C void
nci_gdc_print_channel_sectionA_cfg(const struct gdc_sectionA_cfg *cfg, int ch)
{
	IA_CSS_TRACE_1(NCI_GDC, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d bpp_control            = 0x%08X\n", ch, cfg->bpp_control);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d oxdim                  = %10d\n", ch, cfg->output_dim.width);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d oydim                  = %10d\n", ch, cfg->output_dim.height);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d ixdim (deprecated)     = %10d\n", ch, cfg->input_dim.width);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d iydim (deprecated)     = %10d\n", ch, cfg->input_dim.height);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d dx (deprecated)        = %10d\n", ch, cfg->dx);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d dy (deprecated)        = %10d\n", ch, cfg->dy);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d src_end (deprecated)   = %10d\n", ch, cfg->src_end);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d src_wrap(deprecated)   = %10d\n", ch, cfg->src_wrap);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d src_stride             = %10d\n", ch, cfg->src_stride);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d perf_mode              = %10d\n", ch, cfg->perf_mode);

	(void) cfg;
	(void) ch;
}

NCI_GDC_STORAGE_CLASS_C void
nci_gdc_print_channel_sectionB_cfg(const struct gdc_sectionB_cfg *cfg, int ch)
{
	IA_CSS_TRACE_1(NCI_GDC, DEBUG, "%s:\n", __func__);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d scan_mode              = 0x%08X\n", ch, cfg->mode_config.scan_mode);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d proc_mode              = 0x%08X\n", ch, cfg->mode_config.proc_mode);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d src_packed             = 0x%08X\n", ch, cfg->mode_config.src_packed);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d dst_packed             = 0x%08X\n", ch, cfg->mode_config.dst_packed);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d gro_rpwc               = 0x%08X\n", ch, cfg->mode_config.gro_rpwc);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d coord_ratio            = 0x%08X\n", ch, cfg->mode_config.coord_ratio);

	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_ack_addr            = 0x%08X\n", ch, cfg->op_buf_cfg.buf_config.cmd_addr);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_ack_cmd_token       = 0x%08X\n", ch, cfg->op_buf_cfg.buf_config.token);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_buf_num             = %10d\n", ch, cfg->op_buf_cfg.buf_config.num_buf);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_buf_addr            = 0x%08X\n", ch, cfg->op_buf_cfg.buf_properties.addr);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_buf_stride          = %10d\n", ch, cfg->op_buf_cfg.buf_properties.stride);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d op_buf_line_stride     = %10d\n", ch, cfg->op_buf_cfg.buf_properties.line_stride);

	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d ip_ack_addr            = 0x%08X\n", ch, cfg->ip_buf_cfg.cmd_addr);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d ip_ack_cmd_token       = 0x%08X\n", ch, cfg->ip_buf_cfg.token);

	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d fragment_ack_addr      = 0x%08X\n", ch, cfg->frag_done_evt.ack_address);
	IA_CSS_TRACE_2(NCI_GDC, DEBUG, "\t ch%d fragment_ack_data      = 0x%08X\n", ch, cfg->frag_done_evt.ack_data);

	(void) cfg;
	(void) ch;
}

#endif /* __NCI_GDC_PRINT_INLINE_H */
