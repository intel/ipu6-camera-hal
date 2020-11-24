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
#ifndef _device_property_types_h_
#define _device_property_types_h_

/* Generated with DAI generator 1.3.0 */

/* system to device type property mapping */
struct dai_device_user_properties_s {
	const struct dai_dev_bb_output_formatter_property_s * const all_bb_output_formatter_properties;
	const struct dai_dev_bbgdc4_property_s * const all_bbgdc4_properties;
	const struct dai_dev_bbpolyphase_scaler_property_s * const all_bbpolyphase_scaler_properties;
	const struct dai_dev_bbtnr_property_s * const all_bbtnr_properties;
	const struct dai_dev_cell_property_s * const all_cell_properties;
	const struct dai_dev_dfm_property_s * const all_dfm_properties;
	const struct dai_dev_dvs_controller_property_s * const all_dvs_controller_properties;
	const struct dai_dev_mbr_property_s * const all_mbr_properties;
	const struct dai_dev_vec_to_str_v3_property_s * const all_vec_to_str_v3_properties;
	const struct dai_dev_wpt_property_s * const all_wpt_properties;
};
typedef struct dai_device_user_properties_s dai_device_user_properties_t;

#endif /* _device_property_types_h_ */
