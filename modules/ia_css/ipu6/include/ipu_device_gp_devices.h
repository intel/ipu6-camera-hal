/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IPU_DEVICE_GP_DEVICES_H
#define __IPU_DEVICE_GP_DEVICES_H
#include "math_support.h"
#include "type_support.h"

enum ipu_device_gp_id {
    IPU_DEVICE_GP_PSA = 0,
    IPU_DEVICE_GP_ISA_STATIC,
    IPU_DEVICE_GP_ISA_RUNTIME,
    IPU_DEVICE_GP_NUM_GP
};

/* The PSA_* naming in the enum members refers to the internal separation of
 * the PSA. Using it this way should make this change transparent
 * to higher layers.
 *
 * For details on the values and usage of the muxes see Figures 3-1
 * in section 2.1.3 and Figure 3-3 in section 2.2.2 of the IPU-5 General
 * Fixed Function MAS. Additionally, refer to the respective description
 * fields in the RDL files on the SDK (either psa_ip_top_system.rdl
 * or isa_ps_system.rdl).
 */
enum ipu_device_gp_psa_mux_id {
    /* Mux/demuxes */
    /* 0 - BNLM output pixel stream; 1 - BNLM output delta stream */
    IPU_DEVICE_GP_PSA_1_NOISE_MUX = 0,
    /* 0 - To XNR; 1 - WB/DM */
    IPU_DEVICE_GP_PSA_2_STILLS_MODE_MUX,
    /* 0 - To Gamma_Star; 1 - To VCA; 2 - To VCSC */
    IPU_DEVICE_GP_PSA_2_V2S_RGB_4_DEMUX,
    /* 0 - input from Gamma_star; 1 - input from VCA; 2 input from DMSC */
    IPU_DEVICE_GP_PSA_2_S2V_F_INPUT_MUX,
    /* 0 - To IEFD. 1 - To GLTM; 2 - To DM */
    IPU_DEVICE_GP_PSA_2_V2S_5_DEMUX,
    /* 0 - VCSC output is chosen; 1 - BNLM output is chosen */
    IPU_DEVICE_GP_PSA_3_VCSC_BP_MUX,
    /* 0 - v2s_1 output; 1 - ISA downscaled stream; 2 - ISA original sized stream */
    IPU_DEVICE_GP_LBFF_V2S_1_OUTPUT_MUX,
    /* Device blockers */
    /* BNLM pixel output block enable */
    IPU_DEVICE_GP_PSA_1_BNLM_PIXEL_YUV_STRM_BLK,
    /* BNLM pixel output block enable */
    IPU_DEVICE_GP_PSA_1_BNLM_PIXEL_BAYER_STRM_BLK,
    /* BNLM delta output block enable */
    IPU_DEVICE_GP_PSA_1_BNLM_DELTA_STRM_BLK,
    /* Block pixel stream to gltm from vcsc */
    IPU_DEVICE_GP_PSA_3_VCSC_TO_GLTM_BLK,
    /* Block pixel stream to gltm from bnlm */
    IPU_DEVICE_GP_PSA_3_BNLM_TO_GLTM_BLK,
    /* Frame size reg for strmCrop H */
    IPU_DEVICE_GP_LBFF_S2V_H_FRAME_SIZE,
    IPU_DEVICE_GP_PSA_MUX_NUM_MUX
};

enum ipu_device_gp_isa_static_mux_id {
    /* Muxes/demuxes */
    /* 0 - to ISL.S2V; 1 - to PSA */
    IPU_DEVICE_GP_ISA_STATIC_ISA_ORIG_OUT_DEMUX_SEL = 0,
    /* 0 - to ISL.S2V; 1 - to PSA */
    IPU_DEVICE_GP_ISA_STATIC_ISA_SCALED_A_OUT_DEMUX_SEL,
    /* 0 - Input Correction; 1 - B2B mux */
    IPU_DEVICE_GP_ISA_STATIC_AWB_MUX_SEL,
    /* 0 - Input Correction; 1 - B2B mux */
    IPU_DEVICE_GP_ISA_STATIC_AE_MUX_SEL,
    /* 0 - Input Correction; 1 - B2B mux */
    IPU_DEVICE_GP_ISA_STATIC_AF_MUX_SEL,
    /* 0 - to Lsc; 1 - to Dpc; 2 - to X2b */
    IPU_DEVICE_GP_ISA_STATIC_ISA_INPUT_CORR_DEMUX_SEL,
    /* 0 - Input correction; 1 - Dpc; 2 - X2b */
    IPU_DEVICE_GP_ISA_STATIC_LSC_MUX_SEL,
    /* 0 - to B2b; 1 - to Dpc; 2 - X2b*/
    IPU_DEVICE_GP_ISA_STATIC_LSC_DEMUX_SEL,
    /* 0 - to Lsc; 1 - to B2b; 2 - to X2b */
    IPU_DEVICE_GP_ISA_STATIC_DPC_DEMUX_SEL,
    /* 0 - Lsc; 1 - Input correction; 2 - X2b */
    IPU_DEVICE_GP_ISA_STATIC_DPC_MUX_SEL,
    /* 0 - Lsc; 1 - X2b; 2 - Input correction */
    IPU_DEVICE_GP_ISA_STATIC_X2B_MUX_SEL,
    /* 0 - to Lsc; 1 - to Dpc; 2 - to B2b */
    IPU_DEVICE_GP_ISA_STATIC_X2B_DEMUX_SEL,
    /* 0 - Lsc; 1 - Dpc; 2 - X2b */
    IPU_DEVICE_GP_ISA_STATIC_B2B_MUX_SEL,
    /* 0 - Through X2B SVE RGBIR; 1 - Bypass */
    IPU_DEVICE_GP_ISA_STATIC_SVE_RGBIR_BP_MUX_DEMUX_SEL,
    /* 0 - X2B SVE RGBIR; 1- X2B MD */
    IPU_DEVICE_GP_ISA_STATIC_IR_DEPTH_MUX_SEL,
    /* 0 - External PAF CH0/1; 1 - DPC extracted PAF CH0/1; 2 - X2B extracted PAF CH0/1 */
    IPU_DEVICE_GP_ISA_STATIC_PAF_SRC_SEL,
    /* 0 - to Dol mux; 1 - to Dol */
    IPU_DEVICE_GP_ISA_STATIC_MAIN_INPUT_DOL_DEMUX_SEL,
    /* 0 - Main input; 1 - Dol */
    IPU_DEVICE_GP_ISA_STATIC_DOL_MUX_SEL,
    /* Blockers */
    /* ISA input correction Port block */
    IPU_DEVICE_GP_ISA_STATIC_ISA_INPUT_CORR_PORT_BLK,
    /* AWB Mux input correction Port block */
    IPU_DEVICE_GP_ISA_STATIC_AWBMUX_INPUT_CORR_PORT_BLK,
    /* AE Mux input correction Port block */
    IPU_DEVICE_GP_ISA_STATIC_AEMUX_INPUT_CORR_PORT_BLK,
    /* AF Mux input correction Port block */
    IPU_DEVICE_GP_ISA_STATIC_AFMUX_INPUT_CORR_PORT_BLK,
    /* AWB Mux B2B Mux Port block */
    IPU_DEVICE_GP_ISA_STATIC_AWBMUX_B2B_MUX_PORT_BLK,
    /* AE Mux B2B Mux Port block */
    IPU_DEVICE_GP_ISA_STATIC_AEMUX_B2B_MUX_PORT_BLK,
    /* AF Mux B2B Mux Port block */
    IPU_DEVICE_GP_ISA_STATIC_AFMUX_B2B_MUX_PORT_BLK,
    /* PAF stream sync configuration */
    IPU_DEVICE_GP_ISA_STATIC_PAF_STREAM_SYNC_CFG,
    /* Paf Mux gddpc Port block */
    IPU_DEVICE_GP_ISA_STATIC_PAF_GDDPC_PORT_BLK,
    /* Paf Mux input isa Port block */
    IPU_DEVICE_GP_ISA_STATIC_PAF_ISA_STREAM_PORT_BLK,
    /* Paf Mux X2B Port block */
    IPU_DEVICE_GP_ISA_STATIC_PAF_X2B_PORT_BLK,
    /* SIS A port block */
    IPU_DEVICE_GP_ISA_STATIC_SIS_A_PORT_BLK,
    /* SIS B port block */
    IPU_DEVICE_GP_ISA_STATIC_SIS_B_PORT_BLK,
    /* Lsc mux Input Corr demux port block */
    IPU_DEVICE_GP_ISA_STATIC_LSC_MUX_INPUT_CORR_DEMUX_PORT_BLK,
    /* Dpc mux Lsc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_DPC_MUX_INPUT_CORR_DEMUX_PORT_BLK,
    /* Dpc mux Lsc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_DPC_MUX_LSC_DEMUX_PORT_BLK,
    /* Dpc mux X2b demux port block */
    IPU_DEVICE_GP_ISA_STATIC_DPC_MUX_X2B_DEMUX_PORT_BLK,
    /* Dpc mux Lsc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_LSC_MUX_DPC_DEMUX_PORT_BLK,
    /* Lsc mux X2b demux port block */
    IPU_DEVICE_GP_ISA_STATIC_LSC_MUX_X2B_DEMUX_PORT_BLK,
    /* X2b mux Lsc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_X2B_MUX_LSC_DEMUX_PORT_BLK,
    /* X2b mux Dpc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_X2B_MUX_DPC_DEMUX_PORT_BLK,
    /* X2b mux Input Corr demux port block */
    IPU_DEVICE_GP_ISA_STATIC_X2B_MUX_INPUT_CORR_DEMUX_PORT_BLK,
    /* B2b mux Lsc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_B2B_MUX_LSC_DEMUX_PORT_BLK,
    /* B2b mux X2b demux port block */
    IPU_DEVICE_GP_ISA_STATIC_B2B_MUX_X2B_DEMUX_PORT_BLK,
    /* B2b mux Dpc demux port block */
    IPU_DEVICE_GP_ISA_STATIC_B2B_MUX_DPC_DEMUX_PORT_BLK,
    /* B2b B2b mux port block */
    IPU_DEVICE_GP_ISA_STATIC_B2B_B2B_MUX_PORT_BLK,
    /* Stat B2b mux port block */
    IPU_DEVICE_GP_ISA_STATIC_3A_STAT_B2B_MUX_PORT_BLK,
    /* Stat Input correction mux port block */
    IPU_DEVICE_GP_ISA_STATIC_3A_STAT_INPUT_CORR_MUX_PORT_BLK,
    /* Sis A B2b port block */
    IPU_DEVICE_GP_ISA_STATIC_SIS_A_B2B_PORT_BLK,
    /* Sis B B2b port block */
    IPU_DEVICE_GP_ISA_STATIC_SIS_B_B2B_PORT_BLK,
    /* ISA Orig B2b port block */
    IPU_DEVICE_GP_ISA_STATIC_ISA_ORIG_B2B_PORT_BLK,
    /* B2r B2b port block */
    IPU_DEVICE_GP_ISA_STATIC_B2R_B2B_PORT_BLK,
    /* ISA Scaled A Out R2i port block */
    IPU_DEVICE_GP_ISA_STATIC_ISA_SCALED_A_OUT_R2I_PORT_BLK,
    /* ISA Scaled B Out R2i port block */
    IPU_DEVICE_GP_ISA_STATIC_ISA_SCALED_B_OUT_R2I_PORT_BLK,
    /* ISA Full Out R2i port block */
    IPU_DEVICE_GP_ISA_STATIC_ISA_FULL_OUT_R2I_PORT_BLK,
    /* Ir Depth Out X2b port block */
    IPU_DEVICE_GP_ISA_STATIC_IR_DEPTH_OUT_X2B_PORT_BLK,
    /* Main input Dol demux port block */
    IPU_DEVICE_GP_ISA_STATIC_MAIN_INPUT_DOL_DEMUX_PORT_BLK,
    /* 0 - pif_yuv_mode disabled; 1 - pif_yuv_mode enabled */
    IPU_DEVICE_GP_ISA_PIF_YUV_MODE_EN,
    IPU_DEVICE_GP_ISA_STATIC_MUX_NUM_MUX
};

enum ipu_device_gp_isa_runtime_mux_id {
    /* frame dim */
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_ORIG_OUT,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_FULL_OUT,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_SCALED_A,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_SCALED_B,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_SIS_A,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_SIS_B,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_STRMCROP_A,
    IPU_DEVICE_GP_ISA_RUNTIME_FRAME_SIZE_STRMCROP_B,
    IPU_DEVICE_GP_ISA_RUNTIME_MUX_NUM_MUX
};

/* The value below is the largest *MUX_NUM_MUX of the mux enums. */
#define IPU_DEVICE_GP_MAX_NUM ((uint32_t)(IPU_DEVICE_GP_ISA_STATIC_MUX_NUM_MUX))

#endif /* __IPU_DEVICE_GP_DEVICES_H */
