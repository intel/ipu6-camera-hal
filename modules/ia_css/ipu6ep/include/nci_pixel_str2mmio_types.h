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
#ifndef __NCI_PIXEL_STR2MMIO_TYPES_H
#define __NCI_PIXEL_STR2MMIO_TYPES_H

#include "type_support.h"

/* Refer to the str2mmio MAS to understand the logic behind it */
#define NCI_PIXEL_STR2MMIO_NO_REMAP_VALUE (0xE4)

typedef enum {
	width_8bit = 0,
	width_16bit = 1,
	width_32bit = 2
} nci_str2mmio_pixwidth;

/* public */
struct nci_pixel_str2mmio_cfg {
	nci_str2mmio_pixwidth pix_width;
	uint32_t start_address;
	uint32_t end_address;
	uint32_t stride;
	uint32_t num_items;
	uint32_t block_when_no_cmd;
	uint32_t ack_base_addr;
	uint32_t sidpid;
	uint32_t datafield_remap_cfg;
	uint32_t rpwc_enable;
};

enum nci_pixel_str2mmio_sections {
	NCI_PIXEL_STR2MMIO_SECTION0,
	NCI_PIXEL_STR2MMIO_NOF_SECTIONS
};

#endif /* __NCI_PIXEL_STR2MMIO_TYPES_H */
