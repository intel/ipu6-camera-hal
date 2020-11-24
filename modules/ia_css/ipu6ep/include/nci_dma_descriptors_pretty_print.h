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

#ifndef __NCI_DMA_DECRIPTORS_PRETTY_PRINT_H
#define __NCI_DMA_DECRIPTORS_PRETTY_PRINT_H

#include "storage_class.h"
#include "misc_support.h"
#include "assert_support.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "nci_dma_descriptors_types.h"
#include "ia_css_trace.h"

#define NCI_DMA_DECRIPTORS_PRETTY_PRINT_TRACE_METHOD		IA_CSS_TRACE_METHOD_NATIVE
#define NCI_DMA_DECRIPTORS_PRETTY_PRINT_TRACE_LEVEL_DEBUG	IA_CSS_TRACE_LEVEL_DISABLED

STORAGE_CLASS_INLINE void nci_dma_print_hex(
	const uint32_t val)
{
	NOT_USED(val);
	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t val 0x%08x\n", val);
}

STORAGE_CLASS_INLINE void nci_dma_print_dev_id(
	const enum nci_dma_device_id dev_id)
{
	NOT_USED(dev_id);
	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t dev_id 0x%08x\n", dev_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_group_id(
	const enum ipu_device_dma_group_id group_id)
{
	NOT_USED(group_id);
	IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);
	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t group_id 0x%08x\n", group_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_bank_id(
	const enum nci_dma_device_id dev_id,
	const enum ipu_device_dma_group_id group_id,
	const uint32_t bank_id)
{
	NOT_USED(dev_id);
	NOT_USED(group_id);
	NOT_USED(group_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(N_IPU_DEVICE_DMA_GROUP_ID > (unsigned int)group_id);
	IA_CSS_ASSERT(IPU_DEVICE_DMA_CHANNEL_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_CHANNEL_GROUP_ID == group_id &&
		ipu_device_dma_channel_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_REQUEST_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_REQUEST_GROUP_ID == group_id &&
		ipu_device_dma_request_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_GLOBAL_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_GLOBAL_GROUP_ID == group_id &&
		ipu_device_dma_global_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_MASTER_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_MASTER_GROUP_ID == group_id &&
		ipu_device_dma_master_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_SPAN_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_SPAN_GROUP_ID == group_id &&
		ipu_device_dma_span_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_TERMINAL_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_TERMINAL_GROUP_ID == group_id &&
		ipu_device_dma_terminal_banks(dev_id) > bank_id));
	IA_CSS_ASSERT(IPU_DEVICE_DMA_UNIT_GROUP_ID != group_id ||
		(IPU_DEVICE_DMA_UNIT_GROUP_ID == group_id &&
		ipu_device_dma_unit_banks(dev_id) > bank_id));

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t bank_id 0x%08x\n", bank_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_master_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int master_id)
{
	NOT_USED(dev_id);
	NOT_USED(master_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_master_banks(dev_id) > master_id);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t master_id 0x%08x\n", master_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_request_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int request_id)
{
	NOT_USED(dev_id);
	NOT_USED(request_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_request_banks(dev_id) > request_id);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t request_id 0x%08x\n", request_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_global_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int global_id)
{
	NOT_USED(dev_id);
	NOT_USED(global_id);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_global_banks(dev_id) > global_id);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t global_id 0x%08x\n", global_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_channel_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int channel_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(channel_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_channels(dev_id) > channel_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t channel_id 0x%08x\n", channel_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_span_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int span_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(span_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_spans(dev_id) > span_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t span_id 0x%08x\n", span_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_terminal_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int terminal_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(terminal_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_terminals(dev_id) > terminal_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t terminal_id 0x%08x\n", terminal_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_unit_id(
	const enum nci_dma_device_id dev_id,
	const unsigned int unit_id,
	const enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(dev_id);
	NOT_USED(unit_id);
	NOT_USED(bank_mode);

	IA_CSS_ASSERT(NCI_DMA_NUM_DEVICES > (unsigned int)dev_id);
	IA_CSS_ASSERT(ipu_device_dma_units(dev_id) > unit_id);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);

	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t unit_id 0x%08x\n", unit_id);
}

STORAGE_CLASS_INLINE void nci_dma_print_bank_mode(enum nci_dma_bank_mode bank_mode)
{
	NOT_USED(bank_mode);
	IA_CSS_ASSERT(N_NCI_DMA_BANK_MODE > (unsigned int)bank_mode);
	IA_CSS_TRACE_1(NCI_DMA_DECRIPTORS_PRETTY_PRINT, DEBUG, "\t bank_mode 0x%08x\n", bank_mode);
}

#endif /* __NCI_DMA_DECRIPTORS_PRETTY_PRINT_H */
