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

/**
 * @file
 * @brief device address and routing functions
 *
 * The DAI functions provide system- and viewpoint-independent access to devices
 * and enable the use to move data from and to these devices.
 *
 */

#ifndef DAI_DEVICE_ACCESS_H_
#define DAI_DEVICE_ACCESS_H_

#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#include "dai_device_access_event_queue.h"
#include "dai_device_access_external_config.h"
#include "dai_device_access_external_data.h"
#include "dai_device_access_internal_config.h"
#include "dai_device_access_internal_data.h"
#include "dai_device_access_types.h"
#include "dai_types.h"

#include <vied/vied_subsystem_access_types.h>

/** @cond DOXYGEN_SKIP_MACROS */
#ifndef _DAI_DEVICE_ACCESS_INLINE
#if defined(CFG_DAI_DEVICE_ACCESS_LIB_IMPL)
#define _DAI_DEVICE_ACCESS_INLINE
#else
#define _DAI_DEVICE_ACCESS_INLINE static inline
#endif
#endif
/** @endcond */

/**
 * @defgroup device_access DAI - routing interface
 *
 * @addtogroup device_access
 * @{
 */

/* Device access handle */
/**
 * @brief	get device handle
 *
 *		returns a device handle for the given device.
 *
 * @param[in] type	device type ID
 * @param[in] device	device ID
 * @return		device handle
 *
 */
_DAI_DEVICE_ACCESS_INLINE
const dai_device_t *_dai_device_get_handle(dai_type_id_t type,
					   dai_device_id_t device);

/* Device addressing */
/**
 * @brief	get route to a target device
 *
 * 		To access a specific device the internal route to that device
 *		is needed.
 *		The internal route is the information that is needed to access
 *		some slave port of some device from the current point of view.
 *
 * 		The route has the type dai_device_route_t and is returned by
 *		this function.
 *		The target device is specified by the type and instance ID.
 *		The third argument is the port ID of the target slave port.
 *
 * @param[in] type	target device type ID
 * @param[in] device	target device instance ID
 * @param[in] port	target device slave port ID
 * @return		device route
 *
 */
_DAI_DEVICE_ACCESS_INLINE
const dai_device_route_t *dai_device_get_route(dai_type_id_t type,
					       dai_device_id_t device,
					       dai_device_port_id_t port);

/**
 * @}
 */

/**
 * @defgroup device_access_simple DAI - simple device access
 *
 * @addtogroup device_access_simple
 * @{
 */
/**
 * @brief	stores 8 bit value in device
 *
 *		device store for 8 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @param[in] data	data to store
 * @return              nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_8(const dai_device_route_t *route,
			dai_address_t address, uint8_t data);

/**
 * @brief	stores 16 bit value in device
 *
 *		device store for 16 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_16(const dai_device_route_t *route,
			 dai_address_t address, uint16_t data);

/**
 * @brief	stores 32 bit value in device
 *
 *		device store for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_32(const dai_device_route_t *route,
			 dai_address_t address, uint32_t data);

/**
 * @brief	stores data in device
 *
 *		stores byte array of variable length in device
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @param[in] data	pointer to source memory buffer
 * @param[in] size	size of source memory buffer
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_store(const dai_device_route_t *route,
		      dai_address_t address, const void *data, unsigned int size);

/**
 * @brief	loads 8 bit value from device
 *
 *		device load for 8 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint8_t dai_device_load_8(const dai_device_route_t *route,
			  dai_address_t address);

/**
 * @brief	loads 16 bit value from device
 *
 *		device load for 16 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint16_t dai_device_load_16(const dai_device_route_t *route,
			    dai_address_t address);

/**
 * @brief	loads 32 bit value from device
 *
 *		device load for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_load_32(const dai_device_route_t *route,
			    dai_address_t address);

/**
 * @brief	loads data from device
 *
 *		loads byte array of variable length from device
 *
 * @param[in] route	route to target device
 * @param[in] address	address relative to base address
 * @param[in] data	pointer to target memory buffer
 * @param[in] size	size of target memory buffer
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_load(const dai_device_route_t *route,
		     dai_address_t address, void *data, unsigned int size);

/**
 * @}
 */

/**
 * @defgroup device_access_register DAI - register access
 *
 * @addtogroup device_access_register
 * @{
 */

/**
 * @brief	stores 32 bit value in register
 *
 *		register store for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] reg	register ID
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_reg_store_32(const dai_device_route_t *route,
			     dai_device_reg_id_t reg, uint32_t data);

/**
 * @brief	loads 32 bit value from register
 *
 *		register load for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] reg	register ID
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_reg_load_32(const dai_device_route_t *route,
				dai_device_reg_id_t reg);

/**
 * @brief	stores 32 bit value in register
 *
 *		register store to register bank for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] reg	register ID
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_reg_bank_store_32(const dai_device_route_t *route,
				  dai_device_regbank_id_t bank, int bank_index,
				  dai_device_reg_id_t reg, uint32_t data);

/**
 * @brief	loads 32 bit value from register
 *
 *		register load from register bank for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] reg	register ID
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_reg_bank_load_32(const dai_device_route_t *route,
				     dai_device_regbank_id_t bank, int bank_index,
				     dai_device_reg_id_t reg);

/**
 * @}
 */

/**
 * @defgroup device_access_memory DAI - memory access
 *
 * @addtogroup device_access_memory
 * @{
 */

/**
 * @brief	stores 8 bit value in device memory
 *
 *		device memory store for 8 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @param[in] data	data to store
 * @return              nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_store_8(const dai_device_route_t *route,
			    dai_device_memory_id_t mem, dai_address_t mem_addr,
			    uint8_t data);

/**
 * @brief	stores 16 bit value in device memory
 *
 *		device memory store for 16 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_store_16(const dai_device_route_t *route,
			     dai_device_memory_id_t mem, dai_address_t mem_addr,
			     uint16_t data);

/**
 * @brief	stores 32 bit value in device memory
 *
 *		device memory store for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @param[in] data	data to store
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_store_32(const dai_device_route_t *route,
			     dai_device_memory_id_t mem, dai_address_t mem_addr,
			     uint32_t data);

/**
 * @brief	stores data in device memory
 *
 *		stores byte array of variable length in device memory
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @param[in] data	pointer to source memory buffer
 * @param[in] size	size of source memory buffer
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_store(const dai_device_route_t *route,
			  dai_device_memory_id_t mem, dai_address_t mem_addr,
			  const void *data, unsigned int size);

/**
 * @brief	loads 8 bit value from device memory
 *
 *		device memory load for 8 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint8_t dai_device_mem_load_8(const dai_device_route_t *route,
			      dai_device_memory_id_t mem, dai_address_t mem_addr);

/**
 * @brief	loads 16 bit value from device memory
 *
 *		device memory load for 16 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint16_t dai_device_mem_load_16(const dai_device_route_t *route,
				dai_device_memory_id_t mem, dai_address_t mem_addr);

/**
 * @brief	loads 32 bit value from device memory
 *
 *		device memory load for 32 bit value
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @return		loaded value
 */
_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_mem_load_32(const dai_device_route_t *route,
				dai_device_memory_id_t mem, dai_address_t mem_addr);

/**
 * @brief	loads data from device memory
 *
 *		loads byte array of variable length from device memory
 *
 * @param[in] route	route to target device
 * @param[in] mem	memory ID
 * @param[in] mem_addr	address relative to base address of memory
 * @param[in] data	pointer to target memory buffer
 * @param[in] size	size of target memory buffer
 * @return		nothing
 */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_load(const dai_device_route_t *route,
			 dai_device_memory_id_t mem, dai_address_t mem_addr,
			 void *data, unsigned int size);

#if defined(CFG_DAI_DEVICE_ACCESS_INLINE_IMPL)
#undef _DAI_DEVICE_ACCESS_INLINE
#endif

/**
 * @}
 */

/** @cond DOXYGEN_SKIP_MACROS */
#if !defined(CFG_DAI_DEVICE_ACCESS_LIB_IMPL)
#include "dai_device_access_impl.h"
#endif
/** @endcond */

#endif  /* DAI_DEVICE_ACCESS_H_ */
