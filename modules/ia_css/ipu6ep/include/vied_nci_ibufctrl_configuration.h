/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2014 - 2016 Intel Corporation.
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

#ifndef __VIED_NCI_IBUFCTRL_CONFIGURATION_H
#define __VIED_NCI_IBUFCTRL_CONFIGURATION_H

#include <ibuf_cntrl_2600_defs.h>
#include "ipu_device_ibufctrl_properties_defs.h"

#define IBUFCTRL_MAX_SIDS		IPU_DEVICE_IBC_MAX_SIDS
#define IBUFCTRL_MAX_DESTS_PER_SID	IPU_DEVICE_IBC_MAX_DESTS_PER_SID

/* Note:
 * - Below Macros definitions are generic to both ISYS, PSYS
 *   So, device specific properties cannot be present in their
 *   definition.
 *   Thus, below maximum values are hand calculated by
 *   comparing properties of all devices for specific Subsystem.
 *   Modify them once new device of same type gets added to subsytem.
 */
#define IBUFCTRL_MAX_IRQS 8
#define IBUFCTRL_MAX_FEEDERS 0xE

#define IBUFCTRL_MAX_DESTS ((IBUFCTRL_MAX_SIDS)*(IBUFCTRL_MAX_DESTS_PER_SID))

#endif /* __VIED_NCI_IBUFCTRL_CONFIGURATION_H */
