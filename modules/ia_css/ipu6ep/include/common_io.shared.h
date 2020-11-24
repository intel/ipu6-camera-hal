/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __COMMON_IO_SHARED_H
#define __COMMON_IO_SHARED_H

#include "storage_class.h"
#include "ia_css_common_io_types.h"

typedef __register struct {
	uint16_t        width;  /**< Logical dimensions */
	uint16_t        height; /**< Logical dimensions */
} s_resolution_t;

STORAGE_CLASS_INLINE s_resolution_t
set_resolution(unsigned width, unsigned height);

STORAGE_CLASS_INLINE s_resolution_t
get_resolution(p_terminal_config cfg);

STORAGE_CLASS_INLINE bool
terminal_format_is_tiley(p_terminal_config cfg);

STORAGE_CLASS_INLINE bool
terminal_format_is_msb_aligned(p_terminal_config cfg);

#ifndef PIPE_GENERATION
#include "common_io.shared.c"
#endif

#endif /* __COMMON_IO_SHARED_H */
