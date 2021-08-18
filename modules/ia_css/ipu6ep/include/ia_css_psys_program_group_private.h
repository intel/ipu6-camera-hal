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

#ifndef __IA_CSS_PSYS_PROGRAM_GROUP_PRIVATE_H
#define __IA_CSS_PSYS_PROGRAM_GROUP_PRIVATE_H

#include "ia_css_psys_manifest_types.h"
#include "ia_css_terminal_manifest_types.h"
#include "ia_css_kernel_bitmap.h"
#include "ia_css_program_group_data.h"
#include "vied_nci_psys_resource_model.h"
#include "ia_css_rbm_manifest_types.h"
#include <type_support.h>
#include <math_support.h>
#include <platform_support.h>

/**
 * @addtogroup group_psysapi
 * @{
 */

#define SIZE_OF_PROGRAM_GROUP_MANIFEST_STRUCT_IN_BITS \
    ((IA_CSS_KERNEL_BITMAP_BITS) \
    + (IA_CSS_PROGRAM_GROUP_ID_BITS) \
    + (5 * IA_CSS_UINT16_T_BITS) \
    + (5 * IA_CSS_UINT8_T_BITS) \
    + (5 * IA_CSS_UINT8_T_BITS))

/** @brief Meta-data for a program group
 *
 * Describes the contents and layout of the program group
 * components in memory.
 *
 * Offsets are relative to the base address of the program group
 * manifest object.
 */
struct ia_css_program_group_manifest_s {
    /** Indicate kernels are present in this program group */
    ia_css_kernel_bitmap_t kernel_bitmap;
    /** Referral ID to program group FW */
    ia_css_program_group_ID_t ID;
    /** Offset to program manifest list.  One entry for each program. */
    uint16_t program_manifest_offset;
    /** Offset to terminal manifest list.  One entry for each terminal. */
    uint16_t terminal_manifest_offset;
    /** Offset to private data (not part of the official API) */
    uint16_t private_data_offset;
    /** Offset to RBM manifest */
    uint16_t rbm_manifest_offset;
    /** Size of this structure */
    uint16_t size;
    /** Storage alignment requirement (in uint8_t) */
    uint8_t alignment;
    /** Total number of kernels in this program group */
    uint8_t kernel_count;
    /** Total number of program in this program group */
    uint8_t program_count;
    /** Total number of terminals on this program group */
    uint8_t terminal_count;
    /** Total number of independent subgraphs in this program group */
    uint8_t subgraph_count;
    /** Padding; esnures that rbm_manifest starts on 64bit alignment */
    uint8_t reserved[5];
};

#define SIZE_OF_PROGRAM_MANIFEST_EXT_STRUCT_IN_BYTES \
    ((VIED_NCI_RESOURCE_BITMAP_BITS * VIED_NCI_N_DEV_DFM_ID) \
    + (VIED_NCI_RESOURCE_BITMAP_BITS * VIED_NCI_N_DEV_DFM_ID) \
    + (VIED_NCI_RESOURCE_SIZE_BITS * VIED_NCI_N_DATA_MEM_TYPE_ID * 2) \
    + (VIED_NCI_RESOURCE_SIZE_BITS * VIED_NCI_N_DEV_CHN_ID * 2) \
    + (2 * IA_CSS_UINT8_T_BITS * IA_CSS_MAX_INPUT_DEC_RESOURCES) \
    + (2 * IA_CSS_UINT8_T_BITS * IA_CSS_MAX_OUTPUT_DEC_RESOURCES) \
    + (IA_CSS_UINT8_T_BITS * VIED_NCI_N_DEV_DFM_ID) + \
    + (N_PADDING_UINT8_IN_PROGRAM_MANIFEST_EXT * IA_CSS_UINT8_T_BITS))

/** "Extended" meta-data for a single program
 *
 * Mostly, if not completely, internal to FW and of no interest to the SW stack.
 */
struct ia_css_program_manifest_ext_s {
#if VIED_NCI_N_DEV_DFM_ID > 0
    /** DFM port allocation of this program */
    vied_nci_resource_bitmap_t dfm_port_bitmap[VIED_NCI_N_DEV_DFM_ID];
    /** Active DFM ports which need a kick
     * If an empty port is configured to run in active mode, the empty
     * port and the corresponding full port(s) in the stream must be kicked.
     * The empty port must always be kicked after the full port.
     */
    vied_nci_resource_bitmap_t dfm_active_port_bitmap[VIED_NCI_N_DEV_DFM_ID];
#endif
#if VIED_NCI_N_DATA_MEM_TYPE_ID > 0
    /** (external) Memory allocation size needs of this program */
    vied_nci_resource_size_t ext_mem_size[VIED_NCI_N_DATA_MEM_TYPE_ID];
    vied_nci_resource_size_t ext_mem_offset[VIED_NCI_N_DATA_MEM_TYPE_ID];
#endif
#if VIED_NCI_N_DEV_CHN_ID > 0
    /** Device channel allocation size needs of this program */
    vied_nci_resource_size_t dev_chn_size[VIED_NCI_N_DEV_CHN_ID];
    vied_nci_resource_size_t dev_chn_offset[VIED_NCI_N_DEV_CHN_ID];
#endif
#if VIED_NCI_N_DEV_DFM_ID > 0
    /** DFM ports are relocatable if value is set to 1.
     * The flag is per dfm port type.
     * This will not be supported for now.
     */
    uint8_t is_dfm_relocatable[VIED_NCI_N_DEV_DFM_ID];
#endif
#if IA_CSS_MAX_INPUT_DEC_RESOURCES > 0
    /** DEC compression flush service entry, describing which streams
     *  require flush service handling for decompression (input).
     *  @note stream ID's are the ID's used by the MMU
     */
    uint8_t dec_resources_input[IA_CSS_MAX_INPUT_DEC_RESOURCES];
    /** Association of streams in dec_resources_input with the terminal
     *  they belong to.
     */
    uint8_t dec_resources_input_terminal[IA_CSS_MAX_INPUT_DEC_RESOURCES];
#endif
#if IA_CSS_MAX_OUTPUT_DEC_RESOURCES > 0
    /** DEC compression flush service entry, describing which streams
     *  require flush service handling for compression (output).
     *  @note stream ID's are the ID's used by the MMU
     */
    uint8_t dec_resources_output[IA_CSS_MAX_OUTPUT_DEC_RESOURCES];
    /** Association of streams in dec_resources_output with the terminal
     *  they belong to.
     */
    uint8_t dec_resources_output_terminal[IA_CSS_MAX_OUTPUT_DEC_RESOURCES];
#endif
/** Padding bytes for 32bit alignment*/
#if N_PADDING_UINT8_IN_PROGRAM_MANIFEST_EXT > 0
    uint8_t padding[N_PADDING_UINT8_IN_PROGRAM_MANIFEST_EXT];
#endif
};

#define SIZE_OF_PROGRAM_MANIFEST_STRUCT_IN_BITS \
    (IA_CSS_KERNEL_BITMAP_BITS \
    + IA_CSS_PROGRAM_ID_BITS \
    + IA_CSS_PROGRAM_TYPE_BITS \
    + (1 * IA_CSS_UINT16_T_BITS) \
    + (IA_CSS_PROCESS_MAX_CELLS * VIED_NCI_RESOURCE_ID_BITS) \
    + (VIED_NCI_RESOURCE_ID_BITS) \
    + (6 * IA_CSS_UINT8_T_BITS) \
    + (N_PADDING_UINT8_IN_PROGRAM_MANIFEST * IA_CSS_UINT8_T_BITS))

/** Meta-data for a single program
 *
 * This structure contains only the information required for resource
 * management and construction of the process group.
 */
struct ia_css_program_manifest_s {
    /** Indicate which kernels lead to this program being used */
    ia_css_kernel_bitmap_t kernel_bitmap;
    /** offset to add to reach parent. This is negative value.*/
    int16_t parent_offset;
    uint8_t program_dependency_offset;
    uint8_t terminal_dependency_offset;
    /** Size of this structure */
    uint8_t size;
    /** offset to ia_css_program_manifest_ext_s, 0 if there is none */
    uint8_t program_extension_offset;
    /** Specification of for exclusive or parallel programs */
    uint8_t program_type; /* ia_css_program_type_t */
    /** Referral ID to a specific program FW, valid ID's != 0 */
    ia_css_program_ID_t ID;
    /** Array of all the cells this program needs */
    vied_nci_resource_id_t cells[IA_CSS_PROCESS_MAX_CELLS];
    /** (exclusive) indication of a cell type to be used by this program */
    vied_nci_resource_id_t cell_type_id;
    /** Number of programs this program depends on */
    uint8_t program_dependency_count;
    /** Number of terminals this program depends on */
    uint8_t terminal_dependency_count;
    /** Padding bytes for 32bit alignment*/
#if N_PADDING_UINT8_IN_PROGRAM_MANIFEST > 0
    uint8_t padding[N_PADDING_UINT8_IN_PROGRAM_MANIFEST];
#endif
};

/*
 *Calculation for manual size check for struct ia_css_data_terminal_manifest_s
 */
#define SIZE_OF_DATA_TERMINAL_MANIFEST_STRUCT_IN_BITS \
    (SIZE_OF_TERMINAL_MANIFEST_STRUCT_IN_BITS \
    + IA_CSS_FRAME_FORMAT_BITMAP_BITS \
    + IA_CSS_CONNECTION_BITMAP_BITS \
    + IA_CSS_KERNEL_BITMAP_BITS \
    + (4 * (IA_CSS_UINT16_T_BITS * IA_CSS_N_DATA_DIMENSION)) \
    + IA_CSS_UINT16_T_BITS \
    + IA_CSS_UINT8_T_BITS \
    + (4*IA_CSS_UINT8_T_BITS))

/** Meta-data specific for a data terminal
 *
 * Contains mostly capability and constraints information
 */
struct ia_css_data_terminal_manifest_s {
    /** Data terminal base */
    ia_css_terminal_manifest_t base;
    /** Supported (4CC / MIPI / parameter) formats */
    ia_css_frame_format_bitmap_t frame_format_bitmap;
    /** Indicate which kernels lead to this terminal being used */
    ia_css_kernel_bitmap_t kernel_bitmap;
    /** Minimum size of the frame */
    uint16_t min_size[IA_CSS_N_DATA_DIMENSION];
    /** Maximum size of the frame */
    uint16_t max_size[IA_CSS_N_DATA_DIMENSION];
    /** Minimum size of a fragment that the program port can accept */
    uint16_t min_fragment_size[IA_CSS_N_DATA_DIMENSION];
    /** Maximum size of a fragment that the program port can accept */
    uint16_t max_fragment_size[IA_CSS_N_DATA_DIMENSION];
    /** Indicate if this terminal is derived from a principal terminal
     *  @note Can't find meaningful use of this field. */
    uint16_t terminal_dependency;
    /** Indicate what (streaming) interface types this terminal supports */
    ia_css_connection_bitmap_t connection_bitmap;
    /** Indicates if compression is supported on the data associated with
     * this terminal. '1' indicates compression is supported,
     * '0' otherwise
     */
    uint8_t compression_support;
    uint8_t reserved[4];
};

/* ============  Program Control Init Terminal Manifest - START ============ */
#define N_PADDING_UINT8_IN_PROGCTRLINIT_MANIFEST_PROGRAM_DESC_STRUCT 4
struct ia_css_program_control_init_manifest_program_desc_s {
    uint16_t load_section_count;
    uint16_t connect_section_count;
    uint8_t padding[N_PADDING_UINT8_IN_PROGCTRLINIT_MANIFEST_PROGRAM_DESC_STRUCT];
};

#define N_PADDING_UINT8_IN_PROGCTRLINIT_TERMINAL_MANIFEST_STRUCT 2
/** Meta-data specific for a program control init terminal
 *
 * Program control init terminals have parameters related
 * to system device (i.e. non-algorithmic devices, like DMA's)
 * configuration.
 */
struct ia_css_program_control_init_terminal_manifest_s {
    ia_css_terminal_manifest_t base;
    /** Number of programs in program group.  This terminal can contain
     * parameters for each program.  Also size of the array at program_desc_offset.
    */
    uint32_t program_count;
    /**
     * Points to array of ia_css_program_control_init_manifest_program_desc_t
     * with size program_count.  The descriptor describes the format of the
     * parameter payload, which is sent separately.
     */
    uint16_t program_desc_offset;
    /** align to 64 */
    uint8_t padding[N_PADDING_UINT8_IN_PROGCTRLINIT_TERMINAL_MANIFEST_STRUCT];
};
/* ============  Program Control Init Terminal Manifest - END ============ */

extern void ia_css_program_manifest_init(
    ia_css_program_manifest_t    *blob,
    const uint8_t    program_needs_extension,
    const uint8_t    program_dependency_count,
    const uint8_t    terminal_dependency_count);

/** @} */

#endif /* __IA_CSS_PSYS_PROGRAM_GROUP_PRIVATE_H */
