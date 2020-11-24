/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __NCI_PIFCONV_CTRL_PRIVATE_TYPES_H
#define __NCI_PIFCONV_CTRL_PRIVATE_TYPES_H

#include "nci_pifconv_ctrl_types.h"
#include "type_support.h"

/*
 * Register map from one example Pifconv_ctrl instance from SDK.
 * This nci module needs an update/versioning in case device address
 * map goes through a change!
-------------------------------------------------------------------
-- Device ipu_isl_ctrl_ps_top_pif_conv_a : registers accessible from port crq_in
--Address	--	Rd only		Register_name
    36F200 		 0 		reg_PIF_CONVERTER_CTRL_Info
    36F204 		 0 		reg_PIF_CONVERTER_INPUT_DIM_Info
    36F208 		 0 		reg_PIF_CONVERTER_CROP_COL_Info
    36F20C 		 0 		reg_PIF_CONVERTER_CROP_ROW_Info
--------------------------------------------------------------------
*/
#define NCI_PIFCONV_CTRL_SECTION0_OFFSET	0

struct nci_pifconv_ctrl_private_cfg_section0 {
	uint32_t ctrl_info;
	uint32_t input_dim_info;
	uint32_t crop_col_info;
	uint32_t crop_row_info;
};

#endif /* __NCI_PIFCONV_CTRL_PRIVATE_TYPES_H */
