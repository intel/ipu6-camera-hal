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
#ifndef _dfm_properties_types_h_
#define _dfm_properties_types_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_info_types.h>
/* type definition for a struct that holds lists of all properties for devices of type 'CustomDeviceType dfm (0 instances, 3 derived types)' */
struct dai_dev_dfm_property_s {
	const dai_property_value8_t * const implementation;
	const dai_property_value8_t * const mst_cio_cs_run;
	const dai_property_value8_t * const native_cio2_dblock;
	const dai_property_value8_t * const native_cioraccept;
	const dai_property_value8_t * const native_ciosrmd;
	const dai_property_value8_t * const * const not_translate2_dblock;
	const dai_property_value8_t * const * const not_translate_srmd;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_15_8;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_23_16;
	const dai_property_value8_t * const p_cmd_bank_entr_begin_31_24;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_39_32;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_47_40;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_55_48;
	const dai_property_value8_t * const p_cmd_bank_entr_begin_63_56;
	const dai_property_value16_t * const p_cmd_bank_entr_begin_7_0;
	const dai_property_value16_t * const p_cmd_bank_entr_end_15_8;
	const dai_property_value16_t * const p_cmd_bank_entr_end_23_16;
	const dai_property_value8_t * const p_cmd_bank_entr_end_31_24;
	const dai_property_value16_t * const p_cmd_bank_entr_end_39_32;
	const dai_property_value16_t * const p_cmd_bank_entr_end_47_40;
	const dai_property_value16_t * const p_cmd_bank_entr_end_55_48;
	const dai_property_value8_t * const p_cmd_bank_entr_end_63_56;
	const dai_property_value16_t * const p_cmd_bank_entr_end_7_0;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_15_8;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_23_16;
	const dai_property_value8_t * const p_cmd_bank_entr_middle_31_24;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_39_32;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_47_40;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_55_48;
	const dai_property_value8_t * const p_cmd_bank_entr_middle_63_56;
	const dai_property_value16_t * const p_cmd_bank_entr_middle_7_0;
	const dai_property_value8_t * const p_dev_port_pairs_num;
	const dai_property_value8_t * const p_dp_en_0;
	const dai_property_value8_t * const p_dp_en_1;
	const dai_property_value8_t * const p_dp_en_10;
	const dai_property_value8_t * const p_dp_en_11;
	const dai_property_value8_t * const p_dp_en_12;
	const dai_property_value8_t * const p_dp_en_13;
	const dai_property_value8_t * const p_dp_en_14;
	const dai_property_value8_t * const p_dp_en_15;
	const dai_property_value8_t * const p_dp_en_16;
	const dai_property_value8_t * const p_dp_en_17;
	const dai_property_value8_t * const p_dp_en_18;
	const dai_property_value8_t * const p_dp_en_19;
	const dai_property_value8_t * const p_dp_en_2;
	const dai_property_value8_t * const p_dp_en_20;
	const dai_property_value8_t * const p_dp_en_21;
	const dai_property_value8_t * const p_dp_en_22;
	const dai_property_value8_t * const p_dp_en_23;
	const dai_property_value8_t * const p_dp_en_24;
	const dai_property_value8_t * const p_dp_en_25;
	const dai_property_value8_t * const p_dp_en_26;
	const dai_property_value8_t * const p_dp_en_27;
	const dai_property_value8_t * const p_dp_en_28;
	const dai_property_value8_t * const p_dp_en_29;
	const dai_property_value8_t * const p_dp_en_3;
	const dai_property_value8_t * const p_dp_en_30;
	const dai_property_value8_t * const p_dp_en_31;
	const dai_property_value8_t * const p_dp_en_4;
	const dai_property_value8_t * const p_dp_en_5;
	const dai_property_value8_t * const p_dp_en_6;
	const dai_property_value8_t * const p_dp_en_7;
	const dai_property_value8_t * const p_dp_en_8;
	const dai_property_value8_t * const p_dp_en_9;
	const dai_property_value16_t * const p_rf_depth;
	const dai_property_value8_t * const p_sdf_reg_en_0;
	const dai_property_value8_t * const p_sdf_reg_en_1;
	const dai_property_value8_t * const p_sdf_reg_en_10;
	const dai_property_value8_t * const p_sdf_reg_en_11;
	const dai_property_value8_t * const p_sdf_reg_en_12;
	const dai_property_value8_t * const p_sdf_reg_en_13;
	const dai_property_value8_t * const p_sdf_reg_en_14;
	const dai_property_value8_t * const p_sdf_reg_en_15;
	const dai_property_value8_t * const p_sdf_reg_en_2;
	const dai_property_value8_t * const p_sdf_reg_en_3;
	const dai_property_value8_t * const p_sdf_reg_en_4;
	const dai_property_value8_t * const p_sdf_reg_en_5;
	const dai_property_value8_t * const p_sdf_reg_en_6;
	const dai_property_value8_t * const p_sdf_reg_en_7;
	const dai_property_value8_t * const p_sdf_reg_en_8;
	const dai_property_value8_t * const p_sdf_reg_en_9;
	const dai_property_value8_t * const p_sdf_regs_num;
};
typedef struct dai_dev_dfm_property_s dai_dev_dfm_property_t;

#endif /* _dfm_properties_types_h_ */
