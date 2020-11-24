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
#ifndef __DEC400_OFS_CONFIG_H
#define __DEC400_OFS_CONFIG_H

#include "ia_css_terminal_types.h"
#include "ia_css_common_io_types.h"
#include "ipu_device_ofs_devices.h"

#define DEC400_OFS_OUT_NUMBER_OF_STREAMS (2)

/* This function configures the DEC400 for NV12-linear output on either the display or main OFS pins */
void dec400_config_ofs_nv12_linear(
	p_terminal_config terminal_ptr,
	uint32_t stream_id_array[DEC400_OFS_OUT_NUMBER_OF_STREAMS],
	uint8_t *payload_buffer_y,
	uint8_t *payload_buffer_uv);

/* This function configures the DEC400 for NV12-Tile-Y output on either the display or main OFS pins */
void dec400_config_ofs_nv12_tile_y(
	p_terminal_config terminal_ptr,
	uint32_t stream_id_array[DEC400_OFS_OUT_NUMBER_OF_STREAMS],
	uint8_t *payload_buffer_y,
	uint8_t *payload_buffer_uv);

#endif /* __DEC400_OFS_CONFIG_H */
