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

#ifndef __IA_CSS_PROGRAM_GROUP_PARAM_SIM_H
#define __IA_CSS_PROGRAM_GROUP_PARAM_SIM_H

/*! \file */

/** @file ia_css_program_group_param.sim.h
 *
 * Define the methods on the program group parameter object: Simulation only
 */
#include <ia_css_program_group_param_types.h>

#include <ia_css_psys_manifest_types.h>

#include <type_support.h>

/* Simulation */

/*! Create a program group parameter object from specification

 @param	specification[in]		specification (index)
 @param	manifest[in]			program group manifest

 @return NULL on error
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_create(
	const unsigned int			specification,
	const ia_css_program_group_manifest_t	*manifest);

/*! Destroy the program group parameter object

 @param	program_group_param[in]		program group parameter object

 @return NULL
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_destroy(
	ia_css_program_group_param_t			*param);

/*! Compute the size of storage required for allocating
 * the program group parameter object

 @param	program_count[in]	Number of programs in the process group
 @param	terminal_count[in]	Number of terminals on the process group
 @param	fragment_count[in]	Number of fragments on the terminals of
				the process group

 @return 0 on error
 */
size_t ia_css_sizeof_program_group_param(
	const uint8_t	program_count,
	const uint8_t	terminal_count,
	const uint16_t	fragment_count);

/*! Allocate (the store of) a program group parameter object

 @param	program_count[in]	Number of programs in the process group
 @param	terminal_count[in]	Number of terminals on the process group
 @param	fragment_count[in]	Number of fragments on the terminals of
				the process group

 @return program group parameter pointer, NULL on error
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_alloc(
	const uint8_t	program_count,
	const uint8_t	terminal_count,
	const uint16_t	fragment_count);

/*! Free (the store of) a program group parameter object

 @param	program_group_param[in]		program group parameter object

 @return NULL
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_free(
	ia_css_program_group_param_t			*param);

/*! Print the program group parameter object to file/stream

 @param	param[in]	program group parameter object
 @param	fid[out]	file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_group_param_print(
	const ia_css_program_group_param_t	*param,
	void					*fid);

/*! Allocate (the store of) a program parameter object

 @return program parameter pointer, NULL on error
 */
extern ia_css_program_param_t *ia_css_program_param_alloc(void);

/*! Free (the store of) a program parameter object

 @param	param[in]	program parameter object

 @return NULL
 */
extern ia_css_program_param_t *ia_css_program_param_free(
	ia_css_program_param_t					*param);

/*! Print the program parameter object to file/stream

 @param	param[in]					program parameter object
 @param	fid[out]					file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_param_print(
	const ia_css_program_param_t	*param,
	void				*fid);

/*! Allocate (the store of) a terminal parameter object

 @return terminal parameter pointer, NULL on error
 */
extern ia_css_terminal_param_t *ia_css_terminal_param_alloc(void);

/*! Free (the store of) a terminal parameter object

 @param	param[in]	terminal parameter object

 @return NULL
 */
extern ia_css_terminal_param_t *ia_css_terminal_param_free(
	ia_css_terminal_param_t					*param);

/*! Print the terminal parameter object to file/stream

 @param	param[in]	terminal parameter object
 @param	fid[out]	file/stream handle

 @return < 0 on error
 */
extern int ia_css_terminal_param_print(
	const ia_css_terminal_param_t	*param,
	void				*fid);

#endif /* __IA_CSS_PROGRAM_GROUP_PARAM_SIM_H */
