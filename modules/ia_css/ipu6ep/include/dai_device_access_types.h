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
 * @brief type definitions for device address and routing functions
 *
 * 	This file provides necessary DAI type definitions for device access.
 */

#ifndef DAI_DEVICE_ACCESS_TYPES_H_
#define DAI_DEVICE_ACCESS_TYPES_H_

#include "dai_types.h"
#include <vied/vied_subsystem_access_types.h>

#define INVALID_ADDRESS 0xFFFFFFFF      	//!< address used for non-existing connections

typedef unsigned int   dai_type_id_t;		//!< type ID. Values provided as enum in system introspect
typedef unsigned int   dai_device_id_t;		//!< device ID. Values provided as enum in system introspect

typedef unsigned int   dai_device_port_id_t;	//!< device port ID. Values provided as enum in system introspect
typedef unsigned int   dai_device_regbank_id_t;	//!< register bank ID. Values provided as enum in system introspect
typedef unsigned int   dai_device_reg_id_t;	//!< register ID. Values provided as enum in system introspect
typedef unsigned int   dai_device_memory_id_t;	//!< memory ID. Values provided as enum in system introspect

typedef struct dai_register_bank_s {
	dai_address_t        bank_address;      //!< pointer to register address map for hierarchical register access
	const int            bank_stride;       //!< pointer to register stride map for hierarchical register access
} dai_register_bank_t;

/**
 * @brief Type for the internal routes of a device
 *
 * 	This type defines the internal route that is used when a device A needs
 * 	to access a device B and device A is the device where the current program is
 * 	executed.
 *
 * 	Example: The HiveProcessor accesses a memory.
 */
typedef struct dai_internal_route_s {
	dai_master_port_id_t        master_port;	//!< master port ID
	dai_address_t               slave_address;	//!< address from master to slave
	const dai_register_bank_t   *bank_address_map;	//!< pointer to register banks for hierarchical register access
	const int                   *address_map;	//!< pointer to register map behind the target slave port
} dai_internal_route_t;

typedef struct dai_internal_route_s dai_device_route_t;	//!< Type definition to the internal route struct
typedef struct dai_internal_route_s dai_device_t;	//!< Type definition for the device handle

#endif  /* DAI_DEVICE_ACCESS_TYPES_H_ */
