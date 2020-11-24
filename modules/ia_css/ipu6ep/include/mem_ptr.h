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

#ifndef __MEM_PTR_H
#define __MEM_PTR_H

/*
 * This file is included on every cell {SP,ISP,host} and on every system
 * that uses the ISP cell. It defines the API to DLI bridge
 *
 * System and cell specific interfaces and inline code are included
 * conditionally through Makefile path settings.
 *
 * 	- .        system and cell agnostic interfaces, constants and identifiers
 *	- public:  system agnostic, cell specific interfaces
 *	- private: system dependent, cell specific interfaces & inline implementations
 *	- global:  system specific constants and identifiers
 *	- local:   system and cell specific constants and identifiers
 */

#include <system_global.h> /* buf_mem_t */
#include <storage_class.h> /* __register */

/** Defines the mem_ptr_t type data structure
 */
typedef __register struct {
	unsigned  plane_id;          /**< ID number of allocated BAMEM plane, otherwise this field is kept unused*/
	unsigned  buf_offset;        /**< Offset within a memory*//*TODO: remove "buff_" prefix*/
	buf_mem_t buf_mem;           /**< Memory*//*TODO: remove "buff_" prefix*/
} mem_ptr_t;

typedef struct {
	unsigned  plane_id;          /**< ID number of allocated BAMEM plane, otherwise this field is kept unused*/
	unsigned  buf_offset;        /**< Offset within a memory*//*TODO: remove "buff_" prefix*/
	buf_mem_t buf_mem;           /**< Memory*//*TODO: remove "buff_" prefix*/
} mem_ptr_t_mem;

#ifndef __INLINE_MEM_PTR__
#define STORAGE_CLASS_MEM_PTR_H STORAGE_CLASS_EXTERN
#define STORAGE_CLASS_MEM_PTR_C
#else  /* __INLINE_MEM_PTR__ */
#define STORAGE_CLASS_MEM_PTR_H STORAGE_CLASS_INLINE
#define STORAGE_CLASS_MEM_PTR_C STORAGE_CLASS_INLINE
#endif /* __INLINE_MEM_PTR__ */

/****************************************************************************/
/* Public interface */
/****************************************************************************/

STORAGE_CLASS_MEM_PTR_H mem_ptr_t mem_ptr_create(unsigned offset, buf_mem_t mem);
STORAGE_CLASS_MEM_PTR_H mem_ptr_t mem_ptr_create_plane(unsigned plane_id, buf_mem_t mem);
STORAGE_CLASS_MEM_PTR_H mem_ptr_t mem_ptr_create_plane_with_offset(unsigned plane_id, buf_mem_t mem, unsigned offset);

STORAGE_CLASS_MEM_PTR_H mem_ptr_t mem_ptr_add(mem_ptr_t ptr, unsigned offset);

STORAGE_CLASS_MEM_PTR_H void store_mem_ptr_t(mem_ptr_t_mem *out, mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H mem_ptr_t load_mem_ptr_t(mem_ptr_t_mem *in);

STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_valid_mem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_bmplane(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_dmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_bamem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_isp_dmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_isp_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_isp_bamem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_tserver_dmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_lb_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_isl_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_gdc_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_ofs_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_tnr_input_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_tnr_ref_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_transfer_vmem(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H bool mem_ptr_is_reg(mem_ptr_t ptr);

STORAGE_CLASS_MEM_PTR_H buf_mem_t get_mem_ptr_mem(mem_ptr_t ptr);

STORAGE_CLASS_MEM_PTR_H unsigned get_mem_ptr_offset(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H unsigned mem_ptr_get_plane_id(mem_ptr_t ptr);

STORAGE_CLASS_MEM_PTR_H unsigned get_dmem_ptr(mem_ptr_t ptr);
STORAGE_CLASS_MEM_PTR_H unsigned mem_ptr_get_nway(mem_ptr_t ptr);

STORAGE_CLASS_MEM_PTR_H unsigned get_mem_unit_size_in_byte(mem_ptr_t m);

#include "mem_ptr_local.h"
#include "mem_ptr_public.h"
#if defined(__INLINE_MEM_PTR__) && !defined(PIPE_GENERATION)
#include "mem_ptr.c"
#include "mem_ptr_private.h"
#endif /* __INLINE_MEM_PTR__ */

#endif /* __MEM_PTR_H */
