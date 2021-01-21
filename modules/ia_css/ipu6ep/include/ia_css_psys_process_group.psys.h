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

 @param    process_group[in]        process group object

 Note: Run indicates that the process group will execute

 Precondition: The process group must be started or
 suspended and the processes have acquired the necessary
 internal resources

 @return < 0 on error
 */
extern int ia_css_process_group_run(
    ia_css_process_group_t                    *process_group);

/*! Perform the stop command on the process group

 @param    process_group[in]        process group object

 Note: Stop indicates that the process group has completed execution

 Postcondition: The external resoruces can now be detached

 @return < 0 on error
 */
extern int ia_css_process_group_stop(
    ia_css_process_group_t                    *process_group);

#endif /* __IA_CSS_PSYS_PROCESS_GROUP_PSYS_H */
