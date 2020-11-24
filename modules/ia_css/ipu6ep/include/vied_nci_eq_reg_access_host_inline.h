/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2016 Intel Corporation.
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

#ifndef __VIED_NCI_EQ_REG_ACCESS_HOST_INLINE_H
#define __VIED_NCI_EQ_REG_ACCESS_HOST_INLINE_H

#include "vied_nci_eq_reg_access.h"
#include <ipu_device_subsystem_ids.h>
#include "ipu_device_eq_properties.h"
#include "assert_support.h"
#include "misc_support.h"
#include "ia_css_cmem.h"

/* Used for Host to access Event Queue registers */

NCI_EQ_STORAGE_CLASS_C
void event_queue_ip_reg_store(
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value)
{
	uint32_t port = ipu_device_eq_ip_address(eq_id);
	ia_css_cmem_store_32(SSID, port + reg, value);
}

NCI_EQ_STORAGE_CLASS_C
void event_queue_ip_cond_reg_store(
	bool cond,
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value)
{
	uint32_t port = ipu_device_eq_ip_address(eq_id);
	ia_css_cond_cmem_store_32(cond, SSID, port + reg, value);
}

NCI_EQ_STORAGE_CLASS_C
uint32_t event_queue_ip_reg_load(
	enum nci_eq_device_id eq_id,
	uint32_t reg)
{
	uint32_t port = ipu_device_eq_ip_address(eq_id);
	return ia_css_cmem_load_32(SSID, port + reg);
}

NCI_EQ_STORAGE_CLASS_C
uint32_t event_queue_ip_cond_reg_load(
	bool cond,
	enum nci_eq_device_id eq_id,
	uint32_t reg)
{
	uint32_t port = ipu_device_eq_ip_address(eq_id);

	return ia_css_cond_cmem_load_32(cond, SSID, port + reg);
}

NCI_EQ_STORAGE_CLASS_C
void event_queue_op_reg_store(
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value)
{
	NOT_USED(eq_id);
	NOT_USED(reg);
	NOT_USED(value);
	assert(0); /* this port is not accessible from the host */
}

NCI_EQ_STORAGE_CLASS_C
uint32_t event_queue_op_reg_load(
	enum nci_eq_device_id eq_id,
	uint32_t reg)
{
	NOT_USED(eq_id);
	NOT_USED(reg);
	assert(0); /* this port is not accessible from the host */
	return 0;
}

#endif /* __VIED_NCI_EQ_REG_ACCESS_HOST_INLINE_H */
