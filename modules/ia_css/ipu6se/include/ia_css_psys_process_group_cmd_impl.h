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

#ifndef __IA_CSS_PSYS_PROCESS_GROUP_CMD_IMPL_H
#define __IA_CSS_PSYS_PROCESS_GROUP_CMD_IMPL_H

#include "type_support.h"
#include "ia_css_psys_process_group.h"
#include "ia_css_rbm_manifest_types.h"

#define N_UINT64_IN_PROCESS_GROUP_STRUCT	2
#define N_UINT32_IN_PROCESS_GROUP_STRUCT	8
#define N_UINT16_IN_PROCESS_GROUP_STRUCT	5
#define N_UINT8_IN_PROCESS_GROUP_STRUCT		8
#define N_PADDING_UINT8_IN_PROCESS_GROUP_STRUCT	2

#define SIZE_OF_PROCESS_GROUP_STRUCT_BITS \
	(IA_CSS_RBM_BITS \
	+ IA_CSS_KERNEL_BITMAP_BITS \
	+ N_UINT64_IN_PROCESS_GROUP_STRUCT * IA_CSS_UINT64_T_BITS \
	+ N_UINT32_IN_PROCESS_GROUP_STRUCT * IA_CSS_UINT32_T_BITS \
	+ IA_CSS_PROGRAM_GROUP_ID_BITS \
	+ IA_CSS_PROCESS_GROUP_STATE_BITS \
	+ VIED_VADDRESS_BITS \
	+ VIED_NCI_RESOURCE_BITMAP_BITS \
	+ N_UINT16_IN_PROCESS_GROUP_STRUCT * IA_CSS_UINT16_T_BITS \
	+ N_UINT8_IN_PROCESS_GROUP_STRUCT * IA_CSS_UINT8_T_BITS \
	+ N_PADDING_UINT8_IN_PROCESS_GROUP_STRUCT * IA_CSS_UINT8_T_BITS)

/** Process group object describing an instance of a Program Group
 *
 *  This is the root structure passed in a number of PG commands via
 *  the syscom queuing interface to firmware from the host.  A pointer
 *  to this structure in main memroy is passed in the following
 *  PG commands: start, resume, suspend, stop, abort.
 *
 *  A process group is created based on a program group specification
 *  described in the program group manifest.  Briefly, a program group
 *  contains programs which in turn contain kernels.  A program group
 *  also has terminals, which connect it to the outside world.  Each
 *  terminal represents either parameters or image data flowing in or
 *  out of the group.  Terminals come in different types and may have
 *  addtional attributes.  For example, a data termainal will have
 *  a frame format attribute.
 *  @see ia_css_process_group_create()
 *  @see ia_css_program_group_manifest_t and related API and:
 *  @see ia_css_pkg_dir_entry_get_type() and
 *  ia_css_client_pkg_get_pg_manifest_offset_size() (and in general the
 *  ia_css_pkg_dir* functions) for how to get the program group manifest
 *  array from the binary release package.
 *
 *  @note Some of the fields within are used internally within the
 *  firmware only, after the firmware copies the process group structure
 *  to its internel cache.  The original in main memory is not updated.
 *  All such fields should be initialized to zero, unless otherwise
 *  noted.
 *  See the individual field descriptions for which fields are "internal".
 */
struct ia_css_process_group_s {
	/** User (callback) token / user context reference,
	 * zero is an error value
	 */
	uint64_t token;
	/** private token / context reference, zero is an error value */
	uint64_t private_token;
	/** PG routing bitmap used to set connection between program group
	 *  components */
	ia_css_rbm_t routing_bitmap;
	/** PG kernel bitmap.  Marks which program kernels (i.e. kernel==device, when impelementation is fixed)
	 *  are enabled. This bitmap covers all kernels in the group, no matter which program they belong to. */
	ia_css_kernel_bitmap_t kernel_bitmap;
	/** Size of this structure in bytes */
	uint32_t size;
	/** The timestamp when PG load starts
	 *  @note Internal firmware use only. */
	/**< PSYS server init time in cycless */
	uint32_t psys_server_init_cycles;
	/**< The timestamp when PG load starts */
	uint32_t pg_load_start_ts;
	/** PG load time in cycles
	 *  @note Internal firmware use only. */
	uint32_t pg_load_cycles;
	/** PG init time in cycles
	 *  @note Internal firmware use only. */
	uint32_t pg_init_cycles;
	/** PG processing time in cycles
	 *  @note Internal firmware use only. */
	uint32_t pg_processing_cycles;
	/** Referral ID to program group FW.  This identifies the "type" of the process group
	 *  and associcates the process group with the program group defined in the manifest. */
	/**< PG next frame init time in cycles */
	uint32_t pg_next_frame_init_cycles;
	/**< PG complete time in cycles */
	uint32_t pg_complete_cycles;
	/**< Referral ID to program group FW */
	ia_css_program_group_ID_t ID;
	/** State of the process group FSM */
	ia_css_process_group_state_t state;
	/** Virtual address of process group in the IPU address space */
	vied_vaddress_t ipu_virtual_address;
	/** Bitmap of the compute resources used by the process group  */
	vied_nci_resource_bitmap_t resource_bitmap;
	/** Number of fragments offered on each terminal */
	uint16_t fragment_count;
	/** Current fragment of processing */
	uint16_t fragment_state;
	/** Watermark to control fragment processing.  Set to the same value as "fragment_count"
	 *  unless you know what you are doing.
	 *  @see  fragment_count in this structure. */
	uint16_t fragment_limit;
	/** Offset in bytes relative to the beginning of this structure, to process_offset_table[process_count].
	 *  Must be aligned to a 64 bit boundary.
	 *  process_offset_table is an array of size "process_count", containing offsets to process objects
	 *  (each an instance of ia_css_process_t) belonging to this process group.
	 *  @see process_count in this structure.
	 *
	 *  Usually follows this process group structure. */
	uint16_t processes_offset;
	/** Offset in bytes, relative to the beginning of this structure, to terminal_offset_table[terminal_count].
	 *  Must be aligned to a 64 bit boundary.
	 *  terminal_offset_table is an array of size "terminal_count", containing offsets to terminal objects
	 *  (each an instance of ia_css_terminal_t or a derivative of it) belonging to this process group.
	 *  @see terminal_count in this structure.
	 *
	 *  Usually follows process_offset_table, aligned to 64-bit boundary. */
	uint16_t terminals_offset;
	/** Parameter dependent number of processes in this process group, determined at process
	 *  group creation taking into account the enabled kernels. That is, if all of a program's
	 *  associcated kernels are disabled, then no process is instantiated for that program and
	 *  it must not be counted in the process count.
	 *
	 *  @see processes_offset in this structure
	 *  @see ia_css_program_group_manifest_get_program_count()
	 *  @see ia_css_program_manifest_get_kernel_bitmap()
	 *  @see ia_css_process_group_compute_process_count() */
	uint8_t process_count;
	/** Parameter dependent number of terminals on this process group, determined at process
	 *  group creation.  Must not be greater than the terminal count as defined in the manifest,
	 *  but taking into account the enabled kernels.  That is, for those terminal types that have
	 *  kernel associations, the terminal is diabled if all of its associated kernels are disabled.
	 *  Execptions by terminal type:
	 *  - IA_CSS_TERMINAL_TYPE_PROGRAM - As described above, but parameter descriptors are examined to determine the
	 *  kernel association, as the terminal type itself does not have an associated kernel field or kernel bitmap field.
	 *  - IA_CSS_TERMINAL_TYPE_PROGRAM_CONTROL_INIT -- Always enabled
	 *  - IA_CSS_TERMINAL_TYPE_PARAM_CACHED_IN -- Always enabled, to allow HW disabling configuration to be loaded
	 *
	 *  @see terminals_offset in this structure
	 *  @see ia_css_program_group_manifest_get_terminal_count()
	 *  @see ia_css_program_manifest_get_kernel_bitmap()
	 *  @see ia_css_process_group_is_terminal_enabled()
	 *  @see ia_css_process_group_compute_terminal_count() */
	uint8_t terminal_count;
	/** Parameter dependent number of independent subgraphs in
	 * this process group.
	 * @note Apparently no longer used.
	 */
	uint8_t subgraph_count;
	/** Process group protocol version.  Identifies the state machine and
	 *  message protocol for this process group. */
	uint8_t protocol_version;
	/** Dedicated base queue id used for enqueueing payload buffer sets.
	 *  Each buffer set is either a frame (or fragment) processing request,
	 *  or an opportunistic parameter update (AKA "late binding")
	 *  This queue is enabled once the process group start command has
	 *  been issued. */
	uint8_t base_queue_id;
	/** Number of dedicated queues used */
	uint8_t num_queues;
	/**< Mask the send_pg_done IRQ */
	uint8_t mask_irq;
	/** Error handling enable\disable flag */
	uint8_t error_handling_enable;
#if N_PADDING_UINT8_IN_PROCESS_GROUP_STRUCT > 0
	uint8_t padding[N_PADDING_UINT8_IN_PROCESS_GROUP_STRUCT];
#endif
};

/*! Callback after process group is created. Implementations can provide
 * suitable actions needed when process group is created.

 @param	process_group[in]			process group object
 @param	program_group_manifest[in]		program group manifest
 @param	program_group_param[in]			program group parameters

 @return 0 on success and non-zero on failure
 */
extern int ia_css_process_group_on_create(
	ia_css_process_group_t			*process_group,
	const ia_css_program_group_manifest_t	*program_group_manifest,
	const ia_css_program_group_param_t	*program_group_param);

/*! Callback before process group is about to be destoyed. Any implementation
 * specific cleanups can be done here.

 @param	process_group[in]				process group object

 @return 0 on success and non-zero on failure
 */
extern int ia_css_process_group_on_destroy(
	ia_css_process_group_t					*process_group);

/*
 * Command processor
 */

/*! Execute a command locally or send it to be processed remotely

 @param	process_group[in]		process group object
 @param	cmd[in]					command

 @return < 0 on error
 */
extern int ia_css_process_group_exec_cmd(
	ia_css_process_group_t				*process_group,
	const ia_css_process_group_cmd_t		cmd);

/*! Enqueue a buffer set corresponding to a persistent program group by
 * sending a command to subsystem.

 @param	process_group[in]		process group object
 @param	buffer_set[in]			buffer set
 @param	queue_offset[in]		offset to be used from the queue id
					specified in the process group object
					(0 for first buffer set for frame, 1
					for late binding)

 @return < 0 on error
 */
extern int ia_css_enqueue_buffer_set(
	ia_css_process_group_t				*process_group,
	ia_css_buffer_set_t				*buffer_set,
	unsigned int					queue_offset);

/*! Enqueue a parameter buffer set corresponding to a persistent program
 *  group by sending a command to subsystem.  This allows for
    opportunistic late parameters update, AKA "late binding".

 @param	process_group[in]		process group object
 @param	buffer_set[in]			parameter buffer set

 @return < 0 on error
 */
extern int ia_css_enqueue_param_buffer_set(
	ia_css_process_group_t				*process_group,
	ia_css_buffer_set_t				*buffer_set);

/*! Need to store the 'secure' mode for each PG for FW test app only
 *
 * @param	process_group[in]		process group object
 * @param	secure[in]			parameter buffer set
 *
 * @return < 0 on error
 */
extern int ia_css_process_group_store(
	ia_css_process_group_t				*process_group,
	bool						secure);

/*! Need to get the right context for each PG for FW test app only
 *
 * @param	process_group[in]		process group object
 *
 * @return	psys_syscom context
 */
extern struct ia_css_syscom_context *ia_css_process_group_get_context(
	ia_css_process_group_t *process_group);

#endif /* __IA_CSS_PSYS_PROCESS_GROUP_CMD_IMPL_H */
