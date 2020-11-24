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
 * @brief Subsystem access, this is the layer below the DAI
 */

#ifndef VIED_SUBSYSTEM_ACCESS_H_
#define VIED_SUBSYSTEM_ACCESS_H_

#include <stdint.h>
#include "vied_config.h"
#include "vied_subsystem_access_types.h"

/** @cond DOXYGEN_SKIP_MACROS */
#if defined(CFG_VIED_SUBSYSTEM_ACCESS_LIB_IMPL)
#define _VIED_SUBSYSTEM_ACCESS_INLINE
#else
#ifndef __HIVECC
#define _VIED_SUBSYSTEM_ACCESS_INLINE
#else
#define _VIED_SUBSYSTEM_ACCESS_INLINE static inline
#include "vied_subsystem_access_impl.h"
#endif
#endif
/** @endcond */

/**
 * @brief	stores 8 bit value in subsystem
 *
 *		subsystem store for 8 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @param[in] data	data to store
 * @return              nothing
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
void vied_subsystem_store_8(vied_subsystem_t dev,
			    vied_subsystem_address_t addr, uint8_t  data);

/**
 * @brief	stores 16 bit value in subsystem
 *
 *		subsystem store for 16 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @param[in] data	data to store
 * @return		nothing
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
void vied_subsystem_store_16(vied_subsystem_t dev,
			     vied_subsystem_address_t addr, uint16_t data);

/**
 * @brief	stores 32 bit value in subsystem
 *
 *		subsystem store for 32 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @param[in] data	data to store
 * @return		nothing
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
void vied_subsystem_store_32(vied_subsystem_t dev,
			     vied_subsystem_address_t addr, uint32_t data);

/**
 * @brief	stores data in subsystem
 *
 *		stores byte array of variable length in subsystem
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @param[in] data	pointer to source memory buffer
 * @param[in] size	size of source memory buffer
 * @return		nothing
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
void vied_subsystem_store(vied_subsystem_t dev,
			  vied_subsystem_address_t addr,
			  const void *data, unsigned int size);

/**
 * @brief	loads 8 bit value from subsystem
 *
 *		subsystem load for 8 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @return		loaded value
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
uint8_t vied_subsystem_load_8(vied_subsystem_t dev,
			      vied_subsystem_address_t addr);

/**
 * @brief	loads 16 bit value from subsystem
 *
 *		subsystem load for 16 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @return		loaded value
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
uint16_t vied_subsystem_load_16(vied_subsystem_t dev,
				vied_subsystem_address_t addr);

/**
 * @brief	loads 32 bit value from subsystem
 *
 *		subsystem load for 32 bit value
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @return		loaded value
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
uint32_t vied_subsystem_load_32(vied_subsystem_t dev,
				vied_subsystem_address_t addr);

/**
 * @brief	loads data from subsystem
 *
 *		loads byte array of variable length from subsystem
 *
 * @param[in] dev	subsystem handle
 * @param[in] addr	address relative to base address
 * @param[in] data	pointer to target memory buffer
 * @param[in] size	size of target memory buffer
 * @return		nothing
 */
_VIED_SUBSYSTEM_ACCESS_INLINE
void vied_subsystem_load(vied_subsystem_t dev,
			 vied_subsystem_address_t addr,
			 void *data, unsigned int size);

#endif  /* VIED_SUBSYSTEM_ACCESS_H_ */
