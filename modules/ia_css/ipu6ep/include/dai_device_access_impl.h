/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2020 Intel Corporation.
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

#ifndef HOST_DAI_DEVICE_ACCESS_IMPL_H_
#define HOST_DAI_DEVICE_ACCESS_IMPL_H_

#include <dai/dai_assert.h>
#ifdef ICG_OS_ASSERT
#include <dai/dai_device_property.h>
#endif
#include <dai/dai_system.h>

#include <vied/vied_subsystem_access.h>

/* Device access handle */
_DAI_DEVICE_ACCESS_INLINE
const dai_device_t *_dai_device_get_handle(dai_type_id_t type,
					   dai_device_id_t device)
{
#ifdef ICG_OS_ASSERT
	dai_assert(type < dai_device_get_num_types());
	dai_assert(device < dai_device_get_num_instances(type));
#endif
	return (dai_device_t*) dai_system_get_address_table()[type][device];
}

/* Device addressing */
_DAI_DEVICE_ACCESS_INLINE
const dai_device_route_t *
dai_device_get_route(dai_type_id_t type,
		     dai_device_id_t device,
		     dai_device_port_id_t port)
{
#ifdef ICG_OS_ASSERT
	dai_assert(type < dai_device_get_num_types());
	dai_assert(device < dai_device_get_num_instances(type));
	dai_assert(port < dai_device_get_num_slave_ports(type, device));
#endif
	const dai_internal_route_t *routes = _dai_device_get_handle(type, device);
#ifdef ICG_OS_ASSERT
	dai_assert(INVALID_ADDRESS != routes[port].slave_address);
#endif
	return (const dai_device_route_t *) &routes[port];
}

/* Simple device access */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_8(const dai_device_route_t *route,
			dai_address_t address, uint8_t data)
{
	address = _dai_get_address(route, address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_store_8 to 0x%x: %u\n",
	       address, data);
#endif
	vied_subsystem_store_8(dai_get_active_subsystem(),
			       address, data);
}

_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_16(const dai_device_route_t *route,
			 dai_address_t address, uint16_t data)
{
	address = _dai_get_address(route, address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_store_16 to 0x%x: %u\n",
	       address, data);
#endif
	vied_subsystem_store_16(dai_get_active_subsystem(),
				address, data);
}

_DAI_DEVICE_ACCESS_INLINE
void dai_device_store_32(const dai_device_route_t *route,
			 dai_address_t address, uint32_t data)
{
	address = _dai_get_address(route, address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_store_32 to 0x%x: %u\n",
	       address, data);
#endif
	vied_subsystem_store_32(dai_get_active_subsystem(),
				address, data);
}

_DAI_DEVICE_ACCESS_INLINE
void dai_device_store(const dai_device_route_t *route,
		      dai_address_t address,
		      const void *data, unsigned int size)
{
	address = _dai_get_address(route, address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_store to 0x%x from buffer at %p with size %u\n",
	       address, data, size);
#endif
	vied_subsystem_store(dai_get_active_subsystem(),
			     address, data, size);
}

_DAI_DEVICE_ACCESS_INLINE
uint8_t dai_device_load_8(const dai_device_route_t *route,
			  dai_address_t address)
{
	uint8_t data;
	address = _dai_get_address(route, address);
	data = vied_subsystem_load_8(dai_get_active_subsystem(),
				     address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_load_8 from 0x%x: %u\n",
	       address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE
uint16_t dai_device_load_16(const dai_device_route_t *route,
			    dai_address_t address)
{
	uint16_t data;
	address = _dai_get_address(route, address);
	data = vied_subsystem_load_16(dai_get_active_subsystem(),
				      address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_load_16 from 0x%x: %u\n",
	       address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_load_32(const dai_device_route_t *route,
			    dai_address_t address)
{
	uint32_t data;
	address = _dai_get_address(route, address);
	data = vied_subsystem_load_32(dai_get_active_subsystem(),
				      address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_load_32 from 0x%x: %u\n",
	       address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE
void dai_device_load(const dai_device_route_t *route,
		     dai_address_t address,
		     void *data, unsigned int size)
{
	address = _dai_get_address(route, address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_load from 0x%x to buffer at %p with size %u\n",
	       address, data, size);
#endif
	vied_subsystem_load(dai_get_active_subsystem(),
			    address, data, size);
}

/* Device register access */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_reg_bank_store_32(const dai_device_route_t *route,
				  dai_device_regbank_id_t bank, int bank_index,
				  dai_device_reg_id_t reg, uint32_t data)
{
	dai_address_t address = _dai_get_address_in_bank(route, bank, bank_index, reg);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_reg_store_bank_32 to 0x%x: %u\n", address, data);
#endif
	vied_subsystem_store_32(dai_get_active_subsystem(), address, data);
}

_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_reg_bank_load_32(const dai_device_route_t *route,
				     dai_device_regbank_id_t bank, int bank_index,
				     dai_device_reg_id_t reg)
{
	uint32_t data;
	dai_address_t address = _dai_get_address_in_bank(route,  bank, bank_index, reg);
	data = vied_subsystem_load_32(dai_get_active_subsystem(), address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_reg_load_bank_32 from 0x%x: %u\n", address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE
void dai_device_reg_store_32(const dai_device_route_t *route,
			     dai_device_reg_id_t reg, uint32_t data)
{
	dai_address_t address = _dai_get_register_address(route, reg);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_reg_store_32 to 0x%x: %u\n", address, data);
#endif
	vied_subsystem_store_32(dai_get_active_subsystem(), address, data);
}

_DAI_DEVICE_ACCESS_INLINE
uint32_t dai_device_reg_load_32(const dai_device_route_t *route,
				dai_device_reg_id_t reg)
{
	uint32_t data;
	dai_address_t address = _dai_get_register_address(route, reg);
	data = vied_subsystem_load_32(dai_get_active_subsystem(), address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_reg_load_32 from 0x%x: %u\n", address, data);
#endif
	return data;
}

/* Device memory access */
_DAI_DEVICE_ACCESS_INLINE
void dai_device_mem_store_8(const dai_device_route_t *route,
			    dai_device_memory_id_t mem, dai_address_t mem_addr,
			    uint8_t data)
{
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_store_8 to 0x%x: %u\n", address, data);
#endif
	vied_subsystem_store_8(dai_get_active_subsystem(), address, data);
}

_DAI_DEVICE_ACCESS_INLINE void
dai_device_mem_store_16(const dai_device_route_t *route,
			dai_device_memory_id_t mem, dai_address_t mem_addr,
			uint16_t data)
{
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_store_16 to 0x%x: %u\n", address, data);
#endif
	vied_subsystem_store_16(dai_get_active_subsystem(), address, data);
}

_DAI_DEVICE_ACCESS_INLINE void
dai_device_mem_store_32(const dai_device_route_t *route,
			dai_device_memory_id_t mem, dai_address_t mem_addr,
			uint32_t data)
{
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_store_32 to 0x%x: %u\n", address, data);
#endif
	vied_subsystem_store_32(dai_get_active_subsystem(), address, data);
}

_DAI_DEVICE_ACCESS_INLINE void
dai_device_mem_store(const dai_device_route_t *route,
		     dai_device_memory_id_t mem, dai_address_t mem_addr,
		     const void *data, unsigned int size)
{
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_store to 0x%x from buffer at %p with size %u\n",
	       address, data, size);
#endif
	vied_subsystem_store(dai_get_active_subsystem(), address, data, size);
}

_DAI_DEVICE_ACCESS_INLINE uint8_t
dai_device_mem_load_8(const dai_device_route_t *route,
		      dai_device_memory_id_t mem, dai_address_t mem_addr)
{
	uint8_t data;
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
	data = vied_subsystem_load_8(dai_get_active_subsystem(), address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_load_8 from 0x%x: %u\n", address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE uint16_t
dai_device_mem_load_16(const dai_device_route_t *route,
		       dai_device_memory_id_t mem, dai_address_t mem_addr)
{
	uint16_t data;
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
	data = vied_subsystem_load_16(dai_get_active_subsystem(), address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_load_16 from 0x%x: %u\n", address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE uint32_t
dai_device_mem_load_32(const dai_device_route_t *route,
		       dai_device_memory_id_t mem, dai_address_t mem_addr)
{
	uint32_t data;
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
	data = vied_subsystem_load_32(dai_get_active_subsystem(), address);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_load_32 from 0x%x: %u\n", address, data);
#endif
	return data;
}

_DAI_DEVICE_ACCESS_INLINE void
dai_device_mem_load(const dai_device_route_t *route,
		    dai_device_memory_id_t mem, dai_address_t mem_addr,
		    void *data, unsigned int size)
{
	dai_address_t address = _dai_get_memory_address(route, mem, mem_addr);
#ifdef ICG_OS_DEBUG
	printf("host: dai_device_mem_load from 0x%x to buffer at %p with size %u\n",
	       address, data, size);
#endif
	vied_subsystem_load(dai_get_active_subsystem(), address, data, size);
}

#endif  /* HOST_DAI_DEVICE_ACCESS_IMPL_H_ */
