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

#ifndef __IPU_DEVICE_STR2MMIO_DEVICES_H
#define __IPU_DEVICE_STR2MMIO_DEVICES_H

/*
 * IDs originally defined by DPI. It has been decided not to
 * use them directly in FW. Instead, use the public IDs
 * defined below. This will enable platform agnostic IDs and
 * it's also compatible with future DAI implementations.
 * Kept here only for reference.
 */
enum ipu_device_str2mmio_id {
	IPU_DEVICE_STR2MMIO_PIXEL_S2M_SIS0 = 0,
	IPU_DEVICE_STR2MMIO_PIXEL_S2M_SIS1,
	IPU_DEVICE_STR2MMIO_PIXEL_S2M_IR_DEPTH,
	IPU_DEVICE_STR2MMIO_NUM_DEVS
};

#define NCI_STR2MMIO_NUM_DEVS (IPU_DEVICE_STR2MMIO_NUM_DEVS)

enum ipu_device_str2mmio_type {
	IPU_DEVICE_STR2MMIO_TYPE_MIPI,
	IPU_DEVICE_STR2MMIO_TYPE_PIXEL,
	IPU_DEVICE_STR2MMIO_NUM_TYPES
};

#endif /* __IPU_DEVICE_STR2MMIO_DEVICES_H */
