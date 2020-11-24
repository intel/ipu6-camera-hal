/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2017 Intel Corporation.
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
#ifndef _ibuf_cntrl_2600_defs_h_
#define _ibuf_cntrl_2600_defs_h_

#include <stream2mmio_defs.h>

#define _IBC_2600_REG_ALIGN 4

/* alignment of register banks: */
#define _IBC_2600_REG_IDX_BITS          4
#define _IBC_2600_GROUP_IDX_BITS        3

#define _IBC_2600_GROUP_SHARED          0
#define _IBC_2600_GROUP_PROC_CFG        1
#define _IBC_2600_GROUP_PROC_CMD        2
#define _IBC_2600_GROUP_PROC_STAT       3
#define _IBC_2600_GROUP_DEST_CFG        4
#define _IBC_2600_GROUP_FR_CHECK        5
#define _IBC_2600_GROUP_FEEDER_CFG      6
#define _IBC_2600_GROUP_FEEDER_STAT     7

/* the actual amount of registers: */
#define _IBC_2600_CONFIG_REGS_SHARED                      12
/* for the proc reg sets, the total amount is depended on the nr dests. The number given here is WITHOUT the dest depended registers:*/
#define _IBC_2600_CONFIG_REGS_PROC_CFG                    11
#define _IBC_2600_CONFIG_REGS_PROC_CMD                    8
#define _IBC_2600_CONFIG_REGS_PROC_STAT                   8
#define _IBC_2600_CONFIG_REGS_DEST_CFG                    12
#define _IBC_2600_CONFIG_REGS_FR_CHECK                    7
#define _IBC_2600_CONFIG_REGS_FEEDER_CFG                  13
#define _IBC_2600_CONFIG_REGS_FEEDER_STAT                 4
/* for the proc reg sets, the total amount is depended on the nr dests:*/
#define _IBC_2600_CONFIG_REGS_PROC_CFG_DEST(nr_dests)     (_IBC_2600_CONFIG_REGS_PROC_CFG + (nr_dests))
#define _IBC_2600_CONFIG_REGS_PROC_CMD_DEST(nr_dests)     (_IBC_2600_CONFIG_REGS_PROC_CMD + (nr_dests))
#define _IBC_2600_CONFIG_REGS_PROC_STAT_DEST(nr_dests)    (_IBC_2600_CONFIG_REGS_PROC_STAT + 2*(nr_dests))

/* GDA defines */
#define _IBC_2600_GDA_FREE_PAGE_REG                 0 /* _GDA_CMD_FREE_PAGE */

/* Str2MMIO defines */
#define _IBC_2600_STREAM2MMIO_ACK_REG               1
#define _IBC_2600_STREAM2MMIO_CMD_TOKEN_MSB         _STREAM2MMIO_CMD_TOKEN_CMD_MSB
#define _IBC_2600_STREAM2MMIO_CMD_TOKEN_LSB         _STREAM2MMIO_CMD_TOKEN_CMD_LSB
#define _IBC_2600_STREAM2MMIO_NUM_ITEMS_BITS        _STREAM2MMIO_PACK_NUM_ITEMS_BITS
#define _IBC_2600_STREAM2MMIO_CMD_SYNC              _STREAM2MMIO_CMD_TOKEN_SYNC_FRAME
#define _IBC_2600_STREAM2MMIO_CMD_SYNC_SKIP         _STREAM2MMIO_CMD_TOKEN_SYNC_FRAME_SKIP
#define _IBC_2600_STREAM2MMIO_CMD_STORE_PACKET      _STREAM2MMIO_CMD_TOKEN_STORE_PACKETS
#define _IBC_2600_STREAM2MMIO_CMD_STORE_WORDS       _STREAM2MMIO_CMD_TOKEN_STORE_WORDS
#define _IBC_2600_STREAM2MMIO_ACK_EOF_BIT           _STREAM2MMIO_PACK_ACK_EOF_BIT
#define _IBC_2600_STREAM2MMIO_ACK_EOP_BIT           _STREAM2MMIO_PACK_ACK_EOP_BIT
#define _IBC_2600_STREAM2MMIO_ACK_ERROR_BIT         _STREAM2MMIO_PACK_ACK_ERR_BIT

/* Str2Vec defines */
#define _IBC_2600_S2V_CMD_BITS                      5
#define _IBC_2600_S2V_CMD_INIT                      16
#define _IBC_2600_S2V_CMD_GIVE_N_VEC                1

/* command tokens */
#define _IBC_2600_CMD_INIT_VALUE              0
#define _IBC_2600_CMD_FIELD_BITS              2
#define _IBC_2600_CMD_FR_NR_LSB_IDX           16
#define _IBC_2600_CMD_DEST_IDX(dest_nr)       ((dest_nr) * _IBC_2600_CMD_FIELD_BITS)
#define _IBC_2600_CMD_DEST_DISABLE            (0)
#define _IBC_2600_CMD_STORE_ADDR_NEXT_TOKEN   (1)
#define _IBC_2600_CMD_STORE_ADDR_CONFIG       (2)
#define _IBC_2600_CMD_SYNC_FRAME_SKIP         (3)

/* acknowledge token definition */
#define _IBC_2600_ACK_TYPE_BITS               2

#define _IBC_2600_ACK_TYPE_IDX                0
#define _IBC_2600_ACK_SECURE_TOUCH_IDX        (_IBC_2600_ACK_TYPE_IDX + _IBC_2600_ACK_TYPE_BITS)
#define _IBC_2600_ACK_S2M_ERROR_IDX           (_IBC_2600_ACK_SECURE_TOUCH_IDX + 1)
#define _IBC_2600_ACK_LINE_P_FRAME_ERROR_IDX  (_IBC_2600_ACK_S2M_ERROR_IDX + 1)
#define _IBC_2600_ACK_LINE_P_FRAME_RCVD_IDX   (_IBC_2600_ACK_LINE_P_FRAME_ERROR_IDX + 1)

#define _IBC_2600_ACK_TYPE_INIT               (0)
#define _IBC_2600_ACK_TYPE_STORE              (1)
#define _IBC_2600_ACK_FALSE_CMD               (2)
#define _IBC_2600_ACK_FRAME_SKIPPED           (3)

#define _IBC_2600_ACK_ERROR_TYPE_NO_ERROR     0
#define _IBC_2600_ACK_ERROR_TYPE_NR_ITEMS     1
#define _IBC_2600_ACK_ERROR_TYPE_NR_UNITS     2
#define _IBC_2600_ACK_ERROR_TYPE_NR_LINES     3

/*register numbers in the shared register bank:*/
#define _IBC_2600_CMD_IDRAIN_RECEIVE           0
#define _IBC_2600_CFG_IWAKE_ADDR               1
#define _IBC_2600_CFG_SRST_PROC                2
#define _IBC_2600_CFG_SRST_FEEDER              3
#define _IBC_2600_STAT_ARBITERS_STATUS         4
#define _IBC_2600_CFG_ERROR_REG_SET            5
#define _IBC_2600_CFG_ERROR_IRQ_EN             6
#define _IBC_2600_CFG_SECURE_TOUCH_HANDLING    7
#define _IBC_2600_CFG_GDA_ENABLE               8
#define _IBC_2600_CFG_GDA_ADDRESS              9
#define _IBC_2600_CFG_ENABLE_CLK_GATES         10
#define _IBC_2600_CFG_EN_FLUSH_FOR_IDRAIN      11

#define _IBC_2600_REG_SET_CRUN_ERRORS          12 /* NO PHYSICAL REGISTER!! Only used in HSS model (if needed?)*/

/*register numbers in the proc config bank: */
#define _IBC_2600_PROC_CFG_STR2MMIO_PROC_ADDR         0
#define _IBC_2600_PROC_CFG_STR2MMIO_STORE_CMD         1
#define _IBC_2600_PROC_CFG_UNITS_P_LINE               2
#define _IBC_2600_PROC_CFG_LINES_P_FRAME              3
#define _IBC_2600_PROC_CFG_UNIT_SIZE                  4
#define _IBC_2600_PROC_CFG_UNITS_P_IBUF               5
#define _IBC_2600_PROC_CFG_CAMERA_SELECT              6
#define _IBC_2600_PROC_CFG_PAGE_NR                    7 /* deprecated */
#define _IBC_2600_PROC_CFG_BUFFER_ST_ADDR             7
#define _IBC_2600_PROC_CFG_S2V_NOT_STR2MMIO           8
#define _IBC_2600_PROC_CFG_S2V_N_VEC_P_UNIT           9
#define _IBC_2600_PROC_CFG_INVERT_GDA_EN              10
#define _IBC_2600_PROC_CFG_DEST_ENABLED(dest_nr)      (11 + (dest_nr))

/*register numbers in the proc cmd/ack bank: */
#define _IBC_2600_PROC_CMD_CMD                       (0)
#define _IBC_2600_PROC_CMD_ACK_ADDR                  (1)
#define _IBC_2600_PROC_CMD_EVENTQUE_SIDPID           (2)
#define DEPRECATED_IBC_2600_PROC_CMD_2NDBUF_CMD_ADDR (3) // Kept it to inform in error message
#define DEPRECATED_IBC_2600_PROC_CMD_2ND_BUFFER_ACK  (4) // Kept it to inform in error message
#define _IBC_2600_PROC_CMD_STR2MMIO_ACK              (5)
#define _IBC_2600_PROC_CMD_ERROR_REG                 (6)
#define _IBC_2600_PROC_CMD_CLEAR_ERROR_REG           (7)
#define _IBC_2600_PROC_CMD_DEST_ACK(dest_nr)         (8 + (dest_nr))

/*register numbers in the proc status bank: */
#define _IBC_2600_PROC_STAT_CUR_CMDS                     (0)
#define _IBC_2600_PROC_STAT_CUR_UNIT_IN_LINE             (1)
#define _IBC_2600_PROC_STAT_CUR_LINE                     (2)
#define _IBC_2600_PROC_STAT_MAIN_CNTRL_STATE             (3)
#define DEPRECATED_IBC_2600_PROC_STAT_2NDBUF_CNTRL_STATE (4) // Kept it to inform in error message
#define _IBC_2600_PROC_STAT_DEST0_CUR_ACK_UNIT           (5)
#define _IBC_2600_PROC_STAT_DEST0_CUR_ACK_UNIT_IN_LINE   (6)
#define _IBC_2600_PROC_STAT_DEST0_CUR_ACK_LINE           (7)
#define _IBC_2600_PROC_STAT_DEST_SYNC_STATE(dest_nr)     (8 + ((dest_nr) * 2))
#define _IBC_2600_PROC_STAT_DEST_CUR_CMDS(dest_nr)       (9 + ((dest_nr) * 2))

/* registers per destination configuration set: */
#define _IBC_2600_DEST_CFG_FEED_ADDR                ( 0)
#define _IBC_2600_DEST_CFG_REQUESTER_ADDR           ( 1)
#define _IBC_2600_DEST_CFG_CHANNEL_ADDR             ( 2)
#define _IBC_2600_DEST_CFG_SPAN_A_ADDR              ( 3)
#define _IBC_2600_DEST_CFG_SPAN_B_ADDR              ( 4)
#define _IBC_2600_DEST_CFG_TERMINAL_B_ADDR          ( 5)
#define _IBC_2600_DEST_CFG_DEST_MODE                ( 6)
#define _IBC_2600_DEST_CFG_ST_ADDR                  ( 7)
#define DEPRECATED_IBC_2600_DEST_CFG_DEST_NUM_UNITS ( 8) // Kept it to inform in error message
#define _IBC_2600_DEST_CFG_IWAKE_THRESHOLD          ( 9)
#define _IBC_2600_DEST_CFG_SYNC_DESTS               (10)
#define _IBC_2600_DEST_CFG_DMA_FLUSH_CNTR           (11)

#define _IBC_2600_DEST_CFG_DEST_MODE_IS_FEEDER_IDX      0
#define _IBC_2600_DEST_CFG_DEST_MODE_CONFIG_DMA_IDX     1
#define _IBC_2600_DEST_CFG_DEST_MODE_IWAKE_EN_IDX       2
#define _IBC_2600_DEST_CFG_DEST_MODE_DIS_SPANA_RST_IDX  3
#define _IBC_2600_DEST_CFG_DEST_MODE_DIS_SPANB_RST_IDX  4

/* registers per Frame check: */
#define _IBC_2600_FR_C_CFG_ENABLE               0
#define _IBC_2600_FR_C_CFG_CHECK_MODE           1
#define _IBC_2600_FR_C_CFG_SID_PROC_ID          2
#define _IBC_2600_FR_C_CFG_TRIGGER_OFFSET       3
#define _IBC_2600_FR_C_CFG_TRIGGER_REPEAT_VAL   4
#define _IBC_2600_FR_C_CFG_ADDR                 5
#define _IBC_2600_FR_C_CFG_TOKEN                6

#define _IBC_2600_FR_C_CFG_CHECK_MODE_TYPE_IDX      0
#define _IBC_2600_FR_C_CFG_CHECK_MODE_REPEAT_IDX    1
#define _IBC_2600_FR_C_CFG_CHECK_MODE_AT_EOF_IDX    2
#define _IBC_2600_FR_C_CFG_CHECK_MODE_ADD_VAL_IDX   3
#define _IBC_2600_FR_C_CFG_CHECK_MODE_SCALE_VAL_IDX 4
#define _IBC_2600_FR_C_CFG_CHECK_MODE_ADD_FR_NR_IDX 5

/* configuration register addresses for each feeder: */
#define _IBC_2600_FEED_CMD_CMD                    0
#define _IBC_2600_FEED_DMA_ACK                    1
#define _IBC_2600_FEED_CFG_ACK_ADDR               2
#define _IBC_2600_FEED_CFG_REQUESTER_ADDR         3
#define _IBC_2600_FEED_CFG_CHANNEL_ADDR           4
#define _IBC_2600_FEED_CFG_UNITS_P_LINE_IN        5
#define _IBC_2600_FEED_CFG_UNITS_OUT_P_IN         6
#define _IBC_2600_FEED_CFG_LAST_UNITS_OUT         7
#define _IBC_2600_FEED_CFG_HEIGHT                 8
#define _IBC_2600_FEED_CFG_SIDPID                 9
#define _IBC_2600_FEED_CFG_WAIT_ON_FEEDERS        10
#define _IBC_2600_FEED_CFG_WAIT_ORDER             11
#define _IBC_2600_FEED_CFG_SYNC_FEEDERS           12

/* status register addresses for each feeder: */
#define _IBC_2600_FEED_STAT_CUR_UNIT_IN_LINE_IN   0
#define _IBC_2600_FEED_STAT_CUR_LINE_IN           1
#define _IBC_2600_FEED_STAT_CUR_SND_CMDS          2
#define _IBC_2600_FEED_STAT_CUR_RCVD_ACKS         3

#endif
