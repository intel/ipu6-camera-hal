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

#ifndef __IA_CSS_BASE_TYPES_H
#define __IA_CSS_BASE_TYPES_H

#include "type_support.h"

#define VIED_VADDRESS_BITS				32

#define DEVICE_DESCRIPTOR_ID_BITS			32

#define SIZE_OF_TERMINAL_LOAD_TRANSFER_STRUCT_BITS \
	(2 * IA_CSS_UINT32_T_BITS + \
	VIED_VADDRESS_BITS + \
	2 * IA_CSS_UINT16_T_BITS)

#define SIZE_OF_TERMINAL_STORE_TRANSFER_STRUCT_BITS \
	(2 * IA_CSS_UINT32_T_BITS + \
	VIED_VADDRESS_BITS + \
	2 * IA_CSS_UINT16_T_BITS)

#define SIZE_OF_TERMINAL_LOAD_TRANSFER_STRUCT (SIZE_OF_TERMINAL_LOAD_TRANSFER_STRUCT_BITS / CHAR_BIT)
#define SIZE_OF_TERMINAL_STORE_TRANSFER_STRUCT (SIZE_OF_TERMINAL_STORE_TRANSFER_STRUCT_BITS / CHAR_BIT)

typedef uint32_t vied_vaddress_t;

typedef struct {
	uint8_t device_id;
	uint8_t instance_id;
	uint8_t channel_id;
	uint8_t section_id;
} device_descriptor_fields_t;

typedef union {
	device_descriptor_fields_t fields;
	uint32_t data;
} device_descriptor_id_t;

typedef uint16_t ia_css_process_id_t;

#endif /* __IA_CSS_BASE_TYPES_H */
