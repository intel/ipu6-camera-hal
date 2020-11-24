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
#ifndef _pixel_formatter_defs_h
#define _pixel_formatter_defs_h

/* BANK size */
#define _PF_BANK_SIZE    4     /* Number of register in a register bank */

/* BANK select */
#define _PF_CFG_REG      0
#define _PF_INT_REG      1
#define _PF_BANK_A       2
#define _PF_BANK_B       3
#define _PF_BANK_C       4
#define _PF_BANK_D       5

/* Internal cfg registers */
#define _PF_CFG_CFG_REG  0
#define _PF_CFG_FLU_REG  1
#define _PF_CFG_STA_REG  2

/* Interrupt registers */
#define _PF_INT_SET_REG  0
#define _PF_INT_CLR_REG  1
#define _PF_INT_STA_REG  2
#define _PF_INT_ENB_REG  3

/* BANK registers */
#define _PF_START_REG    0
#define _PF_SIZE_REG     1
#define _PF_OUT_REG      2
#define _PF_PAD_REG      3

/* Registers bits */
#define _PF_CFG_REG_BITS    11
#define _PF_STA_REG_BITS    16
#define _PF_FLUSH_REG_BITS   1
#define _PF_INT_REG_BITS    32
#define _PF_START_REG_BITS  32
#define _PF_SIZE_REG_BITS   32
#define _PF_OUT_REG_BITS     4
#define _PF_PAD_REG_BITS     2

/* Configuration register address alignment */
#define _PF_CFG_REG_ADDRESS_ALIGN 4

#endif
