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

#ifndef __NCI_TNR_INLINE_H
#define __NCI_TNR_INLINE_H

#include "nci_tnr.h"
#include "nci_tnr_storage_class.h"
#include "ipu_device_tnr_devices.h"

#include "type_support.h"
#include "misc_support.h"
#include "assert_support.h"
#include <ipu_device_subsystem_ids.h> /* for SSID values */
#include <dai/dai_device_access.h>
#include "dai_ipu_top_devices.h"

NCI_TNR_STORAGE_CLASS_C
uint32_t nci_tnr_get_nof_sections(void)
{
	return NCI_TNR_NUM_SECTIONS;
}

NCI_TNR_STORAGE_CLASS_C
uint32_t nci_tnr_get_sizeof_section(enum nci_tnr_sections section)
{
	uint32_t size = 0;
	switch(section)
	{
	case NCI_TNR_SECTION0:
		size = sizeof(struct nci_tnr_cfg);
		break;
	default:
		assert(0);
		break;
	}
	return size;
}

NCI_TNR_STORAGE_CLASS_C
uint32_t nci_tnr_get_sizeof_blob(void)
{
	return nci_tnr_get_sizeof_section(NCI_TNR_SECTION0);
}

/* host or cell specific implementation */
#include "nci_tnr_inline_local.h"
#include "nci_tnr_pretty_print.h"

#endif /* __NCI_TNR_INLINE_H */
