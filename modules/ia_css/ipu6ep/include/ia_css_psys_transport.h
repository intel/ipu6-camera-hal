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
