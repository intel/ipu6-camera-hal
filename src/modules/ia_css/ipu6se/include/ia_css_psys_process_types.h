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

#ifndef __IA_CSS_PSYS_PROCESS_TYPES_H
#define __IA_CSS_PSYS_PROCESS_TYPES_H

/*! \file */

/** @file ia_css_psys_process_types.h
 *
 * The types belonging to the terminal/process/process group dynamic module
 */

#include <type_support.h>
#include <vied_nci_psys_system_global.h>

#include <ia_css_psys_manifest_types.h>

#define IA_CSS_PROCESS_INVALID_PROGRAM_IDX  ((uint32_t)-1)

/* private */
typedef enum ia_css_process_group_cmd {
	IA_CSS_PROCESS_GROUP_CMD_NOP = 0,
	IA_CSS_PROCESS_GROUP_CMD_SUBMIT,
	IA_CSS_PROCESS_GROUP_CMD_ATTACH,
	IA_CSS_PROCESS_GROUP_CMD_DETACH,
	IA_CSS_PROCESS_GROUP_CMD_START,
	IA_CSS_PROCESS_GROUP_CMD_DISOWN,
	IA_CSS_PROCESS_GROUP_CMD_RUN,
	IA_CSS_PROCESS_GROUP_CMD_STOP,
	IA_CSS_PROCESS_GROUP_CMD_SUSPEND,
	IA_CSS_PROCESS_GROUP_CMD_RESUME,
	IA_CSS_PROCESS_GROUP_CMD_ABORT,
	IA_CSS_PROCESS_GROUP_CMD_RESET,
	IA_CSS_N_PROCESS_GROUP_CMDS
} ia_css_process_group_cmd_t;

/* private */
#define IA_CSS_PROCESS_GROUP_STATE_BITS	32
typedef enum ia_css_process_group_state {
	IA_CSS_PROCESS_GROUP_ERROR = 0,
	IA_CSS_PROCESS_GROUP_CREATED,
	IA_CSS_PROCESS_GROUP_READY,
	IA_CSS_PROCESS_GROUP_BLOCKED,
	IA_CSS_PROCESS_GROUP_STARTED,
	IA_CSS_PROCESS_GROUP_RUNNING,
	IA_CSS_PROCESS_GROUP_STALLED,
	IA_CSS_PROCESS_GROUP_STOPPED,
	IA_CSS_N_PROCESS_GROUP_STATES
} ia_css_process_group_state_t;

/* private */
typedef enum ia_css_process_cmd {
	IA_CSS_PROCESS_CMD_NOP = 0,
	IA_CSS_PROCESS_CMD_ACQUIRE,
	IA_CSS_PROCESS_CMD_RELEASE,
	IA_CSS_PROCESS_CMD_START,
	IA_CSS_PROCESS_CMD_LOAD,
	IA_CSS_PROCESS_CMD_STOP,
	IA_CSS_PROCESS_CMD_SUSPEND,
	IA_CSS_PROCESS_CMD_RESUME,
	IA_CSS_N_PROCESS_CMDS
} ia_css_process_cmd_t;

/* private */
#define IA_CSS_PROCESS_STATE_BITS	8
typedef enum ia_css_process_state {
	IA_CSS_PROCESS_ERROR = 0,
	IA_CSS_PROCESS_CREATED,
	IA_CSS_PROCESS_READY,
	IA_CSS_PROCESS_STARTED,
	IA_CSS_PROCESS_RUNNING,
	IA_CSS_PROCESS_STOPPED,
	IA_CSS_PROCESS_SUSPENDED,
	IA_CSS_N_PROCESS_STATES
} ia_css_process_state_t;

/* public */
typedef struct ia_css_process_group_s	ia_css_process_group_t;
typedef struct ia_css_process_s		ia_css_process_t;
typedef struct ia_css_process_ext_s	ia_css_process_ext_t;

typedef struct ia_css_data_terminal_s	ia_css_data_terminal_t;

#endif /* __IA_CSS_PSYS_PROCESS_TYPES_H */
