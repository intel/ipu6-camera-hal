/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2018 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_PRIVATE_PG_DATA_H
#define __IA_CSS_PSYS_PRIVATE_PG_DATA_H

#include "ipu_device_acb_devices.h"
#include "ipu_device_gp_devices.h"
#include "type_support.h"

#define PRIV_CONF_INVALID	0xFF
#define MAX_INPUT_BUFFER	(2)
#define MAX_OUTPUT_BUFFER	(2)

struct ia_css_psys_pg_buffer_information_s {
	unsigned int link_id; /* Terminal the buffer is connected to */
	unsigned int buffer_base_addr;
	unsigned int bpe;
	unsigned int buffer_width;
	unsigned int buffer_height;
	unsigned int num_of_buffers;
	unsigned int dfm_port_addr;
};

typedef struct ia_css_psys_pg_buffer_information_s ia_css_psys_pg_buffer_information_t;

typedef struct {
	uint8_t in_select; /**< corresponds to type nci_acb_port_t  0 = ISP, 1 = Acc */
	uint8_t out_select;  /**< corresponds to type nci_acb_port_t  0 = ISP, 1 = Acc */
} ia_css_psys_private_pg_data_acb_t;

struct ia_css_psys_private_pg_data {
	ia_css_psys_private_pg_data_acb_t acb_route[IPU_DEVICE_ACB_NUM_ACB];
	uint8_t psa_mux_conf[IPU_DEVICE_GP_PSA_MUX_NUM_MUX];
	uint8_t isa_mux_conf[IPU_DEVICE_GP_ISA_STATIC_MUX_NUM_MUX];
	ia_css_psys_pg_buffer_information_t input_buffer_info[MAX_INPUT_BUFFER];
	ia_css_psys_pg_buffer_information_t output_buffer_info[MAX_OUTPUT_BUFFER];
};

#endif /* __IA_CSS_PSYS_PRIVATE_PG_DATA_H */
