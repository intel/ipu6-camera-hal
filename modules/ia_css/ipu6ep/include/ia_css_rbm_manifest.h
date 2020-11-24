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

#ifndef __IA_CSS_RBM_MANIFEST_H
#define __IA_CSS_RBM_MANIFEST_H

#include "type_support.h"
#include "ia_css_rbm_manifest_types.h"

/** Returns the descriptor size of the RBM manifest.
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
unsigned int
ia_css_rbm_manifest_get_size(void);

/** Initializes the RBM manifest.
 * @param rbm[in] Routing bitmap.
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
void
ia_css_rbm_manifest_init(struct ia_css_rbm_manifest_s *rbm);

/** Returns a pointer to the array of mux descriptors.
 * @param manifest[in] Routing bitmap manifest.
 * @return NULL on error
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
ia_css_rbm_mux_desc_t *
ia_css_rbm_manifest_get_muxes(const ia_css_rbm_manifest_t *manifest);

/** Returns the size of mux descriptors array.
 * @param manifest[in] Routing bitmap manifest.
 * @return size
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
unsigned int
ia_css_rbm_manifest_get_mux_count(const ia_css_rbm_manifest_t *manifest);

/** Returns a pointer to the array of validation descriptors.
 * @param manifest[in] Routing bitmap manifest.
 * @return NULL on error
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
ia_css_rbm_validation_rule_t *
ia_css_rbm_manifest_get_validation_rules(const ia_css_rbm_manifest_t *manifest);

/** Returns the size of the validation descriptor array.
 * @param manifest[in] Routing bitmap manifest.
 * @return size
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
unsigned int
ia_css_rbm_manifest_get_validation_rule_count(const ia_css_rbm_manifest_t *manifest);

/** Returns a pointer to the array of terminal routing descriptors.
 * @param manifest[in] Routing bitmap manifest.
 * @return NULL on error
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
ia_css_rbm_terminal_routing_desc_t *
ia_css_rbm_manifest_get_terminal_routing_desc(const ia_css_rbm_manifest_t *manifest);

/** \brief Returns the size of the terminal routing descriptor array.
 * Note: pretty printing differs from on host and on IPU.
 * @param manifest[in] Routing bitmap manifest.
 * @return size
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
unsigned int
ia_css_rbm_manifest_get_terminal_routing_desc_count(const ia_css_rbm_manifest_t *manifest);

/** Pretty prints the routing bitmap manifest.
 * @param manifest[in] Routing bitmap manifest.
 */
void
ia_css_rbm_manifest_print(const ia_css_rbm_manifest_t *manifest);

/** \brief Pretty prints a RBM (routing bitmap).
 * Note: pretty printing differs from on host and on IPU.
 * @param rbm[in]             Routing bitmap.
 * @param mux[in]             List of mux descriptors corresponding to rbm.
 * @param mux_desc_count[in]  Number of muxes in list mux.
 */
void
ia_css_rbm_pretty_print(
	const ia_css_rbm_t *rbm,
	const ia_css_rbm_mux_desc_t *mux,
	unsigned int mux_desc_count);

/** \brief check for the validity of a routing bitmap.
 * @param manifest[in] Routing bitmap manifest.
 * @param rbm[in]      Routing bitmap
 * @return true on match.
 */
bool
ia_css_rbm_manifest_check_rbm_validity(
	const ia_css_rbm_manifest_t *manifest,
	const ia_css_rbm_t *rbm);

/** \brief sets, using manifest info, the value of a mux in the routing bitmap.
 * @param rbm[in]             Routing bitmap.
 * @param mux[in]             List of mux descriptors corresponding to rbm.
 * @param mux_count[in]       Number of muxes in list mux.
 * @param gp_dev_id[in]       ID of sub system (PSA/ISA) where the mux is located.
 * @param mux_id[in]          ID of mux to set configuration for.
 * @param value[in]           Value of the mux.
 * @return routing bitmap.
 */
ia_css_rbm_t
ia_css_rbm_set_mux(
	ia_css_rbm_t rbm,
	ia_css_rbm_mux_desc_t *mux,
	unsigned int mux_count,
	unsigned int gp_dev_id,
	unsigned int mux_id,
	unsigned int value);

/** \brief Gets the value of a mux.
 * @param rbm      routing bitmap
 * @param mux_desc mux descriptor
 * @return Mux value corresponding to mux_desc, -1 on error.
 */
IA_CSS_RBM_MANIFEST_STORAGE_CLASS_H
int
ia_css_rbm_get_mux_value(
	const ia_css_rbm_t *rbm,
	const ia_css_rbm_mux_desc_t *mux_desc);

#ifdef __IA_CSS_RBM_MANIFEST_INLINE__
#include "ia_css_rbm_manifest_impl.h"
#endif /* __IA_CSS_RBM_MANIFEST_INLINE__ */

#endif /* __IA_CSS_RBM_MANIFEST_H */
