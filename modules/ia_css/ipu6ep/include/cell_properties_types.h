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
#ifndef _cell_properties_types_h_
#define _cell_properties_types_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
/* type definition for a struct that holds lists of all properties for devices of type 'CustomDeviceType cell (0 instances, 3 derived types)' */
struct dai_dev_cell_property_s {
	const dai_property_value32_t * const control_register;
	const dai_property_value32_t * const * const first_base_address_register;
	const dai_property_value32_t * const icache_master_port;
	const dai_property_value32_t * const icache_segment_size;
	const dai_property_value32_t * const * const memory_slave_port;
	const dai_property_value8_t * const * const num_base_address_registers;
	const dai_property_value32_t * const * const preferred_master_ports;
	const dai_property_value8_t * const program_memory_width;
	const dai_property_value32_t * const start_register;
	const dai_property_value32_t * const status_control_slave_port;
};
typedef struct dai_dev_cell_property_s dai_dev_cell_property_t;

#endif /* _cell_properties_types_h_ */
