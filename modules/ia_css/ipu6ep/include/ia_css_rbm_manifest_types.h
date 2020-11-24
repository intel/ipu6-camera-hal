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

#ifndef __IA_CSS_RBM_MANIFEST_TYPES_H
#define __IA_CSS_RBM_MANIFEST_TYPES_H

#include "ia_css_rbm.h"
#include "vied_nci_psys_resource_model.h"

#ifndef VIED_NCI_RBM_MAX_MUX_COUNT
#error Please define VIED_NCI_RBM_MAX_MUX_COUNT
#endif
#ifndef VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT
#error Please define VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT
#endif
#ifndef VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT
#error Please define VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT
#endif
#ifndef N_PADDING_UINT8_IN_RBM_MANIFEST
#error Please define N_PADDING_UINT8_IN_RBM_MANIFEST
#endif

#define SIZE_OF_RBM_MUX_DESC_S ( \
	(4 * IA_CSS_UINT8_T_BITS))

typedef struct ia_css_rbm_mux_desc_s {
	uint8_t gp_dev_id;
	uint8_t mux_id;
	uint8_t offset;
	uint8_t size_bits;
} ia_css_rbm_mux_desc_t;

#define SIZE_OF_RBM_VALIDATION_RULE_DESC_S ( \
	  (1 * IA_CSS_RBM_BITS) \
	+ (1 * IA_CSS_UINT32_T_BITS))

typedef struct ia_css_rbm_validation_rule_s {
	ia_css_rbm_t match; /* RBM is an array of 32 bit elements */
	uint32_t expected_value;
} ia_css_rbm_validation_rule_t;

#define SIZE_OF_RBM_TERMINAL_ROUTING_DESC_S ( \
	(4 * IA_CSS_UINT8_T_BITS))

typedef struct ia_css_rbm_terminal_routing_desc_s {
	uint8_t terminal_id;
	uint8_t connection_state;
	uint8_t mux_id;
	uint8_t state;
} ia_css_rbm_terminal_routing_desc_t;

#define SIZE_OF_RBM_MANIFEST_S ( \
	  (VIED_NCI_RBM_MAX_MUX_COUNT * SIZE_OF_RBM_MUX_DESC_S) \
	+ (VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT * SIZE_OF_RBM_VALIDATION_RULE_DESC_S) \
	+ (VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT * SIZE_OF_RBM_TERMINAL_ROUTING_DESC_S) \
	+ (3 * IA_CSS_UINT16_T_BITS) \
	+ (N_PADDING_UINT8_IN_RBM_MANIFEST * IA_CSS_UINT8_T_BITS))

typedef struct ia_css_rbm_manifest_s {
#if VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT > 0
	ia_css_rbm_validation_rule_t
		validation_rules[VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT];
#endif
	uint16_t mux_desc_count;
	uint16_t validation_rule_count;
	uint16_t terminal_routing_desc_count;

#if VIED_NCI_RBM_MAX_MUX_COUNT > 0
	ia_css_rbm_mux_desc_t
		mux_desc[VIED_NCI_RBM_MAX_MUX_COUNT];
#endif

#if VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT > 0
	ia_css_rbm_terminal_routing_desc_t
		terminal_routing_desc[VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT];
#endif

#if N_PADDING_UINT8_IN_RBM_MANIFEST > 0
	uint8_t padding[N_PADDING_UINT8_IN_RBM_MANIFEST];
#endif
} ia_css_rbm_manifest_t;

#endif /* __IA_CSS_RBM_MANIFEST_TYPES_H */
