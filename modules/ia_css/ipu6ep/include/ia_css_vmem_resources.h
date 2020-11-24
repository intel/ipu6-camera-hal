/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2020 Intel Corporation.
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

#ifndef __IA_CSS_VMEM_RESOURCES_H
#define __IA_CSS_VMEM_RESOURCES_H

#include "system_global.h"

/* Sizes and offsets are given in vectors */

#define VMEM_SECTION_ISLPS_INPUT_MEM_ID     buf_mem_lb_vmem
#define VMEM_SECTION_ISLPS_INPUT_OFFSET     0
#define VMEM_SECTION_ISLPS_INPUT_SIZE       384

#define VMEM_SECTION_ISLPS_PAF_INPUT_MEM_ID buf_mem_lb_vmem
#define VMEM_SECTION_ISLPS_PAF_INPUT_OFFSET 384
#define VMEM_SECTION_ISLPS_PAF_INPUT_SIZE   96

#define VMEM_SECTION_LBPS_INPUT_MEM_ID      buf_mem_lb_vmem
#define VMEM_SECTION_LBPS_INPUT_OFFSET      480
#define VMEM_SECTION_LBPS_INPUT_SIZE        544

/*
 * TODO: Fix the following offsets for concurrent use-case as per HAS.
 * Currently concurrent execution of ISA/PSA/BB won't work.
 */

/* TNR ref out Y/UV/rec sim*/
#define VMEM_SECTION_BBPS_TNR_REFOUT_MEM_ID	buf_mem_transfer_vmem0
#define VMEM_SECTION_BBPS_TNR_REFOUT_OFFSET	0
#define VMEM_SECTION_BBPS_TNR_REFOUT_SIZE	400

/* Main/Display/PP output Y/UV */
#define VMEM_SECTION_BBPS_OFA_OUTPUT_MEM_ID    	buf_mem_transfer_vmem1
#define VMEM_SECTION_BBPS_OFA_OUTPUT_OFFSET    	0
#define VMEM_SECTION_BBPS_OFA_OUTPUT_SIZE      	1152

/* ISL_PS (SLIM) output sizes (in vectors) */
/* Full resolution + 2xDS + IR/MD */
#define VMEM_SECTION_ISLPS_OUTPUT_MEM_ID	buf_mem_transfer_vmem0
#define VMEM_SECTION_ISLPS_OUTPUT_OFFSET	0
#define VMEM_SECTION_ISLPS_OUTPUT_SIZE		1760

/* ISA2PSA output sizes (in vectors) */
/* 2xSIS + IR/MD */
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT_MEM_ID	buf_mem_transfer_vmem1
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT_OFFSET	(VMEM_SECTION_BBPS_OFA_OUTPUT_OFFSET + VMEM_SECTION_BBPS_OFA_OUTPUT_SIZE)
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT_SIZE	384

#define VMEM_SECTION_ISA2PSA_MEM_ID	(buf_mem_lb_vmem)
#define VMEM_SECTION_ISA2PSA_OFFSET	(0)
#define VMEM_SECTION_ISA2PSA_SIZE	(336 * 3) /* We use triple buffer, where each buffer contain 336 vectors */

/* ISA full (video/still) */
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT1_MEM_ID		buf_mem_transfer_vmem0
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT1_OFFSET		(VMEM_SECTION_BBPS_TNR_REFOUT_OFFSET + VMEM_SECTION_BBPS_TNR_REFOUT_SIZE)
#define VMEM_SECTION_ISA2PSA_ISA_OUTPUT1_SIZE		768

/*PSA Pre IEFD YUV + BNLM Noise (in vectors) */
#define VMEM_SECTION_ISA2PSA_LB_OUTPUT1_MEM_ID		buf_mem_transfer_vmem0
#define VMEM_SECTION_ISA2PSA_LB_OUTPUT1_OFFSET		0 /* mutex with TNR ref*/
#define VMEM_SECTION_ISA2PSA_LB_OUTPUT1_SIZE		960

/*PSA output YUV  (in vectors) */
#define VMEM_SECTION_LBPS_OUTPUT_MEM_ID		buf_mem_transfer_vmem0
#define VMEM_SECTION_LBPS_OUTPUT_OFFSET		(VMEM_SECTION_ISA2PSA_ISA_OUTPUT1_OFFSET + VMEM_SECTION_ISA2PSA_ISA_OUTPUT1_SIZE)
#define VMEM_SECTION_LBPS_OUTPUT_SIZE		576

#endif /* __IA_CSS_VMEM_RESOURCES_H */
