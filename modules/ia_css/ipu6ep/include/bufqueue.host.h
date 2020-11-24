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

#ifndef __BUFQUEUE_HOST_H
#define __BUFQUEUE_HOST_H

#define __INLINE_BUFQUEUE__ /* Always inlined on ISP */
#define STORAGE_CLASS_BUFQUEUE_H STORAGE_CLASS_INLINE
#define STORAGE_CLASS_BUFQUEUE_C STORAGE_CLASS_INLINE

#include "scalar_ls/scalar_load.host.h"
#include "scalar_ls/scalar_store.host.h"

/* Cell independent interface */
#include "bufqueue.shared.h"

#ifndef PIPE_GENERATION
#include "bufqueue.host.c"
#endif

#endif /* __BUFQUEUE_HOST_H */
