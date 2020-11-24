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

/*
 * TODO: IPU5_SDK Verify that this file is correct when IPU5 SDK has been released.
 */

#ifndef __IPU_DEVICE_SUBSYSTEM_IDS_H
#define __IPU_DEVICE_SUBSYSTEM_IDS_H

enum ipu_device_subsystem_id {
#ifdef ISYS_ID_ZERO
	IPU_DEVICE_ISYS, /* = 0 */
	IPU_DEVICE_PSYS, /* = 1 */
#else
	IPU_DEVICE_PSYS, /* = 0 */
	IPU_DEVICE_ISYS, /* = 1 */
#endif
	IPU_DEVICE_BUTTRESS,
	IPU_DEVICE_SUBSYSTEM_IMR,
	IPU_DEVICE_NUM_SUBSYSTEMS
};

#endif /* __IPU_DEVICE_SUBSYSTEM_IDS_H */
