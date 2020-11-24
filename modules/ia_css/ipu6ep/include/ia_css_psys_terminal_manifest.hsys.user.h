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

#ifndef __IA_CSS_PSYS_TERMINAL_MANIFEST_HSYS_USER_H
#define __IA_CSS_PSYS_TERMINAL_MANIFEST_HSYS_USER_H

/*! \file */

/** @file ia_css_psys_terminal.hsys.user.h
 *
 * Define the methods on the termianl manifest object: Hsys user interface
 */

#include <ia_css_psys_manifest_types.h>

/*! Print the terminal manifest object to file/stream

 @param	manifest[in]			terminal manifest object
 @param	fid[out]				file/stream handle

 @return < 0 on error
 */
extern int ia_css_terminal_manifest_print(
	const ia_css_terminal_manifest_t	*manifest,
	void					*fid);

#endif /* __IA_CSS_PSYS_TERMINAL_MANIFEST_HSYS_USER_H */
