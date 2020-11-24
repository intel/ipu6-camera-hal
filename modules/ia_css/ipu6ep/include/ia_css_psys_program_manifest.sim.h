/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2018 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_PROGRAM_MANIFEST_SIM_H
#define __IA_CSS_PSYS_PROGRAM_MANIFEST_SIM_H

/*! \file */

/** @file ia_css_psys_program_manifest.sim.h
 *
 * Define the methods on the program manifest object: Simulation only
 */

#include <ia_css_psys_manifest_types.h>

#include <type_support.h>	/* uint8_t */

/*! Compute the size of storage required for allocating
 * the program manifest object

 @param program_needs_extension[in]	==1 if program manifest needs an extension struct
 @param	program_dependency_count[in]	Number of programs this one depends on
 @param	terminal_dependency_count[in]	Number of terminals this one depends on

 @return 0 on error
 */
extern size_t ia_css_sizeof_program_manifest(
	const uint8_t	program_needs_extension,
	const uint8_t	program_dependency_count,
	const uint8_t	terminal_dependency_count);

/*! Create (the storage for) the program manifest object

 @param	program_dependency_count[in]	Number of programs this one depends on
 @param	terminal_dependency_count[in]	Number of terminals this one depends on

 @return NULL on error
 */
extern ia_css_program_manifest_t *ia_css_program_manifest_alloc(
	const uint8_t	program_dependency_count,
	const uint8_t	terminal_dependency_count);

/*! Destroy (the storage of) the program manifest object

 @param	manifest[in]			program manifest

 @return NULL
 */
extern ia_css_program_manifest_t *ia_css_program_manifest_free(
	ia_css_program_manifest_t *manifest);

#endif /* __IA_CSS_PSYS_PROGRAM_MANIFEST_SIM_H */
