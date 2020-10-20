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

#include <ia_css_kernel_bitmap.h>
#include <type_support.h>
#include <misc_support.h>
#include <assert_support.h>
#include "math_support.h"
#include "ia_css_psys_kernel_trace.h"

#ifndef __IA_CSS_KERNEL_BITMAP_INLINE__
#include "ia_css_kernel_bitmap_impl.h"
#endif /* __IA_CSS_KERNEL_BITMAP_INLINE__ */

ia_css_kernel_bitmap_t ia_css_kernel_bitmap_set_element_from_uint32(
	ia_css_kernel_bitmap_t				bitmap,
	const unsigned int				elem_index,
	const uint32_t					elem_value)
{
	IA_CSS_TRACE_0(PSYSAPI_KERNEL, VERBOSE,
		"ia_css_kernel_bitmap_set_element_from_uint32(): enter:\n");

#ifndef IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS
	COMPILATION_ERROR_IF(sizeof(bitmap.data[elem_index]) != sizeof(elem_value));

	if (elem_index < IA_CSS_KERNEL_BITMAP_NOF_ELEMS) {
		bitmap.data[elem_index] = elem_value;
	} else {
		assert(0);
	}
#else
	COMPILATION_ERROR_IF(IA_CSS_KERNEL_BITMAP_BITS != 64);

	bitmap &= ~((((ia_css_kernel_bitmap_t) 1 << IA_CSS_KERNEL_BITMAP_ELEM_BITS) - 1) <<
							(elem_index * IA_CSS_KERNEL_BITMAP_ELEM_BITS));
	bitmap |= (ia_css_kernel_bitmap_t) elem_value << (elem_index * IA_CSS_KERNEL_BITMAP_ELEM_BITS);
#endif

	return bitmap;
}

ia_css_kernel_bitmap_t ia_css_kernel_bitmap_create_from_uint64(
	const uint64_t value)
{
	const unsigned int bits64 = sizeof(uint64_t) * 8;
	const unsigned int nof_elems_bits64 = bits64 / IA_CSS_KERNEL_BITMAP_ELEM_BITS;
	unsigned int i;
	ia_css_kernel_bitmap_t result;

	IA_CSS_TRACE_0(PSYSAPI_KERNEL, VERBOSE,
		"ia_css_kernel_bitmap_create_from_uint64(): enter:\n");

#ifndef IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS
	result = ia_css_kernel_bitmap_clear();
	for (i = 0; i < MIN(nof_elems_bits64, IA_CSS_KERNEL_BITMAP_NOF_ELEMS); i++) {
		/* masking is done implictly, the MSB bits of casting will bel chopped off */
		result.data[i] = (IA_CSS_KERNEL_BITMAP_ELEM_TYPE)
			(value >> (i * IA_CSS_KERNEL_BITMAP_ELEM_BITS));
	}

#if IA_CSS_KERNEL_BITMAP_BITS < 64
	if ((value >> IA_CSS_KERNEL_BITMAP_BITS) != 0) {
		IA_CSS_TRACE_0(PSYSAPI_KERNEL, ERROR,
			"ia_css_kernel_bitmap_create_from_uint64(): "
			"kernel bitmap is not wide enough to encode value\n");
		assert(0);
	}
#endif
#else
	NOT_USED(i);
	NOT_USED(nof_elems_bits64);
	result = value;
#endif /* IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS */
	return result;
}

uint64_t ia_css_kernel_bitmap_to_uint64(
	const ia_css_kernel_bitmap_t value)
{
	const unsigned int bits64 = sizeof(uint64_t) * 8;
	const unsigned int nof_elems_bits64 = bits64 / IA_CSS_KERNEL_BITMAP_ELEM_BITS;
	unsigned int i;
	uint64_t res = 0;

	IA_CSS_TRACE_0(PSYSAPI_KERNEL, VERBOSE,
		"ia_css_kernel_bitmap_to_uint64(): enter:\n");

	assert((bits64 % IA_CSS_KERNEL_BITMAP_ELEM_BITS) == 0);
	assert(nof_elems_bits64 > 0);

#ifndef IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS
	for (i = 0; i < MIN(IA_CSS_KERNEL_BITMAP_ELEM_BITS, nof_elems_bits64); i++) {
		res |= ((uint64_t)(value.data[i]) << (i * IA_CSS_KERNEL_BITMAP_ELEM_BITS));
	}
	for (; i < IA_CSS_KERNEL_BITMAP_NOF_ELEMS; i++) {
		assert(value.data[i] == 0);
	}
	return res;
#else
	(void)i;
	(void)res;
	(void)nof_elems_bits64;
	return (uint64_t)value;
#endif /* IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS */
}

int ia_css_kernel_bitmap_print(
	const ia_css_kernel_bitmap_t			bitmap,
	void						*fid)
{
	int retval = -1;
	int bit;
	unsigned int bit_index = 0;
	ia_css_kernel_bitmap_t loc_bitmap;

	IA_CSS_TRACE_0(PSYSAPI_KERNEL, INFO,
		"ia_css_kernel_bitmap_print(): enter:\n");

	NOT_USED(fid);
	NOT_USED(bit);

	IA_CSS_TRACE_0(PSYSAPI_KERNEL, INFO, "kernel bitmap {\n");

	loc_bitmap = bitmap;

	for (bit_index = 0; (bit_index < IA_CSS_KERNEL_BITMAP_BITS) &&
		!ia_css_is_kernel_bitmap_empty(loc_bitmap); bit_index++) {

		bit = ia_css_is_kernel_bitmap_set(loc_bitmap, 0);
		loc_bitmap = ia_css_kernel_bitmap_shift(loc_bitmap);
		IA_CSS_TRACE_2(PSYSAPI_KERNEL, INFO, "\t%d\t = %d\n", bit_index, bit);
	}
	IA_CSS_TRACE_0(PSYSAPI_KERNEL, INFO, "}\n");

	retval = 0;
	return retval;
}

