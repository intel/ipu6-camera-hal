/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __VIED_NCI_DMA_DEV_ACCESS_H
#define __VIED_NCI_DMA_DEV_ACCESS_H

#include "assert_support.h"
#include "bit_support.h"
#include "vied_nci_dma_global_defs.h"
#include "ia_css_cmem.h"

#include "ipu_device_subsystem_ids.h"
#include "ipu_device_dma_properties.h"
#include "vied_nci_dma_trace.h"

STORAGE_CLASS_INLINE void nci_dma_store_32(
	const uint32_t addr,
	const uint32_t val)
{
	IA_CSS_TRACE_2(NCI_DMA, DEBUG, "nci_dma_store_32: addr = 0x%08x val = 0x%08x\n", addr, val);
	ia_css_cmem_store_32(SSID, addr, val);
}

STORAGE_CLASS_INLINE uint32_t nci_dma_load_32(const uint32_t addr)
{
	const uint32_t val = ia_css_cmem_load_32(SSID, addr);

	IA_CSS_TRACE_2(NCI_DMA, DEBUG, "nci_dma_load_32: addr = 0x%08x val = 0x%08x\n", addr, val);
	return val;
}

STORAGE_CLASS_INLINE uint32_t nci_dma_desc_mem_addr(
	const uint32_t base_addr,
	const uint32_t id,
	const uint32_t desc_words)
{
	const uint32_t addr = base_addr + id * desc_words * NCI_DMA_CTRLM_DATA_BYTES;

	IA_CSS_TRACE_3(NCI_DMA, DEBUG, "nci_dma_desc_mem_addr: base_addr %d id %d desc_words %d\n",
		base_addr, id, desc_words);
	IA_CSS_TRACE_1(NCI_DMA, DEBUG, "nci_dma_desc_mem_addr: return 0x%08x\n", addr);
	return addr;
}

STORAGE_CLASS_INLINE void nci_dma_reg_store(
	const uint32_t addr,
	const uint32_t val)
{
	IA_CSS_TRACE_2(NCI_DMA, DEBUG, "nci_dma_reg_store: addr 0x%08x val 0x%08x\n", addr, val);
	nci_dma_store_32(addr, val);
}

STORAGE_CLASS_INLINE uint32_t nci_dma_reg_load(const uint32_t addr)
{
	const uint32_t val = nci_dma_load_32(addr);

	IA_CSS_TRACE_1(NCI_DMA, DEBUG, "nci_dma_reg_load: addr 0x%08x\n", addr);
	IA_CSS_TRACE_1(NCI_DMA, DEBUG, "nci_dma_reg_load: return 0x%08x\n", val);
	return val;
}

STORAGE_CLASS_INLINE uint32_t nci_dma_reg_addr_int(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id,
	const uint32_t bank,
	const uint32_t reg)
{
	const uint32_t int_base_addr = ipu_device_dma_int_base_address(dev_id);
	const uint32_t group_id_idx = ipu_device_dma_group_id_idx(dev_id);
	const uint32_t bank_id_idx = ipu_device_dma_bank_id_idx(dev_id);

	const uint32_t addr =
		(int_base_addr +					/* int base addr	*/
		((group_id << group_id_idx) + (bank << bank_id_idx)) +	/* bank addr	*/
		(reg * NCI_DMA_CTRLS_DATA_BYTES));			/* register addr	*/

	IA_CSS_TRACE_4(NCI_DMA, DEBUG, "nci_dma_reg_addr_int: dev_id %d group_id %d bank %d reg %d\n",
		dev_id, group_id, bank, reg);
	IA_CSS_TRACE_1(NCI_DMA, DEBUG, "nci_dma_reg_addr_int: return 0x%08x\n", addr);
	return addr;
}

STORAGE_CLASS_INLINE uint32_t nci_dma_reg_addr(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id,
	const uint32_t bank,
	const uint32_t reg)
{
	const uint32_t base_addr = ipu_device_dma_base_address(dev_id);
	const uint32_t group_id_idx = ipu_device_dma_group_id_idx(dev_id);
	const uint32_t bank_id_idx = ipu_device_dma_bank_id_idx(dev_id);

	const uint32_t addr =
		(base_addr +							/* base_addr		*/
		((group_id	<< group_id_idx) + (bank << bank_id_idx)) +	/* bank addr		*/
		(reg * NCI_DMA_CTRLS_DATA_BYTES));				/* register addr	*/
	IA_CSS_TRACE_4(NCI_DMA, DEBUG, "nci_dma_reg_addr: dev_id %d group_id %d bank %d reg %d\n",
		dev_id, group_id, bank, reg);
	IA_CSS_TRACE_1(NCI_DMA, DEBUG, "nci_dma_reg_addr: return 0x%08x\n", addr);
	return addr;
}

STORAGE_CLASS_INLINE uint32_t nci_dma_reg_addr_offset(const enum nci_dma_device_id dev_id,
						    const enum ipu_device_dma_group_id group_id,
						    const uint32_t bank,
						    const uint32_t reg)
{
	const uint32_t base_addr = ipu_device_dma_base_address(dev_id);
	return nci_dma_reg_addr(dev_id, group_id, bank, reg) - base_addr;
}

STORAGE_CLASS_INLINE void nci_dma_desc_pack(
	uint8_t **const pbuf,
	const uint32_t val,
	const uint32_t bits)
{
	const uint32_t bytes = bitpos_to_bytepos(bits);
	uint32_t i;

	assert(NULL != pbuf);
	for (i = 0; i < bytes; i++) {
		**pbuf = byte_slice(val, i, 1);
		++(*pbuf);
	}
}

STORAGE_CLASS_INLINE uint32_t nci_dma_desc_unpack(
	uint8_t **const pbuf,
	const uint32_t bits)
{
	const uint32_t bytes = bitpos_to_bytepos(bits);
	uint32_t i;
	uint32_t val = 0;

	assert(NULL != pbuf);
	for (i = 0; i < bytes; i++) {
		val = bit_lshift_OR(**pbuf, (i * 8), val);
		++(*pbuf);
	}

	return val;
}

#ifdef NCI_DMA_CMEM_CACHE
STORAGE_CLASS_INLINE void nci_dma_desc_word_store(
	uint32_t MEM(cmem) *addr,
	uint32_t data)
#else
STORAGE_CLASS_INLINE void nci_dma_desc_word_store(
	uint32_t *addr,
	uint32_t data)
#endif
{
#ifdef C_RUN
	ia_css_cmem_store_32(SSID, (ia_css_cmem_address_t)addr, data);
#else
	(*addr = data) NO_ALIAS;
#endif
}

#ifdef NCI_DMA_CMEM_CACHE
STORAGE_CLASS_INLINE uint32_t nci_dma_desc_word_load(
	uint32_t MEM(cmem) *addr)
#else
STORAGE_CLASS_INLINE uint32_t nci_dma_desc_word_load(
	uint32_t *addr)
#endif
{
#ifdef C_RUN
	return ia_css_cmem_load_32(SSID, (ia_css_cmem_address_t)addr);
#else
	return *addr;
#endif
}

STORAGE_CLASS_INLINE void nci_dma_desc_mem_store(
	const uint32_t addr,
	const uint32_t *buf,
	const uint32_t size)
{
	uint32_t i = 0;

	assert(NULL != buf);
	for (i = 0; i < size; i++) {
		nci_dma_store_32(addr + i * NCI_DMA_CTRLM_DATA_BYTES, *buf);
		buf++;
	}
}

STORAGE_CLASS_INLINE void nci_dma_desc_mem_load(
	const uint32_t addr,
	uint32_t *buf,
	const uint32_t size)
{
	uint32_t i = 0;

	assert(NULL != buf);
	for (i = 0; i < size; i++) {
		*buf = nci_dma_load_32(addr + i * NCI_DMA_CTRLM_DATA_BYTES);
		buf++;
	}
}

#endif /* __VIED_NCI_DMA_DEV_ACCESS_H */
