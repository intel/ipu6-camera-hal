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
#ifndef _dfm_properties_h_
#define _dfm_properties_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_assert.h>
#include <dai/dai_device_access_types.h>
#include <dai/dai_device_info_types.h>
#include <dai/dai_subsystem_property.h>
#include "dai_ipu_top_devices.h"
#include "device_property_types.h"
#include "dfm_properties_types.h"
/* returns a reference to a list of property structs using device ID and the global get_properties function */
static inline const dai_dev_dfm_property_t *
_dai_dfm_get_properties(void)
{
	return dai_subsystem_get_properties()->device_user_properties->all_dfm_properties;
}

/* get property implementation for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_implementation(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->implementation[dev];
}

/* get property mst_cio_cs_run for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_mst_cio_cs_run(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->mst_cio_cs_run[dev];
}

/* get property native_cio2_dblock for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_native_cio2_dblock(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->native_cio2_dblock[dev];
}

/* get property native_cioraccept for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_native_cioraccept(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->native_cioraccept[dev];
}

/* get property native_ciosrmd for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_native_ciosrmd(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->native_ciosrmd[dev];
}

/* get property not_translate2_dblock for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_not_translate2_dblock(dai_device_id_t dev, int index)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->not_translate2_dblock[dev][index];
}

/* get property not_translate_srmd for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_not_translate_srmd(dai_device_id_t dev, int index)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->not_translate_srmd[dev][index];
}

/* get property p_cmd_bank_entr_begin_15_8 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_15_8(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_15_8[dev];
}

/* get property p_cmd_bank_entr_begin_23_16 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_23_16(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_23_16[dev];
}

/* get property p_cmd_bank_entr_begin_31_24 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_31_24(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_31_24[dev];
}

/* get property p_cmd_bank_entr_begin_39_32 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_39_32(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_39_32[dev];
}

/* get property p_cmd_bank_entr_begin_47_40 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_47_40(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_47_40[dev];
}

/* get property p_cmd_bank_entr_begin_55_48 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_55_48(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_55_48[dev];
}

/* get property p_cmd_bank_entr_begin_63_56 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_63_56(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_63_56[dev];
}

/* get property p_cmd_bank_entr_begin_7_0 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_begin_7_0(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_begin_7_0[dev];
}

/* get property p_cmd_bank_entr_end_15_8 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_15_8(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_15_8[dev];
}

/* get property p_cmd_bank_entr_end_23_16 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_23_16(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_23_16[dev];
}

/* get property p_cmd_bank_entr_end_31_24 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_31_24(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_31_24[dev];
}

/* get property p_cmd_bank_entr_end_39_32 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_39_32(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_39_32[dev];
}

/* get property p_cmd_bank_entr_end_47_40 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_47_40(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_47_40[dev];
}

/* get property p_cmd_bank_entr_end_55_48 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_55_48(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_55_48[dev];
}

/* get property p_cmd_bank_entr_end_63_56 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_63_56(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_63_56[dev];
}

/* get property p_cmd_bank_entr_end_7_0 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_end_7_0(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_end_7_0[dev];
}

/* get property p_cmd_bank_entr_middle_15_8 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_15_8(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_15_8[dev];
}

/* get property p_cmd_bank_entr_middle_23_16 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_23_16(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_23_16[dev];
}

/* get property p_cmd_bank_entr_middle_31_24 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_31_24(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_31_24[dev];
}

/* get property p_cmd_bank_entr_middle_39_32 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_39_32(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_39_32[dev];
}

/* get property p_cmd_bank_entr_middle_47_40 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_47_40(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_47_40[dev];
}

/* get property p_cmd_bank_entr_middle_55_48 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_55_48(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_55_48[dev];
}

/* get property p_cmd_bank_entr_middle_63_56 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_63_56(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_63_56[dev];
}

/* get property p_cmd_bank_entr_middle_7_0 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_cmd_bank_entr_middle_7_0(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_cmd_bank_entr_middle_7_0[dev];
}

/* get property p_dev_port_pairs_num for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dev_port_pairs_num(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dev_port_pairs_num[dev];
}

/* get property p_dp_en_0 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_0(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_0[dev];
}

/* get property p_dp_en_1 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_1(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_1[dev];
}

/* get property p_dp_en_10 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_10(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_10[dev];
}

/* get property p_dp_en_11 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_11(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_11[dev];
}

/* get property p_dp_en_12 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_12(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_12[dev];
}

/* get property p_dp_en_13 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_13(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_13[dev];
}

/* get property p_dp_en_14 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_14(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_14[dev];
}

/* get property p_dp_en_15 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_15(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_15[dev];
}

/* get property p_dp_en_16 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_16(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_16[dev];
}

/* get property p_dp_en_17 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_17(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_17[dev];
}

/* get property p_dp_en_18 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_18(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_18[dev];
}

/* get property p_dp_en_19 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_19(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_19[dev];
}

/* get property p_dp_en_2 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_2(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_2[dev];
}

/* get property p_dp_en_20 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_20(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_20[dev];
}

/* get property p_dp_en_21 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_21(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_21[dev];
}

/* get property p_dp_en_22 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_22(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_22[dev];
}

/* get property p_dp_en_23 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_23(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_23[dev];
}

/* get property p_dp_en_24 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_24(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_24[dev];
}

/* get property p_dp_en_25 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_25(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_25[dev];
}

/* get property p_dp_en_26 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_26(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_26[dev];
}

/* get property p_dp_en_27 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_27(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_27[dev];
}

/* get property p_dp_en_28 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_28(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_28[dev];
}

/* get property p_dp_en_29 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_29(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_29[dev];
}

/* get property p_dp_en_3 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_3(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_3[dev];
}

/* get property p_dp_en_30 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_30(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_30[dev];
}

/* get property p_dp_en_31 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_31(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_31[dev];
}

/* get property p_dp_en_4 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_4(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_4[dev];
}

/* get property p_dp_en_5 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_5(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_5[dev];
}

/* get property p_dp_en_6 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_6(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_6[dev];
}

/* get property p_dp_en_7 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_7(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_7[dev];
}

/* get property p_dp_en_8 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_8(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_8[dev];
}

/* get property p_dp_en_9 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_dp_en_9(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_dp_en_9[dev];
}

/* get property p_rf_depth for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_rf_depth(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_rf_depth[dev];
}

/* get property p_sdf_reg_en_0 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_0(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_0[dev];
}

/* get property p_sdf_reg_en_1 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_1(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_1[dev];
}

/* get property p_sdf_reg_en_10 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_10(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_10[dev];
}

/* get property p_sdf_reg_en_11 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_11(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_11[dev];
}

/* get property p_sdf_reg_en_12 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_12(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_12[dev];
}

/* get property p_sdf_reg_en_13 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_13(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_13[dev];
}

/* get property p_sdf_reg_en_14 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_14(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_14[dev];
}

/* get property p_sdf_reg_en_15 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_15(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_15[dev];
}

/* get property p_sdf_reg_en_2 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_2(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_2[dev];
}

/* get property p_sdf_reg_en_3 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_3(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_3[dev];
}

/* get property p_sdf_reg_en_4 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_4(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_4[dev];
}

/* get property p_sdf_reg_en_5 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_5(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_5[dev];
}

/* get property p_sdf_reg_en_6 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_6(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_6[dev];
}

/* get property p_sdf_reg_en_7 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_7(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_7[dev];
}

/* get property p_sdf_reg_en_8 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_8(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_8[dev];
}

/* get property p_sdf_reg_en_9 for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_reg_en_9(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_reg_en_9[dev];
}

/* get property p_sdf_regs_num for a device of type dfm */
static inline dai_property_value_t
dai_dfm_get_p_sdf_regs_num(dai_device_id_t dev)
{
	dai_assert(dev < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return _dai_dfm_get_properties()->p_sdf_regs_num[dev];
}

#endif /* _dfm_properties_h_ */
