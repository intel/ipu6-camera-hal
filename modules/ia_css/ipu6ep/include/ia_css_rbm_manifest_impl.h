
/**
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

#include "ia_css_rbm_manifest.h"
#include "ia_css_rbm_trace.h"

#include "type_support.h"
#include "math_support.h"
#include "error_support.h"
#include "assert_support.h"
#include "print_support.h"

STORAGE_CLASS_INLINE
void __ia_css_rbm_manifest_check_struct(void)
{
	COMPILATION_ERROR_IF(
		sizeof(ia_css_rbm_manifest_t) != (SIZE_OF_RBM_MANIFEST_S / IA_CSS_UINT8_T_BITS));
	COMPILATION_ERROR_IF(
		(sizeof(ia_css_rbm_manifest_t) % 8 /* 64 bit */) != 0);
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
unsigned int
ia_css_rbm_manifest_get_size(void)
{
	unsigned int size = sizeof(struct ia_css_rbm_manifest_s);

	return ceil_mul(size, sizeof(uint64_t));
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
void
ia_css_rbm_manifest_init(struct ia_css_rbm_manifest_s *rbm)
{
	rbm->mux_desc_count = 0;
	rbm->terminal_routing_desc_count = 0;
	rbm->validation_rule_count = 0;
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
ia_css_rbm_mux_desc_t *
ia_css_rbm_manifest_get_muxes(const ia_css_rbm_manifest_t *manifest)
{
#if VIED_NCI_RBM_MAX_MUX_COUNT == 0
	(void)manifest;
	return NULL;
#else
	return (ia_css_rbm_mux_desc_t *)manifest->mux_desc;
#endif
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
unsigned int
ia_css_rbm_manifest_get_mux_count(const ia_css_rbm_manifest_t *manifest)
{
	return manifest->mux_desc_count;
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
ia_css_rbm_validation_rule_t *
ia_css_rbm_manifest_get_validation_rules(const ia_css_rbm_manifest_t *manifest)
{
#if VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT == 0
	(void)manifest;
	return NULL;
#else
	return (ia_css_rbm_validation_rule_t *)manifest->validation_rules;
#endif
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
unsigned int
ia_css_rbm_manifest_get_validation_rule_count(const ia_css_rbm_manifest_t *manifest)
{
	return manifest->validation_rule_count;
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
ia_css_rbm_terminal_routing_desc_t *
ia_css_rbm_manifest_get_terminal_routing_desc(const ia_css_rbm_manifest_t *manifest)
{
#if VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT == 0
	(void)manifest;
	return NULL;
#else
	return (ia_css_rbm_terminal_routing_desc_t *)manifest->terminal_routing_desc;
#endif
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
unsigned int
ia_css_rbm_manifest_get_terminal_routing_desc_count(const ia_css_rbm_manifest_t *manifest)
{
	return manifest->terminal_routing_desc_count;
}

IA_CSS_RBM_MANIFEST_STORAGE_CLASS_C
int
ia_css_rbm_get_mux_value(
	const ia_css_rbm_t *rbm,
	const ia_css_rbm_mux_desc_t *mux_desc)
{
	int mux_val;
	unsigned int rbm_val = ia_css_rbm_range_val(*rbm, mux_desc->offset, mux_desc->size_bits);
	/*
	* Decode rbm_val to mux_val, mux_val = log2(rbm_val),
	* Max valid rbm_val is 1000, corresponds to 4 inputs mux,
	* All values which are not power of 2 are not valid, return -1,
	*/
	switch (rbm_val) {
	case 1:
	/* Fall through */
	case 2:
	/* Fall through */
	case 4:
		mux_val = rbm_val >> 1;
		break;
	case 8:
		mux_val = 3;
		break;
	default:
		mux_val = -1;
		break;
	}
	return mux_val;
}
