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
#ifndef _dvs_controller_properties_types_h_
#define _dvs_controller_properties_types_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
/* type definition for a struct that holds lists of all properties for devices of type 'CustomDeviceType dvs_controller (0 instances, 1 derived types)' */
struct dai_dev_dvs_controller_property_s {
	const dai_property_value8_t * const _hwp_level_num;
	const dai_property_value8_t * const implementation;
};
typedef struct dai_dev_dvs_controller_property_s dai_dev_dvs_controller_property_t;

#endif /* _dvs_controller_properties_types_h_ */
