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

#ifndef __ISP_GLOBAL_H
#define __ISP_GLOBAL_H

#ifndef PIPE_GENERATION
#include <type_support.h>
#if HAS_ISP0
#include <idsp_params.h>
#endif
#endif /* PIPE_GENERATION */

#include "system_global.h"

/* Number of elements per 512 bit vector, depending on bits per element */
#define ELEMS_PER_VECTOR_8BPE           (64)
#define ELEMS_PER_VECTOR_16BPE          (32)
#define ELEMS_PER_VECTOR_32BPE          (16)

/* ISP SC Registers */
/* Since Status and Control register contains reset, start break etc bits */
#define ISP_SC_REG                      idsp_break_flag_register

/* ISP SC Register bits */
#define ISP_START_BIT                   idsp_start_flag_bit
#define ISP_IDLE_BIT                    idsp_ready_flag_bit
#define ISP_SLEEPING_BIT                idsp_sleeping_flag_bit

/* BAMEM */
#define PLANE_ID_NO_PLANE               53 /*invalid plane*/

#endif /* __ISP_GLOBAL_H */
