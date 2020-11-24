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
#ifndef _default_system_dai_subsystem_property_impl_h_
#define _default_system_dai_subsystem_property_impl_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_subsystem_property.h>
#include <dai/dai_subsystem_property_types.h>
#include <dai/dai_system_types.h>
#include "dai_ipu_top_device_info.h"
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

#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#ifndef SSID
#define SSID 0x0
#endif

_DAI_SUBSYSTEM_PROPERTY_INLINE
const dai_subsystem_properties_t * dai_subsystem_get_properties(void)
{
	return &dai_ipu_top_properties;
}

_DAI_SUBSYSTEM_PROPERTY_INLINE
void dai_set_active_subsystem_properties(dai_subsystem_t ssid)
{
	(void) ssid;
	return;
}

#endif /* _default_system_dai_subsystem_property_impl_h_ */
