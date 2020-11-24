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

#ifndef __NCI_DEC400_REG_ACCESS_H
#define __NCI_DEC400_REG_ACCESS_H

#include "ia_css_cmem.h"
#include "storage_class.h"
#include "ipu_device_dec400_properties.h"
#include <ipu_device_subsystem_ids.h> /* for SSID values */
#include "type_support.h"
#include "nci_dec400_trace.h"

/**
 * nci_dec400_reg_store()
 */
STORAGE_CLASS_INLINE
void nci_dec400_reg_store(unsigned int address, unsigned int value)
{
	IA_CSS_TRACE_2(NCI_DEC400, DEBUG,
				   "nci_dec400_reg_store || {address = 0x%08x} {value = 0x%08x}\n",
				   address, value);
	ia_css_cmem_store_32(SSID, address, value);
}

/**
 * nci_dec400_reg_load()
 */
STORAGE_CLASS_INLINE
unsigned int nci_dec400_reg_load(unsigned int address)
{
	unsigned int value;

	value = ia_css_cmem_load_32(SSID, address);
	IA_CSS_TRACE_2(NCI_DEC400, DEBUG,
				   "nci_dec400_reg_load || {address = 0x%08x} {value = 0x%08x}\n",
				   address, value);

	return value;
}

#endif /* __NCI_DEC400_REG_ACCESS_H */
