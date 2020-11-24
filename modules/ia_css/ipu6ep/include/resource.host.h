/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __RESOURCE_HOST_H
#define __RESOURCE_HOST_H

#include "resource_types.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE dma_resource_t
resource_to_dmachannel_with_inc(const resource_t resource, unsigned inc);

STORAGE_CLASS_INLINE dma_resource_t
resource_to_dmachannel(const resource_t resource);

#include "resource.host.c"

#endif /* __RESOURCE_HOST_H */
