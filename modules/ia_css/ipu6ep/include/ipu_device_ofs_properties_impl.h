/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_OFS_PROPERTIES_IMPL_H
#define __IPU_DEVICE_OFS_PROPERTIES_IMPL_H

/* Properties of the OFS devices in PSYS */
#include "ipu_device_ofs_properties_struct.h" /* from interface */
#include "ipu_device_ofs_properties_defs.h" /* from the subsystem */
#include "ipu_device_ofs_devices.h" /* from the subsystem */
#include "ipu_device_ofs_properties_storage_class.h"

OFS_PROPERTIES_DATA_STORAGE_CLASS_C struct ipu_device_ofs_prop_s ipu_device_ofs_prop[IPU_DEVICE_OFS_NUM] = {
	{
		{
			{
				IPU_DEVICE_OFS_OF_BASE_ADDR
			},
			{
				0
			},
			{
				0
			}
		},
		{
			{
				IPU_DEVICE_OFS_SC0_BASE_ADDR,
				IPU_DEVICE_OFS_SC0_ELEMENTS_PER_VECTOR,
				IPU_DEVICE_OFS_SC0_NUM_POLYPHASE_TAPS,
				IPU_DEVICE_OFS_SC0_NUM_PHASES,
				IPU_DEVICE_OFS_SC0_INPUT_DATA_BPP,
				IPU_DEVICE_OFS_SC0_OUTPUT_DATA_BPP,
				IPU_DEVICE_OFS_SC0_INTER_DATA_BPP,
				IPU_DEVICE_OFS_SC0_MAX_BITS_PER_COEF,
				IPU_DEVICE_OFS_SC0_OF_TO_SCALER_REL_MSG_Q_ADDR,
				IPU_DEVICE_OFS_SC0_SCALER_TO_OF_RDY_MSG_Q_ADDR
			},
			{
				IPU_DEVICE_OFS_SC1_BASE_ADDR,
				IPU_DEVICE_OFS_SC1_ELEMENTS_PER_VECTOR,
				IPU_DEVICE_OFS_SC1_NUM_POLYPHASE_TAPS,
				IPU_DEVICE_OFS_SC1_NUM_PHASES,
				IPU_DEVICE_OFS_SC1_INPUT_DATA_BPP,
				IPU_DEVICE_OFS_SC1_OUTPUT_DATA_BPP,
				IPU_DEVICE_OFS_SC1_INTER_DATA_BPP,
				IPU_DEVICE_OFS_SC1_MAX_BITS_PER_COEF,
				IPU_DEVICE_OFS_SC1_OF_TO_SCALER_REL_MSG_Q_ADDR,
				IPU_DEVICE_OFS_SC1_SCALER_TO_OF_RDY_MSG_Q_ADDR
			}
		},
		{
			{
				IPU_DEVICE_OFS_DATABUS_BUFFER1_BASE_ADDRESS
			}
		}
	}
};

#endif /* __IPU_DEVICE_OFS_PROPERTIES_IMPL_H */
