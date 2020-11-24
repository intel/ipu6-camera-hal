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
#ifndef _dai_internal_routes_ipu_top_entry_ext_definitions_h_
#define _dai_internal_routes_ipu_top_entry_ext_definitions_h_

/* Generated with DAI generator 1.3.0 */

#include <dai/dai_device_access_types.h>
#include "dai_internal_routes_ipu_top_entry_impl.h"
#include "device_types.h"
/* Handles to the internal routes per device types */
 const dai_internal_route_t * const * const dai_ipu_top_entry_device_types[NUM_DAI_DEVICE_TYPES] = {
	dai_ipu_top_type_bb_output_formatter_internal_route,
	dai_ipu_top_type_bbgdc4_internal_route,
	dai_ipu_top_type_bbpolyphase_scaler_internal_route,
	dai_ipu_top_type_bbtnr_internal_route,
	dai_ipu_top_type_cell_internal_route,
	dai_ipu_top_type_dfm_internal_route,
	dai_ipu_top_type_dvs_controller_internal_route,
	dai_ipu_top_type_entry_internal_route,
	dai_ipu_top_type_isa_ps_gp_regs_block_internal_route,
	dai_ipu_top_type_isf_cio_switch_info_internal_route,
	dai_ipu_top_type_isf_cio_switch_vc2_info_internal_route,
	dai_ipu_top_type_lbff_infra_gp_regs_block_internal_route,
	dai_ipu_top_type_mbr_internal_route,
	dai_ipu_top_type_psa_1_gp_regs_block_internal_route,
	dai_ipu_top_type_psa_2_gp_regs_block_internal_route,
	dai_ipu_top_type_psa_3_gp_regs_block_internal_route,
	dai_ipu_top_type_tr_cio_switch_info_internal_route,
	dai_ipu_top_type_vec_to_str_v3_internal_route,
	dai_ipu_top_type_wpt_internal_route
};

#endif /* _dai_internal_routes_ipu_top_entry_ext_definitions_h_ */
