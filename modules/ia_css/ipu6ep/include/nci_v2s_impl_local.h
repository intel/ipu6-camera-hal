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

#ifndef __NCI_V2S_IMPL_LOCAL_H
#define __NCI_V2S_IMPL_LOCAL_H

#include "nci_v2s.h"
#include "nci_v2s_types.h"
#include "nci_v2s_private_types.h"
#include "assert_support.h"
#include "misc_support.h"

NCI_V2S_STORAGE_CLASS_C
uint32_t nci_v2s_fill_config(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		const struct nci_v2s_cfg *cfg,
		void *buffer)
{
	char *buffer_char = (char *)buffer;
	assert(cfg != NULL);
	assert(buffer_char != NULL);
	assert(dev_id < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);

	/* not used in host implementation */
	NOT_USED(dev_id);

	/* HSD 1304805686: Because of bug in BAYER, All patterns are manually configured
	 * So section 3 is NOT configured/filled anymore.
	 */

	buffer_char += nci_v2s_fill_section0(cfg, buffer_char);
	buffer_char += nci_v2s_fill_section1(cfg, buffer_char);
	buffer_char += nci_v2s_fill_section2(cfg, buffer_char);

	return nci_v2s_get_sizeof_blob();
}

NCI_V2S_STORAGE_CLASS_C
void nci_v2s_set_command(
	int32_t dev_id,
	const struct nci_v2s_cmd *cfg)
{
	NOT_USED(dev_id);
	NOT_USED(cfg);
};

#endif /* __NCI_V2S_IMPL_LOCAL_H */
