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

#ifndef __IA_CSS_PSYS_PROCESS_PSYS_H
#define __IA_CSS_PSYS_PROCESS_PSYS_H

/*! \file */

/** @file ia_css_psys_process.psys.h
 *
 * Define the methods on the process object: Psys embedded interface
 */

#include <ia_css_psys_process_types.h>

/*
 * Process manager
 */

/*! Acquire the resources specificed in process object

 @param	process[in]				process object

 Postcondition: This is a try process if any of the
 resources is not available, all succesfully acquired
 ones will be release and the function will return an
 error

 @return < 0 on error
 */
extern int ia_css_process_acquire(ia_css_process_t *process);

/*! Release the resources specificed in process object

 @param	process[in]				process object

 @return < 0 on error
 */
extern int ia_css_process_release(ia_css_process_t *process);

#endif /* __IA_CSS_PSYS_PROCESS_PSYS_H */
