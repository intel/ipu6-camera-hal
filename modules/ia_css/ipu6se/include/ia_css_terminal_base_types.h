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

#ifndef __IA_CSS_TERMINAL_BASE_TYPES_H
#define __IA_CSS_TERMINAL_BASE_TYPES_H

#include "type_support.h"
#include "ia_css_terminal_defs.h"

#define N_UINT16_IN_TERMINAL_STRUCT		3
#define N_UINT32_IN_TERMINAL_STRUCT		1
#define N_PADDING_UINT8_IN_TERMINAL_STRUCT	1

#define SIZE_OF_TERMINAL_STRUCT_BITS \
	(IA_CSS_TERMINAL_TYPE_BITS \
	+ N_UINT32_IN_TERMINAL_STRUCT * IA_CSS_UINT32_T_BITS \
	+ IA_CSS_TERMINAL_ID_BITS  \
	+ N_UINT16_IN_TERMINAL_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_PADDING_UINT8_IN_TERMINAL_STRUCT * IA_CSS_UINT8_T_BITS)

/* ==================== Base Terminal - START ==================== */
/** Base structure for all terminal types.
 *
 *  Terminals describe an opening between a program group and the outside world.
 *  They come in various types (data, statisitics, parameters, etc.) which may have
 *  extensions specific to those types.  This structure is the base for all of those and
 *  must be the first field in any  subclassed terminal structure type.
 */
struct ia_css_terminal_s {						/**< Base terminal */
	ia_css_terminal_type_t			terminal_type;		/**< Type ID of the terminal.  This type enum defines the purpose of the terminal and its structure. */
	uint32_t				payload_size;	/**< Size of buffer passed in buffer set for this terminal */
	int16_t					parent_offset;		/**< Offset to the containing process group */
	uint16_t				size;			/**< Size of this whole terminal layout-structure, including subclassed extensions. */
	uint16_t				tm_index;		/**< Index of the terminal manifest object in the program group manifest. */
	ia_css_terminal_ID_t			ID;			/**< Absolute referal ID for this terminal, valid ID's != 0 */
	uint8_t					padding[N_PADDING_UINT8_IN_TERMINAL_STRUCT];
};
/* ==================== Base Terminal - END ==================== */

#endif /* __IA_CSS_TERMINAL_BASE_TYPES_H */
