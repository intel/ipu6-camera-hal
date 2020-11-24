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

#ifndef __NCI_PIFCONV_CTRL_IMPL_H
#define __NCI_PIFCONV_CTRL_IMPL_H

#include "nci_pifconv_ctrl.h"
#include "nci_pifconv_ctrl_types.h"
#include "nci_pifconv_ctrl_trace.h"
#include "nci_pifconv_ctrl_private_types.h"
#include "nci_pifconv_ctrl_reg_pack_unpack.h" /* pack_reg_PifConverter_pif_converter_ctrl(), ... */
#include "assert_support.h"
#include "misc_support.h"

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_fill_section0(
	const struct nci_pifconv_ctrl_cfg *cfg,
	void *buffer);

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_get_nof_sections(void)
{
	return NCI_PIFCONV_CTRL_NOF_SECTIONS;
}

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_get_sizeof_section(unsigned int section_id)
{
	switch (section_id) {
	case NCI_PIFCONV_CTRL_SECTION0:
	     return sizeof(struct nci_pifconv_ctrl_private_cfg_section0);
	default:
		IA_CSS_TRACE_0(NCI_PIFCONV_CTRL, ERROR, "nci_pifconv_ctrl_get_sizeof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_get_sizeof_blob(void)
{
	return nci_pifconv_ctrl_get_sizeof_section(NCI_PIFCONV_CTRL_SECTION0);
}

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_get_offsetof_section(
		unsigned int section_id)
{
	switch (section_id) {
	case NCI_PIFCONV_CTRL_SECTION0:
		return NCI_PIFCONV_CTRL_SECTION0_OFFSET;
	default:
		IA_CSS_TRACE_0(NCI_PIFCONV_CTRL, ERROR, "nci_pifconv_ctrl_get_offsetof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_PIFCONV_CTRL_STORAGE_CLASS_C
uint32_t nci_pifconv_ctrl_fill_section0(
	const struct nci_pifconv_ctrl_cfg *cfg,
	void *buffer)
{
	struct nci_pifconv_ctrl_private_cfg_section0 *priv = (struct nci_pifconv_ctrl_private_cfg_section0 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	priv->ctrl_info = pack_reg_PifConverter_pif_converter_ctrl(
				NCI_UNSIGNED(cfg->pif_hdr_mask),
				NCI_UNSIGNED(cfg->crop_en));
	priv->input_dim_info = pack_reg_PifConverter_pif_converter_input_dim(
				NCI_UNSIGNED(cfg->input_dim_line_width),
				NCI_UNSIGNED(cfg->input_dim_nof_lines));
	priv->crop_col_info = pack_reg_PifConverter_pif_converter_crop_col(
				NCI_UNSIGNED(cfg->crop_col_start),
				NCI_UNSIGNED(cfg->crop_col_end));
	priv->crop_row_info = pack_reg_PifConverter_pif_converter_crop_row(
				NCI_UNSIGNED(cfg->crop_row_start),
				NCI_UNSIGNED(cfg->crop_row_end));

	return nci_pifconv_ctrl_get_sizeof_section(NCI_PIFCONV_CTRL_SECTION0);
}

#include "nci_pifconv_ctrl_impl_local.h"

#endif /* __NCI_PIFCONV_CTRL_IMPL_H */
