/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2023 Intel Corporation.
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

#ifndef __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H
#define __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H

/*max sizes*/
#define VIED_NCI_DEV_DFM_LB_EMPTY_PORT_ID_MAX 	5
#define VIED_NCI_N_DEV_DFM_ID_MAX 		(VIED_NCI_DEV_DFM_LB_EMPTY_PORT_ID_MAX+1)
#define VIED_NCI_N_DATA_MEM_TYPE_ID_MAX 	6
#define VIED_NCI_DEV_CHN_DMA_ISA_ID_MAX 	4
#define VIED_NCI_N_DEV_CHN_ID_MAX 		(VIED_NCI_DEV_CHN_DMA_ISA_ID_MAX+1)

#define IA_CSS_MAX_INPUT_DEC_RESOURCES		4
#define IA_CSS_MAX_OUTPUT_DEC_RESOURCES		4

#define IA_CSS_PROCESS_MAX_CELLS		1

/* The amount of padding bytes needed to make
 * ia_css_process_s/ia_css_process_ext_s/
 * ia_css_program_manifest_s/ia_css_program_manifest_ext_s
 * structures 32 bit aligned (source files have check on this)
 */
#define N_PADDING_UINT8_IN_PROCESS_EXT_STRUCT 	0
#define	N_PADDING_UINT8_IN_PROCESS_STRUCT	0
#define N_PADDING_UINT8_IN_PROGRAM_MANIFEST_EXT 2
#define	N_PADDING_UINT8_IN_PROGRAM_MANIFEST	0

#endif /* __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H */
