/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2016 Intel Corporation.
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
#ifndef _mipi_backend_defs_h
#define _mipi_backend_defs_h

#include "mipi_backend_common_defs.h"

#define MIPI_BACKEND_REG_ALIGN                    4 // assuming 32 bit control bus width

#define _HRT_MIPI_BACKEND_NOF_IRQS                         3 // sid_lut

// to be copied from stream2mmio defs (CSIX PH definition)
#define  MSID_LSB                 32
#define  MSID_MSB                 35
#define  MSID_WIDTH               (MSID_MSB - MSID_LSB + 1)

#define  PID_LSB                  36
#define  PID_MSB                  39
#define  PID_WIDTH                (PID_MSB - PID_LSB + 1)

#define  ODD_EVEN_BIT             63

// PSID LUT DEFINITION
#define _HRT_MIPI_BACKEND_LUT_ENTRY_ACTIVE_BIT  0
#define _HRT_MIPI_BACKEND_LUT_ENTRY_MSID_LSB    1
#define _HRT_MIPI_BACKEND_LUT_ENTRY_MSID_MSB    4
#define _HRT_MIPI_BACKEND_LUT_ENTRY_PORTID_LSB  5
#define _HRT_MIPI_BACKEND_LUT_ENTRY_PORTID_MSB  8

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MIPI Backend Register IDs
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _HRT_MIPI_BACKEND_STATUS_REG_IDX                               0
#define _HRT_MIPI_BACKEND_MEM_ALIGNMENT_CONFIG_REG_IDX                 1
#define _HRT_MIPI_BACKEND_COMP_FORMAT_REG0_IDX                         2
#define _HRT_MIPI_BACKEND_RAW16_CONFIG_REG_IDX                         3
#define _HRT_MIPI_BACKEND_RAW18_CONFIG_REG_IDX                         4
#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_IDX                           5
#define _HRT_MIPI_BACKEND_IRQ_STATUS_REG_IDX                           6
#define _HRT_MIPI_BACKEND_IRQ_CLEAR_REG_IDX                            7
#define _HRT_MIPI_BACKEND_CUST_EN_REG_IDX                              8
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_REG_IDX                      9
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S0P0_REG_IDX                   10
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S0P1_REG_IDX                   11
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S0P2_REG_IDX                   12
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S0P3_REG_IDX                   13
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S1P0_REG_IDX                   14
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S1P1_REG_IDX                   15
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S1P2_REG_IDX                   16
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S1P3_REG_IDX                   17
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S2P0_REG_IDX                   18
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S2P1_REG_IDX                   19
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S2P2_REG_IDX                   20
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_S2P3_REG_IDX                   21
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_REG_IDX                  22

// number of fixed / always existing registers
#define _HRT_MIPI_BACKEND_NOF_REGISTERS                               23

// number of lut entry registers depends on the number of sids.
#define _HRT_MIPI_BACKEND_LUT_ENTRY_REG(idx)                          (_HRT_MIPI_BACKEND_NOF_REGISTERS + idx)
#define _HRT_MIPI_BACKEND_LUT_ENTRY_0_REG_IDX                         (_HRT_MIPI_BACKEND_NOF_REGISTERS + 0)

// Set of macros only to be used by C-tests, this is not CHDL compatible
// number of dtype rename registers depends on number of sids.
#define _HRT_MIPI_BACKEND_DTYPE_RENAME_REG(idx, NofSids) \
 (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+idx):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(1*NofSids)+idx)

#define _HRT_MIPI_BACKEND_DTYPE_RENAME_REG0_IDX(NofSids) \
 (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+0):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(1*NofSids)+0)

#define _HRT_MIPI_BACKEND_PIXFIELD_REMAP_REG(idx, NofSids) \
  (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+1):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(2*NofSids)+idx)

#define _HRT_MIPI_BACKEND_PIXFIELD_REMAP_0_REG_IDX(NofSids) \
  (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+1):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(2*NofSids)+0)

#define _HRT_MIPI_BACKEND_DISABLE_DUPLICATED_SYNC_REG(idx, NofSids) \
  (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+1+1):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(3*NofSids)+idx)

#define _HRT_MIPI_BACKEND_DISABLE_DUPLICATED_SYNC_0_REG_IDX(NofSids) \
  (NofSids==0)?(_HRT_MIPI_BACKEND_NOF_REGISTERS+1+1):(_HRT_MIPI_BACKEND_NOF_REGISTERS+(3*NofSids)+0)

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MIPI Backend Register Widths
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _HRT_MIPI_BACKEND_STATUS_REG_WIDTH                    1
#define _HRT_MIPI_BACKEND_MEM_ALIGNMENT_CONFIG_REG_WIDTH      2
#define _HRT_MIPI_BACKEND_COMP_FORMAT_REG_WIDTH               32

#define _HRT_MIPI_BACKEND_RAW16_CONFIG_REG_WIDTH              14
#define _HRT_CSS_RECEIVER_2400_BE_RAW16_DATAID_IDX            0
#define _HRT_CSS_RECEIVER_2400_BE_RAW16_EN_IDX                13

#define _HRT_MIPI_BACKEND_RAW18_CONFIG_REG_WIDTH              16
#define _HRT_CSS_RECEIVER_2400_BE_RAW18_DATAID_IDX            0
#define _HRT_CSS_RECEIVER_2400_BE_RAW18_OPTION_IDX            13
#define _HRT_CSS_RECEIVER_2400_BE_RAW18_EN_IDX                15

#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_WIDTH                15
#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_OPTION_LSB           0
#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_OPTION_MSB           1
#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_DTYPE_LSB            2
#define _HRT_MIPI_BACKEND_FORCE_RAW8_REG_DTYPE_MSB            14

#define _HRT_MIPI_BACKEND_IRQ_STATUS_REG_WIDTH                _HRT_MIPI_BACKEND_NOF_IRQS
#define _HRT_MIPI_BACKEND_IRQ_CLEAR_REG_WIDTH                 0

#define _HRT_MIPI_BACKEND_DTYPE_RENAME_REG_WIDTH              (13+1) // new dtype + feature enable
#define _HRT_MIPI_BACKEND_LUT_ENTRY_REG_WIDTH                 (4+4+1) // port_id + max_sid_width + active
#define _HRT_MIPI_BACKEND_PIXFIELD_REMAP_REG_WIDTH            (8)
#define _HRT_MIPI_BACKEND_DISABLE_SYNC_DUPLICATION_REG_WIDTH  1

/////////////////////////////////////////////////////////////////////////////////////////////////////

#define _HRT_MIPI_BACKEND_MAX_NOF_LUT_ENTRIES               32  // to satisfy hss model static array declaration
#define _HRT_MIPI_BACKEND_MAX_NOF_DTYPE_RENAME_REGS         32  // to satisfy hss model static array declaration

#define _HRT_MIPI_BACKEND_CHANNEL_ID_WIDTH                 2
#define _HRT_MIPI_BACKEND_FORMAT_TYPE_WIDTH                6
#define _HRT_MIPI_BACKEND_PACKET_ID_WIDTH                  _HRT_MIPI_BACKEND_CHANNEL_ID_WIDTH + _HRT_MIPI_BACKEND_FORMAT_TYPE_WIDTH

#define _HRT_MIPI_BACKEND_STREAMING_PIX_A_LSB                 0
#define _HRT_MIPI_BACKEND_STREAMING_PIX_A_MSB(pix_width)     (_HRT_MIPI_BACKEND_STREAMING_PIX_A_LSB + (pix_width) - 1)
#define _HRT_MIPI_BACKEND_STREAMING_PIX_A_VAL_BIT(pix_width) (_HRT_MIPI_BACKEND_STREAMING_PIX_A_MSB(pix_width) + 1)
#define _HRT_MIPI_BACKEND_STREAMING_PIX_B_LSB(pix_width)     (_HRT_MIPI_BACKEND_STREAMING_PIX_A_VAL_BIT(pix_width) + 1)
#define _HRT_MIPI_BACKEND_STREAMING_PIX_B_MSB(pix_width)     (_HRT_MIPI_BACKEND_STREAMING_PIX_B_LSB(pix_width) + (pix_width) - 1)
#define _HRT_MIPI_BACKEND_STREAMING_PIX_B_VAL_BIT(pix_width) (_HRT_MIPI_BACKEND_STREAMING_PIX_B_MSB(pix_width) + 1)
#define _HRT_MIPI_BACKEND_STREAMING_SOP_BIT(pix_width)       (_HRT_MIPI_BACKEND_STREAMING_PIX_B_VAL_BIT(pix_width) + 1)
#define _HRT_MIPI_BACKEND_STREAMING_EOP_BIT(pix_width)       (_HRT_MIPI_BACKEND_STREAMING_SOP_BIT(pix_width) + 1)
#define _HRT_MIPI_BACKEND_STREAMING_WIDTH(pix_width)         (_HRT_MIPI_BACKEND_STREAMING_EOP_BIT(pix_width) + 1)

/*************************************************************************************************/
/* Custom Decoding                                                                               */
/* These Custom Defs are defined based on design-time config in "mipi_backend_pixel_formatter.chdl" !! */
/*************************************************************************************************/
#define _HRT_MIPI_BACKEND_CUST_EN_IDX                     0     /* 2bits */
#define _HRT_MIPI_BACKEND_CUST_EN_DATAID_IDX              2     /* 13bits MIPI DATA ID */
#define _HRT_MIPI_BACKEND_CUST_EN_HIGH_PREC_IDX           15     // 1 bit
#define _HRT_MIPI_BACKEND_CUST_EN_WIDTH                   16
//#define _HRT_MIPI_BACKEND_CUST_MODE_ALL                   1     /* Enable Custom Decoding for all DATA IDs */
//#define _HRT_MIPI_BACKEND_CUST_MODE_ONE                   3     /* Enable Custom Decoding for ONE DATA ID, programmed in CUST_EN_DATA_ID */
//#define _HRT_MIPI_BACKEND_CUST_EN_OPTION_IDX              1

/* Data State config = {get_bits(6bits), valid(1bit)}  */
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_S0_IDX          0     /* 7bits */
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_S1_IDX          8     /* 7bits */
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_S2_IDX          16    /* was 14 7bits */
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_WIDTH           24    /* was 21*/
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_VALID_IDX       0     /* 1bits */
#define _HRT_MIPI_BACKEND_CUST_DATA_STATE_GETBITS_IDX     1     /* 6bits */

/* Pixel Extractor config */
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_DATA_ALIGN_IDX     0     /* 6bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_PIX_ALIGN_IDX      6     /* 5bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_PIX_MASK_IDX       11    /* was 10 18bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_PIX_EN_IDX         31    /* was 28 1bits */

#define _HRT_MIPI_BACKEND_CUST_PIX_EXT_WIDTH              32    /* was 29 */

/* Pixel Valid & EoP config = {[eop,valid](especial), [eop,valid](normal)} */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_P0_IDX        0    /* 4bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_P1_IDX        4    /* 4bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_P2_IDX        8    /* 4bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_P3_IDX        12   /* 4bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_WIDTH         16
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_NOR_VALID_IDX 0    /* Normal (NO less get_bits case) Valid - 1bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_NOR_EOP_IDX   1    /* Normal (NO less get_bits case) EoP - 1bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_ESP_VALID_IDX 2    /* Especial (less get_bits case) Valid - 1bits */
#define _HRT_MIPI_BACKEND_CUST_PIX_VALID_EOP_ESP_EOP_IDX   3    /* Especial (less get_bits case) EoP - 1bits */

/*************************************************************************************************/
/* MIPI backend output streaming interface definition                                            */
/* These parameters define the fields within the streaming bus. These should also be used by the */
/* subsequent block, ie stream2mmio.                                                             */
/*************************************************************************************************/
/* The pipe backend - stream2mmio should be design time configurable in                          */
/*   PixWidth - Number of bits per pixel                                                         */
/*   PPC      - Pixel per Clocks                                                                 */
/*   NumSids  - Max number of source Ids (ifc's)  and derived from that:                         */
/*   SidWidth - Number of bits required for the sid parameter                                    */
/* In order to keep this configurability, below Macro's have these as a parameter                */
/*************************************************************************************************/

#define HRT_MIPI_BACKEND_STREAM_EOP_BIT                      0
#define HRT_MIPI_BACKEND_STREAM_SOP_BIT                      1
#define HRT_MIPI_BACKEND_STREAM_EOF_BIT                      2
#define HRT_MIPI_BACKEND_STREAM_SOF_BIT                      3

#define HRT_MIPI_BACKEND_STREAM_CHID_LS_BIT                  4
#define HRT_MIPI_BACKEND_STREAM_CHID_MS_BIT(sid_width)      (HRT_MIPI_BACKEND_STREAM_CHID_LS_BIT+(sid_width)-1)

#define HRT_MIPI_BACKEND_STREAM_PIX_VAL_BIT(sid_width,p)    (HRT_MIPI_BACKEND_STREAM_SOF_BIT ) + (1)+(sid_width)+(p)

#define HRT_MIPI_BACKEND_STREAM_PIX_LS_BIT(sid_width,ppc,pix_width,p) (HRT_MIPI_BACKEND_STREAM_PIX_VAL_BIT(sid_width,ppc)+ ((pix_width)*p))
#define HRT_MIPI_BACKEND_STREAM_PIX_MS_BIT(sid_width,ppc,pix_width,p) (HRT_MIPI_BACKEND_STREAM_PIX_LS_BIT(sid_width,ppc,pix_width,p) + (pix_width) - 1)

#define HRT_MIPI_BACKEND_STREAM_ERR_FLAG_BIT(sid_width,ppc,pix_width)                     (HRT_MIPI_BACKEND_STREAM_PIX_MS_BIT(sid_width,ppc,pix_width,(ppc-1)) + 1)
#define HRT_MIPI_BACKEND_STREAM_ERR_CAUSE_LS_BIT(sid_width,ppc,pix_width)                 (HRT_MIPI_BACKEND_STREAM_ERR_FLAG_BIT(sid_width,ppc,pix_width) + 1)
#define HRT_MIPI_BACKEND_STREAM_ERR_CAUSE_MS_BIT(err_cause_width,sid_width,ppc,pix_width) (HRT_MIPI_BACKEND_STREAM_ERR_CAUSE_LS_BIT(sid_width,ppc,pix_width) + err_cause_width - 1)
#define HRT_MIPI_BACKEND_STREAM_FRAME_ID_LS_BIT(err_cause_width,sid_width,ppc,pix_width)  (HRT_MIPI_BACKEND_STREAM_ERR_CAUSE_MS_BIT(err_cause_width,sid_width,ppc,pix_width) + 1)
#define HRT_MIPI_BACKEND_STREAM_FRAME_ID_MS_BIT(err_cause_width,sid_width,ppc,pix_width)  (HRT_MIPI_BACKEND_STREAM_FRAME_ID_LS_BIT(err_cause_width,sid_width,ppc,pix_width) + 16 - 1)

#if 0
//#define HRT_MIPI_BACKEND_STREAM_PIX_BITS                    14
//#define HRT_MIPI_BACKEND_STREAM_CHID_BITS                    4
//#define HRT_MIPI_BACKEND_STREAM_PPC                          4
#endif

#define HRT_MIPI_BACKEND_STREAM_BITS(sid_width,ppc,pix_width)         (HRT_MIPI_BACKEND_STREAM_PIX_MS_BIT(sid_width,ppc,pix_width,(ppc-1))+1)

/* SP and LP LUT BIT POSITIONS */
#define HRT_MIPI_BACKEND_LUT_PKT_DISREGARD_BIT              0                                                                                           // 0
#define HRT_MIPI_BACKEND_LUT_SID_LS_BIT                     HRT_MIPI_BACKEND_LUT_PKT_DISREGARD_BIT + 1                                                  // 1
#define HRT_MIPI_BACKEND_LUT_SID_MS_BIT(sid_width)          (HRT_MIPI_BACKEND_LUT_SID_LS_BIT+(sid_width)-1)                                             // 1 + (4) - 1 = 4
#define HRT_MIPI_BACKEND_LUT_MIPI_CH_ID_LS_BIT(sid_width)   HRT_MIPI_BACKEND_LUT_SID_MS_BIT(sid_width) + 1                                              // 5
#define HRT_MIPI_BACKEND_LUT_MIPI_CH_ID_MS_BIT(sid_width)   HRT_MIPI_BACKEND_LUT_MIPI_CH_ID_LS_BIT(sid_width) + _HRT_MIPI_BACKEND_CHANNEL_ID_WIDTH - 1  // 6
#define HRT_MIPI_BACKEND_LUT_MIPI_FMT_LS_BIT(sid_width)     HRT_MIPI_BACKEND_LUT_MIPI_CH_ID_MS_BIT(sid_width) + 1                                       // 7
#define HRT_MIPI_BACKEND_LUT_MIPI_FMT_MS_BIT(sid_width)     HRT_MIPI_BACKEND_LUT_MIPI_FMT_LS_BIT(sid_width) + _HRT_MIPI_BACKEND_FORMAT_TYPE_WIDTH - 1   // 12

/* #define HRT_MIPI_BACKEND_SP_LUT_BITS(sid_width)             HRT_MIPI_BACKEND_LUT_MIPI_CH_ID_MS_BIT(sid_width) + 1                                       // 7          */

#define HRT_MIPI_BACKEND_SP_LUT_BITS(sid_width)             HRT_MIPI_BACKEND_LUT_SID_MS_BIT(sid_width) + 1
#define HRT_MIPI_BACKEND_LP_LUT_BITS(sid_width)             HRT_MIPI_BACKEND_LUT_MIPI_FMT_MS_BIT(sid_width) + 1                                         // 13

// temp solution
//#define HRT_MIPI_BACKEND_STREAM_PIXA_VAL_BIT                HRT_MIPI_BACKEND_STREAM_CHID_MS_BIT  + 1                                    // 8
//#define HRT_MIPI_BACKEND_STREAM_PIXB_VAL_BIT                HRT_MIPI_BACKEND_STREAM_PIXA_VAL_BIT + 1                                    // 9
//#define HRT_MIPI_BACKEND_STREAM_PIXC_VAL_BIT                HRT_MIPI_BACKEND_STREAM_PIXB_VAL_BIT + 1                                    // 10
//#define HRT_MIPI_BACKEND_STREAM_PIXD_VAL_BIT                HRT_MIPI_BACKEND_STREAM_PIXC_VAL_BIT + 1                                    // 11
//#define HRT_MIPI_BACKEND_STREAM_PIXA_LS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXD_VAL_BIT + 1                                    // 12
//#define HRT_MIPI_BACKEND_STREAM_PIXA_MS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXA_LS_BIT  + HRT_MIPI_BACKEND_STREAM_PIX_BITS - 1 // 25
//#define HRT_MIPI_BACKEND_STREAM_PIXB_LS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXA_MS_BIT + 1                                     // 26
//#define HRT_MIPI_BACKEND_STREAM_PIXB_MS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXB_LS_BIT  + HRT_MIPI_BACKEND_STREAM_PIX_BITS - 1 // 39
//#define HRT_MIPI_BACKEND_STREAM_PIXC_LS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXB_MS_BIT + 1                                     // 40
//#define HRT_MIPI_BACKEND_STREAM_PIXC_MS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXC_LS_BIT  + HRT_MIPI_BACKEND_STREAM_PIX_BITS - 1 // 53
//#define HRT_MIPI_BACKEND_STREAM_PIXD_LS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXC_MS_BIT + 1                                     // 54
//#define HRT_MIPI_BACKEND_STREAM_PIXD_MS_BIT                 HRT_MIPI_BACKEND_STREAM_PIXD_LS_BIT  + HRT_MIPI_BACKEND_STREAM_PIX_BITS - 1 // 67

// vc hidden in pixb data (passed as raw12 the pipe)
#define HRT_MIPI_BACKEND_STREAM_VC_LS_BIT(sid_width,ppc,pix_width)  HRT_MIPI_BACKEND_STREAM_PIX_LS_BIT(sid_width,ppc,pix_width,1) + 10  //HRT_MIPI_BACKEND_STREAM_PIXB_LS_BIT + 10 // 36
#define HRT_MIPI_BACKEND_STREAM_VC_MS_BIT(sid_width,ppc,pix_width)  HRT_MIPI_BACKEND_STREAM_VC_LS_BIT(sid_width,ppc,pix_width) + 1    // 37

#endif /* _mipi_backend_defs_h */
