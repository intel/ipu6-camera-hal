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
#ifndef _vec_to_str_v3_properties_h_
#define _vec_to_str_v3_properties_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_assert.h>
#include <dai/dai_device_access_types.h>
#include <dai/dai_device_info_types.h>
#include <dai/dai_subsystem_property.h>
#include "dai_ipu_top_devices.h"
#include "device_property_types.h"
#include "vec_to_str_v3_properties_types.h"
/* returns a reference to a list of property structs using device ID and the global get_properties function */
static inline const dai_dev_vec_to_str_v3_property_t *
_dai_vec_to_str_v3_get_properties(void)
{
	return dai_subsystem_get_properties()->device_user_properties->all_vec_to_str_v3_properties;
}

/* get property hwp_ack_cmd_range for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_ack_cmd_range(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_ack_cmd_range[dev];
}

/* get property hwp_ack_fifo_depth for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_ack_fifo_depth(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_ack_fifo_depth[dev];
}

/* get property hwp_cmd_fifo_depth for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_cmd_fifo_depth(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_cmd_fifo_depth[dev];
}

/* get property hwp_nof_bufs for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_nof_bufs(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_nof_bufs[dev];
}

/* get property hwp_nof_strm_comps for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_nof_strm_comps(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_nof_strm_comps[dev];
}

/* get property hwp_pif_output_support for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_pif_output_support(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_pif_output_support[dev];
}

/* get property hwp_strm_comp_w for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_strm_comp_w(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_strm_comp_w[dev];
}

/* get property hwp_strm_fifo_depth for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_strm_fifo_depth(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_strm_fifo_depth[dev];
}

/* get property hwp_vec_comp_w for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_hwp_vec_comp_w(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->hwp_vec_comp_w[dev];
}

/* get property implementation for a device of type vec_to_str_v3 */
static inline dai_property_value_t
dai_vec_to_str_v3_get_implementation(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);
	return _dai_vec_to_str_v3_get_properties()->implementation[dev];
}

#endif /* _vec_to_str_v3_properties_h_ */
