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

#ifndef __IA_CSS_PROGRAM_GROUP_PARAM_PRIVATE_H
#define __IA_CSS_PROGRAM_GROUP_PARAM_PRIVATE_H

#include <ia_css_program_group_param.h>
#include <ia_css_psys_manifest_types.h>
#include <ia_css_psys_program_group_manifest.h>
#include <ia_css_psys_terminal_manifest.h>
#include <ia_css_kernel_bitmap.h>
#include <ia_css_program_group_data.h>
#include <type_support.h>

#define N_PADDING_UINT8_IN_PROGRAM_GROUP_PARAM_STRUCT 7
#define SIZE_OF_PROGRAM_GROUP_PARAM_STRUCT_IN_BITS \
	(IA_CSS_KERNEL_BITMAP_BITS \
	+ (3 * IA_CSS_UINT32_T_BITS) \
	+ IA_CSS_UINT16_T_BITS \
	+ (3 * IA_CSS_UINT8_T_BITS) \
	+ (N_PADDING_UINT8_IN_PROGRAM_GROUP_PARAM_STRUCT * IA_CSS_UINT8_T_BITS))

/* tentative; co-design with ISP algorithm */
struct ia_css_program_group_param_s {
	/** The enable bits for each individual kernel */
	ia_css_kernel_bitmap_t kernel_enable_bitmap;
	/** Size of this structure */
	uint32_t size;
	/** Offset of program parameters array from the base of this structure.  See ia_css_program_param_s */
	uint32_t program_param_offset;
	/** Offset of terminal parameters array from the base of this structure.  See ia_css_terminal_param_s */
	uint32_t terminal_param_offset;
	/** Number of (explicit) fragments to use in a frame */
	uint16_t fragment_count;
	/** Number of active programs */
	uint8_t program_count;
	/** Number of active terminals */
	uint8_t terminal_count;
	/** Program group protocol version */
	uint8_t protocol_version;
	uint8_t padding[N_PADDING_UINT8_IN_PROGRAM_GROUP_PARAM_STRUCT];
};

#define SIZE_OF_PROGRAM_PARAM_STRUCT_IN_BITS \
	(IA_CSS_KERNEL_BITMAP_BITS \
	+ IA_CSS_UINT32_T_BITS \
	+ IA_CSS_INT32_T_BITS)

/* private */
struct ia_css_program_param_s {
	/** Bitmap of kernel encapsulated by this program.  If all of these kernels are
	 *  disabled, the program will be disabled too. */
	ia_css_kernel_bitmap_t kernel_enable_bitmap;
	/** Size of this structure */
	uint32_t size;
	/** Offset to add to reach parent. This is negative value.*/
	int32_t parent_offset;
};

#define SIZE_OF_TERMINAL_PARAM_STRUCT_IN_BITS \
	(IA_CSS_UINT32_T_BITS \
	+ IA_CSS_FRAME_FORMAT_TYPE_BITS \
	+ IA_CSS_INT32_T_BITS \
	+ (IA_CSS_UINT16_T_BITS * IA_CSS_N_DATA_DIMENSION) \
	+ (IA_CSS_UINT16_T_BITS * IA_CSS_N_DATA_DIMENSION) \
	+ (IA_CSS_UINT16_T_BITS * IA_CSS_N_DATA_DIMENSION) \
	+ IA_CSS_INT32_T_BITS \
	+ IA_CSS_UINT16_T_BITS \
	+ IA_CSS_UINT8_T_BITS \
	+ (IA_CSS_UINT8_T_BITS * 1))

#endif /* __IA_CSS_PROGRAM_GROUP_PARAM_PRIVATE_H */
