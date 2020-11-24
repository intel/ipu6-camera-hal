/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_CELL_DEVICES_H
#define __IPU_DEVICE_CELL_DEVICES_H

#define SPC0_CELL  ipu_sp_control_tile_ps_sp
#define SPP0_CELL  ipu_sp_proxy_tile_ps_0_sp
#define ISP0_CELL  ipu_par_idsp_0_idsp_tile_top_idsp

enum ipu_device_psys_cell_id {
	SPC0,
	SPP0,
#ifdef HAS_ISP0
	ISP0,
#endif
};

#ifdef HAS_ISP0
#define NUM_CELLS (ISP0 + 1)
#define NUM_ISP_CELLS 1
#else
#define NUM_CELLS (SPP0 + 1)
#define NUM_ISP_CELLS 0
#endif

#endif /* __IPU_DEVICE_CELL_DEVICES_H */
