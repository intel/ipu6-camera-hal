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

#ifndef __IA_CSS_TERMINAL_MANIFEST_BASE_TYPES_H
#define __IA_CSS_TERMINAL_MANIFEST_BASE_TYPES_H

#include "ia_css_terminal_defs.h"

/**
 * @addtogroup group_psysapi
 * @{
 */

typedef struct ia_css_terminal_manifest_attributes_t {
	uint16_t category:1; /**< load vs. connect.  @see ia_css_terminal_cat_t */
	uint16_t direction:1; /**< input vs. output. @see ia_css_terminal_dir_t */
	uint16_t rou:2; /**< Rate Of Update.  @see ia_css_terminal_rou_t */
	uint16_t connect_buf_type:2; /**< image data vs. meta-data. @see ia_css_connect_buf_type_t */
	uint16_t reserved:10;
	} ia_css_terminal_manifest_attributes_t;
#define IA_CSS_TERMINAL_ATTRIBUTE_BITS IA_CSS_UINT16_T_BITS

#define N_PADDING_UINT8_IN_TERMINAL_MAN_STRUCT	0
#define SIZE_OF_TERMINAL_MANIFEST_STRUCT_IN_BITS \
	(IA_CSS_TERMINAL_TYPE_BITS \
	+ IA_CSS_UINT32_T_BITS \
	+ IA_CSS_UINT16_T_BITS \
	+ IA_CSS_UINT16_T_BITS \
	+ IA_CSS_TERMINAL_ATTRIBUTE_BITS \
	+ IA_CSS_TERMINAL_ID_BITS \
	+ IA_CSS_TERMINAL_ID_BITS \
	+ (N_PADDING_UINT8_IN_TERMINAL_MAN_STRUCT*IA_CSS_UINT8_T_BITS))

/* ==================== Base Terminal Manifest - START ==================== */
struct ia_css_terminal_manifest_s {
	ia_css_terminal_type_t				terminal_type;		/**< Type ia_css_terminal_type_t */
	/**  Max size of payload buffer for instances of this terminal
	 *
	 *   For load terminals, this is the size of all sections, as defined in the manifest
	 *   descriptors.  For connect terminals, this is the max buffer size.
	 */
	uint32_t					max_payload_size;
	int16_t						parent_offset;		/**< Offset to the program group manifest */
	uint16_t					size;			/**< Size of this whole terminal-manifest layout-structure */
	ia_css_terminal_manifest_attributes_t	attributes; /**< Bit fields describing the terminal attributes */
	ia_css_terminal_ID_t				ID; /**< The unique identifier of this terminal in the PG */
	/** The unique identifier of another terminal in the PG, associated with this one.
	 *  Set to IA_CSS_TERMINAL_INVALID_ID by default (--> no associated terminal) */
	ia_css_terminal_ID_t				assoc_ID;
#if N_PADDING_UINT8_IN_TERMINAL_MAN_STRUCT > 0
	uint8_t						padding[N_PADDING_UINT8_IN_TERMINAL_MAN_STRUCT];
#endif
};

typedef struct ia_css_terminal_manifest_s
	ia_css_terminal_manifest_t;

/* ==================== Base Terminal Manifest - END ==================== */

/** @} */

#endif /* __IA_CSS_TERMINAL_MANIFEST_BASE_TYPES_H */
