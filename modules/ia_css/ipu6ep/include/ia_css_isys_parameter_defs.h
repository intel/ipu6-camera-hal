/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2016 Intel Corporation.
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

#ifndef __IA_CSS_ISYS_PARAMETER_DEFS_H
#define __IA_CSS_ISYS_PARAMETER_DEFS_H

#include "ia_css_isys_parameter_defs_dep.h"
#include "ia_css_isa_parameter_defs.h"

/* If in the future differences are found they need to be moved to the dep file */

enum ia_css_isys_exclusive_group_id {
	IA_CSS_ISYS_EXCLUSIVE_GROUP_INVALID,
	IA_CSS_ISYS_EXCLUSIVE_GROUP_BLC,
	IA_CSS_ISYS_EXCLUSIVE_GROUP_LSC,
	/* Add here more exclusive kernel group ID's if they exist */
	N_IA_CSS_ISYS_EXCLUSIVE_GROUP
};

enum ia_css_isys_kernel_state {
	IA_CSS_ISYS_KERNEL_STATE_DISABLED,
	IA_CSS_ISYS_KERNEL_STATE_ENABLED,
	IA_CSS_ISYS_KERNEL_STATE_EXCLUDED,
	N_IA_CSS_ISYS_KERNEL_STATE
};

#endif /* __IA_CSS_ISYS_PARAMETER_DEFS_H */
