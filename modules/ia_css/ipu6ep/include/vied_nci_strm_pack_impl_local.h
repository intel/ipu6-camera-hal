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

#ifndef __VIED_NCI_STRM_PACK_IMPL_LOCAL_H
#define __VIED_NCI_STRM_PACK_IMPL_LOCAL_H

#include "vied_nci_strm_pack.h"
/* for vied_nci_strm_pack_config_t, vied_nci_strm_pack_route_t, IPU_DEVICE_STRM_PACK_G... */
#include "vied_nci_strm_pack_types.h"
/* for NCI_STRM_PACK_TRACE_METHOD, NCI_STRM_PACK_TRACE_LEVEL_VERBOSE      */
#include "vied_nci_strm_pack_trace.h"
#include "vied_nci_strm_pack_private_types.h"

NCI_STRM_PACK_STORAGE_CLASS_C
uint32_t nci_strm_pack_fill_section0(const nci_strm_pack_control_config_t *public_cfg,
	void *buffer)
{
	struct nci_strm_pack_private_cfg *private_cfg = (struct nci_strm_pack_private_cfg *) buffer;
	unsigned int bypass;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	bypass = public_cfg->bypass;

	if (bypass > 1) {
		IA_CSS_TRACE_1(NCI_STRM_PACK, ERROR,
			"strm pack bypass should be one bit, wrong value:%d\n",
			bypass);
		assert(0);
	}

	private_cfg->pack_en.Dword = 0;
	private_cfg->pack_en.fields.pack_en = !bypass;

	return nci_strm_pack_get_sizeof_section(NCI_STRM_PACK_SECTION0);
}

NCI_STRM_PACK_STORAGE_CLASS_C uint32_t nci_strm_pack_config(
	const nci_strm_pack_control_config_t *public_cfg,
	void *buffer)
{
	uint32_t size_sec0 = 0;

	assert(public_cfg != NULL);
	assert(buffer != NULL);

	size_sec0 = nci_strm_pack_fill_section0(public_cfg, buffer);
	nci_strm_pack_print_section0(buffer);

	return size_sec0;
}

NCI_STRM_PACK_STORAGE_CLASS_C
void nci_strm_pack_print_section0(void *buffer)
{
	struct nci_strm_pack_private_cfg *private_cfg = (struct nci_strm_pack_private_cfg *)buffer;

	IA_CSS_TRACE_1(NCI_STRM_PACK, INFO, "strm pack: pack enable:%d\n",
		private_cfg->pack_en.fields.pack_en);

}

NCI_STRM_PACK_STORAGE_CLASS_C
void nci_strm_pack_print_sections(void *buffer)
{
	assert(buffer != NULL);
	nci_strm_pack_print_section0(buffer);
}

#endif /*__VIED_NCI_STRM_PACK_IMPL_LOCAL_H */
