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

#ifndef __NCI_V2S_IMPL_H
#define __NCI_V2S_IMPL_H

#include "nci_v2s.h"
#include "nci_v2s_types.h"
#include "ia_css_trace.h"
#include "nci_v2s_trace.h"
#include "nci_v2s_private_types.h"
#include "nci_v2s_reg_pack_unpack.h"
#include "vec_to_str_v3_regmem.h"
#include "dai_ipu_top_devices.h"
#include "assert_support.h"
#include "misc_support.h"

#define _V2S_REG_ALIGN 4
#define _V2S_PROC_N_VEC_OPCODE	0x01

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_pack_component_reg(
		const struct nci_v2s_component_cfg *cfg);

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section0(
	const struct nci_v2s_cfg *cfg,
	void *buffer);

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section1(
	const struct nci_v2s_cfg *cfg,
	void *buffer);

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section2(
	const struct nci_v2s_cfg *cfg,
	void *buffer);

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_nof_sections(void)
{
	return NCI_V2S_NOF_SECTIONS;
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_sizeof_section(unsigned int section_id)
{
	switch (section_id) {
	case NCI_V2S_SECTION0:
	     return sizeof(struct nci_v2s_private_cfg_section0);
	case NCI_V2S_SECTION1:
	     return sizeof(struct nci_v2s_private_cfg_section1);
	case NCI_V2S_SECTION2:
		return sizeof(struct nci_v2s_private_cfg_section2);
	default:
		IA_CSS_TRACE_0(NCI_V2S, ERROR, "nci_v2s_get_sizeof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_sizeof_blob(void)
{
	return nci_v2s_get_sizeof_section(NCI_V2S_SECTION0) +
		nci_v2s_get_sizeof_section(NCI_V2S_SECTION1) +
		nci_v2s_get_sizeof_section(NCI_V2S_SECTION2);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_offsetof_section(
		unsigned int section_id)
{
	switch (section_id) {
	case NCI_V2S_SECTION0:
		return _V2S_REG_ALIGN * DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_CMD_CFG;
	case NCI_V2S_SECTION1:
		return _V2S_REG_ALIGN * DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_STRIDE_CFG;
	case NCI_V2S_SECTION2:
		return _V2S_REG_ALIGN * DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_BUFF_0_CFG_EN;
	default:
		IA_CSS_TRACE_0(NCI_V2S, ERROR, "nci_v2s_get_offsetof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_pack_component_reg(
		const struct nci_v2s_component_cfg *cfg)
{
	return pack_reg_v2s_comp_cfg(
			cfg->en,
			cfg->src_vec,
			cfg->start_idx,
			cfg->offset_idx);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section0(
	const struct nci_v2s_cfg *cfg,
	void *buffer)
{
	struct nci_v2s_private_cfg_section0 *priv = (struct nci_v2s_private_cfg_section0 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	/* make sure no one sets cfg->dev_cfg.cmd_cfg_cmd_eol_end_en HSD: 1305948469 */
	assert(cfg->dev_cfg.cmd_cfg_cmd_eol_end_en == 0);
	priv->cmd_cfg = cfg->dev_cfg.cmd_cfg_cmd_eol_end_en;
	priv->ack_cfg = pack_reg_v2s_ack_cfg(
				cfg->ack_cfg.ack_cfg_ack_k_vec,
				cfg->ack_cfg.ack_cfg_ack_eol_en);
	priv->ack_msg = pack_reg_v2s_ack_msg(
				cfg->ack_cfg.ack_msg_message,
				cfg->ack_cfg.ack_msg_cmd,
				cfg->ack_cfg.ack_msg_pid,
				cfg->ack_cfg.ack_msg_sid);
	priv->ack_addr = cfg->ack_cfg.ack_addr;
	priv->frame_dim = pack_reg_v2s_frame_dim(
				cfg->dev_cfg.frame_dim_width,
				cfg->dev_cfg.frame_dim_height);

	/*h_incr and v_incr is 2 for BOTH BAYER and YUV420*/
	const int V2S_BAYER_QUAD_STREAM_ELEM_H_SIZE = 2;
	const int V2S_BAYER_QUAD_STREAM_ELEM_V_SIZE = 2;
	priv->strm_dim.strm_dim_h_incr = V2S_BAYER_QUAD_STREAM_ELEM_H_SIZE;
	priv->strm_dim.strm_dim_v_incr = V2S_BAYER_QUAD_STREAM_ELEM_V_SIZE;

	return nci_v2s_get_sizeof_section(NCI_V2S_SECTION0);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section1(
	const struct nci_v2s_cfg *cfg,
	void *buffer)
{
	struct nci_v2s_private_cfg_section1 *priv = (struct nci_v2s_private_cfg_section1 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	priv->stride_cfg = cfg->dev_cfg.stride_cfg_en;

	return nci_v2s_get_sizeof_section(NCI_V2S_SECTION1);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_section2(
	const struct nci_v2s_cfg *cfg,
	void *buffer)
{
	struct nci_v2s_private_cfg_section2 *priv = (struct nci_v2s_private_cfg_section2 *) buffer;
	int i = 0;

	assert(cfg != NULL);
	assert(buffer != NULL);

	for (i = 0; i < V2S_NUM_BUFFERS; i++) {
		priv->buf_cfg[i] = cfg->buf_cfg[i];
	}

	/* HSD 1304805686: V2S auto cfg does not work propertly for BAYER format
	 * So ALL supported formats are manually configured
	 */
	if (cfg->format == V2S_FORMAT_BAYER_ON_YUV420_4PPC) {

		const struct nci_v2s_component_cfg v2s_component_cfg_gr = {
			1,  /* en         */
			0,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_r = {
			1,  /* en         */
			2,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_u = {
			0,  /* en         */
			0,  /* src_vec    */
			0,  /* start_idx  */
			0,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_b = {
			1,  /* en         */
			1,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_gb = {
			1,  /* en         */
			3,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_v = {
			0,  /* en         */
			0,  /* src_vec    */
			0,  /* start_idx  */
			0,  /* offset_idx */
		};

		/* Component Config */
		priv->comp_cfg[0] = nci_v2s_pack_component_reg(&v2s_component_cfg_gr);
		priv->comp_cfg[1] = nci_v2s_pack_component_reg(&v2s_component_cfg_r);
		priv->comp_cfg[2] = nci_v2s_pack_component_reg(&v2s_component_cfg_u);
		priv->comp_cfg[3] = nci_v2s_pack_component_reg(&v2s_component_cfg_b);
		priv->comp_cfg[4] = nci_v2s_pack_component_reg(&v2s_component_cfg_gb);
		priv->comp_cfg[5] = nci_v2s_pack_component_reg(&v2s_component_cfg_v);

		/* Enable Buffer */
		for(i = 0; i < 4; i++)
			priv->buf_cfg[i].buffer_en = 1;
	} else if (cfg->format == V2S_FORMAT_YUV420_4PPC) {

		const struct nci_v2s_component_cfg v2s_component_cfg_y00 = {
			1,  /* en         */
			0,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_y01 = {
			1,  /* en         */
			1,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_u = {
			1,  /* en         */
			2,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_y10 = {
			1,  /* en         */
			0,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_y11 = {
			1,  /* en         */
			1,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_v = {
			1,  /* en         */
			3,  /* src_vec    */
			0,  /* start_idx  */
			1,  /* offset_idx */
		};

		/* Component Config */
		priv->comp_cfg[0] = nci_v2s_pack_component_reg(&v2s_component_cfg_y00);
		priv->comp_cfg[1] = nci_v2s_pack_component_reg(&v2s_component_cfg_y01);
		priv->comp_cfg[2] = nci_v2s_pack_component_reg(&v2s_component_cfg_u);
		priv->comp_cfg[3] = nci_v2s_pack_component_reg(&v2s_component_cfg_y10);
		priv->comp_cfg[4] = nci_v2s_pack_component_reg(&v2s_component_cfg_y11);
		priv->comp_cfg[5] = nci_v2s_pack_component_reg(&v2s_component_cfg_v);

		/* Enable Buffer */
		for (i = 0; i < 4; i++)
			priv->buf_cfg[i].buffer_en = 1;
	} else if (cfg->format == V2S_FORMAT_RGB_4PPC) {

		const struct nci_v2s_component_cfg v2s_component_cfg_r00 = {
			1,  /* en         */
			0,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_r01 = {
			1,  /* en         */
			0,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_g00 = {
			1,  /* en         */
			1,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_g01 = {
			1,  /* en         */
			1,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_b00 = {
			1,  /* en         */
			2,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_b01 = {
			1,  /* en         */
			2,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_r10 = {
			1,  /* en         */
			3,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_r11 = {
			1,  /* en         */
			3,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_g10 = {
			1,  /* en         */
			4,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg v2s_component_cfg_g11 = {
			1,  /* en         */
			4,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_b10 = {
			1,  /* en         */
			5,  /* src_vec    */
			0,  /* start_idx  */
			2,  /* offset_idx */
		};
		const struct nci_v2s_component_cfg  v2s_component_cfg_b11 = {
			1,  /* en         */
			5,  /* src_vec    */
			1,  /* start_idx  */
			2,  /* offset_idx */
		};

		/* Component Config */
		priv->comp_cfg[0] = nci_v2s_pack_component_reg(&v2s_component_cfg_r00);
		priv->comp_cfg[1] = nci_v2s_pack_component_reg(&v2s_component_cfg_r01);
		priv->comp_cfg[2] = nci_v2s_pack_component_reg(&v2s_component_cfg_g00);
		priv->comp_cfg[3] = nci_v2s_pack_component_reg(&v2s_component_cfg_g01);
		priv->comp_cfg[4] = nci_v2s_pack_component_reg(&v2s_component_cfg_b00);
		priv->comp_cfg[5] = nci_v2s_pack_component_reg(&v2s_component_cfg_b01);
		priv->comp_cfg[6] = nci_v2s_pack_component_reg(&v2s_component_cfg_r10);
		priv->comp_cfg[7] = nci_v2s_pack_component_reg(&v2s_component_cfg_r11);
		priv->comp_cfg[8] = nci_v2s_pack_component_reg(&v2s_component_cfg_g10);
		priv->comp_cfg[9] = nci_v2s_pack_component_reg(&v2s_component_cfg_g11);
		priv->comp_cfg[10] = nci_v2s_pack_component_reg(&v2s_component_cfg_b10);
		priv->comp_cfg[11] = nci_v2s_pack_component_reg(&v2s_component_cfg_b11);

		/* Enable Buffer */
		for (i = 0; i < V2S_NUM_BUFFERS; i++)
			priv->buf_cfg[i].buffer_en = 1;
	}

	return nci_v2s_get_sizeof_section(NCI_V2S_SECTION2);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_command_offset(void)
{
	return (_V2S_REG_ALIGN * DAI_VEC_TO_STR_V3_REG_VEC_TO_STR_V3_CMD_FIFO_TAIL);
}

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_get_command_token(unsigned nof_vectors)
{
	return pack_reg_v2s_cmd_fifo_tail(_V2S_PROC_N_VEC_OPCODE, 0, nof_vectors);
}

#include "nci_v2s_impl_local.h"
#include "nci_v2s_pretty_print.h"

#endif /* __NCI_V2S_IMPL_H */
