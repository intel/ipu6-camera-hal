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
#ifndef _wpt_properties_types_h_
#define _wpt_properties_types_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
/* type definition for a struct that holds lists of all properties for devices of type 'CustomDeviceType wpt (0 instances, 5 derived types)' */
struct dai_dev_wpt_property_s {
	const dai_property_value8_t * const implementation;
	const dai_property_value8_t * const native_cio2_dblock;
	const dai_property_value8_t * const native_cioraccept;
	const dai_property_value8_t * const native_ciosrmd;
	const dai_property_value8_t * const num_watch_points;
	const dai_property_value8_t * const trace_ddren_bit;
	const dai_property_value8_t * const trace_depth;
	const dai_property_value8_t * const trace_entry_depth;
	const dai_property_value8_t * const trace_npken_bit;
};
typedef struct dai_dev_wpt_property_s dai_dev_wpt_property_t;

#endif /* _wpt_properties_types_h_ */
