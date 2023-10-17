/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
