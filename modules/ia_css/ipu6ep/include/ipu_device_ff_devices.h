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

#ifndef __IPU_DEVICE_FF_DEVICES_H
#define __IPU_DEVICE_FF_DEVICES_H

enum ipu_device_ff_id {
	/* PSA fixed functions */
	IPU_DEVICE_FF_RYNR_BNLM = 0,
	IPU_DEVICE_FF_DEMOSAIC_DEMOSAIC,
	IPU_DEVICE_FF_ACM_GLIM,           /* ACM is called VCA in HW */
	IPU_DEVICE_FF_ACM_ACM,            /* ACM is called VCA in HW */
	IPU_DEVICE_FF_GAMMASTAR,
	IPU_DEVICE_FF_GTC_CSC_CDS,
	IPU_DEVICE_FF_GTC_GTM,
	IPU_DEVICE_FF_YUV1_SPLITTER,
	IPU_DEVICE_FF_YUV1_IEFD,
	IPU_DEVICE_FF_DVS_YBIN,
	IPU_DEVICE_FF_DVS_DVS,
	IPU_DEVICE_FF_VCSC,
	IPU_DEVICE_FF_GLTM,
	IPU_DEVICE_FF_XNR_VHF,
	IPU_DEVICE_FF_XNR_HF,
	IPU_DEVICE_FF_XNR_HF_SE,
	IPU_DEVICE_FF_XNR_MF,
	IPU_DEVICE_FF_XNR_MF_SE,
	IPU_DEVICE_FF_XNR_LF,
	IPU_DEVICE_FF_XNR_LF_SE,
	IPU_DEVICE_FF_XNR_LFE,
	IPU_DEVICE_FF_XNR_VLF,
	IPU_DEVICE_FF_XNR_VLF_SE,
	IPU_DEVICE_FF_XNR_VCR,
	IPU_DEVICE_FF_STREAM_CROP_PSA_H,
	/* ISA fixed functions */
	IPU_DEVICE_FF_X2B_MD,
	IPU_DEVICE_FF_X2B_SVE_RGBIR_RGBIR,
	IPU_DEVICE_FF_X2B_SVE_RGBIR_SVE,
	IPU_DEVICE_FF_ICA_INL,
	IPU_DEVICE_FF_ICA_GBL,
	IPU_DEVICE_FF_ICA_PCLN_HDR,
	IPU_DEVICE_FF_LSC_LSC,
	IPU_DEVICE_FF_GDDPC_PEXT,
	IPU_DEVICE_FF_GDDPC_PEXT_PEXT,
	IPU_DEVICE_FF_GDDPC_PEXT_SAM,
	IPU_DEVICE_FF_SIS_A_SIS,
	IPU_DEVICE_FF_SIS_B_SIS,
	IPU_DEVICE_FF_B2B_WBA,
	IPU_DEVICE_FF_B2B_LCA,
	IPU_DEVICE_FF_B2R_DM,
	IPU_DEVICE_FF_R2I_DS_A_SCALER,
	IPU_DEVICE_FF_R2I_SIE_CSC,
	IPU_DEVICE_FF_AWB_AWRG,
	IPU_DEVICE_FF_AF_AF,
	IPU_DEVICE_FF_PAF,
	IPU_DEVICE_FF_AE_WGHT_HIST,
	IPU_DEVICE_FF_AE_CCM,
	IPU_DEVICE_FF_STREAM_CROP_A,
	IPU_DEVICE_FF_STREAM_CROP_B,
	IPU_DEVICE_FF_NUM_FF
};

#define IPU_DEVICE_FF_LAST_PSA_FF IPU_DEVICE_FF_XNR_VLF
#define IPU_DEVICE_FF_NUM_PSA_FF  (IPU_DEVICE_FF_LAST_PSA_FF + 1)
#define IPU_DEVICE_FF_NUM_ISA_FF \
	(IPU_DEVICE_FF_NUM_FF - IPU_DEVICE_FF_LAST_PSA_FF)

#endif /* __IPU_DEVICE_FF_DEVICES_H */
