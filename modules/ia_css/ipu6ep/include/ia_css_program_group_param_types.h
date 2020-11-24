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

#ifndef __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H
#define __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H

/*! \file */

/** @file ia_css_program_group_param_types.h
 *
 * Define the parameter objects that are necessary to create the process
 * groups i.e. enable parameters and parameters to set-up frame descriptors
 */

#include <ia_css_program_group_data.h>
#include <ia_css_kernel_bitmap.h>	/* ia_css_kernel_bitmap_t */
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
	uint32_t size;	/**< Size of this structure */
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
	uint8_t bpp;	/**< Bits per pixel.  The number of bits with significant data. */
	uint8_t bpe;	/**< Bits per element  The number of bits required to store the pixel.  Must be >= bpp. */
};

typedef struct ia_css_program_group_param_s	ia_css_program_group_param_t;
typedef struct ia_css_program_param_s		ia_css_program_param_t;
typedef struct ia_css_terminal_param_s		ia_css_terminal_param_t;

#endif /* __IA_CSS_PROGRAM_GROUP_PARAM_TYPES_H */
