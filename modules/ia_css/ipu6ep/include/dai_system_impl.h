/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2017 Intel Corporation.
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
#ifndef _default_system_dai_system_impl_h_
#define _default_system_dai_system_impl_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_access_types.h>
#include <dai/dai_system_types.h>
#include <vied/vied_subsystem_access_initialization.h>
#include "device_types.h"
/**
 * WARNING
 *
 * This is a default implementation of the subsystem interface.
 *
 * It can only be used when there is a single subsystem.
 *
 * For multi-subsystem access, the developer of the system has to provide this
 * file and define the mapping between subsystem ID and address/property
 * tables.
 *
 * MP 2015-April-15
 */

#include <dai/dai_subsystem_property.h>

#if defined(__KERNEL__)
#include <linux/types.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#endif

#ifndef SSID
#define SSID 0x0
#endif

extern const  dai_internal_route_t * const * const dai_ipu_top_entry_device_types[NUM_DAI_DEVICE_TYPES];
/* use subsystem base address in host libs */
#ifdef _DAI_ENTIRE_SYSTEM
/* if we use the entire system the device routes start at host
 * in this case the subsystem base address is zero */
const vied_subsystem_base_address_t _dai_subsystem_base_addr_list[1] = {0x0};
#endif

_DAI_SYSTEM_INLINE
void dai_subsystem_access_initialize(dai_subsystem_t ssid)
{
	vied_subsystem_access_initialize(ssid);
}

_DAI_SYSTEM_INLINE
void dai_subsystem_access_initialize_default(void)
{
#ifdef _DAI_ENTIRE_SYSTEM
	vied_subsystems_access_initialize(1, _dai_subsystem_base_addr_list);
#else
	vied_subsystem_access_initialize(SSID);
#endif
}

/* Memory (de)allocation */
_DAI_SYSTEM_INLINE
void *dai_system_malloc(size_t size)
{
#if defined(__KERNEL__)
	void *res = kmalloc(size, GFP_KERNEL);
#else
	void *res = malloc(size);
#endif
	return res;
}

_DAI_SYSTEM_INLINE
void dai_system_free(void *ptr)
{
#if defined(__KERNEL__)
	kfree(ptr);
#else
	free(ptr);
#endif
}

/* Retrieve address map for a subsystem */
_DAI_SYSTEM_INLINE
const dai_internal_route_t * const * const * dai_system_get_address_table(void)
{
	return dai_ipu_top_entry_device_types;
}

/* Get active subsystem */
_DAI_SYSTEM_INLINE
dai_subsystem_t dai_get_active_subsystem(void)
{
	return SSID;
}

/* Set active address map for a subsystem */
_DAI_SYSTEM_INLINE
void dai_set_active_subsystem(dai_subsystem_t subsystem_id)
{
	dai_set_active_subsystem_properties(subsystem_id);
	return;
}

#endif /* _default_system_dai_system_impl_h_ */
