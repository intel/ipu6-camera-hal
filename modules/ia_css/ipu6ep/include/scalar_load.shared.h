/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2017 Intel Corporation.
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

#ifndef __SCALAR_LOAD_SHARED_H
#define __SCALAR_LOAD_SHARED_H

#include <type_support.h>
#include <mem_ptr.h>

/* Cell independent interface */

STORAGE_CLASS_SCALAR_LOAD_H unsigned
load_scalar(mem_ptr_t ptr, unsigned offset);

STORAGE_CLASS_SCALAR_LOAD_H uint16_t load_scalar_16(mem_ptr_t ptr, uint32_t offset);

STORAGE_CLASS_SCALAR_LOAD_H uint8_t load_scalar_8(mem_ptr_t ptr, uint32_t offset);

#if defined(__INLINE_SCALAR_LOAD__) && !defined(PIPE_GENERATION)
#include "scalar_load.shared.c"
#endif

#endif /* __SCALAR_LOAD_SHARED_H */
