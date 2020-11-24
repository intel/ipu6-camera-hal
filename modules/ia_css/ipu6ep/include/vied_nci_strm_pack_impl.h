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

#ifndef __VIED_NCI_STRM_PACK_IMPL_H
#define __VIED_NCI_STRM_PACK_IMPL_H

#include "assert_support.h"
#include "vied_nci_strm_pack.h"
/* for NCI_STRM_PACK_TRACE_METHOD, NCI_STRM_PACK_TRACE_LEVEL_VERBOSE      */
#include "vied_nci_strm_pack_trace.h"
#include "vied_nci_strm_pack_private_types.h"
#include "vied_nci_strm_pack_storage_class.h"

NCI_STRM_PACK_STORAGE_CLASS_C
uint8_t nci_strm_pack_get_numberof_sections(void)
{
	return NCI_STRM_PACK_NUM_SECTIONS;
}

NCI_STRM_PACK_STORAGE_CLASS_C
uint32_t nci_strm_pack_get_sizeof_section(unsigned int sec)
{
	uint32_t size = 0;

	assert(sec < NCI_STRM_PACK_NUM_SECTIONS);

	if (sec == NCI_STRM_PACK_SECTION0) {
		size = sizeof(struct nci_strm_pack_private_cfg);
	} else {
		IA_CSS_TRACE_0(NCI_STRM_PACK, ERROR, "nci_strm_pack_get_sizeof_section(): invalid section id\n");
		assert(0);
	}
	return size;
}

NCI_STRM_PACK_STORAGE_CLASS_C
uint32_t nci_strm_pack_get_offsetof_section(unsigned int sec)
{
	uint32_t offset = 0;

	assert(sec < NCI_STRM_PACK_NUM_SECTIONS);

	if (sec == NCI_STRM_PACK_SECTION0) {
		offset = 0;
	} else {
		IA_CSS_TRACE_0(NCI_STRM_PACK, ERROR, "nci_strm_pack_get_offsetof_section(): invalid section id\n");
		assert(0);
	}

	return offset;
}

NCI_STRM_PACK_STORAGE_CLASS_C uint32_t nci_strm_pack_get_sizeof_sections(void)
{
	return nci_strm_pack_get_sizeof_section(NCI_STRM_PACK_SECTION0);
}

#include "vied_nci_strm_pack_impl_local.h"

#endif /* __VIED_NCI_STRM_PACK_IMPL_H */
