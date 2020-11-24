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

#ifndef __NCI_PIXEL_STR2MMIO_IMPL_H
#define __NCI_PIXEL_STR2MMIO_IMPL_H

#include "nci_pixel_str2mmio.h"
#include "nci_pixel_str2mmio_types.h"
#include "nci_pixel_str2mmio_trace.h"
#include "nci_pixel_str2mmio_private_types.h"
#include "assert_support.h"
#include "misc_support.h"

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_fill_section0(
	const struct nci_pixel_str2mmio_cfg *cfg,
	void *buffer);

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_nof_sections(void)
{
	return NCI_PIXEL_STR2MMIO_NOF_SECTIONS;
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_sizeof_section(unsigned int section_id)
{
	switch (section_id) {
	case NCI_PIXEL_STR2MMIO_SECTION0:
	     return sizeof(struct nci_pixel_str2mmio_private_cfg_section0);
	default:
		IA_CSS_TRACE_0(NCI_PIXEL_STR2MMIO, ERROR, "nci_pixel_str2mmio_get_sizeof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_sizeof_blob(void)
{
	return nci_pixel_str2mmio_get_sizeof_section(NCI_PIXEL_STR2MMIO_SECTION0);
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_offsetof_section(
		unsigned int section_id)
{
	switch (section_id) {
	case NCI_PIXEL_STR2MMIO_SECTION0:
		return NCI_PIXEL_STR2MMIO_SECTION0_OFFSET;
	default:
		IA_CSS_TRACE_0(NCI_PIXEL_STR2MMIO, ERROR, "nci_pixel_str2mmio_get_offsetof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_fill_section0(
	const struct nci_pixel_str2mmio_cfg *cfg,
	void *buffer)
{
	struct nci_pixel_str2mmio_private_cfg_section0 *priv = (struct nci_pixel_str2mmio_private_cfg_section0 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	/* Range checks for the configuration */
	assert((cfg->pix_width & ~STR2MMIO_PIX_WIDTH_REG_MASK) == 0);
	assert((cfg->start_address & ~STR2MMIO_MPORT_AW_MASK) == 0);
	assert((cfg->end_address & ~STR2MMIO_MPORT_AW_MASK) == 0);
	assert((cfg->stride & ~STR2MMIO_MPORT_AW_MASK) == 0);
	assert((cfg->num_items & ~STR2MMIO_NUM_ITEMS_REG_MASK) == 0);
	assert((cfg->block_when_no_cmd & ~STR2MMIO_BLOCK_WHEN_NO_CMD_REG_MASK) == 0);
	assert((cfg->ack_base_addr & ~STR2MMIO_ACK_BASE_ADDR_REG_MASK) == 0);
	assert((cfg->sidpid & ~STR2MMIO_SIDPID_REG_MASK) == 0);
	assert((cfg->rpwc_enable & ~STR2MMIO_RPWC_ENABLE_REG_MASK) == 0);

	priv->pix_width = cfg->pix_width;
	priv->start_address = cfg->start_address;
	priv->end_address = cfg->end_address;
	priv->stride = cfg->stride;
	priv->num_items = cfg->num_items;
	priv->block_when_no_cmd = cfg->block_when_no_cmd;
	priv->ack_base_addr = cfg->ack_base_addr;
	priv->sidpid = cfg->sidpid;
	priv->datafield_remap_cfg = cfg->datafield_remap_cfg;
	priv->rpwc_enable = cfg->rpwc_enable;

	return nci_pixel_str2mmio_get_sizeof_section(NCI_PIXEL_STR2MMIO_SECTION0);
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_command_reg_offset(void)
{
	return NCI_PIXEL_STR2MMIO_COMMAND_REG_OFFSET;
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_value_for_store_command(void)
{
	return _STREAM2MMIO_CMD_TOKEN_STORE_WORDS;
}

NCI_PIXEL_STR2MMIO_STORAGE_CLASS_C
uint32_t nci_pixel_str2mmio_get_value_for_sync_command(void)
{
	return _STREAM2MMIO_CMD_TOKEN_SYNC_FRAME;
}

#include "nci_pixel_str2mmio_impl_local.h"

#endif /* __NCI_PIXEL_STR2MMIO_IMPL_H */
