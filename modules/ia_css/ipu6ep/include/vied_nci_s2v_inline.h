/**
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

#ifndef __VIED_NCI_S2V_INLINE_H
#define __VIED_NCI_S2V_INLINE_H

#include "vied_nci_s2v_inline_common.h"

/* Implementation */
NCI_S2V_STORAGE_CLASS_C
void nci_s2v_fill_section0_dep(
	struct nci_s2v_private_cfg *private_cfg,
	const struct nci_s2v_cfg *public_cfg)
{
	private_cfg->frame_width_en		= public_cfg->frame_width_en;
	private_cfg->frame_width		= public_cfg->frame_width;
	private_cfg->uv_interleaved_en  = public_cfg->uv_interleaved_en;
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_print_section0_dep(struct nci_s2v_private_cfg *private_cfg)
{
	(void)private_cfg;
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tframe_width_en = 0x%08x\n", private_cfg->frame_width_en);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tframe_width = 0x%08x\n", private_cfg->frame_width);
	IA_CSS_TRACE_1(VIED_NCI_S2V, DEBUG, "\tuv_interleaved_en = 0x%08x\n", private_cfg->uv_interleaved_en);

}

/*
 * Host/Cell specific implementations.
 * It uses the functions above, thus located at the end of the file.
 */
#include "vied_nci_s2v_inline_local.h"

#endif /* __VIED_NCI_S2V_INLINE_H */
