/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2017 Intel Corporation.
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
#ifndef _entry_regmem_h_
#define _entry_regmem_h_

/* Generated with DAI generator 1.3.0 */

/* enumerate for all memories for devices of type entry */
typedef enum {
	NUM_DAI_ENTRY_MEMORY_ID = 0
} dai_entry_memory_id_t;

/* enumerate for all register bank types for devices of type entry */
typedef enum {
	NUM_DAI_ENTRY_RB_TYPES = 0
} dai_entry_rb_types_t;

/* enumerate for all registers for slave port cio_m_upstream for devices of type entry */
typedef enum {
	NUM_DAI_ENTRY_CIO_M_UPSTREAM_REGISTERS = 0
} dai_entry_cio_m_upstream_registers_t;

/* enumerate for all registers for slave port is_npk for devices of type entry */
typedef enum {
	NUM_DAI_ENTRY_IS_NPK_REGISTERS = 0
} dai_entry_is_npk_registers_t;

/* enumerate for all registers for slave port mt_butt_npk for devices of type entry */
typedef enum {
	NUM_DAI_ENTRY_MT_BUTT_NPK_REGISTERS = 0
} dai_entry_mt_butt_npk_registers_t;

#endif /* _entry_regmem_h_ */
