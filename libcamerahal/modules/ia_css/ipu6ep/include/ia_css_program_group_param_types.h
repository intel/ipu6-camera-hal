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

#ifndef __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H
#define __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H

/*! \file */

/** @file ia_css_program_group_param_types.h
 *
 * Define the parameter objects that are necessary to create the process
 * groups i.e. enable parameters and parameters to set-up frame descriptors
 */

#include <ia_css_program_group_data.h>
#include <ia_css_kernel_bitmap.h>    /* ia_css_kernel_bitmap_t */
#include <ia_css_terminal_defs.h>

#include <type_support.h>
/*! make this public so that driver can populate,
 * size, bpp, dimensions for all terminals.
 *
 * Currently one API is provided to get frame_format_type.
 *
 * frame_format_type is set during ia_css_terminal_param_init().
 * Value for that is const and binary specific.
 */
struct ia_css_terminal_param_s {
    uint32_t size;    /**< Size of this structure */
    /** Indicates if this is a generic type or inbuild
     * with variable size descriptor
     */
    ia_css_frame_format_type_t frame_format_type;
    /** offset to add to reach parent. This is negative value.*/
    int32_t parent_offset;
    uint16_t dimensions[IA_CSS_N_DATA_DIMENSION];/**< Logical dimensions */
    /** Mapping to the index field of the terminal descriptor */
    uint16_t index[IA_CSS_N_DATA_DIMENSION];
    /** Logical fragment dimension,
     * TODO: fragment dimensions can be different per fragment
     */
    uint16_t fragment_dimensions[IA_CSS_N_DATA_DIMENSION];
    uint32_t stride;/**< Stride (buffer width) of a frame in bytes */
    uint16_t offset;/**< Offset in bytes to first fragment */
    uint8_t bpp;    /**< Bits per pixel.  The number of bits with significant data. */
    uint8_t bpe;    /**< Bits per element  The number of bits required to store the pixel.  Must be >= bpp. */
};

typedef struct ia_css_program_group_param_s    ia_css_program_group_param_t;
typedef struct ia_css_program_param_s        ia_css_program_param_t;
typedef struct ia_css_terminal_param_s        ia_css_terminal_param_t;

#endif /* __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H */
