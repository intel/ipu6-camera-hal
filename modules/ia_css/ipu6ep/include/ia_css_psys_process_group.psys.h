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

#ifndef __IA_CSS_PSYS_PROCESS_GROUP_PSYS_H
#define __IA_CSS_PSYS_PROCESS_GROUP_PSYS_H

/*! \file */

/** @file ia_css_psys_process_group.psys.h
 *
 * Define the methods on the process group object: Psys embedded interface
 */

#include <ia_css_psys_process_types.h>

/*
 * Dispatcher
 */

/*! Perform the run command on the process group

 @param	process_group[in]		process group object

 Note: Run indicates that the process group will execute

 Precondition: The process group must be started or
 suspended and the processes have acquired the necessary
 internal resources

 @return < 0 on error
 */
extern int ia_css_process_group_run(
	ia_css_process_group_t					*process_group);

/*! Perform the stop command on the process group

 @param	process_group[in]		process group object

 Note: Stop indicates that the process group has completed execution

 Postcondition: The external resoruces can now be detached

 @return < 0 on error
 */
extern int ia_css_process_group_stop(
	ia_css_process_group_t					*process_group);

#endif /* __IA_CSS_PSYS_PROCESS_GROUP_PSYS_H */
