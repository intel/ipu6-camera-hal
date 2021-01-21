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

#ifndef __IA_CSS_PSYS_TERMINAL_MANIFEST_H
#define __IA_CSS_PSYS_TERMINAL_MANIFEST_H

/*! \file */

/** @file ia_css_psys_terminal_manifest.h
 *
 * Define the methods on the terminal manifest object that are not part of a
 * single interface
 */

#include <ia_css_psys_manifest_types.h>

#include <ia_css_psys_terminal_manifest.sim.h>

#include <ia_css_psys_terminal_manifest.hsys.user.h>

#include <ia_css_program_group_data.h>    /* ia_css_frame_format_bitmap_t */
#include <ia_css_kernel_bitmap.h>    /* ia_css_kernel_bitmap_t */

#include <type_support.h>        /* size_t */
#include "ia_css_terminal_manifest.h"
#include "ia_css_terminal_manifest_base_types.h"
#include "ia_css_psys_static_storage_class.h"

/*!
 * @addtogroup group_psysapi
 * @{
 */

/*! Check if the terminal manifest object specifies a spatial param terminal
 * type

 @param [in]    manifest            terminal manifest object

 @return is_parameter_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_spatial_parameter_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Check if the terminal manifest object specifies a program terminal type

 @param [in]    manifest            terminal manifest object

 @return is_parameter_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_program_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Check if the terminal manifest object specifies a program control init terminal type
 *
 * @param [in]    manifest            terminal manifest object
 *
 * @return is_parameter_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_program_control_init_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Check if the terminal manifest object specifies a (cached) parameter
 * terminal type

 @param [in]    manifest            terminal manifest object

 @return is_parameter_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_parameter_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Check if the terminal manifest object specifies a (sliced) parameter
 * terminal type

 @param [in]    manifest            terminal manifest object

 @return is_parameter_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_sliced_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Check if the terminal manifest object specifies a data terminal type

 @param [in]    manifest            terminal manifest object

 @return is_data_terminal, false on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_is_terminal_manifest_data_terminal(
    const ia_css_terminal_manifest_t        *manifest);

/*! Get the broad terminal category

 @param    manifest[in]            terminal manifest object

 @return Terminal category enum

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_cat_t ia_css_terminal_manifest_get_category(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the broad terminal category

 @param    manifest[in]            terminal manifest object
 @param    category[in]            terminal category

 @return < 0 on invalid arguments

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_category(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_cat_t category);

/*! Get the direction of data flow for the terminal payload

 @param    manifest[in]            terminal manifest object

 @return Terminal direction enum

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_dir_t ia_css_terminal_manifest_get_direction(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the direction of data flow for the terminal payload

 @param    manifest[in]            terminal manifest object
 @param    direction[in]            terminal direction

 @return < 0 on invalid arguments

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_direction(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_dir_t direction);

/*! Get the Rate Of Update (ROU) for the terminal payload

 @param    manifest[in]            terminal manifest object

 @return Terminal rate-of-update enum

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_rou_t ia_css_terminal_manifest_get_rate_of_update(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the Rate Of Update (ROU) for the terminal payload

 @param    manifest[in]            terminal manifest object
 @param    rate_of_update[in]        terminal rate of update

 @return < 0 on invalid arguments

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_rate_of_update(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_terminal_rou_t rate_of_update);

/*! Get the buffer type for the connect terminal pointer

 This function is intended for connect terminals only.  If called
 on a load terminal, it will return IA_CSS_CONNECT_BUF_TYPE_UNKNOWN

 @param    manifest[in]            terminal manifest object

 @return Connect terminal buffer type enum

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_connect_buf_type_t ia_css_terminal_manifest_get_connect_buffer_type(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the buffer type for the connect terminal pointer

  Load terminals must be set only to IA_CSS_CONNECT_BUF_TYPE_UNKNOWN

 @param    manifest[in]            terminal manifest object

 @return < 0 on invalid arguments

  @note New for IPU7 POC
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_connect_buffer_type(
    ia_css_terminal_manifest_t        *manifest,
    ia_css_connect_buf_type_t connect_buf_type);

/*! Get the stored size of the terminal manifest object

 @param [in]    manifest            terminal manifest object

 @return size, 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
size_t ia_css_terminal_manifest_get_size(
    const ia_css_terminal_manifest_t        *manifest);

/*! Get the (pointer to) the program group manifest parent of the terminal
 * manifest object

 @param [in]    manifest            terminal manifest object

 @return the pointer to the parent, NULL on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_program_group_manifest_t *ia_css_terminal_manifest_get_parent(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the (pointer to) the program group manifest parent of the terminal
 * manifest object

 @param [in]    manifest            terminal manifest object
 @param    [in]    terminal_offset        this terminal's offset from
                    program_group_manifest base address.

 @return < 0 on invalid arguments
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_parent_offset(
    ia_css_terminal_manifest_t            *manifest,
    int32_t                        terminal_offset);

/*! Get the type of the terminal manifest object

 @param [in]    manifest            terminal manifest object

 @return terminal type, limit value (IA_CSS_N_TERMINAL_TYPES) on invalid
    manifest argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_type_t ia_css_terminal_manifest_get_type(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the type of the terminal manifest object

 @param [in]    manifest            terminal manifest object
 @param    [in]    terminal_type        terminal type

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_type(
    ia_css_terminal_manifest_t            *manifest,
    const ia_css_terminal_type_t            terminal_type);

/*! Set the ID of the terminal manifest object

 @param [in]    manifest            terminal manifest object
 @param    [in]    ID                terminal ID

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_ID(
    ia_css_terminal_manifest_t            *manifest,
    const ia_css_terminal_ID_t            ID);

/*! Get the type of the terminal manifest object

 @param [in]    manifest            terminal manifest object

 @return  terminal id, IA_CSS_TERMINAL_INVALID_ID on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_ID_t ia_css_terminal_manifest_get_ID(
    const ia_css_terminal_manifest_t        *manifest);

/*! Get the ID of the associated connection terminal

  Only valid for load terminals.

  @param    manifest[in]            terminal manifest object

  @return  terminal id, IA_CSS_TERMINAL_INVALID_ID on invalid manifest argument
  OR if there is no associated connect terminal
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_terminal_ID_t ia_css_terminal_manifest_get_connect_ID(
    const ia_css_terminal_manifest_t        *manifest);

/*! Set the ID of the associated connection terminal

  Only valid for load terminals.

  @param    manifest[in]            terminal manifest object
  @param    ID[in]                terminal ID

  @return < 0 on invalid manifest argument (including when
  manifest describes a connect terminal, unless ID is
  IA_CSS_TERMINAL_INVALID_ID)
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_connect_ID(
    ia_css_terminal_manifest_t            *manifest,
    const ia_css_terminal_ID_t            ID);

/*! Returns the maximum payload size for an instance of the terminal
   described in a manifest entry.

   The payload size is the sum of the sizes of all
   parameter sections for load terminals, or the maximum buffer size
   for connect terminals.

   @param manifest[in] Terminal manifest handle

   @return Maximum size in bytes of the terminal payload
 */
IA_CSS_PARAMETERS_STORAGE_CLASS_H
unsigned int ia_css_terminal_manifest_get_max_payload_size(
    const ia_css_terminal_manifest_t *manifest);

/*! Returns the maximum payload size for an instance of the terminal
   described in a manifest entry.

   The payload size is the sum of the sizes of all
   parameter sections for load terminals, or the maximum buffer size
   for connect terminals.

   @param manifest[in] Terminal manifest handle
   @param max_payload_size[in] Maximum payload size

  @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_terminal_manifest_set_max_payload_size(
    ia_css_terminal_manifest_t            *manifest,
    uint32_t                             max_payload_size);

/*! Get the supported frame types of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object

 @return frame format bitmap, 0 on invalid manifest argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_frame_format_bitmap_t
    ia_css_data_terminal_manifest_get_frame_format_bitmap(
        const ia_css_data_terminal_manifest_t        *manifest);

/*! Set the chosen frame type for the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    bitmap            frame format bitmap

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_data_terminal_manifest_set_frame_format_bitmap(
    ia_css_data_terminal_manifest_t            *manifest,
    ia_css_frame_format_bitmap_t            bitmap);

/*! Check if the (data) terminal manifest object supports compression

 @param [in]    manifest            (data) terminal manifest object

 @return compression_support, true if compression is supported
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
bool ia_css_data_terminal_manifest_can_support_compression(
    const ia_css_data_terminal_manifest_t        *manifest);

/*! Set the compression support feature of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    compression_support        set true to support compression

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_data_terminal_manifest_set_compression_support(
    ia_css_data_terminal_manifest_t            *manifest,
    bool                        compression_support);

/*! Set the supported connection types of the terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    bitmap            connection bitmap

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_data_terminal_manifest_set_connection_bitmap(
    ia_css_data_terminal_manifest_t *manifest, ia_css_connection_bitmap_t bitmap);

/*! Get the connection bitmap of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object

 @return connection bitmap, 0 on invalid manifest argument
*/
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_connection_bitmap_t ia_css_data_terminal_manifest_get_connection_bitmap(
        const ia_css_data_terminal_manifest_t        *manifest);

/*! Get the kernel dependency of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object

 @return kernel bitmap, 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_data_terminal_manifest_get_kernel_bitmap(
    const ia_css_data_terminal_manifest_t        *manifest);

/*! Set the kernel dependency of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    kernel_bitmap        kernel dependency bitmap

 @return < 0 on invalid manifest argument
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
int ia_css_data_terminal_manifest_set_kernel_bitmap(
    ia_css_data_terminal_manifest_t            *manifest,
    const ia_css_kernel_bitmap_t            kernel_bitmap);

/*! Set the unique kernel dependency of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    index            kernel dependency bitmap index

 @return < 0 on invalid argument(s)
 */
extern int ia_css_data_terminal_manifest_set_kernel_bitmap_unique(
    ia_css_data_terminal_manifest_t            *manifest,
    const unsigned int                index);

/*! Set the min size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    min_size            Minimum size of the frame array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_set_min_size(
    ia_css_data_terminal_manifest_t *manifest,
    const uint16_t min_size[IA_CSS_N_DATA_DIMENSION]);

/*! Set the max size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    max_size            Maximum size of the frame array

  @return < 0 on invalid manifest argument
  */
extern int ia_css_data_terminal_manifest_set_max_size(
    ia_css_data_terminal_manifest_t *manifest,
    const uint16_t max_size[IA_CSS_N_DATA_DIMENSION]);

/*! Get the min size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    min_size            Minimum size of the frame array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_get_min_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t min_size[IA_CSS_N_DATA_DIMENSION]);

/*! Get the max size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    max_size            Maximum size of the frame array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_get_max_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t max_size[IA_CSS_N_DATA_DIMENSION]);

/*! Set the min fragment size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    min_size            Minimum size of the fragment array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_set_min_fragment_size(
    ia_css_data_terminal_manifest_t *manifest,
    const uint16_t min_size[IA_CSS_N_DATA_DIMENSION]);

/*! Set the max fragment size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    max_size            Maximum size of the fragment array

  @return < 0 on invalid manifest argument
  */
extern int ia_css_data_terminal_manifest_set_max_fragment_size(
    ia_css_data_terminal_manifest_t *manifest,
    const uint16_t max_size[IA_CSS_N_DATA_DIMENSION]);

/*! Get the min fragment size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    min_size            Minimum size of the fragment array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_get_min_fragment_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t min_size[IA_CSS_N_DATA_DIMENSION]);

/*! Get the max fragment size of the (data) terminal manifest object

 @param [in]    manifest            (data) terminal manifest object
 @param    [in]    max_size            Maximum size of the fragment array

 @return < 0 on invalid manifest argument
 */
extern int ia_css_data_terminal_manifest_get_max_fragment_size(
    const ia_css_data_terminal_manifest_t *manifest,
    uint16_t max_size[IA_CSS_N_DATA_DIMENSION]);

/*!
 * Get the program control init connect section count for program prog.
 * @param [in] prog program control init terminal program desc
 * @return number of connect section for program prog.
 */

extern
unsigned int ia_css_program_control_init_terminal_manifest_get_connect_section_count(
    const ia_css_program_control_init_manifest_program_desc_t *prog);

/*!
 * Get the program control init load section count for program prog.
 * @param [in] prog program control init terminal program desc
 * @return number of load section for program prog.
 */

extern
unsigned int ia_css_program_control_init_terminal_manifest_get_load_section_count(
    const ia_css_program_control_init_manifest_program_desc_t *prog);

/*!
 * Get the program control init terminal manifest size.
 * @param [in] nof_programs        Number of programs.
 * @param [in] nof_load_sections        Array of size nof_programs,
 *                    encoding the number of load sections.
 * @param [in] nof_connect_sections    Array of size nof_programs,
 *                    encoding the number of connect sections.
 * @return < 0 on invalid manifest argument
 */
extern
unsigned int ia_css_program_control_init_terminal_manifest_get_size(
    const uint16_t nof_programs,
    const uint16_t *nof_load_sections,
    const uint16_t *nof_connect_sections);

/*!
 * Get the program control init terminal manifest program desc.
 * @param [in] terminal        Program control init terminal.
 * @param [in] program        Number of programs.
 * @return program control init terminal program desc (or NULL if error).
 */
extern
ia_css_program_control_init_manifest_program_desc_t *
ia_css_program_control_init_terminal_manifest_get_program_desc(
    const ia_css_program_control_init_terminal_manifest_t *terminal,
    unsigned int program);

/*!
 * Initialize the program control init terminal manifest.
 * @param [in] nof_programs        Number of programs
 * @param [in] nof_load_sections        Array of size nof_programs,
 *                    encoding the number of load sections.
 * @param [in] nof_connect_sections    Array of size nof_programs,
 *                    encoding the number of connect sections.
 * @return < 0 on invalid manifest argument
 */
extern
int ia_css_program_control_init_terminal_manifest_init(
    ia_css_program_control_init_terminal_manifest_t *terminal,
    const uint16_t nof_programs,
    const uint16_t *nof_load_sections,
    const uint16_t *nof_connect_sections);

/*!
 * Pretty prints the program control init terminal manifest.
 * @param [in] terminal        Program control init terminal.
 */
extern
void ia_css_program_control_init_terminal_manifest_print(
    ia_css_program_control_init_terminal_manifest_t *terminal);

#ifdef __IA_CSS_PSYS_STATIC_INLINE__
#include "ia_css_psys_terminal_manifest_impl.h"
#endif

#endif /* __IA_CSS_PSYS_TERMINAL_MANIFEST_H */
