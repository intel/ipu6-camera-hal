/*
 * Copyright (C) 2020 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_IMPL_H
#define __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_IMPL_H

#include <ia_css_psys_program_group_manifest.h>
#include <ia_css_psys_program_manifest.h>
#include <ia_css_psys_terminal_manifest.h>
#include <ia_css_kernel_bitmap.h>
#include "ia_css_psys_program_group_private.h"
#include "ia_css_terminal_manifest_types.h"
#include "ia_css_psys_private_pg_data.h"
#include <vied_nci_psys_system_global.h>	/* Safer bit mask functions */
#include "ia_css_psys_static_trace.h"
#include "ia_css_rbm_manifest_types.h"
#include <error_support.h>
#include <assert_support.h>
#include <misc_support.h>

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
size_t ia_css_program_group_manifest_get_size(
	const ia_css_program_group_manifest_t *manifest)
{
	size_t size = 0;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_size(): enter:\n");

	if (manifest != NULL) {
		size = manifest->size;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_size invalid argument\n");
	}
	return size;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_group_ID_t
ia_css_program_group_manifest_get_program_group_ID(
	const ia_css_program_group_manifest_t *manifest)
{
	ia_css_program_group_ID_t id = IA_CSS_PROGRAM_GROUP_INVALID_ID;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_program_group_ID(): enter:\n");

	if (manifest != NULL) {
		id = manifest->ID;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_program_group_ID invalid argument\n");
	}
	return id;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_program_group_manifest_set_program_group_ID(
	ia_css_program_group_manifest_t *manifest,
	ia_css_program_group_ID_t id)
{
	int retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_set_program_group_ID(): enter:\n");

	if (manifest != NULL) {
		manifest->ID = id;
		retval = 0;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_set_program_group_ID invalid argument\n");
	}
	return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
int ia_css_program_group_manifest_set_alignment(
	ia_css_program_group_manifest_t *manifest,
	const uint8_t alignment)
{
	int retval = -1;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_set_alignment(): enter:\n");

	if (manifest != NULL) {
		manifest->alignment = alignment;
		retval = 0;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_set_alignment invalid argument\n");
	}
	return retval;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_group_manifest_get_alignment(
	const ia_css_program_group_manifest_t *manifest)
{
	uint8_t alignment = 0;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_alignment(): enter:\n");

	if (manifest != NULL) {
		alignment = manifest->alignment;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_alignment invalid argument\n");
	}
	return alignment;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
void *ia_css_program_group_manifest_get_private_data(
	const ia_css_program_group_manifest_t *manifest)
{
	void *private_data = NULL;

	IA_CSS_TRACE_1(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_private_data(%p): enter:\n",
		manifest);

	verifexit(manifest != NULL);

	private_data = (void *)((const char *)manifest +
				manifest->private_data_offset);
EXIT:
	if (NULL == manifest) {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_private_data invalid argument\n");
	}
	return private_data;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_rbm_manifest_t *ia_css_program_group_manifest_get_rbm_manifest(
	const ia_css_program_group_manifest_t *manifest)
{
	ia_css_rbm_manifest_t *rbm_manifest = NULL;

	IA_CSS_TRACE_1(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_rbm_manifest(%p): enter:\n",
		manifest);

	verifexit(manifest != NULL);

	rbm_manifest = (ia_css_rbm_manifest_t *)((const char *)manifest +
				manifest->rbm_manifest_offset);

EXIT:
	if (NULL == manifest) {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_rbm_manifest invalid argument\n");
	}
	return rbm_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_manifest_t *
ia_css_program_group_manifest_get_prgrm_mnfst(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int program_index)
{
	ia_css_program_manifest_t *prg_manifest_base;
	uint8_t *program_manifest = NULL;
	uint8_t program_count;
	unsigned int i;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_prgrm_mnfst(%p,%d): enter:\n",
		manifest, program_index);

	program_count =
		ia_css_program_group_manifest_get_program_count(manifest);

	verifexit(manifest != NULL);
	verifexit(program_index < program_count);

	prg_manifest_base = (ia_css_program_manifest_t *)((char *)manifest +
		manifest->program_manifest_offset);
	if (program_index < program_count) {
		program_manifest = (uint8_t *)prg_manifest_base;
		for (i = 0; i < program_index; i++) {
			program_manifest += ((ia_css_program_manifest_t *)
					program_manifest)->size;
		}
	}

EXIT:
	if (NULL == manifest || program_index >= program_count) {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_prgrm_mnfst invalid argument\n");
	}
	return (ia_css_program_manifest_t *)program_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_data_terminal_manifest_t *
ia_css_program_group_manifest_get_data_terminal_manifest(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_data_terminal_manifest_t *data_terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_data_terminal_manifest(%p, %d): enter:\n",
		manifest, (int)terminal_index);

	terminal_manifest =
		ia_css_program_group_manifest_get_term_mnfst(manifest,
			terminal_index);

	verifexit(ia_css_is_terminal_manifest_data_terminal(terminal_manifest));

	data_terminal_manifest =
		(ia_css_data_terminal_manifest_t *)terminal_manifest;
EXIT:
	return data_terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_param_terminal_manifest_t *
ia_css_program_group_manifest_get_param_terminal_manifest(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_param_terminal_manifest_t *param_terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_param_terminal_manifest(%p, %d): enter:\n",
		manifest, (int)terminal_index);

	terminal_manifest =
		ia_css_program_group_manifest_get_term_mnfst(manifest,
			terminal_index);

	verifexit(ia_css_is_terminal_manifest_parameter_terminal(
			terminal_manifest));
	param_terminal_manifest =
		(ia_css_param_terminal_manifest_t *)terminal_manifest;
EXIT:
	return param_terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_spatial_param_terminal_manifest_t *
ia_css_program_group_manifest_get_spatial_param_terminal_manifest(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_spatial_param_terminal_manifest_t *
		spatial_param_terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_spatial_param_terminal_manifest(%p, %d): enter:\n",
		manifest, (int)terminal_index);

	terminal_manifest =
		ia_css_program_group_manifest_get_term_mnfst(manifest,
			terminal_index);

	verifexit(ia_css_is_terminal_manifest_spatial_parameter_terminal(
			terminal_manifest));

	spatial_param_terminal_manifest =
		(ia_css_spatial_param_terminal_manifest_t *)terminal_manifest;
EXIT:
	return spatial_param_terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_sliced_param_terminal_manifest_t *
ia_css_program_group_manifest_get_sliced_param_terminal_manifest(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_sliced_param_terminal_manifest_t *
		sliced_param_terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_sliced_param_terminal_manifest(%p, %d): enter:\n",
		manifest, (int)terminal_index);

	terminal_manifest =
		ia_css_program_group_manifest_get_term_mnfst(manifest,
			terminal_index);

	verifexit(ia_css_is_terminal_manifest_sliced_terminal(
			terminal_manifest));

	sliced_param_terminal_manifest =
		(ia_css_sliced_param_terminal_manifest_t *)terminal_manifest;
EXIT:
	return sliced_param_terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_program_terminal_manifest_t *
ia_css_program_group_manifest_get_program_terminal_manifest(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_program_terminal_manifest_t *program_terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_program_terminal_manifest(%p, %d): enter:\n",
		manifest, (int)terminal_index);

	terminal_manifest =
		ia_css_program_group_manifest_get_term_mnfst(manifest,
			terminal_index);

	verifexit(ia_css_is_terminal_manifest_program_terminal(
			terminal_manifest));

	program_terminal_manifest =
		(ia_css_program_terminal_manifest_t *)terminal_manifest;
 EXIT:
	return program_terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
ia_css_terminal_manifest_t *
ia_css_program_group_manifest_get_term_mnfst(
	const ia_css_program_group_manifest_t *manifest,
	const unsigned int terminal_index)
{
	ia_css_terminal_manifest_t *terminal_manifest = NULL;
	ia_css_terminal_manifest_t *terminal_manifest_base;
	uint8_t terminal_count;
	uint8_t i = 0;
	uint32_t offset;

	IA_CSS_TRACE_2(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_term_mnfst(%p,%d): enter:\n",
		manifest, (int)terminal_index);

	verifexit(manifest != NULL);

	terminal_count =
		ia_css_program_group_manifest_get_terminal_count(manifest);

	verifexit(terminal_index < terminal_count);

	terminal_manifest_base =
		(ia_css_terminal_manifest_t *)((char *)manifest +
		manifest->terminal_manifest_offset);
	terminal_manifest = terminal_manifest_base;
	while (i < terminal_index) {
		offset =
		(uint32_t)ia_css_terminal_manifest_get_size(terminal_manifest);
		terminal_manifest = (ia_css_terminal_manifest_t *)
				((char *)terminal_manifest + offset);
		i++;
	}
EXIT:
	return terminal_manifest;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_group_manifest_get_program_count(
	const ia_css_program_group_manifest_t *manifest)
{
	uint8_t program_count = 0;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_program_count(): enter:\n");

	if (manifest != NULL) {
		program_count = manifest->program_count;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_program_count invalid argument\n");
	}
	return program_count;
}

IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
uint8_t ia_css_program_group_manifest_get_terminal_count(
	const ia_css_program_group_manifest_t *manifest)
{
	uint8_t terminal_count = 0;

	IA_CSS_TRACE_0(PSYSAPI_STATIC, VERBOSE,
		"ia_css_program_group_manifest_get_terminal_count(): enter:\n");

	if (manifest != NULL) {
		terminal_count = manifest->terminal_count;
	} else {
		IA_CSS_TRACE_0(PSYSAPI_STATIC, WARNING,
			"ia_css_program_group_manifest_get_terminal_count invalid argument\n");
	}
	return terminal_count;
}

#endif /* __IA_CSS_PSYS_PROGRAM_GROUP_MANIFEST_IMPL_H */
