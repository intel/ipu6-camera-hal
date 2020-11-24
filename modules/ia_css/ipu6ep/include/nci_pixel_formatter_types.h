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

#ifndef __NCI_PIXEL_FORMATTER_TYPES_H
#define __NCI_PIXEL_FORMATTER_TYPES_H

#include "type_support.h"
#include "storage_class.h"
#include "pixel_formatter_defs.h"

/* Pixel formatter register layout
 * contains single section that needs
 * to be calculated and configured by
 * firmware. Rest of the registers values
 * originate from P2P and hence are not
 * configured using this NCI interface.
 */
typedef enum {
	NCI_PIXEL_FORMATTER_SECTION0 = 0,
	NCI_PIXEL_FORMATTER_NUM_SECTIONS
} nci_pixel_formatter_section_t;

/**
 * nci_pixel_formatter_pad_mode - pad enum
 */
typedef enum {
	pad_all_zero  = 0,
	pad_all_ones  = 1,
	pad_clone_2x2 = 2,
	pad_clone_4x4 = 3
} nci_pixel_formatter_pad_mode;

typedef enum {
	A_off_B_off 			= 0,
	A_bayerquad_B_off 		= 1,
	A_off_B_bayerquad 		= 2,
	A_bayerquad_B_bayerquad = 3
} nci_pixel_formatter_merge_mode_single_stream;

typedef struct {
	nci_pixel_formatter_merge_mode_single_stream out_reg_mode;
	uint32_t out_muxA_sel;
	uint32_t out_muxB_sel;
	nci_pixel_formatter_pad_mode pad_mode;
} nci_pixel_formatter_config_t;

typedef enum {
	SINGLE_STREAM_MERGE_MODE = 0,
	SINGLE_DUAL_STREAM_MUX_MODE = 1,
	SINGLE_STREAM_MERGE_MODE_BYPASS_BE = 2
} nci_pixel_formatter_cfg_mode_sel;

#endif /* __NCI_PIXEL_FORMATTER_TYPES_H */
