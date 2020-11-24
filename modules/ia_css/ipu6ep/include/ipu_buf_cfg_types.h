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

#ifndef __IPU_BUF_CFG_TYPES_H
#define __IPU_BUF_CFG_TYPES_H

#include "type_support.h"

/* WARNING: DON'T CHANGE THE ORDER OF THE FIELDS AS THEY REPRESENT THE REGISTER
 * LAYOUT.
 */

struct ipu_buf_properties {
	uint32_t addr;			/* Start address of the Buffer */
	uint32_t stride;		/* Buffer stride */
	uint32_t line_stride;		/* Line stride of the buffer */
};

struct ipu_buf_cfg {
	uint32_t cmd_addr;		/* addr where release or enqueue cmd need to be sent */
	uint32_t token;			/* Token which to be sent as release or enqueue cmd */
	uint32_t num_buf;		/* Number of the buffers */
};

#endif /* __IPU_BUF_CFG_TYPES_H */
