/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2018 Intel Corporation.
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

#ifndef __SYSTEM_GLOBAL_H
#define __SYSTEM_GLOBAL_H

#ifndef PIPE_GENERATION
#include <type_support.h>
#if HAS_ISP0
#include <idsp_params.h>
#else
#define ISP_NWAY_LOG2	5
#define ISP_VEC_NELEMS	(1<<ISP_NWAY_LOG2)
#define ISP_NWAY	ISP_VEC_NELEMS
#define ISP_BAMEM_NWAYS	64
#endif
#include "ipu_device_gdc_properties_defs.h"
#endif /* PIPE_GENERATION */

#define HIVE_ISP_DDR_WORD_BITS (512)
#ifndef HIVE_ISP_DDR_WORD_BYTES
#define HIVE_ISP_DDR_WORD_BYTES  (HIVE_ISP_DDR_WORD_BITS/8)
#endif

#define HAS_TRANSFER_VMEM	1
#define HAS_TRANSFER_VMEM1	1
#define HAS_DMA_INTERNAL	1
#define HAS_BB				1
#define HAS_GDC_VMEM		1
#define HAS_OFS_VMEM		1
#define HAS_LB_VMEM			1
#define HAS_ISL_VMEM		1
#define HAS_TNR_INPUT_VMEM	1
#define HAS_TNR_REF_VMEM	1

#if HAS_ISP0
typedef enum {
	ISP0_ID = 0,
} isp_ID_t;

#define N_ISP_ID (ISP0_ID + 1)
#else
#define N_ISP_ID (0)
#endif

typedef enum {
	GDC0_ID = 0,
	GDC1_ID,
	N_GDC_ID
} gdc_ID_t;

typedef enum {
	dev_chn, /* device channels, external resource */
	ext_mem, /* external memories */
	int_mem, /* internal memories */
	int_chn, /* internal channels, user defined */
	dfm_port /* device port, external resource */
} resource_type_t;

/* if this enum is extended with other memory resources, pls also extend the function resource_to_memptr() */
typedef enum {
	vied_nci_dev_chn_dma_ext0,		/* LB */
	vied_nci_dev_chn_dma_ext1_read,		/* HBfrX */
	vied_nci_dev_chn_dma_ext1_write,	/* HBtX */
	vied_nci_dev_chn_dma_internal,
	vied_nci_dev_chn_dma_isa,
} resource_id_t;
#define resource_id_N (vied_nci_dev_chn_dma_isa + 1)

#define buf_mem_isl_vmem vied_nci_isl_vmem_type
#define buf_mem_lb_vmem vied_nci_lb_vmem_type
#define buf_mem_transfer_vmem0 vied_nci_transfer_vmem0_type
#define buf_mem_transfer_vmem1 vied_nci_transfer_vmem1_type

/* enum listing the different memories within a program group.
 * This enum is used in the mem_ptr_t type.
 * Be aware that this enum is also used to index LUT's
 */
typedef enum {
	buf_mem_invalid = 0,
	buf_mem_dmem_prog0,
	buf_mem_vmem_prog0,
	buf_mem_bamem_prog0,
	buf_mem_dmem_tserver,
#if HAS_TRANSFER_VMEM
	buf_mem_transfer_vmem0,
	buf_mem_transfer_vmem1,
#endif /* HAS_TRANSFER_VMEM */
#if HAS_GDC_VMEM
	buf_mem_gdc_vmem,
#endif /* HAS_GDC_VMEM */
#if HAS_OFS_VMEM
	buf_mem_ofs_vmem,
#endif /* HAS_OFS_VMEM */
#if HAS_TNR_INPUT_VMEM
	buf_mem_tnr_input_vmem,
#endif
#if HAS_TNR_REF_VMEM
	buf_mem_tnr_ref_vmem,
#endif
#if HAS_LB_VMEM
	buf_mem_lb_vmem,
#endif /* HAS_LB_VMEM */
#if HAS_ISL_VMEM
	buf_mem_isl_vmem,
#endif /* HAS_ISL_VMEM */
	buf_mem_reg,
} buf_mem_t;

#define buf_mem_N (buf_mem_reg + 1)

#define BYTES_PER_ELEMENT 2
#define VECTOR_SIZE_IN_BYTES ((ISP_NWAY) * (BYTES_PER_ELEMENT))
#define DMEM_WORD_SIZE_IN_BYTES 4

#define GDC_VMEM_BASE_DMA_M0        IPU_DEVICE_BBGDC_GDC_VMEM_BASE_DMA_M0
#define REGISTER_MAP_BASE_DMA_M0    IPU_DEVICE_BBGDC_REGISTER_MAP_BASE_DMA_M0

#define REG_SIZE_BYTES (1)

/* TODO: Variable NWAY */
#define ISP_NWAY_LOG2                   5

#ifdef PIPE_GENERATION
#define PIPEMEM(x) MEM(x)

/* TODO: This may need modification for variable NWAY
 * Identifiers needed by genpipe are declared here:
 */
#define ISP_NWAY                        (1<<ISP_NWAY_LOG2)
#else
#define PIPEMEM(x)
#endif

/* The number of data bytes in a vector disregarding the reduced precision */
#define ISP_VEC_ELEMBITS                16
#define ISP_VEC_BYTES                   (ISP_VEC_NELEMS*sizeof(uint16_t))
#define ISP_DMEM_WORD_SIZE_IN_BYTES     4

#define BAMEM_ELEMS_PER_ISP_ELEM (ISP_BAMEM_NWAYS/ISP_NWAY)

#endif /* __SYSTEM_GLOBAL_H */
