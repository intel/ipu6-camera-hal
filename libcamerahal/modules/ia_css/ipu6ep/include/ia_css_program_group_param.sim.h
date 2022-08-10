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

 @param    specification[in]        specification (index)
 @param    manifest[in]            program group manifest

 @return NULL on error
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_create(
    const unsigned int            specification,
    const ia_css_program_group_manifest_t    *manifest);

/*! Destroy the program group parameter object

 @param    program_group_param[in]        program group parameter object

 @return NULL
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_destroy(
    ia_css_program_group_param_t            *param);

/*! Compute the size of storage required for allocating
 * the program group parameter object

 @param    program_count[in]    Number of programs in the process group
 @param    terminal_count[in]    Number of terminals on the process group
 @param    fragment_count[in]    Number of fragments on the terminals of
                the process group

 @return 0 on error
 */
size_t ia_css_sizeof_program_group_param(
    const uint8_t    program_count,
    const uint8_t    terminal_count,
    const uint16_t    fragment_count);

/*! Allocate (the store of) a program group parameter object

 @param    program_count[in]    Number of programs in the process group
 @param    terminal_count[in]    Number of terminals on the process group
 @param    fragment_count[in]    Number of fragments on the terminals of
                the process group

 @return program group parameter pointer, NULL on error
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_alloc(
    const uint8_t    program_count,
    const uint8_t    terminal_count,
    const uint16_t    fragment_count);

/*! Free (the store of) a program group parameter object

 @param    program_group_param[in]        program group parameter object

 @return NULL
 */
extern ia_css_program_group_param_t *ia_css_program_group_param_free(
    ia_css_program_group_param_t            *param);

/*! Print the program group parameter object to file/stream

 @param    param[in]    program group parameter object
 @param    fid[out]    file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_group_param_print(
    const ia_css_program_group_param_t    *param,
    void                    *fid);

/*! Allocate (the store of) a program parameter object

 @return program parameter pointer, NULL on error
 */
extern ia_css_program_param_t *ia_css_program_param_alloc(void);

/*! Free (the store of) a program parameter object

 @param    param[in]    program parameter object

 @return NULL
 */
extern ia_css_program_param_t *ia_css_program_param_free(
    ia_css_program_param_t                    *param);

/*! Print the program parameter object to file/stream

 @param    param[in]                    program parameter object
 @param    fid[out]                    file/stream handle

 @return < 0 on error
 */
extern int ia_css_program_param_print(
    const ia_css_program_param_t    *param,
    void                *fid);

/*! Allocate (the store of) a terminal parameter object

 @return terminal parameter pointer, NULL on error
 */
extern ia_css_terminal_param_t *ia_css_terminal_param_alloc(void);

/*! Free (the store of) a terminal parameter object

 @param    param[in]    terminal parameter object

 @return NULL
 */
extern ia_css_terminal_param_t *ia_css_terminal_param_free(
    ia_css_terminal_param_t                    *param);

/*! Print the terminal parameter object to file/stream

 @param    param[in]    terminal parameter object
 @param    fid[out]    file/stream handle

 @return < 0 on error
 */
extern int ia_css_terminal_param_print(
    const ia_css_terminal_param_t    *param,
    void                *fid);

#endif /* __IA_CSS_PROGRAM_GROUP_PARAM_SIM_H */
