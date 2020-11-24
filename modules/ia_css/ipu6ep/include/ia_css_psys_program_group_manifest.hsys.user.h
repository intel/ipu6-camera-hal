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

#ifndef __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_HSYS_USER_H
#define __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_HSYS_USER_H

/*! \file */

/** @file ia_css_psys_program_group_manifest.hsys.user.h
 *
 * Define the methods on the program group manifest object: Hsys user interface
 */

#include <ia_css_psys_manifest_types.h>

#include <type_support.h>	/* bool */

/*! Print the program group manifest object to file/stream
    in textual format

 @param	manifest[in]		program group manifest object
 @param	fid[out]		file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_group_manifest_print(
	const ia_css_program_group_manifest_t *manifest,
	void *fid);

/*! Read the program group manifest object from file/stream
  in binary format.

 @see ia_css_program_group_manifest_write()

 @param	fid[in]			file/stream handle

 @return NULL on error
 */
extern ia_css_program_group_manifest_t *ia_css_program_group_manifest_read(
	void *fid);

/*! Write the program group manifest object to file/stream
  in binary format

 @see ia_css_program_group_manifest_read()

 @param	manifest[in]		program group manifest object
 @param	fid[out]		file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_group_manifest_write(
	const ia_css_program_group_manifest_t *manifest,
	void *fid);

/*! Boolean test if the program group manifest is valid

 @param	manifest[in]		program group manifest

 @return true if program group manifest is correct, false on error
 */
extern bool ia_css_is_program_group_manifest_valid(
	const ia_css_program_group_manifest_t *manifest);

#endif /* __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_HSYS_USER_H */
