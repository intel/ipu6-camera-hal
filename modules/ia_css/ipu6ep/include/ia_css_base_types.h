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
