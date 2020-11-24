/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2020 Intel Corporation.
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

#ifndef __IPU_DEVICE_S2V_DEVICES_H
#define __IPU_DEVICE_S2V_DEVICES_H

/*
 * IDs originally defined by DPI. It has been decided not to
 * use them directly in FW. Instead, use the public IDs
 * defined below. This will enable platform agnostic IDs and
 * it's also compatible with future DAI implementations.
 * Kept here only for reference.
 */
enum ipu_device_s2v_dev_id {
	IPU_DEVICE_S2V_LBFF_INFRA_C_ID,	/* S2V C */
	IPU_DEVICE_S2V_LBFF_INFRA_H_ID,	/* S2V H */
	IPU_DEVICE_S2V_ISA_ORIG_BAYER_ID,
	IPU_DEVICE_S2V_ISA_SCALED0_YUV420_ID,
	IPU_DEVICE_S2V_ISA_FULL_YUV420_ID,
	IPU_DEVICE_S2V_NUM_DEVS
};

/*
 * Public IDs to be used by the NCI user.
 */
enum nci_s2v_device_id {
	NCI_S2V_BAYER1_ID		= IPU_DEVICE_S2V_LBFF_INFRA_C_ID,
	NCI_S2V_YUV1_ID			= IPU_DEVICE_S2V_LBFF_INFRA_H_ID,
	NCI_S2V_ISA_ORIG_BAYER_ID	= IPU_DEVICE_S2V_ISA_ORIG_BAYER_ID,
	NCI_S2V_ISA_SCALED0_YUV420_ID	= IPU_DEVICE_S2V_ISA_SCALED0_YUV420_ID,
	NCI_S2V_ISA_FULL_YUV420_ID	= IPU_DEVICE_S2V_ISA_FULL_YUV420_ID
};

#define NCI_S2V_NUM_DEVICES (IPU_DEVICE_S2V_NUM_DEVS)
#endif /* __IPU_DEVICE_S2V_DEVICES_H */
