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

#ifndef __IA_CSS_PSYS_PROCESS_PRIVATE_TYPES_H
#define __IA_CSS_PSYS_PROCESS_PRIVATE_TYPES_H

#include "ia_css_psys_process_types.h"
#include "vied_nci_psys_resource_model.h"

#define    N_UINT32_IN_PROCESS_STRUCT                0
#define    N_UINT16_IN_PROCESS_STRUCT                1
#define    N_UINT8_IN_PROCESS_STRUCT                8

#if IA_CSS_PROCESS_STATE_BITS != 8
#error IA_CSS_PROCESS_STATE_BITS expected to be 8
#endif

#define SIZE_OF_PROCESS_EXTENSION \
     ((VIED_NCI_RESOURCE_BITMAP_BITS * VIED_NCI_N_DEV_DFM_ID) \
    + (VIED_NCI_RESOURCE_BITMAP_BITS * VIED_NCI_N_DEV_DFM_ID) \
    + (VIED_NCI_N_DATA_MEM_TYPE_ID * VIED_NCI_RESOURCE_SIZE_BITS) \
    + (VIED_NCI_N_DEV_CHN_ID * VIED_NCI_RESOURCE_SIZE_BITS) \
    + (VIED_NCI_N_DATA_MEM_TYPE_ID * VIED_NCI_RESOURCE_ID_BITS) \
    + (N_PADDING_UINT8_IN_PROCESS_EXT_STRUCT * 8))

/** Extended process attributes describing more resource requirements
 *  (and some DFM handling details ) beyond those covered in
 *  ia_css_process_s.
 */
struct ia_css_process_ext_s {
#if VIED_NCI_N_DEV_DFM_ID > 0
    /** DFM port allocated to this process */
    vied_nci_resource_bitmap_t dfm_port_bitmap[VIED_NCI_N_DEV_DFM_ID];
    /** Active DFM ports which need a kick */
    vied_nci_resource_bitmap_t dfm_active_port_bitmap[VIED_NCI_N_DEV_DFM_ID];
#endif
#if VIED_NCI_N_DATA_MEM_TYPE_ID > 0
    /** (external) Memory allocation offset given to this process */
    vied_nci_resource_size_t ext_mem_offset[VIED_NCI_N_DATA_MEM_TYPE_ID];
#endif
#if VIED_NCI_N_DEV_CHN_ID > 0
    /** Device channel allocation offset given to this process */
    vied_nci_resource_size_t dev_chn_offset[VIED_NCI_N_DEV_CHN_ID];
#endif
#if VIED_NCI_N_DATA_MEM_TYPE_ID > 0
    /** (external) Memory ID */
    vied_nci_resource_id_t ext_mem_id[VIED_NCI_N_DATA_MEM_TYPE_ID];
#endif
#if N_PADDING_UINT8_IN_PROCESS_EXT_STRUCT > 0
    /** Number of processes (mapped on cells) this process depends on */
    uint8_t padding[N_PADDING_UINT8_IN_PROCESS_EXT_STRUCT];
#endif
};

#define SIZE_OF_PROCESS_STRUCT_BITS \
    ((N_UINT32_IN_PROCESS_STRUCT * 32) \
    + (IA_CSS_PROGRAM_ID_BITS) \
    + (N_UINT16_IN_PROCESS_STRUCT * 16) \
    + (IA_CSS_PROCESS_MAX_CELLS * VIED_NCI_RESOURCE_ID_BITS) \
    + (N_UINT8_IN_PROCESS_STRUCT * 8) \
    + ((N_PADDING_UINT8_IN_PROCESS_STRUCT) * 8))

/** Process attributes describing the instantiation of a single Program
 *  within a Program Group.
 */
struct ia_css_process_s {
    /** Reference to the containing process group */
    int16_t parent_offset;
    /** Size of this structure */
    uint8_t size;
    /** Offset in bytes to Array[dependency_count] of ID's (type vied_nci_resource_id_t)
     *  of the cells that provide input to this process.
     *  Align to 64 bit boundary.
     *  Set to zero if dependency_count is zero.
     *  @see cell_dependency_count in this structure */
    uint8_t cell_dependencies_offset;
    /** Offset in bytes to Array[terminal_dependency_count] of indices (type: uint8_t)
     *  of connected terminals (that is, the terminals that are associated with this process).
     *  Each index is a key into to the process group terminal list.
     *  Set to zero if dependency_count is zero.
     *  @see terminal_dependency_count in this structure
     *  @see ia_css_process_group_get_terminal() */
    uint8_t terminal_dependencies_offset;
    /** Offset in bytes to process extension structure, ia_css_process_ext_s
     *  Align to 32 bit boundary.
     *  Set to zero if extension is not used. The manifest must be queried to
     *  find out if the extensino is necessary.
     *  @see ia_css_program_manifest_process_requires_extension() */
    uint8_t process_extension_offset;
    /** Referal ID to a specific program.  This ID is unique across PG's and
     *  can be used to retreive program meta data (AKA "program descriptor")
     *  from the client package, including the program manifest and even
     *  firmware program code, if the program is to be executed on a DSP. */
    ia_css_program_ID_t ID;
    /** Program index into the list of programs in the containing PG */
    uint8_t program_idx;
    /** State of the process FSM dependent on the parent FSM.  Type is actually: ia_css_process_state_t
     *  Used by the PSYSAPI for its own process state machine and internally by FW.  For the purposes of
     *  the ABI protocol definition, host code should just set this to IA_CSS_PROCESS_READY before sending
     *  an IA_CSS_PROCESS_GROUP_CMD_START command to firmware.  */
    uint8_t state;
    /** Execution resources (e.g. DSP "cells" or fixed HW) required by the process. */
    vied_nci_resource_id_t cells[IA_CSS_PROCESS_MAX_CELLS];
    /** Size of the array of dependent cells pointed at by cell_dependencies_offset
     *  Value must be set to the corresponding value from the manifest entry for this program.
     *  As each program implies a cell (logically, if not physically) to execute the program,
     *  the dependent cell count value in the process is the same as the dependent program count.
     *  @see ia_css_program_manifest_get_program_dependency_count() */
    uint8_t cell_dependency_count;
    /** Number of terminals this process depends on.  This is the size of the array
     *  pointed at by terminal_dependencies_offset
     *  Value must be set to the corresponding value from the manifest entry for this program.
     *  @see ia_css_program_manifest_get_terminal_dependency_count()
     */
    uint8_t terminal_dependency_count;
#if (N_PADDING_UINT8_IN_PROCESS_STRUCT > 0)
    /** Padding bytes for 64bit alignment*/
    uint8_t padding[N_PADDING_UINT8_IN_PROCESS_STRUCT];
#endif /*(N_PADDING_UINT8_IN_PROCESS_STRUCT > 0)*/
};

#endif /* __IA_CSS_PSYS_PROCESS_PRIVATE_TYPES_H */
