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

#ifndef __NCI_PIXEL_FORMATTER_IMPL_LOCAL_H
#define __NCI_PIXEL_FORMATTER_IMPL_LOCAL_H

#include "nci_pixel_formatter.h"
#include "nci_pixel_formatter_types.h"              /* for vied_nci_pixel_formatter_config_t, vied_nci_pixel_formatter_route_t, IPU_DEVICE_PIXEL_FORMATTER_G... */
#include "nci_pixel_formatter_trace.h"              /* for NCI_PIXEL_FORMATTER_TRACE_METHOD, NCI_PIXEL_FORMATTER_TRACE_LEVEL_VERBOSE      */
#include "nci_pixel_formatter_storage_class.h"
#include "ipu_device_pf_properties.h"

NCI_PIXEL_FORMATTER_STORAGE_CLASS_C uint32_t nci_pixel_formatter_config(
	enum ipu_device_pf_dev_id pf_dev_id,
	const nci_pixel_formatter_config_t *pf_cfg,
	void *buffer)
{
	uint32_t size_sec0 = 0;

	assert(pf_cfg != NULL);
	assert(pf_dev_id < IPU_DEVICE_PIXEL_FORMATTER_NUM_DEVS);
	assert(buffer != NULL);

	size_sec0 = nci_pixel_formatter_fill_section0(pf_cfg, buffer);
	return (size_sec0);
}

#endif /* __NCI_PIXEL_FORMATTER_IMPL_LOCAL_H */
