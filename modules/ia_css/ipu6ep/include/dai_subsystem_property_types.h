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

#ifndef DAI_SUBSYSTEM_PROPERTY_TYPES_H_
#define DAI_SUBSYSTEM_PROPERTY_TYPES_H_

#include <dai/dai_device_info_types.h>

/**
 * \brief a struct containing the subsystem properties
 *
 *	This struct contains references to the common and user properties of a
 *	subsystem.
 */
typedef struct dai_subsystem_properties_s {
	const unsigned int device_num_types;				//!< number of device types
	const unsigned int * const device_num_instances;		//!< reference to a list with number of instances per type
	const unsigned int * const * const device_num_slave_ports;	//!< reference to a list with number of slave ports per type and device instance
	const unsigned int * const * const device_num_master_ports;	//!< reference to a list with number of master ports per type and device instance
	const unsigned int * const * const device_num_memories;		//!< reference to a list with number of memories per type and device instance
	const unsigned int * const device_num_registerbanks;		//!< reference to a list with number of registerbank types per device type
	const unsigned int * const * const device_num_registers;	//!< reference to a list with number of registers per type and registerbank types
	const struct dai_device_user_properties_s * const device_user_properties;	//!< reference to user properties
} dai_subsystem_properties_t;

#endif  /* DAI_SUBSYSTEM_PROPERTY_TYPES_H_ */
