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

#ifndef __IA_CSS_PSYS_PROCESS_GROUP_H
#define __IA_CSS_PSYS_PROCESS_GROUP_H

/*! \file */

/** @file ia_css_psys_process_group.h
 *
 * Define the methods on the process object that are not part of
 * a single interface
 */
#include "ia_css_rbm.h"

#include <ia_css_psys_process_types.h>
#include <ia_css_psys_dynamic_storage_class.h>

#include <type_support.h>					/* uint8_t */

/*
 * Creation
 */
#include <ia_css_psys_process_group.hsys.user.h>

/*
 * Registration of user contexts / callback info
 * External resources
 * Sequencing resources
 */
#include <ia_css_psys_process_group.hsys.kernel.h>

/*
 * Dispatcher
 */
#include <ia_css_psys_process_group.psys.h>

/*
 * Access to sub-structure handles / fields
 */

#include "ia_css_terminal.h"

#include "ia_css_terminal_manifest_base_types.h"
#include "ia_css_terminal_manifest_types.h"

/*! Get the number of fragments on the process group

 @param	process_group[in]		process group object

 Note: Future change is to have a fragment count per
 independent subgraph

 @return the fragment count, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint16_t ia_css_process_group_get_fragment_count(
	const ia_css_process_group_t		*process_group);

/*! Get the fragment state on the process group

 @param	 process_group[in]		process group object
 @param	 fragment_state[in]		current fragment of processing

 @return -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_get_fragment_state(
	const ia_css_process_group_t		*process_group,
	uint16_t				*fragment_state);

/*! Set the fragment state on the process group

 @param	process_group[in]		process group object
 @param	fragment_state[in]		current fragment of processing

 @return -1 on error
  */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_fragment_state(
	ia_css_process_group_t			*process_group,
	uint16_t				fragment_state);

/*! Get the number of processes on the process group

 @param	process_group[in]		process group object

 @return the process count, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_process_count(
	const ia_css_process_group_t		*process_group);

/*! Get the number of terminals on the process group

 @param	process_group[in]		process group object

 Note: Future change is to have a terminal count per
 independent subgraph

 @return the terminal count, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_terminal_count(
	const ia_css_process_group_t		*process_group);

/*! Get the PSYS server init time in cycles

 @param	process_group[in]		process group object

 @return PSYS server init time, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_psys_server_init_cycles(
	const ia_css_process_group_t			*process_group);

/*! \deprecated this is old and will get removed */
/*! Get the PG load start timestamp

 @param	process_group[in]		process group object

 @return PG load start timestamp, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_load_start_ts(
	const ia_css_process_group_t			*process_group);

/*! Get the PG load time in cycles

 @param	process_group[in]		process group object

 @return PG load time in cycles, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_load_cycles(
	const ia_css_process_group_t			*process_group);

/*! Get the PG init time in cycles

 @param	process_group[in]		process group object

 @return PG init time in cycles, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_init_cycles(
	const ia_css_process_group_t			*process_group);

/*! Get the PG processing time in cycles

 @param	process_group[in]		process group object

 @return PG processing time in cycles, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_processing_cycles(
	const ia_css_process_group_t			*process_group);

/*! Get the PG next frame init time in cycles

 @param	process_group[in]		process group object

 @return PG processing time in cycles, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_next_frame_init_cycles(
	const ia_css_process_group_t			*process_group);

/*! Get the PG complete time in cycles

 @param	process_group[in]		process group object

 @return PG processing time in cycles, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint32_t ia_css_process_group_get_pg_complete_cycles(
	const ia_css_process_group_t			*process_group);

/*! Get the (pointer to) the <terminal type> terminal of the process group object

 @param	process_group[in]               process group object
 @param	terminal_type[in]               terminal type of terminal

 @return the pointer to the terminal, NULL on error
 */

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_terminal_t *ia_css_process_group_get_terminal_from_type(
		const ia_css_process_group_t *process_group,
		const ia_css_terminal_type_t terminal_type);

/*! Get the (pointer to) the <terminal type> terminal of the process group object
 * for terminals which have only a single instance
 * (cached in, cached out, program, program_ctrl_init)

 @param	process_group[in]               process group object
 @param	terminal_type[in]               terminal type of terminal

 @return the pointer to the terminal, NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
const ia_css_terminal_t *ia_css_process_group_get_single_instance_terminal(
	const ia_css_process_group_t 	*process_group,
	ia_css_terminal_type_t		term_type);

/*! Get the (pointer to) the manifest indexed terminal of the process group object

 @param	process_group[in]		process group object
 @param	manifest_idx[in]		index of the terminal in manifest

 @return the pointer to the terminal, NULL on error
 */

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_terminal_t *ia_css_process_group_get_manifest_idx_terminal(
	const ia_css_process_group_t *process_grp,
	const unsigned int manifest_idx);

/*! Get the (pointer to) the indexed terminal of the process group object

 @param	process_group[in]		process group object
 @param	terminal_index[in]		index of the terminal

 @return the pointer to the terminal, NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_terminal_t *ia_css_process_group_get_terminal(
	const ia_css_process_group_t		*process_group,
	const unsigned int			terminal_index);

/*! Get the (pointer to) the indexed process of the process group object

 @param	process_group[in]		process group object
 @param	process_index[in]		index of the process

 @return the pointer to the process, NULL on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_process_t *ia_css_process_group_get_process(
	const ia_css_process_group_t		*process_group,
	const unsigned int			process_index);

/*! Get the stored size of the process group object

 @param	process_group[in]				process group object

 @return size, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
size_t ia_css_process_group_get_size(
	const ia_css_process_group_t		*process_group);

/*! Get the state of the the process group object

 @param	process_group[in]		process group object

 @return state, limit value on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_process_group_state_t ia_css_process_group_get_state(
	const ia_css_process_group_t		*process_group);

/*! Get the unique ID of program group used by the process group object

 @param	process_group[in]		process group object

 @return ID, 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_program_group_ID_t ia_css_process_group_get_program_group_ID(
	const ia_css_process_group_t		*process_group);

/*! Get the resource bitmap of the process group

 @param	process_group[in]		process group object

 @return the reource bitmap
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
vied_nci_resource_bitmap_t ia_css_process_group_get_resource_bitmap(
	const ia_css_process_group_t		*process_group);

/*! Set the resource bitmap of the process group

 @param	process_group[in]		process group object
 @param	resource_bitmap[in]		the resource bitmap

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_resource_bitmap(
	ia_css_process_group_t			*process_group,
	const vied_nci_resource_bitmap_t	resource_bitmap);

/*! Get the kernel bitmap of the the process group object

 @param	process_group[in] process group object

 @return process group kernel bitmap
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_process_group_get_kernel_bitmap(
	const ia_css_process_group_t *process_group);

/*! Get the routing bitmap of the process group

 @param	process_group[in]   process group object

 @return routing bitmap (pointer)
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
const ia_css_rbm_t *ia_css_process_group_get_routing_bitmap(
	const ia_css_process_group_t *process_group);

/*! Set the routing bitmap of the process group

 @param	process_group[in]   process group object
 @param	rbm[in]		        routing bitmap

 @return < 0 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_routing_bitmap(
	ia_css_process_group_t *process_group,
	const ia_css_rbm_t rbm);

/*! Get IPU virtual address of process group

 @param	 process_group[in]		process group object
 @param	 ipu_vaddress[in/out]	process group ipu virtual address

 @return -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_get_ipu_vaddress(
	const ia_css_process_group_t		*process_group,
	vied_vaddress_t			*ipu_vaddress);

/*! Set IPU virtual address of process group

 @param	process_group[in]		process group object
 @param	ipu_vaddress[in]		process group ipu address

 @return -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_ipu_vaddress(
	ia_css_process_group_t			*process_group,
	vied_vaddress_t			ipu_vaddress);

/*! Get protocol version used by a process group

 @param	process_group[in]		process group object

 @return invalid protocol version on error

 @todo Add new protocol version for IPU7.  Defer this for now,
 because it will create a LOT of disruption.
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_protocol_version(
	const ia_css_process_group_t *process_group);

/*! Get base queue id used by a process group

 @param	process_group[in]		process group object

 @return -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_base_queue_id(
	ia_css_process_group_t *process_group);

/*! Set base queue id used by a process group

 @param	process_group[in]		process group object
 @param	queue_id[in]			process group queue id

 @return invalid queue id on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_base_queue_id(
	ia_css_process_group_t *process_group,
	uint8_t queue_id);

/*! Get number of queues used by a process group

 @param	process_group[in]		process group object

 @return invalid number of queues (0) on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_num_queues(
	ia_css_process_group_t *process_group);

/*! Set number of queues used by a process group

 @param	process_group[in]		process group object
 @param	num_queues[in]			process group number of queues

 @return -1 on error
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_num_queues(
	ia_css_process_group_t *process_group,
	uint8_t num_queues);

/*! Set error handling enable flag used by process group

 @param	process_group[in]		process group object
 @param	error_handling_enable[in]	flag to control error handling

 @return -1 on error, 0 on success
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_process_group_set_error_handling_enable(
	ia_css_process_group_t *process_group,
	const bool error_handling_enable);

/*! Get the error handling enable flag associated to the process group

 @param	process_group[in]		process group object

 @return error_handlnig_enable flag of the process group. On error UINT8_MAX
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
uint8_t ia_css_process_group_get_error_handling_enable(
	const ia_css_process_group_t *process_group);

/** Return true if the process group requires vector processor (i.e. DSP)
 *  resources  */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_process_group_has_vp(const ia_css_process_group_t *process_group);

/*! Check if terminal is enabled based on kernel enable bitmap

 @param	terminal_manifest[in]		terminal manifest object
 @param	enable_bitmap[in]		kernel enable bitmap

 @return true if enabled, false if not enabled
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_process_group_is_terminal_enabled(
	const ia_css_terminal_manifest_t *terminal_manifest,
	ia_css_kernel_bitmap_t enable_bitmap);

/*! Check if data terminal is enabled based on kernel enable bitmap

 @param	data_term_manifest[in]		data terminal manifest object
 @param	enable_bitmap[in]		kernel enable bitmap

 @return true if enabled, false if not enabled
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_process_group_is_data_terminal_enabled(
	const ia_css_data_terminal_manifest_t	*data_term_manifest,
	ia_css_kernel_bitmap_t enable_bitmap);

/*! Check if spatial terminal is enabled based on kernel enable bitmap

 @param	spatial_term_man[in]	spatial	terminal manifest object
 @param	enable_bitmap[in]		kernel enable bitmap

 @return true if enabled, false if not enabled
 */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
bool ia_css_process_group_is_spatial_terminal_enabled(
	const ia_css_spatial_param_terminal_manifest_t *spatial_term_man,
	ia_css_kernel_bitmap_t enable_bitmap);

#ifdef __IA_CSS_PSYS_DYNAMIC_INLINE__
#include "ia_css_psys_process_group_impl.h"
#endif /* __IA_CSS_PSYS_DYNAMIC_INLINE__ */

#endif /* __IA_CSS_PSYS_PROCESS_GROUP_H */
