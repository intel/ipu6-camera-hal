/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2016 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_TERMINAL_MANIFEST_SIM_H
#define __IA_CSS_PSYS_TERMINAL_MANIFEST_SIM_H

/*! \file */

/** @file ia_css_psys_terminal_manifest.sim.h
 *
 * Define the methods on the terminal manifest object: Simulation only
 */

#include <type_support.h>					/* size_t */
#include "ia_css_terminal.h"
#include "ia_css_terminal_manifest.h"
#include "ia_css_terminal_defs.h"

/*! Create (the storage for) the terminal manifest object

 @param	terminal_type[in]	type of the terminal manifest {parameter, data}

 @return NULL on error
 */
extern ia_css_terminal_manifest_t *ia_css_terminal_manifest_alloc(
	const ia_css_terminal_type_t			terminal_type);

/*! Destroy (the storage of) the terminal manifest object

 @param	manifest[in]			terminal manifest

 @return NULL
 */
extern ia_css_terminal_manifest_t *ia_css_terminal_manifest_free(
	ia_css_terminal_manifest_t				*manifest);

#endif /* __IA_CSS_PSYS_TERMINAL_MANIFEST_SIM_H */
