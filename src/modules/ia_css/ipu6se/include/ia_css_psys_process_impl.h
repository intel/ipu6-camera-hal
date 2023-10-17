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

#ifndef __IA_CSS_PSYS_PROCESS_IMPL_H
#define __IA_CSS_PSYS_PROCESS_IMPL_H

#include <ia_css_psys_process.h>

#include <ia_css_psys_process_group.h>
#include <ia_css_psys_program_manifest.h>

#include <error_support.h>
#include <misc_support.h>
#include <assert_support.h>

#include <vied_nci_psys_system_global.h>

#include "ia_css_psys_dynamic_trace.h"
#include "ia_css_psys_process_private_types.h"

/** Function only to be used in FW */
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_ext_t *ia_css_process_get_extension(const ia_css_process_t *process)
{
	DECLARE_ERRVAL
	verifexitval(process != NULL, EFAULT);
	if (process->process_extension_offset != 0) {
		return (ia_css_process_ext_t *)((uint8_t *)process + process->process_extension_offset);
	}
EXIT:
	if (!noerror()) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_get_extension invalid argument\n");
	}
	return NULL;
}

/** Function only to be used in ia_css_psys_process_impl.h and ia_css_psys_process.h */
STORAGE_CLASS_INLINE vied_nci_cell_ID_t ia_css_process_cells_get_cell(const ia_css_process_t *process, int index)
{
	assert(index < IA_CSS_PROCESS_MAX_CELLS);
	return process->cells[index];
}

/** Function only to be used in ia_css_psys_process_impl.h and ia_css_psys_process.h */
STORAGE_CLASS_INLINE void ia_css_process_cells_set_cell(ia_css_process_t *process, int index, vied_nci_cell_ID_t cell_id)
{
	assert(index < IA_CSS_PROCESS_MAX_CELLS);
	process->cells[index] = cell_id;
}

/** Function only to be used in ia_css_psys_process_impl.h and ia_css_psys_process */
STORAGE_CLASS_INLINE void ia_css_process_cells_clear(ia_css_process_t *process)
{
	int i;
	for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
		ia_css_process_cells_set_cell(process, i, VIED_NCI_N_CELL_ID);
	}
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_cell_ID_t ia_css_process_get_cell(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	vied_nci_cell_ID_t	cell_id = VIED_NCI_N_CELL_ID;
	int i = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_cell(): enter:\n");

	verifexitval(process != NULL, EFAULT);

	for (i = 1; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
		assert(VIED_NCI_N_CELL_ID == ia_css_process_cells_get_cell(process, i));
#ifdef __HIVECC
#pragma hivecc unroll
#endif
	}
	cell_id = ia_css_process_cells_get_cell(process, 0);

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_cell invalid argument\n");
	}
	return cell_id;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_mem_ID_t ia_css_process_get_ext_mem_id(
	const ia_css_process_t		*process,
	const vied_nci_mem_type_ID_t	mem_type)
{
	DECLARE_ERRVAL
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_ext_mem(): enter:\n");

	verifexitval(process != NULL && mem_type < VIED_NCI_N_DATA_MEM_TYPE_ID, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	verifexitval(process_ext != NULL, EFAULT);

EXIT:
	if (!noerror()) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_ext_mem invalid argument\n");
		return IA_CSS_PROCESS_INVALID_OFFSET;
	}
	return process_ext->ext_mem_id[mem_type];
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint32_t ia_css_process_get_program_idx(
	const ia_css_process_t		*process)
{
	DECLARE_ERRVAL
	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_program_idx(): enter:\n");

	verifexitval(process != NULL, EFAULT);

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_program_idx invalid argument\n");
		return IA_CSS_PROCESS_INVALID_PROGRAM_IDX;
	}
	return process->program_idx;
}

#if VIED_NCI_N_DEV_CHN_ID > 0
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_process_get_dev_chn(
	const ia_css_process_t		*process,
	const vied_nci_dev_chn_ID_t	dev_chn_id)
{
	DECLARE_ERRVAL
	ia_css_process_ext_t *process_ext = NULL;

	COMPILATION_ERROR_IF(VIED_NCI_N_DATA_MEM_TYPE_ID != VIED_NCI_N_MEM_TYPE_ID - 1); /* check for vied_nci_psys_resource_model.h correctness */

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_dev_chn(): enter:\n");
	verifexitval(process != NULL && dev_chn_id < VIED_NCI_N_DEV_CHN_ID, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	verifexitval(process_ext != NULL, EFAULT);

EXIT:
	if (!noerror()) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_dev_chn(): invalid arguments\n");
		return IA_CSS_PROCESS_INVALID_OFFSET;
	}
	return process_ext->dev_chn_offset[dev_chn_id];
}
#endif /* VIED_NCI_N_DEV_CHN_ID */

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_process_get_int_mem_offset(
	const ia_css_process_t				*process,
	const vied_nci_mem_type_ID_t			mem_id)
{
	(void)process;
	(void)mem_id;
	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
		"ia_css_process_get_int_mem_offset always returns 0"
		"(internally memory feature not used).\n");
	return 0;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_size_t ia_css_process_get_ext_mem_offset(
	const ia_css_process_t				*process,
	const vied_nci_mem_type_ID_t			mem_type_id)
{
	DECLARE_ERRVAL
	vied_nci_resource_size_t ext_mem_offset = IA_CSS_PROCESS_INVALID_OFFSET;
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_ext_mem_offset(): enter:\n");

	verifexitval(process != NULL && mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	verifexitval(process_ext != NULL, EFAULT);

EXIT:
	if (noerror()) {
		ext_mem_offset = process_ext->ext_mem_offset[mem_type_id];
	} else {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_ext_mem_offset invalid argument\n");
	}

	return ext_mem_offset;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
size_t ia_css_process_get_size(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	size_t	size = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_size(): enter:\n");

	verifexitval(process != NULL, EFAULT);

EXIT:
	if (noerror()) {
		size = process->size;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			       "ia_css_process_get_size invalid argument\n");
	}

	return size;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_state_t ia_css_process_get_state(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	ia_css_process_state_t	state = IA_CSS_N_PROCESS_STATES;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_state(): enter:\n");

	verifexitval(process != NULL, EFAULT);

EXIT:
	if (noerror()) {
		state = (ia_css_process_state_t)process->state;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			       "ia_css_process_get_state invalid argument\n");
	}

	return state;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_process_set_state(
	ia_css_process_t					*process,
	ia_css_process_state_t				state)
{
	DECLARE_ERRVAL
	int retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_set_state(): enter:\n");

	verifexitval(process != NULL, EFAULT);

	assert(state <= UINT8_MAX);
	process->state = state;
	retval = 0;
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_set_state invalid argument\n");
	}

	return retval;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_process_get_cell_dependency_count(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	uint8_t	cell_dependency_count = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_cell_dependency_count(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	cell_dependency_count = process->cell_dependency_count;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		"ia_css_process_get_cell_dependency_count invalid argument\n");
	}
	return cell_dependency_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_process_get_terminal_dependency_count(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	uint8_t	terminal_dependency_count = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_terminal_dependency_count(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	terminal_dependency_count = process->terminal_dependency_count;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_terminal_dependency_count invalid argument process\n");
	}
	return terminal_dependency_count;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_process_group_t *ia_css_process_get_parent(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	ia_css_process_group_t	*parent = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_parent(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	if (process->parent_offset == 0) {
		return NULL;
	};
	parent =
	(ia_css_process_group_t *) ((char *)process + process->parent_offset);

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_get_parent invalid argument process\n");
	}
	return parent;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_program_ID_t ia_css_process_get_program_ID(
	const ia_css_process_t					*process)
{
	DECLARE_ERRVAL
	ia_css_program_ID_t		id = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_program_ID(): enter:\n");

	verifexitval(process != NULL, EFAULT);

	id = process->ID;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		    "ia_css_process_get_program_ID invalid argument process\n");
	}
	return id;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_id_t ia_css_process_get_cell_dependency(
	const ia_css_process_t *process,
	const unsigned int cell_num)
{
	DECLARE_ERRVAL
	vied_nci_resource_id_t cell_dependency =
		IA_CSS_PROCESS_INVALID_DEPENDENCY;
	vied_nci_resource_id_t *cell_dep_ptr = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_process_get_cell_dependency(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	verifexitval(cell_num < process->cell_dependency_count, EFAULT);

	cell_dep_ptr =
		(vied_nci_resource_id_t *)
		((char *)process + process->cell_dependencies_offset);
	cell_dependency = *(cell_dep_ptr + cell_num);
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		       "ia_css_process_get_cell_dependency invalid argument\n");
	}
	return cell_dependency;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint8_t ia_css_process_get_terminal_dependency(
	const ia_css_process_t					*process,
	const unsigned int					terminal_num)
{
	DECLARE_ERRVAL
	uint8_t *ter_dep_ptr = NULL;
	uint8_t ter_dep = IA_CSS_PROCESS_INVALID_DEPENDENCY;

	verifexitval(process != NULL, EFAULT);
	verifexitval(terminal_num < process->terminal_dependency_count, EFAULT);

	ter_dep_ptr = (uint8_t *) ((char *)process +
				   process->terminal_dependencies_offset);

	ter_dep = *(ter_dep_ptr + terminal_num);

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		       "ia_css_process_get_terminal_dependency invalid argument\n");
	}
	return ter_dep;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t ia_css_process_get_cells_bitmap(
	const ia_css_process_t *process)
{
	DECLARE_ERRVAL
	vied_nci_resource_bitmap_t bitmap = 0;
	vied_nci_cell_ID_t cell_id;
	int i = 0;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_cell_bitmap(): enter:\n");

	verifexitval(process != NULL, EFAULT);

	for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
		cell_id = ia_css_process_cells_get_cell(process, i);
		if (VIED_NCI_N_CELL_ID != cell_id) {
			bitmap |= (1 << cell_id);
		}
#ifdef __HIVECC
#pragma hivecc unroll
#endif
	}

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		 "ia_css_process_get_cells_bitmap invalid argument process\n");
	}

	return bitmap;
}

#if HAS_DFM
IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t* ia_css_process_get_dfm_port_bitmap_ptr(
	ia_css_process_t *process)
{
	DECLARE_ERRVAL
	vied_nci_resource_bitmap_t *p_bitmap = NULL;
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_dfm_port_bitmap(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	if (process_ext == NULL) {
		return NULL;
	}

	p_bitmap = &process_ext->dfm_port_bitmap[0];
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		 "ia_css_process_get_dfm_port_bitmap invalid argument process\n");
	}

	return p_bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t* ia_css_process_get_dfm_active_port_bitmap_ptr(
	ia_css_process_t *process)
{
	DECLARE_ERRVAL
	vied_nci_resource_bitmap_t *p_bitmap = NULL;
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_dfm_port_bitmap(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	if (process_ext == NULL) {
		return NULL;
	}

	p_bitmap = &process_ext->dfm_active_port_bitmap[0];
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		 "ia_css_process_get_dfm_port_bitmap invalid argument process\n");
	}

	return p_bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t ia_css_process_get_dfm_port_bitmap(
	const ia_css_process_t *process,
	vied_nci_dev_dfm_id_t  dfm_res_id)
{
	DECLARE_ERRVAL
	vied_nci_resource_bitmap_t bitmap = 0;
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_dfm_port_bitmap(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	if (process_ext == NULL) {
		return 0;
	}
	verifexitval(dfm_res_id < VIED_NCI_N_DEV_DFM_ID, EFAULT);
	bitmap = process_ext->dfm_port_bitmap[dfm_res_id];
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		 "ia_css_process_get_dfm_port_bitmap invalid argument process\n");
	}

	return bitmap;
}
#endif /* HAS_DFM */

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_nci_resource_bitmap_t ia_css_process_get_dfm_active_port_bitmap(
	const ia_css_process_t *process,
	vied_nci_dev_dfm_id_t  dfm_res_id)
{
	DECLARE_ERRVAL
	vied_nci_resource_bitmap_t bitmap = 0;
	ia_css_process_ext_t *process_ext = NULL;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		       "ia_css_process_get_dfm_active_port_bitmap(): enter:\n");

	verifexitval(process != NULL, EFAULT);
	process_ext = ia_css_process_get_extension(process);
	if (process_ext == NULL) {
		return 0;
	}
#if HAS_DFM
	verifexitval(dfm_res_id < VIED_NCI_N_DEV_DFM_ID, EFAULT);
	bitmap = process_ext->dfm_active_port_bitmap[dfm_res_id];
#else
	NOT_USED(dfm_res_id);
#endif
EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
		 "ia_css_process_get_dfm_active_port_bitmap invalid argument process\n");
	}
	return bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
bool ia_css_is_process_valid(
	const ia_css_process_t		*process,
	const ia_css_program_manifest_t	*p_manifest)
{
	DECLARE_ERRVAL
	bool invalid_flag = false;
	ia_css_program_ID_t prog_id;

	verifexitval(NULL != process, EFAULT);
	verifexitval(NULL != p_manifest, EFAULT);

	prog_id = ia_css_process_get_program_ID(process);
	verifjmpexit(prog_id == ia_css_program_manifest_get_program_ID(p_manifest));

	invalid_flag = (process->size <= process->cell_dependencies_offset) ||
		   (process->size <= process->terminal_dependencies_offset);

	if (ia_css_has_program_manifest_fixed_cell(p_manifest)) {
		vied_nci_cell_ID_t cell_id;

		cell_id = ia_css_program_manifest_get_cell_ID(p_manifest);
		invalid_flag = invalid_flag ||
			    (cell_id != (vied_nci_cell_ID_t)(ia_css_process_get_cell(process)));
	}
	invalid_flag = invalid_flag ||
		((process->cell_dependency_count +
		  process->terminal_dependency_count) == 0) ||
		(process->cell_dependency_count !=
	ia_css_program_manifest_get_program_dependency_count(p_manifest)) ||
		(process->terminal_dependency_count !=
	ia_css_program_manifest_get_terminal_dependency_count(p_manifest));

	/* TODO: to be removed once all PGs pass validation */
	if (invalid_flag == true) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
			"ia_css_is_process_valid(): false\n");
	}

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_is_process_valid() invalid argument\n");
		return false;
	} else {
		return (!invalid_flag);
	}
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_process_cmd(
	ia_css_process_t					*process,
	const ia_css_process_cmd_t				cmd)
{
	int	retval = -1;
	ia_css_process_state_t	state;

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO, "ia_css_process_cmd(): enter:\n");

	verifexit(process != NULL);

	state = ia_css_process_get_state(process);

	IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO, "ia_css_process_cmd(): state: %d\n", state);

	verifexit(state != IA_CSS_PROCESS_ERROR);
	verifexit(state < IA_CSS_N_PROCESS_STATES);

	switch (cmd) {
	case IA_CSS_PROCESS_CMD_NOP:
		break;
	case IA_CSS_PROCESS_CMD_ACQUIRE:
		verifexit(state == IA_CSS_PROCESS_READY);
		break;
	case IA_CSS_PROCESS_CMD_RELEASE:
		verifexit(state == IA_CSS_PROCESS_READY);
		break;
	case IA_CSS_PROCESS_CMD_START:
		verifexit((state == IA_CSS_PROCESS_READY)
			  || (state == IA_CSS_PROCESS_STOPPED));
		process->state = IA_CSS_PROCESS_STARTED;
		break;
	case IA_CSS_PROCESS_CMD_LOAD:
		verifexit((state == IA_CSS_PROCESS_STARTED)
				|| (state == IA_CSS_PROCESS_READY)
				|| (state == IA_CSS_PROCESS_STOPPED));
		process->state = IA_CSS_PROCESS_RUNNING;
		break;
	case IA_CSS_PROCESS_CMD_STOP:
		verifexit((state == IA_CSS_PROCESS_RUNNING)
			  || (state == IA_CSS_PROCESS_SUSPENDED));
		process->state = IA_CSS_PROCESS_STOPPED;
		break;
	case IA_CSS_PROCESS_CMD_SUSPEND:
		verifexit(state == IA_CSS_PROCESS_RUNNING);
		process->state = IA_CSS_PROCESS_SUSPENDED;
		break;
	case IA_CSS_PROCESS_CMD_RESUME:
		verifexit(state == IA_CSS_PROCESS_SUSPENDED);
		process->state = IA_CSS_PROCESS_RUNNING;
		break;
	case IA_CSS_N_PROCESS_CMDS:	/* Fall through */
	default:
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_cmd invalid cmd (0x%x)\n", cmd);
		goto EXIT;
	}
	retval = 0;
EXIT:
	if (NULL == process) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
			"ia_css_process_cmd invalid argument process\n");
	}
	if (retval != 0) {
		IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_process_cmd failed (%i)\n", retval);
	}
	return retval;
}

#endif /* __IA_CSS_PSYS_PROCESS_IMPL_H */
