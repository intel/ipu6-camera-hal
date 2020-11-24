/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2019 Intel Corporation.
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

#ifndef __VIED_NCI_ACB_TYPES_H
#define __VIED_NCI_ACB_TYPES_H

#include "type_support.h"
#include "storage_class.h"
#include "vied_nci_acb_route_type.h"
#include "vied_nci_acb_local_types.h"

#define IPU_DEVICE_ACB_GA_ACB_CMD_FIFO_TAIL_ADDR (0x00000)
#define IPU_DEVICE_ACB_GA_ACB_BASE_CTRL_ADDR (0x00004)
#define IPU_DEVICE_ACB_GA_ACB_INPUT_FRAME_SIZE_ADDR (0x00008)
#define IPU_DEVICE_ACB_GA_ACB_SCALE_ADDR (0x0000c)

#define IPU_DEVICE_ACB_ACK_CMD_ADDR (0x0000)
#define IPU_DEVICE_ACB_ACK_ADDR_ADDR (0x0004)
#define IPU_DEVICE_ACB_PWR_FEATURE_ADDR (0x0054)

#define IPU_DEVICE_ACB_INIT_CMD_ID (0x0010)
#define IPU_DEVICE_ACB_PROC_N_LINES_CMD_ID (0x0001)
/* bit position in EQ token where the ACB command type (init/process_line) is encoded */
#define IPU_DEVICE_ACB_ACK_TOKEN_COMMAND_TYPE_BIT_POS (19)

/*
* ACB register layout is partitioned into two sections,
* since in register layout the sections are spaced apart
* section0 is the ctrl configuration
* section1 is the ack configuration
*/
typedef enum {
	NCI_ACB_SECTION0 = 0,
	NCI_ACB_SECTION1,
	NCI_ACB_NUM_SECTIONS
} nci_acb_section_t;

typedef struct {
	uint32_t ctrl_id;
} nci_acb_init_cmd_t;

typedef __register struct {
	uint32_t ctrl_id;
	uint32_t config_set;
	uint32_t num_lines;
} nci_acb_process_cmd_t;

typedef struct {
	nci_acb_route_t base_ctrl;
	/* ACB_FRAME_SIZE */
	/* For the FRAGMENT to process, just in ISYS frame == fragment */
	uint32_t frame_width;
	/* For the FRAGMENT to process, just in ISYS frame == fragment */
	uint32_t frame_height;
	/* ACB_ACB_SCALE */
	uint32_t scale_mult;
	uint32_t scale_nf;
} nci_acb_control_config_t;

typedef struct {
	uint32_t clk_gate_status;
} nci_acb_power_feature_t;

typedef struct {
	nci_acb_control_config_t ctrl_cfg;
	nci_acb_ack_config_t ack_cfg;
	nci_acb_power_feature_t pwr_feature;
} nci_acb_config_t;

#endif /* __VIED_NCI_ACB_TYPES_H */
