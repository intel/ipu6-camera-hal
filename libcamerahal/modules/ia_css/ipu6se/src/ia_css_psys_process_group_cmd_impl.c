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

#include "ia_css_psys_process_group_cmd_impl.h"
#include "ia_css_psysapi.h"
#include "ia_css_psys_process.h"
#include "ia_css_psys_process.psys.h"
#include "ia_css_psys_process_group.h"
#include "ia_css_psys_process_group.psys.h"
#include "error_support.h"
#include "vied_nci_psys_system_global.h"
#include "misc_support.h"

#include "ia_css_psys_sim_trace.h"

/* Dummy implementation for sim */
int ia_css_process_group_on_create(
	ia_css_process_group_t					*process_group,
	const ia_css_program_group_manifest_t	*program_group_manifest,
	const ia_css_program_group_param_t		*program_group_param)
{
	NOT_USED(process_group);
	NOT_USED(program_group_manifest);
	NOT_USED(program_group_param);

	IA_CSS_TRACE_0(PSYSAPI_SIM, INFO, "ia_css_process_group_on_create(): enter: \n");

	return 0;
}

/* Dummy implementation for sim */
int ia_css_process_group_on_destroy(
	ia_css_process_group_t					*process_group)
{
	NOT_USED(process_group);

	IA_CSS_TRACE_0(PSYSAPI_SIM, INFO, "ia_css_process_group_on_destroy(): enter: \n");

	return 0;
}

int ia_css_process_group_exec_cmd(
	ia_css_process_group_t					*process_group,
	const ia_css_process_group_cmd_t		cmd)
{
	int	retval = -1;
	ia_css_process_group_state_t	state;

	IA_CSS_TRACE_0(PSYSAPI_SIM, VERBOSE, "ia_css_process_group_exec_cmd(): enter: \n");

	state = ia_css_process_group_get_state(process_group);

	verifexit(process_group != NULL);
	verifexit(state != IA_CSS_PROCESS_GROUP_ERROR);
	verifexit(state < IA_CSS_N_PROCESS_GROUP_STATES);

	switch (cmd) {
	case IA_CSS_PROCESS_GROUP_CMD_NOP:
		break;
	case IA_CSS_PROCESS_GROUP_CMD_SUBMIT:
		verifexit(state == IA_CSS_PROCESS_GROUP_READY);

/* External resource availability checks */
		verifexit(ia_css_can_process_group_submit(process_group));

		process_group->state = IA_CSS_PROCESS_GROUP_BLOCKED;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_ATTACH:
		verifexit(state == IA_CSS_PROCESS_GROUP_READY);
		break;
	case IA_CSS_PROCESS_GROUP_CMD_DETACH:
		verifexit(state == IA_CSS_PROCESS_GROUP_READY);
		break;
	case IA_CSS_PROCESS_GROUP_CMD_START:
		verifexit(state == IA_CSS_PROCESS_GROUP_BLOCKED);

/* External resource state checks */
		verifexit(ia_css_can_process_group_start(process_group));

		process_group->state = IA_CSS_PROCESS_GROUP_STARTED;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_DISOWN:

		/* empty call to match API ownership change between host and firmware */

		break;
	case IA_CSS_PROCESS_GROUP_CMD_RUN:
		verifexit(state == IA_CSS_PROCESS_GROUP_STARTED);
		process_group->state = IA_CSS_PROCESS_GROUP_RUNNING;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_STOP:
		verifexit(state == IA_CSS_PROCESS_GROUP_RUNNING);
		process_group->state = IA_CSS_PROCESS_GROUP_STOPPED;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_SUSPEND:
		verifexit(state == IA_CSS_PROCESS_GROUP_RUNNING);
		process_group->state = IA_CSS_PROCESS_GROUP_STARTED;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_RESUME:
		verifexit(state == IA_CSS_PROCESS_GROUP_STARTED);
		process_group->state = IA_CSS_PROCESS_GROUP_RUNNING;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_ABORT:
		verifexit(((state == IA_CSS_PROCESS_GROUP_RUNNING) || (state == IA_CSS_PROCESS_GROUP_STARTED)));
		process_group->state = IA_CSS_PROCESS_GROUP_STOPPED;
		break;
	case IA_CSS_PROCESS_GROUP_CMD_RESET:
/* We accept a reset command in the stopped state, mostly for simplifying the statemachine test */
		verifexit(((state == IA_CSS_PROCESS_GROUP_RUNNING) || (state == IA_CSS_PROCESS_GROUP_STARTED) || (state == IA_CSS_PROCESS_GROUP_STOPPED)));
		process_group->state = IA_CSS_PROCESS_GROUP_BLOCKED;
		break;
	case IA_CSS_N_PROCESS_GROUP_CMDS:	/* Fall through */
	default:
		verifexit(false);
		break;
	}

	retval = 0;
EXIT:
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_SIM, ERROR, "ia_css_process_group_exec_cmd failed (%i)\n", retval);
	}
	return retval;
}
