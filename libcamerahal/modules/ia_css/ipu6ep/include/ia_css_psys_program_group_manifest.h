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

#ifndef __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_H
#define __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_H

#include "ia_css_psys_static_storage_class.h"

/*! \file */

/** @file ia_css_psys_program_group_manifest.h
 *
 * Define the methods on the program group manifest object that are not part of
 * a single interface.
 *
 * Set functions in this file are for not for production use by the
 * users (testing/simulation only).
 */

#include <ia_css_psys_manifest_types.h>

#include <type_support.h>            /* uint8_t */

#include <ia_css_psys_program_group_manifest.sim.h>

#include <ia_css_psys_program_group_manifest.hsys.user.h>

#include <ia_css_kernel_bitmap.h>        /* ia_css_kernel_bitmap_t */
#include "ia_css_terminal_manifest.h"
#include "ia_css_rbm_manifest_types.h"

#define IA_CSS_PROGRAM_GROUP_INVALID_ALIGNMENT        ((uint8_t)(-1))

/*! Get the stored size of the program group manifest object

 @param    manifest[in]            program group manifest object

 @return size, 0 on invalid argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
size_t ia_css_program_group_manifest_get_size(
    const ia_css_program_group_manifest_t *manifest);

/*! Get the program group ID of the program group manifest object

 @param    manifest[in]            program group manifest object

 @return program group ID, IA_CSS_PROGRAM_GROUP_INVALID_ID on invalid argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_group_ID_t
ia_css_program_group_manifest_get_program_group_ID(
    const ia_css_program_group_manifest_t *manifest);

/*! Set the program group ID of the program group manifest object

 @param    manifest[in]            program group manifest object

 @param program group ID

 @return 0 on success, -1 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_program_group_manifest_set_program_group_ID(
    ia_css_program_group_manifest_t *manifest,
    ia_css_program_group_ID_t id);

/*! Get the storage alignment constraint of the program group binary data

 @param    manifest[in]            program group manifest object

 @return alignment, IA_CSS_PROGRAM_GROUP_INVALID_ALIGNMENT on invalid manifest
    argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_group_manifest_get_alignment(
    const ia_css_program_group_manifest_t *manifest);

/*! Set the storage alignment constraint of the program group binary data

 @param    manifest[in]            program group manifest object
 @param    alignment[in]            alignment desired

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_program_group_manifest_set_alignment(
    ia_css_program_group_manifest_t *manifest,
    const uint8_t alignment);

/*! Get the kernel enable bitmap of the program group

 @param    manifest[in]            program group manifest object

 @return bitmap, 0 on invalid manifest argument
 */
extern ia_css_kernel_bitmap_t
ia_css_program_group_manifest_get_kernel_bitmap(
    const ia_css_program_group_manifest_t *manifest);

/*! Set the kernel enable bitmap of the program group

 @param    manifest[in]            program group manifest object
 @param    kernel bitmap[in]        kernel enable bitmap

 @return < 0 on invalid manifest argument
 */
extern int ia_css_program_group_manifest_set_kernel_bitmap(
    ia_css_program_group_manifest_t *manifest,
    const ia_css_kernel_bitmap_t bitmap);

/*! Get the number of programs in the program group manifest object

 @param    manifest[in]            program group manifest object

 @return program count, 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_group_manifest_get_program_count(
    const ia_css_program_group_manifest_t *manifest);

/*! Get the number of terminals in the program group manifest object

 @param    manifest[in]            program group manifest object

 @return terminal count, 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
uint8_t ia_css_program_group_manifest_get_terminal_count(
    const ia_css_program_group_manifest_t *manifest);

/*! Get the (pointer to) private data blob in the manifest

  Private data is used by FW and is opauque to the SW stack

 @param    manifest[in]            program group manifest object

 @return private data blob, NULL on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
void *ia_css_program_group_manifest_get_private_data(
    const ia_css_program_group_manifest_t *manifest);

/*! Get the (pointer to) routing bitmap (rbm) manifest

 @param    manifest[in]            program group manifest object

 @return rbm manifest, NULL on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_rbm_manifest_t *
ia_css_program_group_manifest_get_rbm_manifest(
    const ia_css_program_group_manifest_t *manifest);

/*! Get the (pointer to) indexed program manifest in the program group manifest
 * object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the program manifest object

 @return program manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_manifest_t *
ia_css_program_group_manifest_get_prgrm_mnfst(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int program_index);

/*! Get the (pointer to) indexed terminal manifest in the program group
 * manifest object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the terminal manifest object

 @return terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_manifest_t *
ia_css_program_group_manifest_get_term_mnfst(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*! Get the (pointer to) indexed data terminal manifest in the program group
 * manifest object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the terminal manifest object

 @return data terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_data_terminal_manifest_t *
ia_css_program_group_manifest_get_data_terminal_manifest(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*! Get the (pointer to) indexed parameter terminal manifest in the program
 * group manifest object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the terminal manifest object

 @return parameter terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_param_terminal_manifest_t *
ia_css_program_group_manifest_get_param_terminal_manifest(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*! Get the (pointer to) indexed spatial param terminal manifest in the program
 * group manifest object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the terminal manifest object

 @return spatial param terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_spatial_param_terminal_manifest_t *
ia_css_program_group_manifest_get_spatial_param_terminal_manifest(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*! Get the (pointer to) indexed sliced param terminal manifest in the program
 * group manifest object

 @param    manifest[in]            program group manifest object
 @param    program_index[in]        index of the terminal manifest object

 @return sliced param terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_sliced_param_terminal_manifest_t *
ia_css_program_group_manifest_get_sliced_param_terminal_manifest(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*! Get the (pointer to) indexed program terminal manifest in the program group
 * manifest object

 @parammanifest[in]program group manifest object
 @paramprogram_index[in]index of the terminal manifest object

 @return program terminal manifest, NULL on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_terminal_manifest_t *
ia_css_program_group_manifest_get_program_terminal_manifest(
    const ia_css_program_group_manifest_t *manifest,
    const unsigned int terminal_index);

/*!    initialize program group manifest

 @param    manifest[in]        program group manifest object
 @param    program_count[in]    number of programs.
 @param    terminal_count[in]    number of terminals.
 @param program_needs_extension[in] Array[program count]
 @param    program_deps[in]    program dependencies for programs in pg.
 @param    terminal_deps[in]    terminal dependencies for programs in pg.
 @param    terminal_type[in]    array of terminal types, binary specific
                static frame data
 @param    cached_in_param_section_count[in]Number of parameter terminal sections
 @param cached_out_param_section_count[in]    Number of parameter out terminal
 @param    spatial_param_section_count[in]        Array[spatial_terminal_count]
                        with sections per cached out
                        terminal
 @param sliced_in_param_section_count[in]    Array[sliced_in_terminal_count]
                        with sections per sliced in
                        terminal
 @param sliced_out_param_section_count[in]    Array[sliced_out_terminal_count]
                        with sections per sliced out
                        terminal
 @param    fragment_param_section_count[in]    Number of fragment parameter
                        sections of the program init
                        terminal,
 @param    kernel_fragment_seq_count[in]        Number of kernel fragment
                        seqence info.
 @param    progctrlinit_load_section_counts[in]    Number of progctrinit load
                        sections (size of array is program_count)
 @param    progctrlinit_connect_section_counts[in]    Number of progctrinit connect
                        sections (size of array is program_count)
 @return none;
 */
extern void ia_css_program_group_manifest_init(
    ia_css_program_group_manifest_t *blob,
    const uint8_t program_count,
    const uint8_t terminal_count,
    const uint8_t *program_needs_extension,
    const uint8_t *program_dependencies,
    const uint8_t *terminal_dependencies,
    const ia_css_terminal_type_t *terminal_type,
    const uint16_t *cached_in_param_section_count,
    const uint16_t *cached_out_param_section_count,
    const uint16_t *spatial_param_section_count,
    const uint16_t *fragment_param_section_count,
    const uint16_t *sliced_in_param_section_count,
    const uint16_t *sliced_out_param_section_count,
    const uint16_t *kernel_fragment_seq_count,
    const uint16_t *progctrlinit_load_section_counts,
    const uint16_t *progctrlinit_connect_section_counts);

#ifdef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_program_group_manifest_impl.h"
#endif /* __IA_CSS_PSYS_STATIC_INLINE__ */

#endif /* __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_H */
