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

#ifndef __IA_CSS_PSYS_TRANSPORT_H
#define __IA_CSS_PSYS_TRANSPORT_H

#include <ia_css_psys_transport_dep.h>		/* ia_css_psys_cmd_queues */
#include <vied_nci_psys_system_global.h>	/* vied_vaddress_t */

#include <type_support.h>

typedef enum ia_css_psys_event_queues {
	/** The in-order queue for event returns */
	IA_CSS_PSYS_EVENT_QUEUE_MAIN_ID,
	IA_CSS_N_PSYS_EVENT_QUEUE_ID
} ia_css_psys_event_queue_ID_t;

typedef enum ia_css_psys_event_types {
	/** No error to report. */
	IA_CSS_PSYS_EVENT_TYPE_SUCCESS = 0,
	/** Unknown unhandled error */
	IA_CSS_PSYS_EVENT_TYPE_UNKNOWN_ERROR = 1,
	/* Retrieving remote object: */
	/** Object ID not found */
	IA_CSS_PSYS_EVENT_TYPE_RET_REM_OBJ_NOT_FOUND = 2,
	/** Objects too big, or size is zero. */
	IA_CSS_PSYS_EVENT_TYPE_RET_REM_OBJ_TOO_BIG = 3,
	/** Failed to load whole process group from tproxy/dma  */
	IA_CSS_PSYS_EVENT_TYPE_RET_REM_OBJ_DDR_TRANS_ERR = 4,
	/** The proper package could not be found */
	IA_CSS_PSYS_EVENT_TYPE_RET_REM_OBJ_NULL_PKG_DIR_ADDR = 5,
	/* Process group: */
	/** Failed to run, error while loading frame */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_LOAD_FRAME_ERR = 6,
	/** Failed to run, error while loading fragment */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_LOAD_FRAGMENT_ERR = 7,
	/** The process count of the process group is zero */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_PROCESS_COUNT_ZERO = 8,
	/** Process(es) initialization */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_PROCESS_INIT_ERR = 9,
	/** Aborted (after host request) */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_ABORT = 10,
	/** NULL pointer in the process group */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_NULL = 11,
	/** Process group validation failed */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_VALIDATION_ERR = 12,
	/** Error handling- invalid frame detected by CSI (ISYS) */
	IA_CSS_PSYS_EVENT_TYPE_PROC_GRP_INVALID_FRAME = 13
} ia_css_psys_event_type_t;

#define IA_CSS_PSYS_CMD_BITS	64
struct ia_css_psys_cmd_s {
	/** The command issued to the process group */
	uint16_t	command;
	/** Message field of the command */
	uint16_t	msg;
	/** The context reference (process group/buffer set/...) */
	uint32_t	context_handle;
};

#define IA_CSS_PSYS_EVENT_BITS	128
typedef struct ia_css_psys_event_s {
	/** The (return) status of the command issued to
	 * the process group this event refers to
	 */
	uint16_t	status;
	/** The command issued to the process group this event refers to */
	uint16_t	command;
	/** The context reference (process group/buffer set/...) */
	uint32_t	context_handle;
	/** This token (size) must match the token registered
	 * in a process group
	 */
	uint64_t	token;
} ia_css_psys_event_s_t;

struct ia_css_psys_buffer_s {
	/** The in-order queue for scheduled process groups */
	void		*host_buffer;
	vied_vaddress_t	*isp_buffer;
};

#endif /* __IA_CSS_PSYS_TRANSPORT_H */
