/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2013 Intel Corporation.
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

/**
 * @file
 * @brief Internally used in the DAI property interface
 */

#ifndef DAI_SUBSYSTEM_PROPERTY_H_
#define DAI_SUBSYSTEM_PROPERTY_H_

#include "dai_types.h"
#include "dai_subsystem_property_types.h"

#include <vied/vied_subsystem_access_types.h>

/** @cond DOXYGEN_SKIP_MACROS */
#ifdef __VIED_CELL
#define _DAI_SUBSYSTEM_PROPERTY_INLINE static inline
#else
#if defined(CFG_DAI_SUBSYSTEM_PROPERTY_LIB_IMPL)
#define _DAI_SUBSYSTEM_PROPERTY_INLINE
#else
#define _DAI_SUBSYSTEM_PROPERTY_INLINE static inline
#include "dai_subsystem_property_impl.h"
#endif
#endif
/** @endcond */

/**
 * @brief retrieve properties for a subsystem
 *
 * 	This function is used by generated DAI code to access the correct
 * 	properties for the specified subsystem ID.
 *
 * @return		reference to property table
 */
_DAI_SUBSYSTEM_PROPERTY_INLINE
const dai_subsystem_properties_t *
dai_subsystem_get_properties(void);

#ifndef __VIED_CELL
_DAI_SUBSYSTEM_PROPERTY_INLINE
void dai_set_active_subsystem_properties(vied_subsystem_t ssid);
#endif

#endif  /* DAI_SUBSYSTEM_PROPERTY_H_ */
