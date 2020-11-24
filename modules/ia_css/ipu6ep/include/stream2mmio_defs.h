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
#ifndef _STREAM2MMMIO_DEFS_H
#define _STREAM2MMMIO_DEFS_H

#include <mipi_backend_defs.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* MIPI_STREAM2MMIO: Incoming MIPI CSI2 stream definition */
#define HRT_CSI2_RX_DATA_LSB             0
#define HRT_CSI2_RX_DATA_MSB            31
#define HRT_CSI2_RX_SOP                 32
#define HRT_CSI2_RX_EOP                 33
#define HRT_CSI2_RX_ERROR_FLAG          34
#define HRT_CSI2_RX_ERROR_CAUSE_LSB     35
#define HRT_CSI2_RX_ERROR_CAUSE_MSB     46
#define HRT_CSI2_RX_FRAME_ID_LSB        47
#define HRT_CSI2_RX_FRAME_ID_MSB        62

/* DO NOT TOUCH THE FOLLOWING */
#define HRT_CSI2_RX_DATA_WIDTH          (HRT_CSI2_RX_DATA_MSB - HRT_CSI2_RX_DATA_LSB + 1)
#define HRT_CSI2_RX_SOP_WIDTH           1
#define HRT_CSI2_RX_EOP_WIDTH           1
#define HRT_CSI2_RX_ERROR_FLAG_WIDTH    1
#define HRT_CSI2_RX_ERROR_CAUSE_WIDTH   HRT_CSI2_RX_ERROR_CAUSE_MSB - HRT_CSI2_RX_ERROR_CAUSE_LSB + 1
#define HRT_CSI2_RX_FRAME_ID_WIDTH      HRT_CSI2_RX_FRAME_ID_MSB - HRT_CSI2_RX_FRAME_ID_LSB + 1

#define HRT_CSI2_RX_PPI_WIDTH            HRT_CSI2_RX_DATA_WIDTH + HRT_CSI2_RX_SOP_WIDTH + \
                                        HRT_CSI2_RX_EOP_WIDTH + HRT_CSI2_RX_ERROR_FLAG_WIDTH + \
                                        HRT_CSI2_RX_ERROR_CAUSE_WIDTH + HRT_CSI2_RX_FRAME_ID_WIDTH
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* MIPI_STREAM2MMIO: Incoming MIPI CSI3 stream definition */
#define HRT_CSI3_RX_DATA_LSB             0
#define HRT_CSI3_RX_DATA_MSB            63
#define HRT_CSI3_RX_SOP                 64
#define HRT_CSI3_RX_EOP                 65
#define HRT_CSI3_RX_ERROR_FLAG          66
#define HRT_CSI3_RX_ERROR_CAUSE_LSB     67
#define HRT_CSI3_RX_ERROR_CAUSE_MSB     82
#define HRT_CSI3_RX_FRAME_ID_LSB        83
#define HRT_CSI3_RX_FRAME_ID_MSB        98
#define HRT_CSI3_RX_LINE_ID_LSB         99
#define HRT_CSI3_RX_LINE_ID_MSB         114
#define HRT_CSI3_RX_DATA_VALID_LSB      115
#define HRT_CSI3_RX_DATA_VALID_MSB      122

/* DO NOT TOUCH THE FOLLOWING */
#define HRT_CSI3_RX_DATA_WIDTH          HRT_CSI3_RX_DATA_MSB - HRT_CSI3_RX_DATA_LSB + 1
#define HRT_CSI3_RX_SOP_WIDTH           1
#define HRT_CSI3_RX_EOP_WIDTH           1
#define HRT_CSI3_RX_ERROR_FLAG_WIDTH    1
#define HRT_CSI3_RX_ERROR_CAUSE_WIDTH   HRT_CSI3_RX_ERROR_CAUSE_MSB - HRT_CSI3_RX_ERROR_CAUSE_LSB + 1
#define HRT_CSI3_RX_FRAME_ID_WIDTH      HRT_CSI3_RX_FRAME_ID_MSB - HRT_CSI3_RX_FRAME_ID_LSB  + 1
#define HRT_CSI3_RX_LINE_ID_WIDTH        HRT_CSI3_RX_LINE_ID_MSB - HRT_CSI3_RX_LINE_ID_LSB + 1
#define HRT_CSI3_RX_DATA_VALID_WIDTH    HRT_CSI3_RX_DATA_VALID_MSB - HRT_CSI3_RX_DATA_VALID_LSB + 1

#define HRT_CSI3_RX_PPI_WIDTH           HRT_CSI3_RX_DATA_WIDTH + HRT_CSI3_RX_SOP_WIDTH + HRT_CSI3_RX_EOP_WIDTH + \
                                        HRT_CSI3_RX_ERROR_FLAG_WIDTH + HRT_CSI3_RX_ERROR_CAUSE_WIDTH + \
                                        HRT_CSI3_RX_FRAME_ID_WIDTH + HRT_CSI3_RX_LINE_ID_WIDTH + \
                                        HRT_CSI3_RX_DATA_VALID_WIDTH
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* MIPI_STREAM2MMIO: Incoming MIPI CPHY stream definition */
#define HRT_CPHY_RX_DATA_LSB             0
#define HRT_CPHY_RX_DATA_MSB            63
#define HRT_CPHY_RX_SOP                 64
#define HRT_CPHY_RX_EOP                 65
#define HRT_CPHY_RX_ERROR_FLAG          66
#define HRT_CPHY_RX_ERROR_CAUSE_LSB     67
#define HRT_CPHY_RX_ERROR_CAUSE_MSB     78
#define HRT_CPHY_RX_FRAME_ID_LSB        79
#define HRT_CPHY_RX_FRAME_ID_MSB        94

/* DO NOT TOUCH THE FOLLOWING */
#define HRT_CPHY_RX_DATA_WIDTH          (HRT_CPHY_RX_DATA_MSB - HRT_CPHY_RX_DATA_LSB + 1)
#define HRT_CPHY_RX_SOP_WIDTH           1
#define HRT_CPHY_RX_EOP_WIDTH           1
#define HRT_CPHY_RX_ERROR_FLAG_WIDTH    1
#define HRT_CPHY_RX_ERROR_CAUSE_WIDTH   HRT_CPHY_RX_ERROR_CAUSE_MSB - HRT_CPHY_RX_ERROR_CAUSE_LSB + 1
#define HRT_CPHY_RX_FRAME_ID_WIDTH      HRT_CPHY_RX_FRAME_ID_MSB - HRT_CPHY_RX_FRAME_ID_LSB + 1

#define HRT_CPHY_RX_PPI_WIDTH            HRT_CPHY_RX_DATA_WIDTH + HRT_CPHY_RX_SOP_WIDTH + \
                                        HRT_CPHY_RX_EOP_WIDTH + HRT_CPHY_RX_ERROR_FLAG_WIDTH + \
                                        HRT_CPHY_RX_ERROR_CAUSE_WIDTH + HRT_CPHY_RX_FRAME_ID_WIDTH

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* MIPI_STREAM2MMIO: CSI2 PACKET_HEADER definition */
#define HRT_CSI2_PH_WORDCOUNT_BITS      16
#define HRT_CSI2_PH_FMT_BITS            6
#define HRT_CSI2_PH_VC_BITS             2

/* MIPI_STREAM2MMIO: CSIX_PACKET_HEADER definition */
#define HRT_CSIX_PH_WORDCOUNT_LSB        0
#define HRT_CSIX_PH_WORDCOUNT_MSB        15
#define HRT_CSIX_PH_DTYPE_LSB           16
#define HRT_CSIX_PH_DTYPE_MSB           28
#define HRT_CSIX_PH_SYNC_LSB            29
#define HRT_CSIX_PH_SYNC_MSB            30
#define HRT_CSIX_PH_SENSOR_TYPE         31

#define HRT_CSIX_PH_SID_LSB             32
#define HRT_CSIX_PH_SID_MSB             35
#define HRT_CSIX_PH_PID_LSB             36
#define HRT_CSIX_PH_PID_MSB             39
#define HRT_CSIX_PH_ODD_EVEN_BIT         63

/* DO NOT TOUCH THE FOLLOWING */
#define HRT_CSIX_PH_DTYPE_WIDTH         (HRT_CSIX_PH_DTYPE_MSB - HRT_CSIX_PH_DTYPE_LSB + 1)
#define HRT_CSIX_PH_SENSOR_TYPE_WIDTH   1
#define HRT_CSIX_PH_SYNC_WIDTH          (HRT_CSIX_PH_SYNC_MSB - HRT_CSIX_PH_SYNC_LSB + 1)
#define HRT_CSIX_PH_WORDCOUNT_WIDTH      (HRT_CSIX_PH_WORDCOUNT_MSB - HRT_CSIX_PH_WORDCOUNT_LSB + 1)
#define HRT_CSIX_PH_SID_WIDTH           (HRT_CSIX_PH_SID_MSB - HRT_CSIX_PH_SID_LSB + 1)
#define HRT_CSIX_PH_PID_WIDTH           (HRT_CSIX_PH_PID_MSB - HRT_CSIX_PH_PID_LSB + 1)
#define HRT_CSIX_PH_ODD_EVEN_WIDTH        1
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NOF_IRQS_PER_SID 3
#define IRQ_BIT_ERR_MODE_ENTERED    0
#define IRQ_BIT_PH_LENGHT_MISMATCH  1
#define IRQ_BIT_LINE_CNTR_OVERFLOW  2

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// NON SID RELATED REGISTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define _MIPI_STREAM2MMIO_COMMON_REG_OFFSET             16

#define _MIPI_STREAM2MMIO_ERR_MODE_DC_CTRL_REG_ID       0 // RW
#define _MIPI_STREAM2MMIO_ERR_MODE_FILL_VAL_REG_ID      1 // RW

#define _MIPI_STREAM2MMIO_VC_ENABLE_REG_ID              2 // WO
#define _MIPI_STREAM2MMIO_VC_DISABLE_REG_ID             3 // WO
#define _MIPI_STREAM2MMIO_VC_STATUS_REG_ID              4 // RO

#define _MIPI_STREAM2MMIO_PORT_ID_REG_ID                5 // RW
#define _MIPI_STREAM2MMIO_FRAME_SYNC_ESC_REG_ID         6 // RW

#define _MIPI_STREAM2MMIO_NOF_COMMON_REGS               7

#define _MIPI_STREAM2MMIO_HSS_ONLY_CRUN_ERROR_REG_ID    _MIPI_STREAM2MMIO_NOF_COMMON_REGS

#define _MIPI_STREAM2MMIO_CSI2_VC_SIDS_ENABLE_REG_WIDTH   1
#define _MIPI_STREAM2MMIO_PORT_ID_REG_WIDTH               4
#define _MIPI_STREAM2MMIO_ERR_MODE_FILL_VAL_REG_WIDTH     1
#define _MIPI_STREAM2MMIO_ERR_MODE_DC_CTRL_REG_WIDTH      16
#define _MIPI_STREAM2MMIO_FRAME_SYNC_ESC_REG_WIDTH        1

//#define _MIPI_STREAM2MMIO_ERR_MODE_ENABLE_REG_WIDTH     1
//#define _MIPI_STREAM2MMIO_ERR_MODE_PAUSE_CNTR_REG_WIDTH 4

///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _STREAM2MMIO_REG_ALIGN                  4

#define _STREAM2MMIO_COMMAND_REG_ID             0
#define _STREAM2MMIO_PIX_WIDTH_ID_REG_ID        1
#define _STREAM2MMIO_START_ADDR_REG_ID          2      /* master port address,NOT Byte */
#define _STREAM2MMIO_END_ADDR_REG_ID            3      /* master port address,NOT Byte */
#define _STREAM2MMIO_STRIDE_REG_ID              4      /* stride in master port words, increment is per packet for long sids, stride is not used for short sid's*/
#define _STREAM2MMIO_NUM_ITEMS_REG_ID           5      /* number of packets for store packets cmd, number of words for store_words cmd */
#define _STREAM2MMIO_BLOCK_WHEN_NO_CMD_REG_ID   6      /* if this register is 1, input will be stalled if there is no pending command for this sid */
//#define _STREAM2MMIO_PACKER_SELECT_REG_ID       _STREAM2MMIO_BLOCK_WHEN_NO_CMD_REG_ID /* packer select is replacing block when no cmd functionality */
#define _STREAM2MMIO_ACK_BASE_ADDR_REG_ID       7
#define _STREAM2MMIO_SIDPID_REG_ID              8
#define _STREAM2MMIO_DATAFIELD_REMAP_CFG_REG_ID 9
#define _STREAM2MMIO_RPWC_ENABLE_REG_ID         10
#define _STREAM2MMIO_DUAL_DEST_CFG_REG_ID       11

#define _STREAM2MMIO_REGS_PER_SID               12
#define _STREAM2MMIO_SID_REG_OFFSET             16

#define _STREAM2MMIO_START_ADDR_REG_WIDTH       (26+6) /* +6 because of IPU6 byte_alignment */
#define _STREAM2MMIO_END_ADDR_REG_WIDTH         (26+6) /* +6 because of IPU6 byte_alignment */
#define _STREAM2MMIO_STRIDE_REG_WIDTH           (26+6) /* +6 because of IPU6 byte_alignment */
//#define _STREAM2MMIO_ACK_BASE_ADDR_REG_WIDTH    (30-2) /* -2 because of IPU6 byte_alignment */
#define _STREAM2MMIO_ACK_BASE_ADDR_REG_WIDTH    (30+2) /* +2 because of IPU6 byte_alignment */
#define _STREAM2MMIO_NUM_ITEMS_REG_WIDTH        (16+6)
#define _STREAM2MMIO_BLOCK_WHEN_NO_CMD_REG_WIDTH 1
#define _STREAM2MMIO_RPWC_ENABLE_REG_WIDTH       1
//#define _STREAM2MMIO_DUAL_DEST_CFG_REG_WIDTH    (1+4) // or 0 when only one packer is instantiated.
//#define _STREAM2MMIO_SINGLE_PACKER_SELECT_SID_REG_WIDTH      0

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// MIPI STREAM2MMIO SPECIFIC
#define _MIPI_STREAM2MMIO_LUT_ENTRY_REG_ID              12
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_REG_ID           13
#define _MIPI_STREAM2MMIO_ERR_MODE_ENABLE_REG_ID        14
#define _MIPI_STREAM2MMIO_CROP_ENABLE_REG_ID            15 // JK TBD: remain for time being until integraion tests are updated
#define _MIPI_STREAM2MMIO_FRAME_MANIPULATE_REG_ID       15

#define _MIPI_STREAM2MMIO_REGS_PER_SID                  16
#define _MIPI_STREAM2MMIO_SID_REG_OFFSET                16

#define _STREAM2MMIO_DUAL_DEST_CFG_REG_WIDTH            8
//#define _STREAM2MMIO_ACK_BASE_ADDR_REG_WIDTH      18
//#define _STREAM2MMIO_SIDPID_REG_WIDTH           --> defined by parameters

//#define _MIPI_STREAM2MMIO_LUT_ENTRY_REG_WIDTH(vc_bits)  (1+1+(vc_bits)+13+1) // OE_SEL + OE_ACT + VC + DTYPE + VALID
#define _MIPI_STREAM2MMIO_LUT_ENTRY_REG_WIDTH           (1+1+5+13+1) // OE_SEL + OE_ACT + VC + DTYPE + VALID
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_REG_WIDTH        32           // nof lines (16) + line length (16)
#define _MIPI_STREAM2MMIO_ERR_MODE_ENABLE_REG_WIDTH     2
#define _MIPI_STREAM2MMIO_CROP_ENABLE_REG_WIDTH         2 // JK TBD: remain for time being until integration tests are updated
#define _MIPI_STREAM2MMIO_FRAME_MANIPULATE_REG_WIDTH    3
//#define _MIPI_STREAM2MMIO_REMOVE_PH_ENABLE_REG_WIDTH    1

#define _MIPI_STREAM2MMIO_LUT_ENTRY_VALID_BIT           0
#define _MIPI_STREAM2MMIO_LUT_ENTRY_DTYPE_LSB           1
#define _MIPI_STREAM2MMIO_LUT_ENTRY_DTYPE_MSB           13
#define _MIPI_STREAM2MMIO_LUT_ENTRY_VC_LSB              14
//#define _MIPI_STREAM2MMIO_LUT_ENTRY_VC_MSB(vc_bits)     (_MIPI_STREAM2MMIO_LUT_ENTRY_VC_LSB + vc_bits - 1)
//#define _MIPI_STREAM2MMIO_LUT_ENTRY_OE_ACT(vc_bits)     (_MIPI_STREAM2MMIO_LUT_ENTRY_VC_LSB + vc_bits - 1) + 1
//#define _MIPI_STREAM2MMIO_LUT_ENTRY_OE_SEL(vc_bits)     (_MIPI_STREAM2MMIO_LUT_ENTRY_VC_LSB + vc_bits - 1) + 1 + 1
#define _MIPI_STREAM2MMIO_LUT_ENTRY_VC_MSB              18
#define _MIPI_STREAM2MMIO_LUT_ENTRY_OE_ACT              19
#define _MIPI_STREAM2MMIO_LUT_ENTRY_OE_SEL              20

#define _MIPI_STREAM2MMIO_FRAME_CONFIG_NOFLINES_LSB     16
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_NOFLINES_MSB     31
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_NOFLINES_WIDTH   16
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_LINELENGTH_LSB   0
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_LINELENGTH_MSB   15
#define _MIPI_STREAM2MMIO_FRAME_CONFIG_LINELENGTH_WIDTH 16

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _STREAM2MMIO_MAX_NOF_SIDS              64      /* value used in hss model */

/* command token definition     */
#define _STREAM2MMIO_CMD_TOKEN_CMD_LSB          0      /* bits 1-0 is for the command field */
#define _STREAM2MMIO_CMD_TOKEN_CMD_MSB          1

#define _STREAM2MMIO_CMD_TOKEN_WIDTH           (_STREAM2MMIO_CMD_TOKEN_CMD_MSB+1-_STREAM2MMIO_CMD_TOKEN_CMD_LSB)

#define _STREAM2MMIO_CMD_TOKEN_STORE_WORDS              0      /* command for storing a number of output words indicated by reg _STREAM2MMIO_NUM_ITEMS */
#define _STREAM2MMIO_CMD_TOKEN_STORE_PACKETS            1      /* command for storing a number of packets indicated by reg _STREAM2MMIO_NUM_ITEMS      */
#define _STREAM2MMIO_CMD_TOKEN_SYNC_FRAME               2      /* command for waiting for a frame start for storing                                               */
#define _STREAM2MMIO_CMD_TOKEN_SYNC_FRAME_SKIP          3      /* command for waiting for a frame start for controlled discarding                                               */

/* acknowledges from packer module */
/* fields: eof   - indicates whether last (short) packet received was an eof packet */
/*         eop   - indicates whether command has ended due to packet end or due to no of words requested has been received */
/*         count - indicates number of words stored */
#define _STREAM2MMIO_PACK_NUM_ITEMS_BITS        16
#define _STREAM2MMIO_PACK_ACK_EOP_BIT           _STREAM2MMIO_PACK_NUM_ITEMS_BITS   // 16
#define _STREAM2MMIO_PACK_ACK_EOF_BIT           (_STREAM2MMIO_PACK_ACK_EOP_BIT+1)  // 17
#define _STREAM2MMIO_PACK_ACK_ERR_BIT           (_STREAM2MMIO_PACK_ACK_EOF_BIT+1)  // 18

/* acknowledge token definition */
#define _STREAM2MMIO_ACK_TOKEN_NUM_ITEMS_LSB    0      /* bits 3-0 is for the command field */
#define _STREAM2MMIO_ACK_TOKEN_NUM_ITEMS_MSB   (_STREAM2MMIO_PACK_NUM_ITEMS_BITS-1)
#define _STREAM2MMIO_ACK_TOKEN_EOP_BIT         _STREAM2MMIO_PACK_ACK_EOP_BIT
#define _STREAM2MMIO_ACK_TOKEN_EOF_BIT         _STREAM2MMIO_PACK_ACK_EOF_BIT
#define _STREAM2MMIO_ACK_TOKEN_VALID_BIT       (_STREAM2MMIO_ACK_TOKEN_EOF_BIT+1)      /* this bit indicates a valid ack    */
                                                                                       /* if there is no valid ack, a read  */
                                                                                       /* on the ack register returns 0     */
#define _STREAM2MMIO_ACK_TOKEN_WIDTH           (_STREAM2MMIO_ACK_TOKEN_VALID_BIT+1)

/* commands for packer module */
#define _STREAM2MMIO_PACK_CMD_STORE_WORDS        0
#define _STREAM2MMIO_PACK_CMD_STORE_LONG_PACKET  1
#define _STREAM2MMIO_PACK_CMD_STORE_SHORT_PACKET 2

#endif /* _STREAM2MMIO_DEFS_H */
