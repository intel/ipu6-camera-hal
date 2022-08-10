/*
 * Copyright (C) 2021 Intel Corporation.
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

#ifndef __VIED_NCI_PSYS_RESOURCE_MODEL_H
#define __VIED_NCI_PSYS_RESOURCE_MODEL_H

#include "type_support.h"
#include "storage_class.h"
#include "vied_nci_psys_resource_model_common.h"

#define HAS_DFM                    1
#define HAS_DMA_INTERNAL        0
#define NON_RELOC_RESOURCE_SUPPORT    1
#define IA_CSS_KERNEL_BITMAP_BITS     128

/* Defines for the routing bitmap in the program group manifest.
 */
#define VIED_NCI_RBM_MAX_MUX_COUNT            60
#define VIED_NCI_RBM_MAX_VALIDATION_RULE_COUNT        27
#define VIED_NCI_RBM_MAX_TERMINAL_DESC_COUNT        4
#define N_PADDING_UINT8_IN_RBM_MANIFEST            2

/*
 * Cell IDs
 */
typedef enum {
    VIED_NCI_SP0_ID = 0,
    VIED_NCI_ISA_ICA_ID,
    VIED_NCI_ISA_LSC_ID,
    VIED_NCI_ISA_DPC_ID,
    VIED_NCI_ISA_B2B_ID,
#if defined(IPU_SYSVER_ipu6v2)
    VIED_NCI_ISA_B2R_R2I_SIE_ID,
#elif defined(IPU_SYSVER_ipu6v3) || defined(IPU_SYSVER_ipu6v4)
    VIED_NCI_ISA_BNLM_ID,
    VIED_NCI_ISA_DM_ID,
    VIED_NCI_ISA_R2I_SIE_ID,
#else
    #error assert 0
#endif
    VIED_NCI_ISA_R2I_DS_A_ID,
    VIED_NCI_ISA_R2I_DS_B_ID,
    VIED_NCI_ISA_AWB_ID,
    VIED_NCI_ISA_AE_ID,
    VIED_NCI_ISA_AF_ID,
    VIED_NCI_ISA_PAF_ID,
} vied_nci_cell_ID_t;

#define VIED_NCI_N_CELL_ID (VIED_NCI_ISA_PAF_ID + 1)

/*
 * In several places we don't need to size the look up tables for the full amount of cells
 * because they are only used for the VP and SP cells and not for the accelerator cells.
 * It is important that these cells are in the beginning of the LUT
 */
#define VIED_NCI_N_PROG_CELLS (VIED_NCI_SP0_ID + 1)

/*
 * Barrier bits (to model process group dependencies)
 * todo verify that this is valid for ipu6
 */
typedef enum {
    VIED_NCI_BARRIER0_ID,
    VIED_NCI_BARRIER1_ID,
    VIED_NCI_BARRIER2_ID,
    VIED_NCI_BARRIER3_ID,
    VIED_NCI_BARRIER4_ID,
    VIED_NCI_BARRIER5_ID,
    VIED_NCI_BARRIER6_ID,
    VIED_NCI_BARRIER7_ID
} vied_nci_barrier_ID_t;

#define VIED_NCI_N_BARRIER_ID (VIED_NCI_BARRIER7_ID + 1)
/*
 * Cell types
 */
typedef enum {
    VIED_NCI_SP_CTRL_TYPE_ID = 0,
    VIED_NCI_SP_SERVER_TYPE_ID,
    VIED_NCI_ACC_ISA_TYPE_ID,
    VIED_NCI_N_CELL_TYPE_ID
} vied_nci_cell_type_ID_t;

/* Not used cell types */
#define VIED_NCI_ACC_PSA_TYPE_ID (VIED_NCI_N_CELL_TYPE_ID)
#define VIED_NCI_ACC_OSA_TYPE_ID (VIED_NCI_N_CELL_TYPE_ID)
#define VIED_NCI_GDC_TYPE_ID (VIED_NCI_N_CELL_TYPE_ID)

/*
 * Memory IDs
 */
typedef enum {
    VIED_NCI_TRANSFER_VMEM0_ID = 0, /* TRANSFER VMEM 0 */
    VIED_NCI_LB_VMEM_ID, /* LB VMEM */
    VIED_NCI_DMEM0_ID, /* SPC0 Dmem */
    VIED_NCI_DMEM1_ID /* SPP0 Dmem */
} vied_nci_mem_ID_t;

#define VIED_NCI_N_MEM_ID (VIED_NCI_DMEM1_ID + 1)
/*
 * Memory types
 */
typedef enum {
    VIED_NCI_TRANSFER_VMEM0_TYPE_ID = 0,
    VIED_NCI_LB_VMEM_TYPE_ID,
    VIED_NCI_DMEM_TYPE_ID,
    VIED_NCI_VMEM_TYPE_ID,
    VIED_NCI_BAMEM_TYPE_ID,
    VIED_NCI_PMEM_TYPE_ID
} vied_nci_mem_type_ID_t;

#define VIED_NCI_N_MEM_TYPE_ID (VIED_NCI_PMEM_TYPE_ID + 1)
/******************************************************/

/* Excluding PMEM */
#define VIED_NCI_N_DATA_MEM_TYPE_ID     5 /* = MAX(vied_nci_mem_type_ID_t) */

#define VIED_NCI_N_SP_CTRL_MEM        2
#define VIED_NCI_N_SP_SERVER_MEM    2
#define VIED_NCI_N_VP_MEM            0
#define VIED_NCI_N_ACC_PSA_MEM        0
#define VIED_NCI_N_ACC_ISA_MEM        0
#define VIED_NCI_N_ACC_OSA_MEM        0
#define VIED_NCI_N_GDC_MEM            0
#define VIED_NCI_N_TNR_MEM            0

#define VIED_NCI_N_VP_CELL        0
#define VIED_NCI_N_ACC_CELL        13
/******************************************************/
/*
 * Device IDs
 */
typedef enum {
    VIED_NCI_DEV_CHN_DMA_EXT0_ID = 0, /*DMA lb*/
    VIED_NCI_DEV_CHN_DMA_EXT1_READ_ID, /*DMA hbfx*/
    VIED_NCI_DEV_CHN_DMA_EXT1_WRITE_ID, /*DMA hbtx*/
    VIED_NCI_DEV_CHN_DMA_ISA_ID,
} vied_nci_dev_chn_ID_t;

#define VIED_NCI_N_DEV_CHN_ID (VIED_NCI_DEV_CHN_DMA_ISA_ID + 1)

/*
 * DFM devices
 */
typedef enum {
    VIED_NCI_DEV_DFM_ISL_FULL_PORT_ID = 0,
    VIED_NCI_DEV_DFM_ISL_EMPTY_PORT_ID
} vied_nci_dev_dfm_id_t;

#define VIED_NCI_N_DEV_DFM_ID (VIED_NCI_DEV_DFM_ISL_EMPTY_PORT_ID + 1)

/*
 * Link IDs
 */
typedef enum {
    VIED_NCI_LINK_OTF_OFFLINE = 0,
    VIED_NCI_LINK_OTF_MAIN_OUTPUT,
    VIED_NCI_LINK_OTF_PDAF_OUTPUT,
    VIED_NCI_LINK_DATA_BARRIER_0,
    VIED_NCI_LINK_DATA_BARRIER_1,
    VIED_NCI_LINK_DATA_BARRIER_2,
    VIED_NCI_LINK_DATA_BARRIER_3,
    VIED_NCI_LINK_DATA_BARRIER_4,
    VIED_NCI_N_LINK_ID
} vied_nci_link_ID_t;

/*
 * Psys server local object caches space
 */
typedef enum {
    VIED_NCI_PSYS_SERVER_LOCAL_OBJECT_CACHE_0 = 0,
    VIED_NCI_PSYS_SERVER_LOCAL_OBJECT_CACHE_1,
    VIED_NCI_N_PSYS_SERVER_LOCAL_OBJECT_CACHES
} vied_nci_psys_server_local_object_cache_t;

/******************************************************/
/*todo verify  for ipu6*/
/*
 * Memory size (previously in vied_nci_psys_system.c)
 * VMEM: in words, 64 Byte per word.
 * TRANSFER VMEM0: in words, 64 Byte per word
 * LB VMEM: in words, 64 Byte per word.
 * DMEM: in words, 4 Byte per word.
 */
#define VIED_NCI_TRANSFER_VMEM0_WORD_SIZE     64
#define VIED_NCI_LB_VMEM_WORD_SIZE     64
#define VIED_NCI_DMEM_WORD_SIZE      4
#define VIED_NCI_VMEM_WORD_SIZE     64

/* ISP VMEM  words $HIVECORES/idsp/include/hive/cell_params.h*/
#define VIED_NCI_TRANSFER_VMEM0_MAX_SIZE    (0x0800) /* Transfer VMEM0 words, ref HAS Transfer*/
#define VIED_NCI_TRANSFER_VMEM1_MAX_SIZE    (0x0800) /* Transfer VMEM1 words, ref HAS Transfer*/
#define VIED_NCI_LB_VMEM_MAX_SIZE        (0x0400) /* LB VMEM words */
#define VIED_NCI_DMEM0_MAX_SIZE            (0x4000)
#define VIED_NCI_DMEM1_MAX_SIZE            (0x1000)

/*
 * Number of channels per device
 */
/*
 * NOTE: once FW code will support DMA with one unit
 * descriptor #channels will be cut but half (36->18).
*/

/* 3 defines below are inferred from pg/ipu6s_isa/src/ipu6s_isa.manifest.cpp */
/* plus some orbitrary number - a safety margin */

#define VIED_NCI_DEV_CHN_DMA_EXT0_MAX_SIZE            (22)
#define VIED_NCI_DEV_CHN_DMA_EXT1_READ_MAX_SIZE        (22)
#define VIED_NCI_DEV_CHN_DMA_EXT1_WRITE_MAX_SIZE    (22)

#define VIED_NCI_DEV_CHN_DMA_IPFD_MAX_SIZE        (0)
#define VIED_NCI_DEV_CHN_DMA_ISA_MAX_SIZE        (2)

/*
 * Number of ports per DFM device
 */
#define VIED_NCI_DEV_DFM_ISL_FULL_PORT_ID_MAX_SIZE        (32)
#define VIED_NCI_DEV_DFM_LB_FULL_PORT_ID_MAX_SIZE        (32)
#define VIED_NCI_DEV_DFM_ISL_EMPTY_PORT_ID_MAX_SIZE        (32)
#define VIED_NCI_DEV_DFM_LB_EMPTY_PORT_ID_MAX_SIZE        (32)
/******************************************************/

/*
 * Storage of the resource and resource type enumerators
 */
#define VIED_NCI_RESOURCE_ID_BITS    8
typedef uint8_t                vied_nci_resource_id_t;

#define VIED_NCI_RESOURCE_SIZE_BITS    16
typedef uint16_t            vied_nci_resource_size_t;

#define VIED_NCI_RESOURCE_BITMAP_BITS    32
typedef uint32_t            vied_nci_resource_bitmap_t;

#define IA_CSS_PROCESS_INVALID_DEPENDENCY    ((vied_nci_resource_id_t)(-1))
#define IA_CSS_PROCESS_INVALID_OFFSET        ((vied_nci_resource_size_t)(-1))

/*
 * Resource specifications
 * Note that the FAS uses the terminology local/remote memory. In the PSYS API,
 * these are called internal/external memory.
 */

/* resource spec for internal (local) memory */
struct vied_nci_resource_spec_int_mem_s {
    vied_nci_resource_id_t        type_id;
    vied_nci_resource_size_t    size;
    vied_nci_resource_size_t    offset;
};

typedef struct vied_nci_resource_spec_int_mem_s
    vied_nci_resource_spec_int_mem_t;

/* resource spec for external (remote) memory */
struct vied_nci_resource_spec_ext_mem_s {
    vied_nci_resource_id_t        type_id;
    vied_nci_resource_size_t    size;
    vied_nci_resource_size_t    offset;
};

typedef struct vied_nci_resource_spec_ext_mem_s
    vied_nci_resource_spec_ext_mem_t;

/* resource spec for device channel */
struct vied_nci_resource_spec_dev_chn_s {
    vied_nci_resource_id_t        type_id;
    vied_nci_resource_size_t    size;
    vied_nci_resource_size_t    offset;
};

typedef struct vied_nci_resource_spec_dev_chn_s
    vied_nci_resource_spec_dev_chn_t;

/* resource spec for DFM port */
struct vied_nci_resource_spec_dfm_port_s {
    vied_nci_resource_id_t        type_id;
    vied_nci_resource_bitmap_t    bitmask;
};

typedef struct vied_nci_resource_spec_dfm_port_s
    vied_nci_resource_spec_dfm_port_t;

/* resource spec for all contiguous resources */
struct vied_nci_resource_spec_s {
    vied_nci_resource_spec_int_mem_t int_mem[VIED_NCI_N_MEM_TYPE_ID];
    vied_nci_resource_spec_ext_mem_t ext_mem[VIED_NCI_N_DATA_MEM_TYPE_ID];
    vied_nci_resource_spec_dev_chn_t dev_chn[VIED_NCI_N_DEV_CHN_ID];
};

typedef struct vied_nci_resource_spec_s vied_nci_resource_spec_t;

#ifndef PIPE_GENERATION

extern const uint8_t /* vied_nci_cell_type_ID_t */ vied_nci_cell_type[VIED_NCI_N_CELL_ID];
extern const uint8_t /* vied_nci_mem_type_ID_t */ vied_nci_mem_type[VIED_NCI_N_MEM_ID];
extern const uint16_t vied_nci_N_cell_mem[VIED_NCI_N_CELL_TYPE_ID];
extern const uint8_t /* vied_nci_mem_type_ID_t */
    vied_nci_cell_mem_type[VIED_NCI_N_CELL_TYPE_ID][VIED_NCI_N_MEM_TYPE_ID];
extern const uint8_t /* vied_nci_mem_ID_t */
    vied_nci_ext_mem[VIED_NCI_N_MEM_TYPE_ID];
extern const uint8_t /* vied_nci_mem_ID_t */
    vied_nci_cell_mem[VIED_NCI_N_CELL_ID][VIED_NCI_N_MEM_TYPE_ID];
extern const uint16_t vied_nci_mem_size[VIED_NCI_N_MEM_ID];
extern const uint16_t vied_nci_mem_word_size[VIED_NCI_N_DATA_MEM_TYPE_ID];
extern const uint16_t vied_nci_dev_chn_size[VIED_NCI_N_DEV_CHN_ID];
extern const uint16_t vied_nci_dfm_port_size[VIED_NCI_N_DEV_DFM_ID];

STORAGE_CLASS_INLINE
uint32_t vied_nci_mem_is_ext_type(const vied_nci_mem_type_ID_t mem_type_id)
{
    return((mem_type_id == VIED_NCI_TRANSFER_VMEM0_TYPE_ID) ||
        (mem_type_id == VIED_NCI_LB_VMEM_TYPE_ID));
}

#endif /* PIPE_GENERATION */
#endif /* __VIED_NCI_PSYS_RESOURCE_MODEL_H */
