/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2020 Intel Corporation.
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

#ifndef __IA_CSS_PROGDESC_ACC_SYMBOL_IDS_H
#define __IA_CSS_PROGDESC_ACC_SYMBOL_IDS_H

#ifndef ISA_KERNEL_ID_START
#define ISA_KERNEL_ID_START 0
#endif /* ISA_KERNEL_ID_START */

enum {
	/* PSA fixed functions */
	/* pre-IPU6, 0 was reserved for WBA */
	/* pre-IPU6, 1 was reserved for RYNR splitter */
	/* pre-IPU6, 2 was reserved for RYNR collector */
	KERNEL_ID_RYNR_BNLM = 3,     /* 3  (0x3) */
	KERNEL_ID_DEMOSAIC_DEMOSAIC, /* 4  (0x4) */
#if HAS_ACM_CCM
	KERNEL_ID_ACM_CCM,           /* 5  (0x5) */
#else
	KERNEL_ID_NO_CCM_SPARE,      /* 5  (0x5) */
#endif
	KERNEL_ID_ACM_ACM,           /* 6  (0x6) */
#if HAS_VCR_IN_VCA
	KERNEL_ID_VCA_VCR2,          /* 7  (0x7) */
#else
	KERNEL_ID_NO_VCR2_SPARE,     /* 7  (0x7) */
#endif
	KERNEL_ID_GAMMASTAR,         /* 8  (0x8) */
	KERNEL_ID_GTC_CSC_CDS,       /* 9  (0x9) */
	KERNEL_ID_GTC_GTM,           /* 10 (0xA) */
	KERNEL_ID_YUV1_SPLITTER,     /* 11 (0xB) */
	KERNEL_ID_YUV1_IEFD,         /* 12 (0xC) */
	/* pre-IPU6, 13 was reserved for KERNEL_ID_YUV1_YDS */
	/* pre-IPU6, 14 was reserved for KERNEL_ID_YUV1_TCC collector */
	KERNEL_ID_DVS_YBIN = 15,     /* 15 (0xF) */
	KERNEL_ID_DVS_DVS,           /* 16 (0x10) */
	KERNEL_ID_VCSC,              /* 17 (0x11) */
	KERNEL_ID_GLTM,              /* 18 (0x12) */
	KERNEL_ID_XNR_VHF,           /* 19 (0x13) */
	KERNEL_ID_XNR_HF,            /* 20 (0x14) */
	KERNEL_ID_XNR_HF_SE,         /* 21 (0x15) */
	KERNEL_ID_XNR_MF,            /* 22 (0x16) */
	KERNEL_ID_XNR_MF_SE,         /* 23 (0x17) */
	KERNEL_ID_XNR_LF,            /* 24 (0x18) */
	KERNEL_ID_XNR_LF_SE,         /* 25 (0x19) */
	KERNEL_ID_XNR_LFE,           /* 26 (0x1A) */
	KERNEL_ID_XNR_VLF,           /* 27 (0x1B) */
	KERNEL_ID_XNR_VLF_SE,        /* 28 (0x1C) */
#if !HAS_VCR_IN_VCA
	KERNEL_ID_XNR_VCR,
#endif
	KERNEL_ID_ESPA,
	KERNEL_ID_ESPA_V2S_PSA_1,
	KERNEL_ID_ACM_GLIM,
	KERNEL_ID_STREAM_CROP_PSA_H,
	KERNEL_ID_DVS_STATS_L0,
	KERNEL_ID_DVS_STATS_L1,
	KERNEL_ID_DVS_STATS_L2,
};

enum {
	/* ISA fixed functions */
	KERNEL_ID_ICA_INL = ISA_KERNEL_ID_START,
	KERNEL_ID_ICA_BLC_GLOBAL,
	KERNEL_ID_BLC_GRID_OB_FF,
	KERNEL_ID_PCLN_HDR,
	KERNEL_ID_LSC_HDR,
	KERNEL_ID_GDDPC_PEXT,
	KERNEL_ID_GDDPC_PEXT_PEXT,
	KERNEL_ID_GDDPC_PEXT_SAM,
	KERNEL_ID_B2R_DEMOSAIC,
#if HAS_FF_B2R_BNR
	KERNEL_ID_B2R_BNR,
#endif
	KERNEL_ID_B2B_WBA,
	KERNEL_ID_B2B_LCA,
#if HAS_FF_R2I_CCM
	KERNEL_ID_R2I_SIE_CCM,
#endif
	KERNEL_ID_R2I_SIE_CSC,
#if HAS_FF_R2I_USM
	KERNEL_ID_R2I_SIE_USM,
#endif
#if HAS_FF_R2I_GTM
	KERNEL_ID_R2I_SIE_GTM,
#endif
	KERNEL_ID_R2I_DS_A,
#if HAS_FF_R2I_DS_B
	KERNEL_ID_R2I_DS_B,
#endif
	KERNEL_ID_AWB_STAT,
	KERNEL_ID_AE_STAT,
	KERNEL_ID_AE_CCM_STAT,
	KERNEL_ID_AF_STAT,
	KERNEL_ID_PIFCONV_ISA_ORIG,
	KERNEL_ID_PIFCONV_ISA_SCALED_A,
#if HAS_FF_R2I_DS_B
	KERNEL_ID_PIFCONV_ISA_SCALED_B,
#endif
	KERNEL_ID_PIFCONV_ISA_FULL_OUT,
	KERNEL_ID_PIFCONV_ISA_IR_DEPTH,
	KERNEL_ID_ESPA_ORIG_OUT,
	KERNEL_ID_ESPA_SCALED_A,
#if HAS_FF_R2I_DS_B
	KERNEL_ID_ESPA_SCALED_B,
#endif
	KERNEL_ID_ESPA_FULL_OUT,
	KERNEL_ID_ESPA_IR_DEPTH,
	KERNEL_ID_ESPA_SIS_A,
	KERNEL_ID_ESPA_SIS_B,
	KERNEL_ID_ESPA_B2B,
	KERNEL_ID_STRMCROP_SIS_A,
	KERNEL_ID_STRMCROP_SIS_B,
	KERNEL_ID_SIS_A,
	KERNEL_ID_SIS_B,
	KERNEL_ID_PIFCONV_SIS_A,
	KERNEL_ID_PIFCONV_SIS_B,
	KERNEL_ID_PIXEL_FORMATTER_HIGH,
	KERNEL_ID_PADDER_ISA_ORIG,
	KERNEL_ID_PADDER_ISA_FULL,
	KERNEL_ID_PADDER_ISA_SCALED_A,
#if HAS_FF_R2I_DS_B
	KERNEL_ID_PADDER_ISA_SCALED_B,
#endif
#if HAS_FF_DOL_DOL
	KERNEL_ID_DOL,
#endif
	KERNEL_ID_X2B_MD,
#if HAS_FF_DOL_DOL
	KERNEL_ID_ICA_INL_SHORT,
	KERNEL_ID_ICA_BLC_GLOBAL_SHORT,
	KERNEL_ID_ICA_PCLN_HDR_SHORT,
	KERNEL_ID_ICA_INL_MEDIUM,
	KERNEL_ID_ICA_BLC_GLOBAL_MEDIUM,
	KERNEL_ID_ICA_PCLN_HDR_MEDIUM,
	KERNEL_ID_PIXEL_FORMATTER_SHORT,
	KERNEL_ID_PIXEL_FORMATTER_MEDIUM,
#endif
	KERNEL_ID_X2B_SVE,
	KERNEL_ID_X2B_RGBIR,
	KERNEL_ID_PAF,
	KERNEL_ID_ISA_N
};

enum {
	KERNEL_ID_GDC_MBR,
	KERNEL_ID_OFS_OF_PIN_MAIN,
	KERNEL_ID_OFS_OF_PIN_DISPLAY,
	KERNEL_ID_OFS_OF_PIN_PP,
	KERNEL_ID_TNR_MBR,
	KERNEL_ID_BB_N
};

enum {
	GDC_MBR_HOMOGRAPHY_REGION_ID = 0,
	GDC_ONLY_CHANNEL_0_CACHED_REGION_ID,
	GDC_ONLY_CHANNEL_1_CACHED_REGION_ID,
	GDC_ONLY_CHANNEL_2_CACHED_REGION_ID,
	GDC_ONLY_LUT_DATAPATH_REGION_ID,
	GDC_MBR_LUT_LDC_DEWARPING_REGION_ID,
	GDC_MBR_WFOV_REGION_ID
};

enum {
	TNR_MRC_CACHED_TERMINAL_REGION_ID = 0,
	TNR_PME0_CACHED_TERMINAL_REGION_ID,
	TNR_PME1_CACHED_TERMINAL_REGION_ID,
	TNR_TNR6_CACHED_TERMINAL_REGION_ID
};

enum {
	TNR_GLOBAL_PROGRAM_TERMINAL_REGION_ID = 0,
	TNR_MRC_PROGRAM_TERMINAL_REGION_ID
};

/* For OFS KERNELS Regions for cached parameters */
/* KERNEL_ID_OFS_OF_PIN_MAIN */
enum {
	OFS_FORMATTER_CACHED_SHARED_REGION_ID = 0,
	OFS_FORMATTER_CACHED_PIN_MAIN_REGION_ID,
};

/* KERNEL_ID_OFS_OF_PIN_DISPLAY */
enum {
	OFS_FORMATTER_CACHED_PIN_DISPLAY_REGION_ID = 0,
	OFS_SCALER_CACHED_PIN_DISPLAY_REGION_ID,
};

/* KERNEL_ID_OFS_OF_PIN_PP */
enum {
	OFS_FORMATTER_CACHED_PIN_PP_REGION_ID = 0,
	OFS_SCALER_CACHED_PIN_PP_REGION_ID,
};

/* OFS KERNELS Regions for non-cached parameters */
/* KERNEL_ID_OFS_OF_PIN_DISPLAY */
enum {
	OFS_SCALER_NON_CACHED_PIN_DISPLAY_REGION_ID = 0,
};

/* KERNEL_ID_OFS_OF_PIN_PP */
enum {
	OFS_SCALER_NON_CACHED_PIN_PP_REGION_ID = 0,
};

#endif /* __IA_CSS_PROGDESC_ACC_SYMBOL_IDS_H */
