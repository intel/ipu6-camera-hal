/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __MEM_PTR_GLOBAL_H
#define __MEM_PTR_GLOBAL_H

#ifndef PIPE_GENERATION
#include <stddef.h>
#include <stdbool.h>
#include <assert_support.h>
#include "ipu_device_cell_properties.h"
#include "ipu_device_databus_properties.h"
#include "system_local.h"
#include "misc_support.h"
#include "storage_class.h"
#endif /* not PIPE_GENERATION*/

#define MEM_PTR_INVALID_ADDRESS  (0xFFFFFF)
#define MEM_PTR_INVALID_SEGMENT  (-1)
#define MEM_SEGMENT_TO_MSB_SHIFT (32-3)

/****************************************************************************/
/* Private function implementations */
/****************************************************************************/

/*
 * GENERAL IMPLEMENTATION NOTE:
 * Several mem_ptr query functions in this file use bitwise & or | instead of
 * boolean && or ||. This is an optimization to reduce hivecc compilation
 * speed and memory usage (HSD 1804189297). The effect is big due to inlining.
 * In general, it is NOT a recommended programming style.
 */

#ifndef PIPE_GENERATION

/* The initialization of the lookup tables has to match the enum declaration.
 * In order to get a compile error when an enum value gets renamed or removed,
 * we use this macro where we touch the enum itself. To get a compile error
 * when an enum value gets added, we test the size.
 */
#define ASSIGN2LUT(mem, val) (mem - mem + val)

/**
 * @brief Lookup table from memory type to cell memory ID.
 * This lookup is only valid for ISP dmem/vmem/bamem.
 */
static const int memoryid_lookup[] = {
	ASSIGN2LUT(buf_mem_invalid, 0),
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_CELL_DMEM),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_CELL_VMEM),
	ASSIGN2LUT(buf_mem_bamem_prog0, IPU_DEVICE_CELL_BAMEM),
	ASSIGN2LUT(buf_mem_dmem_tserver, IPU_DEVICE_CELL_DMEM),
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, 0),
	ASSIGN2LUT(buf_mem_transfer_vmem1, 0),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, 0),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, 0),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, 0),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, 0),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, 0),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, 0),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, 0)
};

/**
 * @brief Lookup table from memory type to the program ID.
 * This lookup is only valid for ISP dmem/vmem/bamem because other memories
 * do not belong to a single cell.
 */
static const int progid_lookup[] = {
	ASSIGN2LUT(buf_mem_invalid, 0),
	ASSIGN2LUT(buf_mem_dmem_prog0, 0),
	ASSIGN2LUT(buf_mem_vmem_prog0, 0),
	ASSIGN2LUT(buf_mem_bamem_prog0, 0),
	ASSIGN2LUT(buf_mem_dmem_tserver, 0),
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, 0),
	ASSIGN2LUT(buf_mem_transfer_vmem1, 0),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, 0),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, 0),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, 0),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, 0),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, 0),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, 0),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, 0)
};

/*
 * The base addresses for the various memories depend on the view point, i.e.
 * the master port that initiates the read/write. There is a base address
 * lookup table for each of the following origins:
 * - ISP
 * - DMA
 * - OFS
 * - GDC
 * - S2V (in LB.FF)
 * - ISL.PS
 *
 * The base addresses for ISP dmem/vmem/bamem depend on the runtime
 * prog_id->cell_id mapping. For ISP viewpoint, the tables contain zeros
 * because the HW segment table will do the address translation. For all
 * other views, the tables give the address of ISP0's memory. This must be
 * incremented with the actual cell ID multiplied by the address difference
 * between two ISPs.
 */

#ifdef HAS_ISP0
/**
 * @brief Memory base address lookup table from ISP viewpoint
 */
static const unsigned baseaddr_lookup_isp[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_dmem_prog0, 0),
	ASSIGN2LUT(buf_mem_vmem_prog0, 0),
	ASSIGN2LUT(buf_mem_bamem_prog0, 0),
	ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS), /* Write only */
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_ISP_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_ISP_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, IPU_DEVICE_DATABUS_ISP_TO_GDC_VMEM_ADDR),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, IPU_DEVICE_DATABUS_ISP_TO_OFS_VMEM_ADDR),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, IPU_DEVICE_TNR_INPMEM_ADDR),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, IPU_DEVICE_TNR_REFMEM_ADDR),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, IPU_DEVICE_DATABUS_ISP_TO_LB_VMEM_ADDR),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, IPU_DEVICE_DATABUS_ISP_TO_ISL_VMEM_ADDR),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, MEM_PTR_INVALID_ADDRESS)
};
#endif /* HAS_ISP0 */

/**
 * @brief Memory base address lookup table from DMA viewpoint
 */
static const unsigned baseaddr_lookup_dma[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
#if HAS_ISP0
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_DATABUS_DMA_TO_ISP_DMEM_ADDR),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_DATABUS_DMA_TO_ISP_VMEM_ADDR),
	ASSIGN2LUT(buf_mem_bamem_prog0, IPU_DEVICE_DATABUS_DMA_TO_ISP_BAMEM_ADDR),
#else
	ASSIGN2LUT(buf_mem_dmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_vmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_bamem_prog0, MEM_PTR_INVALID_ADDRESS),
#endif
	ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS),
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_DMA_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_DMA_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, IPU_DEVICE_DATABUS_DMA_TO_GDC_VMEM_ADDR),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, IPU_DEVICE_DATABUS_DMA_TO_OFS_VMEM_ADDR),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, IPU_DEVICE_TNR_INPMEM_ADDR),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, IPU_DEVICE_TNR_REFMEM_ADDR),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, IPU_DEVICE_DATABUS_DMA_TO_LB_VMEM_ADDR),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, IPU_DEVICE_DATABUS_DMA_TO_ISL_VMEM_ADDR),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, REGISTER_MAP_BASE_DMA_M0)
};

/**
 * @brief Memory base address lookup table from DMA internal viewpoint
 * refer to HSD 1804238408
 */
static const unsigned baseaddr_lookup_dma_int[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
#if HAS_ISP0
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_DATABUS_DMA_INT_TO_ISP_DMEM_ADDR),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_DATABUS_DMA_INT_TO_ISP_VMEM_ADDR),
	ASSIGN2LUT(buf_mem_bamem_prog0, IPU_DEVICE_DATABUS_DMA_INT_TO_ISP_BAMEM_ADDR),
#else
	ASSIGN2LUT(buf_mem_dmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_vmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_bamem_prog0, MEM_PTR_INVALID_ADDRESS),
#endif
	ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS), /* address is not defined in master to slave*/
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_DMA_INT_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_DMA_INT_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, IPU_DEVICE_DATABUS_DMA_INT_TO_GDC_VMEM_ADDR),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, IPU_DEVICE_DATABUS_DMA_INT_TO_OFS_VMEM_ADDR),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, IPU_DEVICE_TNR_INPMEM_ADDR),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, IPU_DEVICE_TNR_REFMEM_ADDR),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, IPU_DEVICE_DATABUS_DMA_INT_TO_LB_VMEM_ADDR),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, IPU_DEVICE_DATABUS_DMA_INT_TO_ISL_VMEM_ADDR),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, REGISTER_MAP_BASE_DMA_M0)
};

/**
 * @brief Memory base address lookup table from GDC viewpoint
 */
static const unsigned baseaddr_lookup_gdc[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
#if HAS_ISP0
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_DATABUS_GDC_TO_ISP_DMEM_ADDR),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_DATABUS_GDC_TO_ISP_VMEM_ADDR),
	ASSIGN2LUT(buf_mem_bamem_prog0, IPU_DEVICE_DATABUS_GDC_TO_ISP_BAMEM_ADDR),
#else
	ASSIGN2LUT(buf_mem_dmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_vmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_bamem_prog0, MEM_PTR_INVALID_ADDRESS),
#endif
	ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS),  /* no access */
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_GDC_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_GDC_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, IPU_DEVICE_DATABUS_GDC_TO_GDC_VMEM_ADDR),
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, IPU_DEVICE_DATABUS_GDC_TO_OFS_VMEM_ADDR),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, IPU_DEVICE_TNR_INPMEM_ADDR),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, IPU_DEVICE_TNR_REFMEM_ADDR),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, MEM_PTR_INVALID_ADDRESS)
};

/**
 * @brief Memory base address lookup table from OFS viewpoint
 */
static const unsigned baseaddr_lookup_ofs[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
#if HAS_ISP0
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_DATABUS_OFS_TO_ISP_DMEM_ADDR),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_DATABUS_OFS_TO_ISP_VMEM_ADDR),
#else
	ASSIGN2LUT(buf_mem_dmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_vmem_prog0, MEM_PTR_INVALID_ADDRESS),
#endif
	ASSIGN2LUT(buf_mem_bamem_prog0, 0),
	ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS),  /* no access */
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_OFS_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_OFS_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, IPU_DEVICE_DATABUS_OFS_TO_OFS_VMEM_ADDR),
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, IPU_DEVICE_TNR_INPMEM_ADDR),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, IPU_DEVICE_TNR_REFMEM_ADDR),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, MEM_PTR_INVALID_ADDRESS)
};

/**
 * @brief Memory base address lookup table from ISA and PSA viewpoint
 */
static const unsigned baseaddr_lookup_lb[] = {
	ASSIGN2LUT(buf_mem_invalid, MEM_PTR_INVALID_ADDRESS),
#if HAS_ISP0
	ASSIGN2LUT(buf_mem_dmem_prog0, IPU_DEVICE_DATABUS_LB_TO_ISP_DMEM_ADDR),
	ASSIGN2LUT(buf_mem_vmem_prog0, IPU_DEVICE_DATABUS_LB_TO_ISP_VMEM_ADDR),
	ASSIGN2LUT(buf_mem_bamem_prog0, IPU_DEVICE_DATABUS_LB_TO_ISP_BAMEM_ADDR),
#else
	ASSIGN2LUT(buf_mem_dmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_vmem_prog0, MEM_PTR_INVALID_ADDRESS),
	ASSIGN2LUT(buf_mem_bamem_prog0, MEM_PTR_INVALID_ADDRESS),
#endif
	 ASSIGN2LUT(buf_mem_dmem_tserver, MEM_PTR_INVALID_ADDRESS),  /* no access */
#if HAS_TRANSFER_VMEM
	ASSIGN2LUT(buf_mem_transfer_vmem0, IPU_DEVICE_DATABUS_LB_TO_TRANSFER_VMEM0_ADDR),
	ASSIGN2LUT(buf_mem_transfer_vmem1, IPU_DEVICE_DATABUS_LB_TO_TRANSFER_VMEM1_ADDR),
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	ASSIGN2LUT(buf_mem_gdc_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	ASSIGN2LUT(buf_mem_ofs_vmem, MEM_PTR_INVALID_ADDRESS), /* no access */
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	ASSIGN2LUT(buf_mem_tnr_input_vmem, MEM_PTR_INVALID_ADDRESS),
#endif /* HAS_TNR_INPUT_VMEM */
#if HAS_TNR_REF_VMEM
	ASSIGN2LUT(buf_mem_tnr_ref_vmem, MEM_PTR_INVALID_ADDRESS),
#endif /* HAS_TNR_REF_VMEM */
#if HAS_LB_VMEM
	ASSIGN2LUT(buf_mem_lb_vmem, IPU_DEVICE_DATABUS_LB_TO_LB_VMEM_ADDR),
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	ASSIGN2LUT(buf_mem_isl_vmem, IPU_DEVICE_DATABUS_LB_TO_ISL_VMEM_ADDR),
#endif /* HAS_ISL_VMEM */
	ASSIGN2LUT(buf_mem_reg, MEM_PTR_INVALID_ADDRESS)
};

/*
 * Dummy check function. Its only purpose is to trigger a compile error if one
 * of the lookup tables has the wrong size.
 */
STORAGE_CLASS_INLINE void
__dummy_check_mem_ptr_private_lookup(void)
{
	CT_ASSERT(sizeof(memoryid_lookup) / sizeof(int) == buf_mem_N);
	CT_ASSERT(sizeof(progid_lookup) / sizeof(int) == buf_mem_N);
#ifdef HAS_ISP0
	CT_ASSERT(sizeof(baseaddr_lookup_isp) / sizeof(int) == buf_mem_N);
#endif
	CT_ASSERT(sizeof(baseaddr_lookup_dma) / sizeof(int) == buf_mem_N);
	CT_ASSERT(sizeof(baseaddr_lookup_dma_int) / sizeof(int) == buf_mem_N);
	CT_ASSERT(sizeof(baseaddr_lookup_gdc) / sizeof(int) == buf_mem_N);
	CT_ASSERT(sizeof(baseaddr_lookup_ofs) / sizeof(int) == buf_mem_N);
	CT_ASSERT(sizeof(baseaddr_lookup_lb) / sizeof(int) == buf_mem_N);
}

#endif /* not PIPE_GENERATION */
#endif /* __MEM_PTR_GLOBAL_H */
