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
