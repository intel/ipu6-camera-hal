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

#include "ia_css_psys_process.h"
#include "ia_css_psys_dynamic_storage_class.h"
#include "ia_css_psys_process_private_types.h"
#include "ia_css_program_group_param_private.h"
#include "math_support.h"

/* *****************************************************
 * Functions to possibly inline
 * ******************************************************/
#ifndef __IA_CSS_PSYS_DYNAMIC_INLINE__
#include "ia_css_psys_process_impl.h"
#endif /* __IA_CSS_PSYS_DYNAMIC_INLINE__ */

/* *****************************************************
 * Functions not to inline and not available to firmware
 * ******************************************************/
#if !defined(__HIVECC)
STORAGE_CLASS_INLINE bool  ia_css_program_manifest_process_requires_extension(
    const ia_css_program_manifest_t            *manifest)
{
    int i;
    bool requires_extension = false;

    verifexit(manifest != NULL);

    for (i = 0; i < VIED_NCI_N_DATA_MEM_TYPE_ID; i++) {
        requires_extension |=
            (ia_css_program_manifest_get_ext_mem_size(manifest, i) != 0);
    }

#if HAS_DFM
    for (i = 0; i < VIED_NCI_N_DEV_CHN_ID; i++) {
        requires_extension |=
            (ia_css_program_manifest_get_dev_chn_size(manifest, i) != 0);
    }
    for (i = 0; i < VIED_NCI_N_DEV_DFM_ID; i++) {
        requires_extension |=
            (ia_css_program_manifest_get_dfm_port_bitmap(manifest, i) != 0);
        requires_extension |=
            (ia_css_program_manifest_get_dfm_active_port_bitmap(manifest, i) != 0);
    }
#endif /* HAS_DFM */
EXIT:
    return requires_extension;
}

size_t ia_css_sizeof_process(
    const ia_css_program_manifest_t            *manifest,
    const ia_css_program_param_t            *param)
{
    size_t    size = 0;

    uint8_t    program_dependency_count;
    uint8_t terminal_dependency_count;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_sizeof_process(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(param != NULL);

    size += sizeof(ia_css_process_t);

    if (ia_css_program_manifest_process_requires_extension(manifest)) {
        /* align start of extension to 32 bit */
        size = CEIL_MUL(size, sizeof(uint32_t));
        size += sizeof(ia_css_process_ext_t);
    }

    program_dependency_count =
        ia_css_program_manifest_get_program_dependency_count(manifest);
    terminal_dependency_count =
        ia_css_program_manifest_get_terminal_dependency_count(manifest);

    size += program_dependency_count*sizeof(vied_nci_resource_id_t);
    size += terminal_dependency_count*sizeof(vied_nci_resource_id_t);

    /* align whole structure to 32 bit */
    size = CEIL_MUL(size, sizeof(uint32_t));
EXIT:
    if (NULL == manifest || NULL == param) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_sizeof_process invalid argument\n");
    }
    return size;
}

ia_css_process_t *ia_css_process_create(
    void                    *raw_mem,
    const ia_css_program_manifest_t        *manifest,
    const ia_css_program_param_t        *param,
    const uint32_t                program_idx)
{
    int retval = -1, size = 0;
    ia_css_process_t *process = NULL;
    bool create_extension;

    uint8_t    program_dependency_count;
    uint8_t    terminal_dependency_count;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
        "ia_css_process_create(): enter:\n");

    verifexit(manifest != NULL);
    verifexit(param != NULL);
    verifexit(raw_mem != NULL);

    process = (ia_css_process_t *)raw_mem;;
    verifexit(process != NULL);
    create_extension = ia_css_program_manifest_process_requires_extension(manifest);

    process->state = IA_CSS_PROCESS_CREATED;

    program_dependency_count =
        ia_css_program_manifest_get_program_dependency_count(manifest);
    terminal_dependency_count =
        ia_css_program_manifest_get_terminal_dependency_count(manifest);

    /* A process requires at least one input or output */
    verifexit((program_dependency_count +
           terminal_dependency_count) != 0);

    size += sizeof(ia_css_process_t);

    if (create_extension == true) {
        /* align start of extension to 32 bit */
        size = CEIL_MUL(size, sizeof(uint32_t));
        process->process_extension_offset = (uint8_t)size;
        size += sizeof(ia_css_process_ext_t);
    } else {
        process->process_extension_offset = 0;
    }
    if (program_dependency_count != 0) {
        process->cell_dependencies_offset = (uint8_t)size;
        size += program_dependency_count * sizeof(vied_nci_resource_id_t);
    } else {
        process->cell_dependencies_offset = 0;
    }
    if (terminal_dependency_count != 0) {
        process->terminal_dependencies_offset = (uint8_t)size;;
        size += terminal_dependency_count * sizeof(uint8_t);
    } else {
        process->terminal_dependencies_offset = 0;
    }

    /* align whole structure to 32 bit */
    size = CEIL_MUL(size, sizeof(uint32_t));

    process->size = (uint8_t)ia_css_sizeof_process(manifest, param);
    assert((size_t)size == process->size);
    /* The following assert also implies that:
     * process->terminal_dependencies_offset,
     * process->cell_dependencies_offset,
     * process->process_extension_offset,
     * are <= UINT8_MAX
     */
    assert(size <= UINT8_MAX);

    process->ID = ia_css_program_manifest_get_program_ID(manifest);
    verifexit(process->ID != 0);
    assert(program_idx <= UINT8_MAX);
    process->program_idx = program_idx;

    process->cell_dependency_count = program_dependency_count;
    process->terminal_dependency_count = terminal_dependency_count;

    process->parent_offset = 0;
    verifexit(ia_css_process_clear_all(process) == 0);

    process->state = IA_CSS_PROCESS_READY;
    retval = 0;

    IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
        "ia_css_process_create(): Created successfully process 0x%lx ID 0x%x\n",
        (unsigned long int)process, process->ID);

EXIT:
    if (NULL == manifest || NULL == param) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_create invalid argument\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_create failed (%i)\n", retval);
        process = ia_css_process_destroy(process);
    }
    return process;
}

ia_css_process_t *ia_css_process_destroy(
    ia_css_process_t *process)
{

    return process;
}

int ia_css_process_set_cell(
    ia_css_process_t                    *process,
    const vied_nci_cell_ID_t                cell_id)
{
    int    retval = -1;
    vied_nci_resource_bitmap_t        bit_mask;
    vied_nci_resource_bitmap_t        resource_bitmap;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_process_set_cell(): enter:\n");

    verifexit(process != NULL);

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);

    parent_state = ia_css_process_group_get_state(parent);
    state = ia_css_process_get_state(process);

/* Some programs are mapped on a fixed cell,
 * when the process group is created
 */
    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
        (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
        (parent_state == IA_CSS_PROCESS_GROUP_CREATED) ||
        /* If the process group has already been created, but no VP cell
         * has been assigned to this process (i.e. not fixed in
         * manifest), then we need to set the cell of this process
         * while its parent state is READY (the ready state is set at
         * the end of ia_css_process_group_create)
         */
        (parent_state == IA_CSS_PROCESS_GROUP_READY)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

/* Some programs are mapped on a fixed cell, thus check is not secure,
 * but it will detect a preset, the process manager will do the secure check
 */
    verifexit(ia_css_process_get_cell(process) ==
          VIED_NCI_N_CELL_ID);

    bit_mask = vied_nci_cell_bit_mask(cell_id);
    resource_bitmap = ia_css_process_group_get_resource_bitmap(parent);

    verifexit(bit_mask != 0);
    verifexit(vied_nci_is_bitmap_clear(bit_mask, resource_bitmap));

    ia_css_process_cells_clear(process);
    ia_css_process_cells_set_cell(process, 0, cell_id);

    resource_bitmap = vied_nci_bitmap_set(resource_bitmap, bit_mask);

    retval = ia_css_process_group_set_resource_bitmap(
            parent, resource_bitmap);
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_cell invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_set_cell failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_clear_cell(
    ia_css_process_t *process)
{
    int    retval = -1;
    vied_nci_cell_ID_t                cell_id;
    ia_css_process_group_t            *parent;
    vied_nci_resource_bitmap_t        resource_bitmap;
    vied_nci_resource_bitmap_t        bit_mask;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_clear_cell(): enter:\n");
    verifexit(process != NULL);

    cell_id = ia_css_process_get_cell(process);
    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);

    parent_state = ia_css_process_group_get_state(parent);
    state = ia_css_process_get_state(process);

    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED)
           || (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    bit_mask = vied_nci_cell_bit_mask(cell_id);
    resource_bitmap = ia_css_process_group_get_resource_bitmap(parent);

    verifexit(bit_mask != 0);
    verifexit(vied_nci_is_bitmap_set(bit_mask, resource_bitmap));

    ia_css_process_cells_clear(process);

    resource_bitmap = vied_nci_bitmap_clear(resource_bitmap, bit_mask);

    retval = ia_css_process_group_set_resource_bitmap(
            parent, resource_bitmap);
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_clear_cell invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_clear_cell failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_set_int_mem(
    ia_css_process_t                *process,
    const vied_nci_mem_type_ID_t            mem_type_id,
    const vied_nci_resource_size_t            offset)
{
    (void)process;
    (void)mem_type_id;
    (void)offset;
    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
        "ia_css_process_set_int_mem should not be called"
        "(internally memory feature not used).\n");
    return 0;
}

int ia_css_process_clear_int_mem(
    ia_css_process_t *process,
    const vied_nci_mem_type_ID_t mem_type_id)
{
    (void)process;
    (void)mem_type_id;
    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
        "ia_css_process_clear_int_mem should not be called"
        "(internally memory feature not used).\n");
    return 0;
}

int ia_css_process_set_ext_mem(
    ia_css_process_t *process,
    const vied_nci_mem_ID_t mem_id,
    const vied_nci_resource_size_t offset)
{
    int    retval = -1;
    ia_css_process_group_t    *parent;
    vied_nci_cell_ID_t    cell_id;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t    state;
    vied_nci_mem_type_ID_t mem_type_id;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_ext_mem(): enter:\n");

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);
    verifexit(process_ext != NULL);

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    cell_id = ia_css_process_get_cell(process);

    parent_state = ia_css_process_group_get_state(parent);
    state = ia_css_process_get_state(process);

    /* TODO : separate process group start and run from
    *      process_group_exec_cmd()
    */
    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    /* Check that the memory actually exists, "vied_nci_has_cell_mem_of_id()"
    * will return false on error
    */

    mem_type_id = vied_nci_mem_get_type(mem_id);
#ifdef HAS_PMEM
    if (((!vied_nci_has_cell_mem_of_id(cell_id, mem_id) &&
        (mem_type_id != VIED_NCI_PMEM_TYPE_ID))
        || vied_nci_mem_is_ext_type(mem_type_id)) &&
        (mem_id < VIED_NCI_N_MEM_ID)) {

        verifexit(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID);
        process_ext->ext_mem_id[mem_type_id] = mem_id;
        process_ext->ext_mem_offset[mem_type_id] = offset;
        retval = 0;
    }
#else /* not HAS_PMEM */
    if (((!vied_nci_has_cell_mem_of_id(cell_id, mem_id))
            || vied_nci_mem_is_ext_type(mem_type_id)) &&
            (mem_id < VIED_NCI_N_MEM_ID)) {

        verifexit(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID);
        process_ext->ext_mem_id[mem_type_id] = mem_id;
        process_ext->ext_mem_offset[mem_type_id] = offset;
        retval = 0;
    }
#endif /* HAS_PMEM */

EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_ext_mem invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_set_ext_mem failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_clear_ext_mem(
    ia_css_process_t *process,
    const vied_nci_mem_type_ID_t mem_type_id)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_clear_ext_mem(): enter:\n");

    verifexit(process != NULL);
    verifexit(mem_type_id < VIED_NCI_N_DATA_MEM_TYPE_ID);
    process_ext = ia_css_process_get_extension(process);
    if (process_ext == NULL) {
        return 0;
    }

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    state = ia_css_process_get_state(process);

    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    parent_state = ia_css_process_group_get_state(parent);

    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));

    process_ext->ext_mem_id[mem_type_id] = VIED_NCI_N_MEM_ID;
    process_ext->ext_mem_offset[mem_type_id] = IA_CSS_PROCESS_INVALID_OFFSET;

    retval = 0;
EXIT:
    if (NULL == process || mem_type_id >= VIED_NCI_N_DATA_MEM_TYPE_ID) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_clear_ext_mem invalid argument\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_clear_ext_mem failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_set_cells_bitmap(
    ia_css_process_t *process,
    const vied_nci_resource_bitmap_t bitmap)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    int array_index = 0;
    int bit_index;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_cells_bitmap(): enter:\n");

    verifexit(process != NULL);
    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    state = ia_css_process_get_state(process);

    parent_state = ia_css_process_group_get_state(parent);

    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
        (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
        (parent_state == IA_CSS_PROCESS_GROUP_CREATED) ||
        (parent_state == IA_CSS_PROCESS_GROUP_READY)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    for (bit_index = 0; bit_index < VIED_NCI_N_CELL_ID; bit_index++) {
        if (vied_nci_is_bit_set_in_bitmap(bitmap, bit_index)) {
            verifexit(array_index < IA_CSS_PROCESS_MAX_CELLS);
            ia_css_process_cells_set_cell(process,
                array_index, (vied_nci_cell_ID_t)bit_index);
            array_index++;
        }
    }
    for (; array_index < IA_CSS_PROCESS_MAX_CELLS; array_index++) {
        ia_css_process_cells_set_cell(process,
            array_index, VIED_NCI_N_CELL_ID);
    }

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_cells_bitmap invalid argument\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_set_cells_bitmap failed (%i)\n", retval);
    }
    return retval;
}

#if VIED_NCI_N_DEV_CHN_ID > 0
int ia_css_process_set_dev_chn(
    ia_css_process_t *process,
    const vied_nci_dev_chn_ID_t dev_chn_id,
    const vied_nci_resource_size_t offset)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_dev_chn(): enter:\n");

    verifexit(process != NULL);
    verifexit(dev_chn_id <= VIED_NCI_N_DEV_CHN_ID);
    process_ext = ia_css_process_get_extension(process);
    verifexit(process_ext != NULL);

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    state = ia_css_process_get_state(process);

    parent_state = ia_css_process_group_get_state(parent);

    /* TODO : separate process group start and run from
    *      process_group_exec_cmd()
    */
    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    process_ext->dev_chn_offset[dev_chn_id] = offset;

    retval = 0;
EXIT:
    if (NULL == process || dev_chn_id >= VIED_NCI_N_DEV_CHN_ID) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_dev_chn invalid argument\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
                    "ia_css_process_set_dev_chn invalid argument\n");
    }
    return retval;
}

int ia_css_process_clear_dev_chn(
    ia_css_process_t *process,
    const vied_nci_dev_chn_ID_t dev_chn_id)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_clear_dev_chn(): enter:\n");

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);
    if (process_ext == NULL) {
        return 0;
    }

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);

    parent_state = ia_css_process_group_get_state(parent);
    state = ia_css_process_get_state(process);

    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED)
           || (parent_state == IA_CSS_PROCESS_GROUP_STARTED)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    verifexit(dev_chn_id <= VIED_NCI_N_DEV_CHN_ID);

    process_ext->dev_chn_offset[dev_chn_id] = IA_CSS_PROCESS_INVALID_OFFSET;

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
             "ia_css_process_clear_dev_chn invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_clear_dev_chn failed (%i)\n", retval);
    }
    return retval;
}
#endif /* VIED_NCI_N_DEV_CHN_ID */

#if HAS_DFM
int ia_css_process_set_dfm_port_bitmap(
    ia_css_process_t                 *process,
    const vied_nci_dev_dfm_id_t      dfm_dev_id,
    const vied_nci_resource_bitmap_t bitmap)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_dfm_port(): enter:\n");

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);
    if (process_ext == NULL && bitmap == 0) {
        return 0;
    }
    verifexit(process_ext != NULL);

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    state = ia_css_process_get_state(process);

    parent_state = ia_css_process_group_get_state(parent);

    /* TODO : separate process group start and run from
    *      process_group_exec_cmd()
    */
    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);

    verifexit(dfm_dev_id <= VIED_NCI_N_DEV_DFM_ID);
    process_ext->dfm_port_bitmap[dfm_dev_id] = bitmap;
    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_set_dfm_port invalid argument\n");
    }
    return retval;
}

int ia_css_process_set_dfm_active_port_bitmap(
    ia_css_process_t                 *process,
    const vied_nci_dev_dfm_id_t      dfm_dev_id,
    const vied_nci_resource_bitmap_t bitmap)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_dfm_active_port_bitmap(): enter:\n");

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);
    if (process_ext == NULL && bitmap == 0) {
        return 0;
    }
    verifexit(process_ext != NULL);

    parent = ia_css_process_get_parent(process);
    verifexit(parent != NULL);
    state = ia_css_process_get_state(process);

    parent_state = ia_css_process_group_get_state(parent);

    /* TODO : separate process group start and run from
    *      process_group_exec_cmd()
    */
    verifexit(((parent_state == IA_CSS_PROCESS_GROUP_BLOCKED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_STARTED) ||
           (parent_state == IA_CSS_PROCESS_GROUP_RUNNING)));
    verifexit(state == IA_CSS_PROCESS_READY || state == IA_CSS_PROCESS_STOPPED);
    verifexit(dfm_dev_id <= VIED_NCI_N_DEV_DFM_ID);
    process_ext->dfm_active_port_bitmap[dfm_dev_id] = bitmap;
    retval = 0;
EXIT:
    if (retval != 0) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_dfm_active_port_bitmap invalid argument\n");
    }
    return retval;
}
#endif /* HAS_DFM */

int ia_css_process_clear_all(
    ia_css_process_t *process)
{
    int    retval = -1;
    ia_css_process_group_t            *parent;
    ia_css_process_group_state_t    parent_state;
    ia_css_process_state_t            state;
    ia_css_process_ext_t *process_ext;
    int    mem_index;
    int    dev_chn_index;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_clear_all(): enter:\n");

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);

    state = ia_css_process_get_state(process);

    parent = ia_css_process_get_parent(process);
    if (parent != NULL) {
        parent_state = ia_css_process_group_get_state(parent);
    } else {
        parent_state = IA_CSS_N_PROCESS_GROUP_STATES;
    }

/* Resource clear can only be called in excluded states contrary to set */
    verifexit((parent_state != IA_CSS_PROCESS_GROUP_RUNNING) ||
           (parent_state == IA_CSS_N_PROCESS_GROUP_STATES));
    verifexit((state == IA_CSS_PROCESS_CREATED) ||
          (state == IA_CSS_PROCESS_READY));

#if VIED_NCI_N_DEV_CHN_ID > 0
    if (process_ext != NULL) {
        for (dev_chn_index = 0; dev_chn_index < VIED_NCI_N_DEV_CHN_ID;
            dev_chn_index++) {
            process_ext->dev_chn_offset[dev_chn_index] =
                IA_CSS_PROCESS_INVALID_OFFSET;
        }
    }
#else
    NOT_USED(dev_chn_index);
#endif /* VIED_NCI_N_DEV_CHN_ID */
    if (process_ext != NULL) {
        /* No difference whether a cell_id has been set or not, clear all */
        for (mem_index = 0; mem_index < VIED_NCI_N_DATA_MEM_TYPE_ID;
            mem_index++) {
            process_ext->ext_mem_id[mem_index] = VIED_NCI_N_MEM_ID;
            process_ext->ext_mem_offset[mem_index] =
                IA_CSS_PROCESS_INVALID_OFFSET;
        }
    }

    ia_css_process_cells_clear(process);

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_clear_all invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_clear_all failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_acquire(
    ia_css_process_t *process)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_process_acquire(): enter:\n");

    verifexit(process != NULL);

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_acquire invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_acquire failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_release(
    ia_css_process_t *process)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, INFO,
        "ia_css_process_release(): enter:\n");

    verifexit(process != NULL);

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_t invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_release failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_print(const ia_css_process_t *process, void *fid)
{
    int        retval = -1;
    int        i, dev_chn_index;
    uint16_t mem_index;
    uint8_t    cell_dependency_count, terminal_dependency_count;
    ia_css_process_ext_t *process_ext;
    NOT_USED(fid);

    IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
        "ia_css_process_print(process 0x%lx): enter:\n", (unsigned long int)process);

    verifexit(process != NULL);
    process_ext = ia_css_process_get_extension(process);

    IA_CSS_TRACE_7(PSYSAPI_DYNAMIC, INFO,
    "\tprocess 0x%lx, sizeof %d, program_idx %d, programID %d, state %d, parent 0x%lx, cell %d\n",
        (unsigned long int)process,
        (int)ia_css_process_get_size(process),
        (int)ia_css_process_get_program_idx(process),
        (int)ia_css_process_get_program_ID(process),
        (int)ia_css_process_get_state(process),
        (unsigned long int)ia_css_process_get_parent(process),
        (int)ia_css_process_get_cell(process));

    if (process_ext != NULL) {
        for (mem_index = 0; mem_index < (int)VIED_NCI_N_DATA_MEM_TYPE_ID;
            mem_index++) {
            /* TODO: in case of an cells_bitmap = [],
            * vied_nci_cell_get_mem_type will return a wrong result.
            */
            IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
                "\texternal index %d, type %d, id %d offset 0x%x\n",
                mem_index,
                (int)vied_nci_cell_get_mem_type(ia_css_process_get_cell(process),
                                mem_index),
                (int)(process_ext->ext_mem_id[mem_index]),
                process_ext->ext_mem_offset[mem_index]);
        }
#if VIED_NCI_N_DEV_CHN_ID > 0
        for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_CHN_ID;
            dev_chn_index++) {
            IA_CSS_TRACE_3(PSYSAPI_DYNAMIC, INFO,
                "\tdevice channel index %d, type %d, offset 0x%x\n",
                dev_chn_index,
                (int)dev_chn_index,
                process_ext->dev_chn_offset[dev_chn_index]);
        }
#else
        NOT_USED(dev_chn_index);
#endif /* VIED_NCI_N_DEV_CHN_ID */
#if HAS_DFM
        for (dev_chn_index = 0; dev_chn_index < (int)VIED_NCI_N_DEV_DFM_ID;
            dev_chn_index++) {
            IA_CSS_TRACE_4(PSYSAPI_DYNAMIC, INFO,
                "\tdfm device index %d, type %d, bitmap 0x%x active_ports_bitmap 0x%x\n",
                dev_chn_index, dev_chn_index,
                process_ext->dfm_port_bitmap[dev_chn_index],
                process_ext->dfm_active_port_bitmap[dev_chn_index]);
        }
#endif
    }

    for (i = 0; i < IA_CSS_PROCESS_MAX_CELLS; i++) {
        IA_CSS_TRACE_2(PSYSAPI_DYNAMIC, INFO,
            "\tcells[%d] = 0x%x\n",
            i, ia_css_process_cells_get_cell(process, i));
    }

    cell_dependency_count =
        ia_css_process_get_cell_dependency_count(process);
    if (cell_dependency_count == 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "\tcell_dependencies[%d] {};\n", cell_dependency_count);
    } else {
        vied_nci_resource_id_t cell_dependency;

        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "\tcell_dependencies[%d] {", cell_dependency_count);
        for (i = 0; i < (int)cell_dependency_count - 1; i++) {
            cell_dependency =
                ia_css_process_get_cell_dependency(process, i);
            IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
                "%4d, ", cell_dependency);
        }
        cell_dependency =
            ia_css_process_get_cell_dependency(process, i);
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "%4d}\n", cell_dependency);
        (void)cell_dependency;
    }

    terminal_dependency_count =
        ia_css_process_get_terminal_dependency_count(process);
    if (terminal_dependency_count == 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "\tterminal_dependencies[%d] {};\n",
            terminal_dependency_count);
    } else {
        uint8_t terminal_dependency;

        terminal_dependency_count =
            ia_css_process_get_terminal_dependency_count(process);
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "\tterminal_dependencies[%d] {",
            terminal_dependency_count);
        for (i = 0; i < (int)terminal_dependency_count - 1; i++) {
            terminal_dependency =
                 ia_css_process_get_terminal_dependency(process, i);
            IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
                "%4d, ", terminal_dependency);
        }
        terminal_dependency =
            ia_css_process_get_terminal_dependency(process, i);
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, INFO,
            "%4d}\n", terminal_dependency);
        (void)terminal_dependency;
    }

    retval = 0;
EXIT:
    if (NULL == process) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_print invalid argument process\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_print failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_set_parent(
    ia_css_process_t                    *process,
    ia_css_process_group_t                    *parent)
{
    int    retval = -1;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
               "ia_css_process_set_parent(): enter:\n");

    verifexit(process != NULL);
    verifexit(parent != NULL);

    process->parent_offset = (uint16_t) ((char *)parent - (char *)process);
    retval = 0;
EXIT:
    if (NULL == process || NULL == parent) {
        IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, WARNING,
            "ia_css_process_set_parent invalid argument\n");
    }
    if (retval != 0) {
        IA_CSS_TRACE_1(PSYSAPI_DYNAMIC, ERROR,
            "ia_css_process_set_parent failed (%i)\n", retval);
    }
    return retval;
}

int ia_css_process_set_cell_dependency(
    const ia_css_process_t                    *process,
    const unsigned int                    dep_index,
    const vied_nci_resource_id_t                id)
{
    int retval = -1;
    uint8_t *process_dep_ptr;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_process_set_cell_dependency(): enter:\n");
    verifexit(process != NULL);

    process_dep_ptr =
        (uint8_t *)process + process->cell_dependencies_offset +
               dep_index*sizeof(vied_nci_resource_id_t);

    *process_dep_ptr = id;
    retval = 0;
EXIT:
    return retval;
}

int ia_css_process_set_terminal_dependency(
    const ia_css_process_t                *process,
    const unsigned int                dep_index,
    const vied_nci_resource_id_t        id)
{
    int retval = -1;
    uint8_t *terminal_dep_ptr;

    IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
        "ia_css_process_set_terminal_dependency(): enter:\n");
    verifexit(process != NULL);
    verifexit(ia_css_process_get_terminal_dependency_count(process) > dep_index);

    terminal_dep_ptr =
        (uint8_t *)process + process->terminal_dependencies_offset +
               dep_index*sizeof(uint8_t);

    *terminal_dep_ptr = id;
    retval = 0;
EXIT:
    return retval;
}

#endif /* !defined(__HIVECC) */
