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

#ifndef __NCI_CIO2STR_IMPL_H
#define __NCI_CIO2STR_IMPL_H

#include "nci_cio2str.h"
#include "nci_cio2str_types.h"
#include "nci_cio2str_trace.h"
#include "nci_cio2str_private_types.h"
#include "assert_support.h"
#include "misc_support.h"

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_fill_section0(
	const struct nci_cio2str_cfg *cfg,
	void *buffer);

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_fill_section1(
	const struct nci_cio2str_cfg *cfg,
	void *buffer);

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_get_nof_sections(void)
{
	return NCI_CIO2STR_NOF_SECTIONS;
}

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_get_sizeof_section(unsigned int section_id)
{
	switch (section_id) {
	case NCI_CIO2STR_SECTION0:
	     return sizeof(struct nci_cio2str_private_cfg_section0);
	case NCI_CIO2STR_SECTION1:
	     return sizeof(struct nci_cio2str_private_cfg_section1);
	default:
		IA_CSS_TRACE_0(NCI_CIO2STR, ERROR, "nci_cio2str_get_sizeof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_get_sizeof_blob(void)
{
	return nci_cio2str_get_sizeof_section(NCI_CIO2STR_SECTION0) +
		nci_cio2str_get_sizeof_section(NCI_CIO2STR_SECTION1);
}

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_get_offsetof_section(
		unsigned int section_id)
{
	switch (section_id) {
	case NCI_CIO2STR_SECTION0:
		return NCI_CIO2STR_SECTION0_OFFSET;
	case NCI_CIO2STR_SECTION1:
		return NCI_CIO2STR_SECTION1_OFFSET;
	default:
		IA_CSS_TRACE_0(NCI_CIO2STR, ERROR, "nci_cio2str_get_offsetof_section: Invalid section id\n");
		assert(0);
		return 0;
	}
}

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_fill_section0(
	const struct nci_cio2str_cfg *cfg,
	void *buffer)
{
	struct nci_cio2str_private_cfg_section0 *priv = (struct nci_cio2str_private_cfg_section0 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	priv->port.sl_addr_mask = cfg->port0.sl_addr_mask;
	priv->port.sl_addr_cmpv = cfg->port0.sl_addr_cmpv;

	return nci_cio2str_get_sizeof_section(NCI_CIO2STR_SECTION0);
}

NCI_CIO2STR_STORAGE_CLASS_C
uint32_t nci_cio2str_fill_section1(
	const struct nci_cio2str_cfg *cfg,
	void *buffer)
{
	struct nci_cio2str_private_cfg_section1 *priv = (struct nci_cio2str_private_cfg_section1 *) buffer;

	assert(cfg != NULL);
	assert(buffer != NULL);

	priv->port.sl_addr_mask = cfg->port1.sl_addr_mask;
	priv->port.sl_addr_cmpv = cfg->port1.sl_addr_cmpv;

	return nci_cio2str_get_sizeof_section(NCI_CIO2STR_SECTION1);
}

#include "nci_cio2str_impl_local.h"

#endif /* __NCI_CIO2STR_IMPL_H */
