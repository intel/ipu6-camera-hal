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
#ifndef _bbpolyphase_scaler_properties_types_h_
#define _bbpolyphase_scaler_properties_types_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
/* type definition for a struct that holds lists of all properties for devices of type 'CustomDeviceType bbpolyphase_scaler (0 instances, 1 derived types)' */
struct dai_dev_bbpolyphase_scaler_property_s {
	const dai_property_value8_t * const cmd_bits;
	const dai_property_value8_t * const cmd_buffer_bits;
	const dai_property_value8_t * const cmd_buffer_idx;
	const dai_property_value8_t * const cmd_cmd_bits;
	const dai_property_value8_t * const cmd_cmd_idx;
	const dai_property_value8_t * const cmd_fifo_lat;
	const dai_property_value8_t * const cmd_fifo_size;
	const dai_property_value8_t * const cmd_height_bits;
	const dai_property_value8_t * const cmd_height_idx;
	const dai_property_value8_t * const cmd_pid_bits;
	const dai_property_value8_t * const cmd_pid_idx;
	const dai_property_value8_t * const cmd_pin_bits;
	const dai_property_value8_t * const cmd_pin_idx;
	const dai_property_value8_t * const cmd_sid_bits;
	const dai_property_value8_t * const cmd_sid_idx;
	const dai_property_value8_t * const cmd_width_bits;
	const dai_property_value8_t * const cmd_width_idx;
	const dai_property_value8_t * const context_port_id;
	const dai_property_value8_t * const control_port_id;
	const dai_property_value8_t * const elements_per_vector;
	const dai_property_value8_t * const implementation;
	const dai_property_value8_t * const input_data_bpp;
	const dai_property_value8_t * const input_port_id;
	const dai_property_value8_t * const inter_data_bpp;
	const dai_property_value8_t * const inter_port_id;
	const dai_property_value8_t * const max_bits_per_coef;
	const dai_property_value8_t * const num_phases;
	const dai_property_value8_t * const num_pins;
	const dai_property_value8_t * const num_polyphase_taps;
	const dai_property_value8_t * const out_fifo_lat;
	const dai_property_value8_t * const output_data_bpp;
	const dai_property_value8_t * const output_port_id;
	const dai_property_value8_t * const * const pin_params;
	const dai_property_value8_t * const pixels_per_cycle;
	const dai_property_value8_t * const protocol_data_width;
};
typedef struct dai_dev_bbpolyphase_scaler_property_s dai_dev_bbpolyphase_scaler_property_t;

#endif /* _bbpolyphase_scaler_properties_types_h_ */
