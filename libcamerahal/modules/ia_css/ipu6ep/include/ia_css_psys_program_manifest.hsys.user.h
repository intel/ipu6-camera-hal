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

#ifndef __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_USER_H
#define __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_USER_H

/*! \file */

/** @file ia_css_psys_program_manifest.hsys.user.h
 *
 * Define the methods on the program manifest object: Hsys user interface
 */

#include <ia_css_psys_manifest_types.h>

/*! Print the program manifest object to file/stream
    in textual format

 @param    manifest[in]            program manifest object
 @param    fid[out]                file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_manifest_print(
    const ia_css_program_manifest_t    *manifest,
    void                *fid);

#endif /* __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_USER_H */
