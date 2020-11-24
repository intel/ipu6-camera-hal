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

#ifndef __NCI_CIO2STR_PRIVATE_TYPES_H
#define __NCI_CIO2STR_PRIVATE_TYPES_H

#include "nci_cio2str_types.h"
#include "type_support.h"

/*
 * Register map from one example Cio2Str instance.
 * This nci module needs an update/versioning in case device address
 * map goes through a change!
-------------------------------------------------------------------
-- Device ipu_isl_ctrl_ps_top_paf_cio2str : registers accessible from port sl_cfg
--Address	--	Rd only		Register_name
--------------------------------------------------------------------
    36F400 		 0 		register : sl_addr_mask_reg0
    36F404 		 0 		register : sl_addr_cmpv_reg0
    36F408 		 1 		register : sl_vec_rcv_cnt_reg0
    36F40C 		 1 		register : sl_vec_valid_reg0
    36F410 		 1 		register : str_out_cnt_reg0
    36F414 		 1 		register : str_status_reg0
    36F418 		 0 		register : sl_addr_mask_reg1
    36F41C 		 0 		register : sl_addr_cmpv_reg1
    36F420 		 1 		register : sl_vec_rcv_cnt_reg1
    36F424 		 1 		register : sl_vec_valid_reg1
    36F428 		 1 		register : str_out_cnt_reg1
    36F42C 		 1 		register : str_status_reg1
    36F430 		 1 		register : sl_vec_status_reg
*/
#define NCI_CIO2STR_SECTION0_OFFSET	(0)
#define NCI_CIO2STR_SECTION1_OFFSET	(0x18)

struct nci_cio2str_private_cfg_section0 {
	struct nci_cio2str_port_config port;
};

struct nci_cio2str_private_cfg_section1 {
	struct nci_cio2str_port_config port;
};

#endif /* __NCI_CIO2STR_PRIVATE_TYPES_H */
