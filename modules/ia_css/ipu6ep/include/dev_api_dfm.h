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

#ifndef __DEV_API_DFM_H
#define __DEV_API_DFM_H

#include "dev_api_dfm_types.h"
#include "dev_api_dfm_storage_class.h" /* for DEV_API_DFM_STORAGE_CLASS_H */

/**
 * To configure all DFM ports in a Stream
 */
DEV_API_DFM_STORAGE_CLASS_H void dev_api_dfm_config_ports(struct dev_api_dfm_stream *stream,
		struct dev_api_dfm_port *port);

/**
 * To configure one DFM port in a stream by creating and writing configuration payloads
 */
DEV_API_DFM_STORAGE_CLASS_H
uint32_t dev_api_dfm_config_port(const struct dev_api_dfm_stream * const stream,
		struct dev_api_dfm_port * const port);

DEV_API_DFM_STORAGE_CLASS_H
uint32_t dev_api_dfm_get_fabric_offset(enum ipu_device_dfm_id dev_id);

#ifdef __INLINE_DEV_API_DFM__
#include "dev_api_dfm_inline.h"
#endif

#endif /* __DEV_API_DFM_H */
