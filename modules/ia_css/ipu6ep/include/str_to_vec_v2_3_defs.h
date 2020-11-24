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
#ifndef _str_to_vec_defs_h_
#define _str_to_vec_defs_h_

#define _STR_TO_VEC_V2_3_USE_BAYER         use_bayer
#define _STR_TO_VEC_V2_3_USE_BAYER_2PPC    use_bayer_2ppc
#define _STR_TO_VEC_V2_3_USE_YUV420        use_yuv420
#define _STR_TO_VEC_V2_3_USE_YUV           use_yuv
#define _STR_TO_VEC_V2_3_USE_RGB           use_rgb        // ww2013'35 BXT

// ------------------------------- User properties      ---------------------
#define _STR_TO_VEC_V2_3_CMD_FIFO_DEPTH      cmd_fifo_depth
#define _STR_TO_VEC_V2_3_INT_ACK_FIFO_DEPTH  int_ack_fifo_depth
#define _STR_TO_VEC_V2_3_ACK_IN_FIFO_DEPTH   ack_in_fifo_depth
#define _STR_TO_VEC_V2_3_STR_IN_FIFO_DEPTH   str_in_fifo_depth
#define _STR_TO_VEC_V2_3_ACK_DL_DEPTH        ack_dl_depth        // cycles to wait before ack sending (if rpwc == 0)
#define _STR_TO_VEC_V2_3_USE_SPIF_INPUT      use_spif_input     // ww2014'20 vnikolin: islc s2v
#define _STR_TO_VEC_V2_3_IS_INPUT_SYS_INST   is_input_sys_inst  // ww2014'20 vnikolin: islc s2v

// ------------------------------- Various -------------------------------
#define _STR_TO_VEC_V2_3_PIXEL_BITS          pixel_bits

#define PIF_EOL_IDX                          0                    // he bit in PIF to indicate the last line has started
#ifndef PIF_EOF_MASK
#define PIF_EOF_MASK                         0x04
#endif

#ifndef PIF_EOP_MASK
#define PIF_EOP_MASK                         0x01
#endif

#ifndef YUV_SUP_PIX_START_BIT
#define YUV_SUP_PIX_START_BIT                10
#endif

#ifndef BAYER_SUP_PIX_START_BIT
#define BAYER_SUP_PIX_START_BIT              8
#endif

#define _STR_TO_VEC_V2_3_CMD_INIT            16
#define _STR_TO_VEC_V2_3_CMD_PROC_N_VEC      1

#define _STR_TO_VEC_V2_3_ACK_INIT            0
#define _STR_TO_VEC_V2_3_ACK_N_VEC           1

#define _STR_TO_VEC_V2_3_REG_ALIGN           4

#define PIX_COMP_PER_STR_BAYER8PPC           8
#define PIX_COMP_PER_STR_BAYER4PPC           4
#define PIX_COMP_PER_STR_YUV420              6
#define PIX_COMP_PER_STR_YUV422              8
#define PIX_COMP_PER_STR_RGB                 12

#define _DYNAMIC_BAYER4PPC                   1                  // ww2014'20 vnikolin: islc s2v
#define _DYNAMIC_YUV420                      0

#define _STR_TO_VEC_BUFFER_SIZE              0x10000            // ww2015'42 vnikolin s2v_memory usage
#define _STR_TO_VEC_BUFREG_WIDTH             32                 // ww2016'08 s2v fix

// ------------------------------- Registers ------------------------------
#define _STR_TO_VEC_V2_3_CMD                           0
#define _STR_TO_VEC_V2_3_ACK_K_VEC_REG                 1
#define _STR_TO_VEC_V2_3_PXL_LINE_REG                  2
#define _STR_TO_VEC_V2_3_LINE_FRAME_REG                3
#define _STR_TO_VEC_V2_3_YUV420_EN_REG                 4
#define _STR_TO_VEC_V2_3_INTERLEAVE_EN_REG             5
#define _STR_TO_VEC_V2_3_DEV_NULL_EN_REG               6
#define _STR_TO_VEC_V2_3_IRQ_FALSE_CMD_REG             7
#define _STR_TO_VEC_V2_3_PXL_CUR_LINE_REG              44
#define _STR_TO_VEC_V2_3_LINES_DONE_REG                45
#define _STR_TO_VEC_V2_3_IO_STATUS_REG                 46
#define _STR_TO_VEC_V2_3_ACK_STATUS_REG                47
#define _STR_TO_VEC_V2_3_MAIN_STATUS_REG               48
#define _STR_TO_VEC_V2_3_TRACK_STATUS_REG              49
#define _STR_TO_VEC_V2_3_CRUN_REG                      50
#define _STR_TO_VEC_V2_3_RPWC_EN_REG                   51
#define _STR_TO_VEC_V2_3_BUF_ADDR_BYTE_ALIGNED_REG     52
#define _STR_TO_VEC_V2_3_FRAME_WIDTH_EN_REG            53
#define _STR_TO_VEC_V2_3_FRAME_WIDTH_REG               54
#define _STR_TO_VEC_V2_3_BUF_STRIDE_ON_EOF_CMD_EN_REG  55
#define _STR_TO_VEC_V2_3_BAYER_MODE_EN_REG             56
#define _STR_TO_VEC_V2_3_UV_INTERLEAVE_EN_REG          57

#define _STR_TO_VEC_V2_3_0_ST_ADDR   8
#define _STR_TO_VEC_V2_3_0_END_ADDR  9
#define _STR_TO_VEC_V2_3_0_OFFSET_0  10
#define _STR_TO_VEC_V2_3_0_OFFSET_1  11
#define _STR_TO_VEC_V2_3_0_STRIDE    12

#define _STR_TO_VEC_V2_3_1_ST_ADDR   13
#define _STR_TO_VEC_V2_3_1_END_ADDR  14
#define _STR_TO_VEC_V2_3_1_OFFSET_0  15
#define _STR_TO_VEC_V2_3_1_OFFSET_1  16
#define _STR_TO_VEC_V2_3_1_STRIDE    17

#define _STR_TO_VEC_V2_3_2_ST_ADDR   18
#define _STR_TO_VEC_V2_3_2_END_ADDR  19
#define _STR_TO_VEC_V2_3_2_OFFSET_0  20
#define _STR_TO_VEC_V2_3_2_OFFSET_1  21
#define _STR_TO_VEC_V2_3_2_STRIDE    22

#define _STR_TO_VEC_V2_3_3_ST_ADDR   23
#define _STR_TO_VEC_V2_3_3_END_ADDR  24
#define _STR_TO_VEC_V2_3_3_OFFSET_0  25
#define _STR_TO_VEC_V2_3_3_OFFSET_1  26
#define _STR_TO_VEC_V2_3_3_STRIDE    27

#define _STR_TO_VEC_V2_3_4_ST_ADDR   28
#define _STR_TO_VEC_V2_3_4_END_ADDR  29
#define _STR_TO_VEC_V2_3_4_OFFSET_0  30
#define _STR_TO_VEC_V2_3_4_OFFSET_1  31
#define _STR_TO_VEC_V2_3_4_STRIDE    32

#define _STR_TO_VEC_V2_3_5_ST_ADDR   33
#define _STR_TO_VEC_V2_3_5_END_ADDR  34
#define _STR_TO_VEC_V2_3_5_OFFSET_0  35
#define _STR_TO_VEC_V2_3_5_OFFSET_1  36
#define _STR_TO_VEC_V2_3_5_STRIDE    37

#define _STR_TO_VEC_V2_3_0_CUR_ADDR  38
#define _STR_TO_VEC_V2_3_1_CUR_ADDR  39
#define _STR_TO_VEC_V2_3_2_CUR_ADDR  40
#define _STR_TO_VEC_V2_3_3_CUR_ADDR  41
#define _STR_TO_VEC_V2_3_4_CUR_ADDR  42
#define _STR_TO_VEC_V2_3_5_CUR_ADDR  43

#endif /* _str_to_vec_defs_h_ */
