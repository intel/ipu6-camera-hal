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
 * @brief Internally used, defines subsystem functions to use on the host
 */

#ifndef DAI_SYSTEM_H_
#define DAI_SYSTEM_H_

#ifdef __VIED_CELL
#error host-only interface
#endif

#include <stddef.h>
#include "dai_system_types.h"
#include "dai_device_access_types.h"

/** @cond DOXYGEN_SKIP_MACROS */
#if defined(CFG_DAI_SYSTEM_LIB_IMPL)
#define _DAI_SYSTEM_INLINE
#else
#define _DAI_SYSTEM_INLINE static inline
#include "dai_system_impl.h"
#endif
/** @endcond */

/**
 * @brief Initialization
 *
 *	This function is used by host code to initialize the
 *	subsystem. It sets the base address for subsystem
 *	access.
 *
 * @param[in] ssid	subsystem id
 * @return		nothing
 */
_DAI_SYSTEM_INLINE
void dai_subsystem_access_initialize(dai_subsystem_t ssid);

/**
 * @brief Initialization of default system
 *
 *	This function is used by host code to initialize the
 *	subsystem. It sets the base address for subsystem
 *	access.
 *
 * @return		nothing
 */
_DAI_SYSTEM_INLINE
void dai_subsystem_access_initialize_default(void);

/**
 * @brief Memory allocation
 *
 *	This function is used by host code to allocate memory.
 *
 * @param[in] size	size of buffer to allocate
 * @return		pointer to allocated buffer
 */
_DAI_SYSTEM_INLINE
void *dai_system_malloc(size_t size);

/**
 * @brief Memory de-allocation
 *
 * 	This function is used by host code to free allocated memory.
 *
 * @param[in] ptr	pointer to allocated buffer
 * @return		nothing
 */
_DAI_SYSTEM_INLINE
void dai_system_free(void *ptr);

/**
 * @brief retrieve address map for a subsystem
 *
 * 	This function is used by generated DAI code to access the correct
 * 	address map for the currently selected subsystem.
 *
 * @return	reference to address map
 */
_DAI_SYSTEM_INLINE
const dai_internal_route_t * const * const * dai_system_get_address_table(void);

/**
 * @brief get active subsystem
 *
 * 	This function is used to get the current active subsystem.
 *
 * @return	subsystem ID
 */
_DAI_SYSTEM_INLINE
vied_subsystem_t dai_get_active_subsystem(void);

/**
 * @brief set active subsystem
 *
 * 	This function is used to set the current active subsystem.
 *
 * @param[in] subsystem_id	subsystem ID
 * @return			nothing
 */
_DAI_SYSTEM_INLINE
void dai_set_active_subsystem(vied_subsystem_t subsystem_id);

#endif  /* DAI_SYSTEM_H_ */
