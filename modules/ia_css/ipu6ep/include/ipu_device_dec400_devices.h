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

#ifndef __IPU_DEVICE_DEC400_DEVICES_H
#define __IPU_DEVICE_DEC400_DEVICES_H
/* DEC400 Devices in PSYS
* Public IDs to be used by the NCI user.
*/
#ifndef USE_DAI
typedef enum {
#if (HAS_DEC400 == 1)
	NCI_DEC400_DECODER,
	NCI_DEC400_ENCODER,
#endif
	NCI_DEC400_NUM_DEVICES
} nci_dec400_device_id;

#if (HAS_DEC400 == 1)
#define NCI_DEC400_MAX_BAYER_STREAMS 3
#else
#define NCI_DEC400_MAX_BAYER_STREAMS 0
#endif

/* PSYS compressor supports YUV420 planar */
#define NCI_DEC400_MAX_PLANES 3

#else
#endif /*USE_DAI */
#endif /* __IPU_DEVICE_DEC400_DEVICES_H */
