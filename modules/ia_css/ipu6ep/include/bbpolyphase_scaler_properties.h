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
#ifndef _bbpolyphase_scaler_properties_h_
#define _bbpolyphase_scaler_properties_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_assert.h>
#include <dai/dai_device_access_types.h>
#include <dai/dai_device_info_types.h>
#include <dai/dai_subsystem_property.h>
#include "bbpolyphase_scaler_properties_types.h"
#include "dai_ipu_top_devices.h"
#include "device_property_types.h"
/* returns a reference to a list of property structs using device ID and the global get_properties function */
static inline const dai_dev_bbpolyphase_scaler_property_t *
_dai_bbpolyphase_scaler_get_properties(void)
{
	return dai_subsystem_get_properties()->device_user_properties->all_bbpolyphase_scaler_properties;
}

/* get property cmd_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_bits[dev];
}

/* get property cmd_buffer_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_buffer_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_buffer_bits[dev];
}

/* get property cmd_buffer_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_buffer_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_buffer_idx[dev];
}

/* get property cmd_cmd_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_cmd_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_cmd_bits[dev];
}

/* get property cmd_cmd_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_cmd_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_cmd_idx[dev];
}

/* get property cmd_fifo_lat for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_fifo_lat(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_fifo_lat[dev];
}

/* get property cmd_fifo_size for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_fifo_size(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_fifo_size[dev];
}

/* get property cmd_height_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_height_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_height_bits[dev];
}

/* get property cmd_height_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_height_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_height_idx[dev];
}

/* get property cmd_pid_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_pid_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_pid_bits[dev];
}

/* get property cmd_pid_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_pid_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_pid_idx[dev];
}

/* get property cmd_pin_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_pin_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_pin_bits[dev];
}

/* get property cmd_pin_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_pin_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_pin_idx[dev];
}

/* get property cmd_sid_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_sid_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_sid_bits[dev];
}

/* get property cmd_sid_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_sid_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_sid_idx[dev];
}

/* get property cmd_width_bits for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_width_bits(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_width_bits[dev];
}

/* get property cmd_width_idx for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_cmd_width_idx(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->cmd_width_idx[dev];
}

/* get property context_port_id for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_context_port_id(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->context_port_id[dev];
}

/* get property control_port_id for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_control_port_id(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->control_port_id[dev];
}

/* get property elements_per_vector for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_elements_per_vector(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->elements_per_vector[dev];
}

/* get property implementation for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_implementation(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->implementation[dev];
}

/* get property input_data_bpp for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_input_data_bpp(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->input_data_bpp[dev];
}

/* get property input_port_id for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_input_port_id(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->input_port_id[dev];
}

/* get property inter_data_bpp for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_inter_data_bpp(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->inter_data_bpp[dev];
}

/* get property inter_port_id for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_inter_port_id(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->inter_port_id[dev];
}

/* get property max_bits_per_coef for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_max_bits_per_coef(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->max_bits_per_coef[dev];
}

/* get property num_phases for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_num_phases(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->num_phases[dev];
}

/* get property num_pins for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_num_pins(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->num_pins[dev];
}

/* get property num_polyphase_taps for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_num_polyphase_taps(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->num_polyphase_taps[dev];
}

/* get property out_fifo_lat for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_out_fifo_lat(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->out_fifo_lat[dev];
}

/* get property output_data_bpp for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_output_data_bpp(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->output_data_bpp[dev];
}

/* get property output_port_id for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_output_port_id(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->output_port_id[dev];
}

/* get property pin_params for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_pin_params(dai_device_id_t dev, int index)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->pin_params[dev][index];
}

/* get property pixels_per_cycle for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_pixels_per_cycle(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->pixels_per_cycle[dev];
}

/* get property protocol_data_width for a device of type bbpolyphase_scaler */
static inline dai_property_value_t
dai_bbpolyphase_scaler_get_protocol_data_width(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_BBPOLYPHASE_SCALER_INSTANCES);
	return _dai_bbpolyphase_scaler_get_properties()->protocol_data_width[dev];
}

#endif /* _bbpolyphase_scaler_properties_h_ */
