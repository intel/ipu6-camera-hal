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

#ifndef __IA_CSS_PSYS_BUFFER_SET_IMPL_H
#define __IA_CSS_PSYS_BUFFER_SET_IMPL_H

#include "error_support.h"
#include "ia_css_psys_dynamic_trace.h"
#include "vied_nci_psys_system_global.h"
#include "ia_css_psys_terminal.hsys.user.h"
#include "cpu_mem_support.h"

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_buffer_set_set_buffer(
	ia_css_buffer_set_t *buffer_set,
	const unsigned int terminal_index,
	const vied_vaddress_t buffer)
{
	DECLARE_ERRVAL
	vied_vaddress_t *buffer_ptr;
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);
	verifexitval(terminal_index < buffer_set->terminal_count, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_buffer(): enter:\n");

	/*
	 * Set address in buffer set object
	 */
	buffer_ptr =
		(vied_vaddress_t *)(
			(char *)buffer_set +
			sizeof(ia_css_buffer_set_t) +
			terminal_index * sizeof(vied_vaddress_t));
	*buffer_ptr = buffer;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_buffer: invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_vaddress_t ia_css_buffer_set_get_buffer(
	const ia_css_buffer_set_t *buffer_set,
	const ia_css_terminal_t *terminal)
{
	DECLARE_ERRVAL
	vied_vaddress_t buffer = VIED_NULL;
	vied_vaddress_t *buffer_ptr;
	int terminal_index;

	verifexitval(buffer_set != NULL, EFAULT);
	verifexitval(terminal != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_buffer(): enter:\n");

	/*
	 * Retrieve terminal index from terminal object
	 */
	terminal_index = ia_css_terminal_get_terminal_index(terminal);
	verifexitval(terminal_index >= 0, EFAULT);
	verifexitval(terminal_index < buffer_set->terminal_count, EFAULT);

	/*
	 * Retrieve address from buffer set object
	 */
	buffer_ptr =
		(vied_vaddress_t *)(
			(char *)buffer_set +
			sizeof(ia_css_buffer_set_t) +
			terminal_index * sizeof(vied_vaddress_t));
	buffer = *buffer_ptr;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_buffer: invalid argument\n");
	}
	return buffer;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_buffer_set_set_ipu_address(
	ia_css_buffer_set_t *buffer_set,
	const vied_vaddress_t ipu_vaddress)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_ipu_address(): enter:\n");

	buffer_set->ipu_virtual_address = ipu_vaddress;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_ipu_address invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_vaddress_t ia_css_buffer_set_get_ipu_address(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	vied_vaddress_t ipu_virtual_address = VIED_NULL;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_ipu_address(): enter:\n");

	ipu_virtual_address = buffer_set->ipu_virtual_address;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_ipu_address: invalid argument\n");
	}
	return ipu_virtual_address;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_buffer_set_set_kernel_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t kernel_enable_bitmap)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_kernel_enable_bitmap(): enter:\n");

	buffer_set->kernel_enable_bitmap = kernel_enable_bitmap;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_kernel_enable_bitmap invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
ia_css_kernel_bitmap_t ia_css_buffer_set_get_kernel_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	ia_css_kernel_bitmap_t kernel_enable_bitmap;
	ia_css_cpu_mem_set_zero(
		&kernel_enable_bitmap,
		sizeof(kernel_enable_bitmap));	/* All 0's */

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_kernel_enable_bitmap(): enter:\n");

	kernel_enable_bitmap = buffer_set->kernel_enable_bitmap;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_kernel_enable_bitmap: invalid argument\n");
	}
	return kernel_enable_bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_terminal_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t terminal_enable_bitmap)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_terminal_enable_bitmap(): enter:\n");

	buffer_set->terminal_enable_bitmap = terminal_enable_bitmap;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_terminal_enable_bitmap invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_buffer_set_get_terminal_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	ia_css_kernel_bitmap_t terminal_enable_bitmap;
	ia_css_cpu_mem_set_zero(
		&terminal_enable_bitmap,
		sizeof(terminal_enable_bitmap));	/* All 0's */

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_terminal_enable_bitmap(): enter:\n");

	terminal_enable_bitmap = buffer_set->terminal_enable_bitmap;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_terminal_enable_bitmap: invalid argument\n");
	}
	return terminal_enable_bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_routing_enable_bitmap(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_kernel_bitmap_t routing_enable_bitmap)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_routing_enable_bitmap(): enter:\n");

	buffer_set->routing_enable_bitmap = routing_enable_bitmap;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_routing_enable_bitmap invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_buffer_set_get_routing_enable_bitmap(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	ia_css_kernel_bitmap_t routing_enable_bitmap;
	ia_css_cpu_mem_set_zero(
		&routing_enable_bitmap,
		sizeof(routing_enable_bitmap));	/* All 0's */

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_routing_enable_bitmap(): enter:\n");

	routing_enable_bitmap = buffer_set->routing_enable_bitmap;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_routing_enable_bitmap: invalid argument\n");
	}
	return routing_enable_bitmap;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
int ia_css_buffer_set_set_rbm(
	ia_css_buffer_set_t *buffer_set,
	const ia_css_rbm_t rbm)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_rbm(): enter:\n");

	buffer_set->rbm = rbm;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_rbm invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_H
ia_css_rbm_t ia_css_buffer_set_get_rbm(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	ia_css_rbm_t rbm;
	ia_css_cpu_mem_set_zero(
		&rbm,
		sizeof(rbm));	/* All 0's */

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_rbm(): enter:\n");

	rbm = buffer_set->rbm;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_rbm: invalid argument\n");
	}
	return rbm;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_buffer_set_set_process_group_handle(
	ia_css_buffer_set_t *buffer_set,
	const vied_vaddress_t process_group_handle)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_process_group_context(): enter:\n");

	buffer_set->process_group_handle = process_group_handle;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_process_group_context invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
vied_vaddress_t ia_css_buffer_set_get_process_group_handle(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	vied_vaddress_t process_group_handle = VIED_NULL;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_process_group_handle(): enter:\n");

	process_group_handle = buffer_set->process_group_handle;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_process_group_handle: invalid argument\n");
	}
	return process_group_handle;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
int ia_css_buffer_set_set_token(
	ia_css_buffer_set_t *buffer_set,
	const uint64_t token)
{
	DECLARE_ERRVAL
	int ret = -1;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_set_token(): enter:\n");

	buffer_set->token = token;

	ret = 0;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_set_token invalid argument\n");
	}
	return ret;
}

IA_CSS_PSYS_DYNAMIC_STORAGE_CLASS_C
uint64_t ia_css_buffer_set_get_token(
	const ia_css_buffer_set_t *buffer_set)
{
	DECLARE_ERRVAL
	uint64_t token = 0;

	verifexitval(buffer_set != NULL, EFAULT);

	IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, VERBOSE,
		"ia_css_buffer_set_get_token(): enter:\n");

	token = buffer_set->token;

EXIT:
	if (haserror(EFAULT)) {
		IA_CSS_TRACE_0(PSYSAPI_DYNAMIC, ERROR,
			"ia_css_buffer_set_get_token: invalid argument\n");
	}
	return token;
}

#endif /* __IA_CSS_PSYS_BUFFER_SET_IMPL_H */
