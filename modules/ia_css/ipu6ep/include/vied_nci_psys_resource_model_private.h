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

#ifndef __VIED_NCI_PSYS_RESOURCE_MODEL_PRIVATE_H
#define __VIED_NCI_PSYS_RESOURCE_MODEL_PRIVATE_H

#include "type_support.h"
#include "storage_class.h"

typedef enum vied_nci_dma_connect_section_id {
	VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_A, /* Cached */
	VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_B, /* Cached */
	VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_A_NON_CACHED,
	VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_B_NON_CACHED
} vied_nci_dma_connect_section_id_t;

#define VIED_NCI_DMA_CONNECT_SECTION_ID_N (VIED_NCI_DMA_CONNECT_SECTION_ID_TERMINAL_B_NON_CACHED + 1)

typedef enum vied_nci_dma_section_id {
	VIED_NCI_DMA_SECTION_ID_CHANNEL_CACHED = 0,
	VIED_NCI_DMA_SECTION_ID_TERMINAL_CACHED,
	VIED_NCI_DMA_SECTION_ID_SPAN_CACHED,
	VIED_NCI_DMA_SECTION_ID_UNIT_CACHED,
	VIED_NCI_DMA_SECTION_ID_CHANNEL_NON_CACHED,
	VIED_NCI_DMA_SECTION_ID_TERMINAL_NON_CACHED,
	VIED_NCI_DMA_SECTION_ID_SPAN_NON_CACHED,
	VIED_NCI_DMA_SECTION_ID_UNIT_NON_CACHED,
	VIED_NCI_DMA_SECTION_ID_REQUESTOR_BANK
} vied_nci_dma_section_id_t;

#define VIED_NCI_DMA_SECTION_ID_CACHED_N (VIED_NCI_DMA_SECTION_ID_UNIT_CACHED + 1)
#define VIED_NCI_DMA_SECTION_ID_N (VIED_NCI_DMA_SECTION_ID_REQUESTOR_BANK + 1)

/* For DMA, configuration reset is invalidation of current descriptor. */
#define VIED_NCI_DMA_NOF_RESET_DESC (1)

/* Device IDs
 * This enum will be uses as device id field in program
 * control terminal descriptor.
 */
typedef enum {
	VIED_NCI_DEVICE_ID_SP = 0,
	VIED_NCI_DEVICE_ID_LB_ACB,
	VIED_NCI_DEVICE_ID_OFS,
	VIED_NCI_DEVICE_ID_GDC,
	VIED_NCI_DEVICE_ID_MBR,
	VIED_NCI_DEVICE_ID_TNR,
	VIED_NCI_DEVICE_ID_DMA,
	VIED_NCI_DEVICE_ID_DFM,
	VIED_NCI_DEVICE_ID_S2V,
	VIED_NCI_DEVICE_ID_V2S,
	VIED_NCI_DEVICE_ID_CIO2STR,
	VIED_NCI_DEVICE_ID_PIFCONV_CTRL,
	VIED_NCI_DEVICE_ID_PIXEL_FORMATTER,
	VIED_NCI_DEVICE_ID_PIXEL_STR2MMIO,
	VIED_NCI_DEVICE_ID_DVS_CONTROLLER,
	VIED_NCI_DEVICE_ID_STRM_PACK,
	VIED_NCI_DEVICE_ID_DEC400,
	VIED_NCI_DEVICE_ID_STREAM_BLOCKER,
	VIED_NCI_DEVICE_ID_NOP, /* used for skipping a device that does not need to be configured */
	VIED_NCI_DEVICE_ID_DUMMY_DDR_DMEM,
	VIED_NCI_DEVICE_NUM
} vied_nci_device_type_id_t;

/*
 * pixel_str2mmio device instances
 */
typedef enum {
	VIED_NCI_PIXEL_STR2MMIO_SIS_A,
	VIED_NCI_PIXEL_STR2MMIO_SIS_B,
	VIED_NCI_PIXEL_STR2MMIO_IR_DEPTH,
	VIED_NCI_PIXEL_STR2MMIO_NUM_DEVS
} vied_nci_pixel_str2mmio_id_t;

/*
 * pixel formatter device instances
 */
typedef enum {
	VIED_NCI_PIXEL_FORMATTER_HIGH_ISL_PS,
	VIED_NCI_PIXEL_FORMATTER_NUM_DEVS
} vied_nci_pixel_formatter_id_t;

/*
 * cio2str device instances
 */
typedef enum {
	VIED_NCI_CIO2STR_PAF_ISL_PS,
	VIED_NCI_CIO2STR_ISL_PS,
	VIED_NCI_CIO2STR_NUM_DEVS
} vied_nci_cio2str_id_t;

typedef enum {
	VIED_NCI_DVS_CONTROLLER_PSA_ID = 0,
	VIED_NCI_DVS_CONTROLLER_NUM_DEVS
} vied_nci_dvs_controller_id_t;

/*
 * pifconv_ctrl device instances
 */
typedef enum {
	VIED_NCI_PIFCONV_CTRL_ISL_PAF_A,
	VIED_NCI_PIFCONV_CTRL_ISL_PAF_B,
	VIED_NCI_PIFCONV_CTRL_ISL_A,
	VIED_NCI_PIFCONV_CTRL_ISL_B,
	VIED_NCI_PIFCONV_CTRL_NUM_DEVS
} vied_nci_pifconv_ctrl_id_t;

/*
 * s2v device instances
 */
typedef enum {
	VIED_NCI_S2V_LBFF_INFRA_C_ID,   /* S2V C */
	VIED_NCI_S2V_LBFF_INFRA_D_ID,   /* S2V C */
	VIED_NCI_S2V_LBFF_INFRA_E_ID,   /* S2V E */
	VIED_NCI_S2V_LBFF_INFRA_F_ID,   /* S2V E */
	VIED_NCI_S2V_LBFF_INFRA_G_ID,   /* S2V E */
	VIED_NCI_S2V_LBFF_INFRA_H_ID,   /* S2V H */
	VIED_NCI_S2V_ISA_ORIG_BAYER_ID,
	VIED_NCI_S2V_ISA_SCALED0_YUV420_ID,
	VIED_NCI_S2V_ISA_FULL_YUV420_ID,
	VIED_NCI_S2V_NUM_DEVS
} vied_nci_s2v_id_t;

/*
 * v2s device instances
 */
typedef enum {
	VIED_NCI_V2S_LBFF_INFRA_VEC2STR_1_ID,
	VIED_NCI_V2S_LBFF_INFRA_VEC2STR_4_ID,
	VIED_NCI_V2S_LBFF_INFRA_VEC2STR_5_ID,
	VIED_NCI_V2S_NUM_DEVS
} vied_nci_v2s_id_t;

/*
 * strm pack device instances
 */
typedef enum {
	VIED_NCI_STRM_PACK_SCALED_A,
	VIED_NCI_STRM_PACK_VIDEO_FULL,
	VIED_NCI_STRM_PACK_STR2VEC_H,
	VIED_NCI_STRM_PACK_NUM_DEVS
} vied_nci_strm_pack_id_t;

/*
 * stream blocker device instances
 */
typedef enum {
	STREAM_BLOCKER_ISL_PS_ORIG_B2B_PORT_BLK,
	STREAM_BLOCKER_ISL_PS_FULL_OUT_R2I_PORT_BLK,
	STREAM_BLOCKER_ISL_PS_IR_DEPTH_OUT_X2B_PORT_BLK,
	STREAM_BLOCKER_ISL_PS_SIS_A_PORT_BLK,
	STREAM_BLOCKER_ISL_PS_SIS_B_PORT_BLK,
	STREAM_BLOCKER_NUM_DEVS
} vied_nci_stream_blocker_id_t;

/*todo check if this is valid for ipu6*/
typedef enum {
	VIED_NCI_SP_0_ID = 0,
	VIED_NCI_SP_1_ID,
	VIED_NCI_SP_2_ID,
} vied_nci_sp_id_t;

typedef enum {
	VIED_NCI_ACB_BNLM_ID = 0,
	VIED_NCI_ACB_DM_ID,
	VIED_NCI_ACB_VCSC_ID,
	VIED_NCI_ACB_GLTM_ID,
	VIED_NCI_ACB_XNR_ID,
	VIED_NCI_ACB_ACM_ID, /* ACM is called VCA in HW */
	VIED_NCI_ACB_GAMMASTAR_ID,
	VIED_NCI_ACB_GTC_ID,
	VIED_NCI_ACB_YUV1_ID,
	VIED_NCI_ACB_DVS_ID,
	VIED_NCI_ACB_ICA_ID,
	VIED_NCI_ACB_LSC_ID,
	VIED_NCI_ACB_GDDPC_PEXT_ID,
	VIED_NCI_ACB_SIS_A_ID,
	VIED_NCI_ACB_SIS_B_ID,
	VIED_NCI_ACB_B2B_ID,
	VIED_NCI_ACB_B2R_ID,
	VIED_NCI_ACB_R2I_DS_A_ID,
	VIED_NCI_ACB_R2I_SIE_ID,
	VIED_NCI_ACB_AWB_STAT_ID,
	VIED_NCI_ACB_AE_STAT_ID,
	VIED_NCI_ACB_AF_STAT_ID,
	VIED_NCI_ACB_X2B_MD_ID,
	VIED_NCI_ACB_X2B_SVE_RGBIR_ID,
	VIED_NCI_ACB_PAF_ID,
	VIED_NCI_N_ID
} vied_nci_lb_acb_id_t;

typedef enum vied_nci_dfm_section_id_t {
	VIED_NCI_DFM_SECTION_ID_CMDBANK = 0,
	VIED_NCI_DFM_SECTION_ID_PORT
} vied_nci_dfm_section_id_t;

#define VIED_NCI_DFM_SECTION_ID_N (VIED_NCI_DFM_SECTION_ID_PORT + 1)

typedef enum vied_nci_dfm_connect_section_id_t {
	VIED_NCI_DFM_CONNECT_SECTION_ID_RC_BUF_CHASING = 0,
	VIED_NCI_DFM_CONNECT_SECTION_ID_CMDBANK_ACK_ADDR,
	VIED_NCI_DFM_CONNECT_SECTION_ID_GATHER_MULTICAST
} vied_nci_dfm_connect_section_id_t;

#define VIED_NCI_DFM_CONNECT_SECTION_ID_N (VIED_NCI_DFM_CONNECT_SECTION_ID_GATHER_MULTICAST + 1)

typedef enum vied_nci_ofs_connect_section_id_t {
	VIED_NCI_OFS_DMA_REGION_ORIGIN = 0,
	VIED_NCI_OFS_DFM_GATHER_MULTICAST_MASK,
	VIED_NCI_OFS_STREAM2GEN_FRAME_IDX
} vied_nci_ofs_connect_section_id_t;

#define VIED_NCI_OFS_CONNECT_SECTION_ID_N (VIED_NCI_OFS_DFM_GATHER_MULTICAST_MASK + 1)

/* Reset info for buffer_ctrl register and reset info for ratio_conv register */
#define VIED_NCI_DFM_NOF_RESET_DESC (2)

typedef enum vied_nci_tnr_connect_section_id_t {
	VIED_NCI_TNR_CONNECT_SECTION_ID_INIT_CMD_WIDTH = 0,
	VIED_NCI_TNR_CONNECT_SECTION_ID_INIT_CMD_HEIGHT,
	VIED_NCI_TNR_CONNECT_SECTION_ID_GATHER_MULTICASK_MASK
} vied_nci_tnr_connect_section_id_t;

#define VIED_NCI_TNR_CONNECT_SECTION_ID_N (VIED_NCI_TNR_CONNECT_SECTION_ID_GATHER_MULTICASK_MASK + 1)

#endif /* __VIED_NCI_PSYS_RESOURCE_MODEL_PRIVATE_H */
