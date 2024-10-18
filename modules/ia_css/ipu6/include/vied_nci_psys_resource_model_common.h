/*
 * Copyright (C) 2023 Intel Corporation.
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

#ifndef __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H
#define __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H

/*max sizes*/
#define VIED_NCI_DEV_DFM_LB_EMPTY_PORT_ID_MAX     5
#define VIED_NCI_N_DEV_DFM_ID_MAX         (VIED_NCI_DEV_DFM_LB_EMPTY_PORT_ID_MAX+1)
#define VIED_NCI_N_DATA_MEM_TYPE_ID_MAX     6
#define VIED_NCI_DEV_CHN_DMA_ISA_ID_MAX     4
#define VIED_NCI_N_DEV_CHN_ID_MAX         (VIED_NCI_DEV_CHN_DMA_ISA_ID_MAX+1)

#define IA_CSS_MAX_INPUT_DEC_RESOURCES        4
#define IA_CSS_MAX_OUTPUT_DEC_RESOURCES        4

#define IA_CSS_PROCESS_MAX_CELLS        1

/* The amount of padding bytes needed to make
 * ia_css_process_s/ia_css_process_ext_s/
 * ia_css_program_manifest_s/ia_css_program_manifest_ext_s
 * structures 32 bit aligned (source files have check on this)
 */
#define N_PADDING_UINT8_IN_PROCESS_EXT_STRUCT     0
#define    N_PADDING_UINT8_IN_PROCESS_STRUCT    0
#define N_PADDING_UINT8_IN_PROGRAM_MANIFEST_EXT 2
#define    N_PADDING_UINT8_IN_PROGRAM_MANIFEST    0

#endif /* __VIED_NCI_PSYS_RESOURCE_MODEL_COMMON_H */
