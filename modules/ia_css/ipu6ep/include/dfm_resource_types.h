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

#ifndef __DFM_RESOURCE_TYPES_H
#define __DFM_RESOURCE_TYPES_H

#include "type_support.h"
#include "system_global.h"
#include "vied_nci_psys_resource_model.h"

#define DFM_FULL_PORT_OFFSET                        (32)

#define DFM_FULL_PORT_NUMBER_TO_RESOURCE_MODEL_OFFSET(port_number) \
	((port_number) % DFM_FULL_PORT_OFFSET)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/** GATHER MULTICAST CALCULATION FUNCTION
 */
#define IS_EMPTY_DFM_PORT(x)                  ((x < DFM_FULL_PORT_OFFSET) ? (1) : (0))
#define DFM_GET_GATHER_MULTICAST_MASK(x)      (1ULL << ((x) % DFM_FULL_PORT_OFFSET))

typedef __register struct {
	resource_type_t type;       /* Resource type */
	vied_nci_dev_dfm_id_t id;   /* Resource instance identifier */
	uint32_t        value;      /* Size of resource*/
	uint32_t        offset;     /* Initial offset of resource*/
} dfm_port_resource_t;

#ifdef PIPE_GENERATION
dfm_port_resource_t alloc_dfm_resource(resource_type_t  type, dfm_resource_id_t id, uint32_t size, uint32_t offset);
#endif

#endif /* __DFM_RESOURCE_TYPES_H */
