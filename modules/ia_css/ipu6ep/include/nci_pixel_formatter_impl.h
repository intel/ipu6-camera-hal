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

#ifndef __NCI_PIXEL_FORMATTER_IMPL_H
#define __NCI_PIXEL_FORMATTER_IMPL_H

#include "assert_support.h"
#include "nci_pixel_formatter.h"
#include "nci_pixel_formatter_trace.h"
#include "nci_pixel_formatter_private_types.h"
#include "nci_pixel_formatter_storage_class.h"
#include "pixel_formatter_defs.h"

NCI_PIXEL_FORMATTER_STORAGE_CLASS_C
uint8_t nci_pixel_formatter_get_numberof_sections(void)
{
	return NCI_PIXEL_FORMATTER_NUM_SECTIONS;
}

NCI_PIXEL_FORMATTER_STORAGE_CLASS_C
uint32_t nci_pixel_formatter_get_sizeof_section(nci_pixel_formatter_section_t sec)
{
	uint32_t size = 0;

	assert(sec < NCI_PIXEL_FORMATTER_NUM_SECTIONS);

	if (sec == NCI_PIXEL_FORMATTER_SECTION0) {
		size = sizeof(struct nci_pixel_formatter_private_cfg);
	} else {
		IA_CSS_TRACE_0(NCI_PIXEL_FORMATTER, ERROR, "nci_pixel_formatter_get_sizeof_section(): invalid section id\n");
		assert(0);
	}
	return size;
}

NCI_PIXEL_FORMATTER_STORAGE_CLASS_C
uint32_t nci_pixel_formatter_get_offsetof_section(nci_pixel_formatter_section_t sec)
{
	uint32_t offset = 0;

	assert(sec < NCI_PIXEL_FORMATTER_NUM_SECTIONS);

	if (sec == NCI_PIXEL_FORMATTER_SECTION0) {
		offset = (((_PF_BANK_A)*_PF_BANK_SIZE)*_PF_CFG_REG_ADDRESS_ALIGN) + ((_PF_OUT_REG)*_PF_CFG_REG_ADDRESS_ALIGN);
	} else {
		IA_CSS_TRACE_0(NCI_PIXEL_FORMATTER, ERROR, "nci_pixel_formatter_get_offsetof_section(): invalid section id\n");
		assert(0);
	}

	return offset;
}

STORAGE_CLASS_INLINE
uint32_t nci_pixel_formatter_fill_section0(
	const nci_pixel_formatter_config_t *public_cfg,
	void *buffer);

STORAGE_CLASS_INLINE
uint32_t nci_pixel_formatter_fill_section0(
	const nci_pixel_formatter_config_t *public_cfg,
	void *buffer)
{
	struct nci_pixel_formatter_private_cfg *private_cfg = (struct nci_pixel_formatter_private_cfg *) buffer;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	/* assert based on max values based on bit width */
	assert(public_cfg->out_muxA_sel <= NCI_PF_BITMASK(NCI_PF_OUT_MUXA_SEL_SIZE));
	assert(public_cfg->out_muxB_sel <= NCI_PF_BITMASK(NCI_PF_OUT_MUXB_SEL_SIZE));
	assert(public_cfg->out_reg_mode <= A_bayerquad_B_bayerquad);

	private_cfg->out_param = NCI_PF_PACK_OUT_REG(public_cfg->out_reg_mode, public_cfg->out_muxA_sel, public_cfg->out_muxB_sel);
	private_cfg->pad_param = (uint32_t)public_cfg->pad_mode;

	return nci_pixel_formatter_get_sizeof_section(NCI_PIXEL_FORMATTER_SECTION0);
}

NCI_PIXEL_FORMATTER_STORAGE_CLASS_H uint32_t nci_pixel_formatter_get_sizeof_sections(void)
{
	return (nci_pixel_formatter_get_sizeof_section(NCI_PIXEL_FORMATTER_SECTION0));
}

/* host or cell specific implementations */
#include "nci_pixel_formatter_impl_local.h"

#endif /* __NCI_PIXEL_FORMATTER_IMPL_H */
