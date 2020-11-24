/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __MEM_PTR_PRIVATE_H
#define __MEM_PTR_PRIVATE_H

#ifndef PIPE_GENERATION
#include <stddef.h>
#include <stdbool.h>
#include <assert_support.h>
#include "ipu_device_cell_properties.h"
#include "ipu_device_databus_properties.h"
#include "system_global.h"
#include "misc_support.h"
#include "storage_class.h"
#include "mem_ptr_global.h"
#endif /* not PIPE_GENERATION*/

/****************************************************************************/
/* Private function implementations */
/****************************************************************************/

#ifndef PIPE_GENERATION

/**
 * @brief Get the databus address for the given pointer.
 * It consists of:
 * - The base offset of the memory, using the given lookup table
 * - The offset field of the mem_ptr
 * - The address difference between the actual cell ID and ISP0, which
 *   depends on the runtime prog_id->cell_id mapping
 * - The offset for accessing bamem as a plane in block access mode
 */
STORAGE_CLASS_INLINE unsigned
mem_ptr_databus_memory_address(mem_ptr_t ptr, const unsigned baseaddr_lookup[])
{
	unsigned addr;

	/* Base address lookup using the given lut */
	assert(ptr.buf_mem < buf_mem_N);
	addr = baseaddr_lookup[ptr.buf_mem];
	assert(addr != MEM_PTR_INVALID_ADDRESS);

	/* Offset between actual cell ID and ISP0 */
	if (mem_ptr_is_dmem(ptr) || mem_ptr_is_vmem(ptr) || mem_ptr_is_bamem(ptr)) {
		assert(0);
		return 0;
		/* this functionality is not supported on the host */
	}

	/* Pointer offset or bamem plane offset */
	if (mem_ptr_is_bmplane(ptr)) {
		assert(0);
		return 0;
		/* this functionality is not supported on the host */
	} else {
		addr += ptr.buf_offset;
	}
	return addr;
}

/**
 * @brief Get the address of ptr, as seen from the DMAs in the transfer IP
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_dma(mem_ptr_t ptr)
{
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_dma);
}

/**
 * @brief Get the address of ptr, as seen from the DMA internal in the transfer IP
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_dma_int(mem_ptr_t ptr)
{
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_dma_int);
}

/**
 * @brief Get the databus address of ptr, as seen from the GDC
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_gdc(mem_ptr_t ptr)
{
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_gdc);
}

/**
 * @brief Get the databus address of ptr, as seen from the OFS
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_ofs(mem_ptr_t ptr)
{
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_ofs);
}

/**
 * @brief Get the databus address of ptr, as seen from the S2V
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_s2v(mem_ptr_t ptr)
{
	/* Address for str2vec is given in vectors iso bytes. See also
	 * https://hsdes.intel.com/home/default.html/article?id=1804328890
	 */
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_lb) / VECTOR_SIZE_IN_BYTES;
}

/**
 * @brief Get the databus address of ptr, as seen from the V2S
 */
STORAGE_CLASS_INLINE uint32_t
get_start_address_for_v2s(mem_ptr_t ptr)
{
	return mem_ptr_databus_memory_address(ptr, baseaddr_lookup_lb);
}

#endif /* not PIPE_GENERATION */
#endif /* __MEM_PTR_PRIVATE_H */
