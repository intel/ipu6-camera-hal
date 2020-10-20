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

#ifndef __IA_CSS_PSYS_TERMINAL_H
#define __IA_CSS_PSYS_TERMINAL_H

/*! \file */

/** @file ia_css_psys_terminal.h
 *
 * Define the methods on the terminal object that are not part of
 * a single interface
 */

#include <ia_css_program_group_data.h>	/* ia_css_frame_t */
#include <ia_css_program_group_param.h>	/* ia_css_program_group_param_t */

#include <ia_css_psys_process_types.h>
#include <ia_css_psys_manifest_types.h>

#include <type_support.h>		/* bool */
#include <print_support.h>		/* FILE */
#include "ia_css_psys_dynamic_storage_class.h"
#include "ia_css_terminal.h"
#include "ia_css_terminal_manifest_base_types.h"

/*
 * Creation
 */
#include <ia_css_psys_terminal.hsys.user.h>

/*! Boolean test if the terminal object type is input

 @param	terminal[in]			terminal object

 @return true if the terminal is input, false otherwise or on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_is_terminal_input(
	const ia_css_terminal_t					*terminal);

/*! Get the stored size of the terminal object

 @param	terminal[in]			terminal object

 @return size, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
size_t ia_css_terminal_get_size(
	const ia_css_terminal_t					*terminal);

/*! Get the type of the terminal object

 @param	terminal[in]			terminal object

 @return the type of the terminal, limit value on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_terminal_type_t ia_css_terminal_get_type(
	const ia_css_terminal_t					*terminal);

/*! Set the type of the terminal object

 @param	terminal[in]			terminal object
 @param	terminal_type[in]		type of the terminal

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_terminal_set_type(
	ia_css_terminal_t		*terminal,
	const ia_css_terminal_type_t	terminal_type);

/*! Get the index of the terminal manifest object

 @param	terminal[in]			terminal object

 @return the index of the terminal manifest object, limit value on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint16_t ia_css_terminal_get_terminal_manifest_index(
	const ia_css_terminal_t					*terminal);

/*! Set the index of the terminal manifest object

 @param	terminal[in]			terminal object
 @param	tm_index[in]			terminal manifest index

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_terminal_set_terminal_manifest_index(
	ia_css_terminal_t	*terminal,
	const uint16_t		tm_index);

/*! Get the payload (buffer) size for all buffers that will be supplied for the terminal

 @param	terminal[in]			terminal object

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_terminal_get_payload_size(
	const ia_css_terminal_t *terminal);

/*! Set the payload (buffer) size for all buffers that will be supplied for the terminal

 @param	terminal[in]			terminal object
 @param	payload_size[in]		Size of buffer that will be supplied to the terminal

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_terminal_set_payload_size(
	ia_css_terminal_t *terminal,
	uint32_t payload_size);

/*! Get id of the terminal object

 @param	terminal[in]			terminal object

 @return id of terminal
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_terminal_ID_t ia_css_terminal_get_ID(
	const ia_css_terminal_t			*terminal);

/*! Get compressed byte of the data terminal object

 @param	dterminal[in]			data terminal object
 @param val[out]			is_compressed value

 @@return < 0 on error
 */

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_data_terminal_is_compressed(
	const ia_css_data_terminal_t *dterminal,
	uint8_t *val);

/*! Get kernel id of the data terminal object

 @param	dterminal[in]			data terminal object

 @return kernel id of terminal
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_data_terminal_get_kernel_id(
	const ia_css_data_terminal_t			*dterminal);

/*! Get the connection type from the terminal object

 @param	terminal[in]			terminal object

 @return buffer type, limit value on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_connection_type_t ia_css_data_terminal_get_connection_type(
	const ia_css_data_terminal_t	*dterminal);

/*! Set the connection type of the terminal object

 @param	terminal[in]			terminal object
 @param	connection_type[in]		connection type

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_data_terminal_set_connection_type(
	ia_css_data_terminal_t				*dterminal,
	const ia_css_connection_type_t			connection_type);

/*! Get link id of the data terminal object

 @param	dterminal[in]			data terminal object

 @return link id of terminal
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_data_terminal_get_link_id(
	const ia_css_data_terminal_t			*dterminal);

/*! Set link id of the terminal object

 @param	terminal[in]			data terminal object
 @param	link_id[in]			synchronization link id

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_data_terminal_set_link_id(
	ia_css_data_terminal_t				*dterminal,
	const uint8_t					link_id);

/*! Set stream2gen buffer size used in stream to GEN connection
 * This function should be called, in case we are in streaming use-case, and
 * we need to configure the stream2gen_buffer_size to be less than full image's height.
 * Note, that the stream2gen_buffer_size is set to a defalut value of full image's height
 * during terminal_create function.

 * @param	terminal[in]			data terminal object
 * @param	stream2gen_buffer_size[in]		Buffer size (Size's units are lines)

 * @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_data_terminal_set_stream2gen_buffer_size(
	ia_css_data_terminal_t *dterminal,
	const uint16_t stream2gen_buffer_size);

/*! Get the (pointer to) the process group parent of the terminal object

 @param	terminal[in]			terminal object

 @return the pointer to the parent, NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_process_group_t *ia_css_terminal_get_parent(
	const ia_css_terminal_t					*terminal);

/*! Set the (pointer to) the process group parent of the terminal object

 @param	terminal[in]	terminal object
 @param	parent[in]	(pointer to the) process group parent object

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_terminal_set_parent(
	ia_css_terminal_t	*terminal,
	ia_css_process_group_t	*parent);

/*! Boolean test if the terminal object type is valid

 @param	terminal[in]			process terminal object
 @param	terminal_manifest[in]		program terminal manifest

 @return true if the process terminal object is correct, false on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_is_terminal_valid(
	const ia_css_terminal_t		 *terminal,
	const ia_css_terminal_manifest_t *terminal_manifest);

/* ================= Program Control Init Terminal - START ================= */

/*!
 * Gets the program init terminal descripor size
 * @param manifest[in]		program control init terminal manifest
 * @param pg_manifest[in]	program group manifest
 * @param kernel_bitmap[in]	kernel enable bitmap
 * @return size, error if < 0.
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
unsigned int
ia_css_program_control_init_terminal_get_descriptor_size(
	const ia_css_program_control_init_terminal_manifest_t *manifest,
	const ia_css_program_group_manifest_t *pg_manifest,
	ia_css_kernel_bitmap_t kernel_bitmap);

/*!
 * Initialize program control init terminal
 * @param nof_fragments[in]	Number of fragments
 * @param terminal[in]		program control init terminal
 * @param manifest[in]		program control init terminal manifest
 * @param pg_manifest[in]	program group manifest
 * @param kernel_bitmap[in]	kernel enable bitmap
 * @return < 0 on erro
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int
ia_css_program_control_init_terminal_init(
	ia_css_program_control_init_terminal_t *terminal,
	const ia_css_program_control_init_terminal_manifest_t *manifest,
	const ia_css_program_group_manifest_t *pg_manifest,
	ia_css_kernel_bitmap_t kernel_bitmap);

/*!
 * Get a program desc for a program control init terminal
  * @param terminal[in]		program control init terminal
 * @param manifest[in]		program control init terminal manifest
 * @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_program_control_init_program_desc_t *
ia_css_program_control_init_terminal_get_program_desc(
	const ia_css_program_control_init_terminal_t *prog_ctrl_init_terminal,
	const unsigned int program_index
);

/*!
 * Pretty prints the program control init termnial
 * @param terminal[in]		program control init terminal
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
void ia_css_program_control_init_terminal_print(
	const ia_css_program_control_init_terminal_t *terminal);

/*!
 * Returns the number of connect sections per program
 * @param program_desc[in] program control init terminal program desc
 * @return number of connect sections
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
unsigned int ia_css_program_control_init_terminal_get_connect_section_count(
	const ia_css_program_control_init_program_desc_t *program_desc);

/*!
 * Returns the number of connect sections per program
 * @param program_desc[in] program control init terminal program desc
 * @return number of connect sections
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
unsigned int ia_css_program_control_init_terminal_get_load_section_count(
	const ia_css_program_control_init_program_desc_t *program_desc);

/*!
 * Returns the memory offset of the first load section in the program
 * @param program_desc[in] program control init terminal program desc
 * @return memory offset
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
unsigned int ia_css_program_control_init_terminal_get_load_section_mem_offset(
	const ia_css_program_control_init_program_desc_t *program_desc);

/*!
 * Gets a load section desc for a program desc
 * of a program control init terminal
 * @param program_desc[in]		program control init terminal program desc
 * @param load_section_index[in]	section index
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_program_control_init_load_section_desc_t *
ia_css_program_control_init_terminal_get_load_section_desc(
	const ia_css_program_control_init_program_desc_t *program_desc,
	const unsigned int load_section_index
);

/*!
 * Gets process_id from program desc
 * of a program control init terminal
 * @param program_desc[in]		program control init terminal program desc
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_id_t ia_css_program_control_init_terminal_get_process_id(
	const ia_css_program_control_init_program_desc_t *program_desc);

/*!
 * Set control info of program desc
 * of a program control init terminal
 * @param program_desc[in]	program control init terminal program desc
 * @param process_id 		unique process id used to identify the process
 * among all active process
 * @param num_done_events 	number of events required to close the process
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
void ia_css_program_control_init_terminal_set_control_info(
	ia_css_program_control_init_program_desc_t *program_desc,
	ia_css_process_id_t process_id,
	uint8_t num_done_events);

/*!
 * Gets num_done_events value from program desc
 * of a program control init terminal
 * @param program_desc[in]		program control init terminal program desc
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_program_control_init_terminal_get_num_done_events(
	const ia_css_program_control_init_program_desc_t *program_desc);

/*!
 * Gets a connect section desc for a program desc
 * of a program control init terminal
 * @param program_desc[in]		program control init terminal program desc
 * @param connect_section_index[in]	section index
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_program_control_init_connect_section_desc_t *
ia_css_program_control_init_terminal_get_connect_section_desc(
	const ia_css_program_control_init_program_desc_t *program_desc,
	const unsigned int connect_section_index
);

/* ================= Program Control Init Terminal - END ================= */

#ifdef __IA_CSS_PSYS_DYNAMIC_INLINE__
#include "ia_css_psys_terminal_impl.h"
#endif /* __IA_CSS_PSYS_DYNAMIC_INLINE__ */

#endif /* __IA_CSS_PSYS_TERMINAL_H */
