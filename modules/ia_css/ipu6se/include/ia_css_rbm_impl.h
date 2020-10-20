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

#include "ia_css_rbm.h"
#include "type_support.h"
#include "misc_support.h"
#include "assert_support.h"
#include "math_support.h"
#include "ia_css_rbm_trace.h"

STORAGE_CLASS_INLINE int ia_css_rbm_compute_weight(
	const ia_css_rbm_t bitmap);

STORAGE_CLASS_INLINE ia_css_rbm_t ia_css_rbm_shift(
	const ia_css_rbm_t bitmap);

IA_CSS_RBM_STORAGE_CLASS_C
bool ia_css_is_rbm_intersection_empty(
	const ia_css_rbm_t bitmap0,
	const ia_css_rbm_t bitmap1)
{
	ia_css_rbm_t intersection;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_is_rbm_intersection_empty(): enter:\n");

	intersection = ia_css_rbm_intersection(bitmap0, bitmap1);
	return ia_css_is_rbm_empty(intersection);
}

IA_CSS_RBM_STORAGE_CLASS_C
bool ia_css_is_rbm_empty(
	const ia_css_rbm_t bitmap)
{
	unsigned int i;
	bool is_empty = true;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_is_rbm_empty(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		is_empty &= bitmap.data[i] == 0;
	}
	return is_empty;
}

IA_CSS_RBM_STORAGE_CLASS_C
bool ia_css_is_rbm_equal(
	const ia_css_rbm_t bitmap0,
	const ia_css_rbm_t bitmap1)
{
	unsigned int i;
	bool is_equal = true;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_is_rbm_equal(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		is_equal = is_equal && (bitmap0.data[i] == bitmap1.data[i]);
	}
	return is_equal;
}

IA_CSS_RBM_STORAGE_CLASS_C
bool ia_css_is_rbm_subset(
	const ia_css_rbm_t bitmap0,
	const ia_css_rbm_t bitmap1)
{
	unsigned int i;
	bool is_subset = true;

	/* checks if bitmap1 is subset of bitmap 0 */
	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_is_rbm_subset(): enter:\n");

	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		is_subset &= ((bitmap0.data[i] & bitmap1.data[i]) == bitmap1.data[i]);
	}

	return is_subset;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_clear(void)
{
	unsigned int i;
	ia_css_rbm_t bitmap = {{0} };

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_clear(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		bitmap.data[i] = 0;
	}
	return bitmap;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_complement(
	const ia_css_rbm_t bitmap)
{
	unsigned int i;
	ia_css_rbm_t result = {{0} };

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_complement(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		result.data[i] = ~bitmap.data[i];
	}
	return result;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_union(
	const ia_css_rbm_t bitmap0,
	const ia_css_rbm_t bitmap1)
{
	unsigned int i;
	ia_css_rbm_t result = {{0} };

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_union(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		result.data[i] = (bitmap0.data[i] | bitmap1.data[i]);
	}
	return result;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_intersection(
	const ia_css_rbm_t bitmap0,
	const ia_css_rbm_t bitmap1)
{
	unsigned int i;
	ia_css_rbm_t result = {{0} };

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_intersection(): enter:\n");
	for (i = 0; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		result.data[i] = (bitmap0.data[i] & bitmap1.data[i]);
	}
	return result;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_set(
	const ia_css_rbm_t bitmap,
	const unsigned int index)
{
	ia_css_rbm_t bit_mask;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_set(): enter:\n");

	bit_mask = ia_css_rbm_bit_mask(index);
	return ia_css_rbm_union(bitmap, bit_mask);
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_set_element_from_uint32(
	const ia_css_rbm_t bitmap,
	const unsigned int elem_index,
	const uint32_t     elem_value)
{
	ia_css_rbm_t result = bitmap;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_set_element_from_uint32(): enter:\n");

	COMPILATION_ERROR_IF(sizeof(bitmap.data[elem_index]) != sizeof(elem_value));

	if (elem_index < IA_CSS_RBM_NOF_ELEMS) {
		result.data[elem_index] = elem_value;
	} else {
		assert(0);
	}

	return result;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_create_from_uint64(
	const uint64_t value)
{
	unsigned int i;
	ia_css_rbm_t result;
	const unsigned int bits64 = sizeof(uint64_t) * 8;
	const unsigned int nof_elems_bits64 = bits64 / IA_CSS_RBM_ELEM_BITS;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_create_from_uint64(): enter:\n");

	result = ia_css_rbm_clear();
	for (i = 0; i < MIN(nof_elems_bits64, IA_CSS_RBM_NOF_ELEMS); i++) {
		/* masking is done implictly, the MSB bits of casting will be chopped off */
		result.data[i] = (IA_CSS_RBM_ELEM_TYPE)
			(value >> (i * IA_CSS_RBM_ELEM_BITS));
	}

	return result;
}

IA_CSS_RBM_STORAGE_CLASS_C
uint64_t ia_css_rbm_to_uint64(
	const ia_css_rbm_t value)
{
	const unsigned int bits64 = sizeof(uint64_t) * 8;
	const unsigned int nof_elems_bits64 = bits64 / IA_CSS_RBM_ELEM_BITS;
	unsigned int i;
	uint64_t res = 0;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_to_uint64(): enter:\n");

	assert((bits64 % IA_CSS_RBM_ELEM_BITS) == 0);
	assert(nof_elems_bits64 > 0);

	for (i = 0; i < MIN(IA_CSS_RBM_NOF_ELEMS, nof_elems_bits64); i++) {
		res |= ((uint64_t)(value.data[i]) << (i * IA_CSS_RBM_ELEM_BITS));
	}
	for (; i < IA_CSS_RBM_NOF_ELEMS; i++) {
		assert(value.data[i] == 0);
	}
	return res;
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_unset(
	const ia_css_rbm_t bitmap,
	const unsigned int index)
{
	ia_css_rbm_t result;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_unset(): enter:\n");

	result = ia_css_rbm_bit_mask(index);
	result = ia_css_rbm_complement(result);
	return ia_css_rbm_intersection(bitmap, result);
}

IA_CSS_RBM_STORAGE_CLASS_C
ia_css_rbm_t ia_css_rbm_bit_mask(
	const unsigned int index)
{
	unsigned int elem_index;
	unsigned int elem_bit_index;
	ia_css_rbm_t bit_mask = ia_css_rbm_clear();

	assert(index < IA_CSS_RBM_BITS);

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_bit_mask(): enter:\n");
	if (index < IA_CSS_RBM_BITS) {
		elem_index = index / IA_CSS_RBM_ELEM_BITS;
		elem_bit_index = index % IA_CSS_RBM_ELEM_BITS;
		assert(elem_index < IA_CSS_RBM_NOF_ELEMS);

		bit_mask.data[elem_index] = 1 << elem_bit_index;
	}
	return bit_mask;
}

STORAGE_CLASS_INLINE
int ia_css_rbm_compute_weight(
	const ia_css_rbm_t bitmap)
{
	ia_css_rbm_t loc_bitmap;
	int weight = 0;
	int i;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_compute_weight(): enter:\n");

	loc_bitmap = bitmap;

	/* In fact; do not need the iterator "i" */
	for (i = 0; (i < IA_CSS_RBM_BITS) &&
		!ia_css_is_rbm_empty(loc_bitmap); i++) {
		weight += ia_css_is_rbm_set(loc_bitmap, 0);
		loc_bitmap = ia_css_rbm_shift(loc_bitmap);
	}

	return weight;
}

IA_CSS_RBM_STORAGE_CLASS_C
int ia_css_rbm_range_val(
	const ia_css_rbm_t bitmap,
	const unsigned int index,
	const unsigned int size)
{
	unsigned int elem_index;
	unsigned int elem_bit_index;
	unsigned int res;
	unsigned int elem_index_end;
	unsigned int mask;

	IA_CSS_TRACE_0(RBM, VERBOSE, "ia_css_rbm_range_val(): enter:\n");
	assert(index < IA_CSS_RBM_BITS);
	/* Extract the bit range from the data array relevane entry */
	elem_index = index / IA_CSS_RBM_ELEM_BITS;
	elem_index_end = (index + size - 1) / IA_CSS_RBM_ELEM_BITS;
	elem_bit_index = index % IA_CSS_RBM_ELEM_BITS;
	assert(elem_index < IA_CSS_RBM_NOF_ELEMS);
	mask =  (1 << size) - 1;
	res = (bitmap.data[elem_index] >> elem_bit_index) & mask;
	/* In case that the bit range is divided between 2 different data array entries */
	if (elem_index != elem_index_end) {
		unsigned int first_elem_size = IA_CSS_RBM_ELEM_BITS - index;
		unsigned int second_elem_size = size - first_elem_size;

		mask = (1 << second_elem_size) - 1;
		res |= ((bitmap.data[elem_index_end] & mask) << first_elem_size);
	}
	return res;
}

IA_CSS_RBM_STORAGE_CLASS_C
int ia_css_is_rbm_set(
	const ia_css_rbm_t bitmap,
	const unsigned int index)
{
	unsigned int elem_index;
	unsigned int elem_bit_index;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_is_rbm_set(): enter:\n");

	assert(index < IA_CSS_RBM_BITS);

	elem_index = index / IA_CSS_RBM_ELEM_BITS;
	elem_bit_index = index % IA_CSS_RBM_ELEM_BITS;
	assert(elem_index < IA_CSS_RBM_NOF_ELEMS);
	return (((bitmap.data[elem_index] >> elem_bit_index) & 0x1) == 1);
}

STORAGE_CLASS_INLINE
ia_css_rbm_t ia_css_rbm_shift(
	const ia_css_rbm_t bitmap)
{
	int i;
	unsigned int lsb_current_elem = 0;
	unsigned int lsb_previous_elem = 0;
	ia_css_rbm_t loc_bitmap;

	IA_CSS_TRACE_0(RBM, VERBOSE,
		"ia_css_rbm_shift(): enter:\n");

	loc_bitmap = bitmap;

	for (i = IA_CSS_RBM_NOF_ELEMS - 1; i >= 0; i--) {
		lsb_current_elem = bitmap.data[i] & 0x01;
		loc_bitmap.data[i] >>= 1;
		loc_bitmap.data[i] |= (lsb_previous_elem << (IA_CSS_RBM_ELEM_BITS - 1));
		lsb_previous_elem = lsb_current_elem;
	}
	return loc_bitmap;
}

IA_CSS_RBM_STORAGE_CLASS_C
int ia_css_rbm_print(
	const ia_css_rbm_t bitmap,
	void               *fid)
{
	int i;
	const IA_CSS_RBM_ELEM_TYPE *data = &bitmap.data[0];

	NOT_USED(fid);

	CT_ASSERT(IA_CSS_RBM_NOF_ELEMS > 0);
	CT_ASSERT(IA_CSS_RBM_ELEM_BITS <= 32); /* because we print 32 bit integers */

	if (IA_CSS_RBM_NOF_ELEMS == 1) {
		IA_CSS_TRACE_1(RBM, INFO, "ia_css_rbm_print(): rbm: %08x\n",
			data[0]);

	} else if (IA_CSS_RBM_NOF_ELEMS == 2) {
		IA_CSS_TRACE_2(RBM, INFO, "ia_css_rbm_print(): rbm: %08x %08x\n",
			data[1], data[0]);

	} else if (IA_CSS_RBM_NOF_ELEMS == 3) {
		IA_CSS_TRACE_3(RBM, INFO, "ia_css_rbm_print(): rbm: %08x %08x %08x\n",
			data[2], data[1], data[0]);

	} else if (IA_CSS_RBM_NOF_ELEMS == 4) {
		IA_CSS_TRACE_4(RBM, INFO, "ia_css_rbm_print(): rbm: %08x %08x %08x %08x\n",
			data[3], data[2], data[1], data[0]);

	} else {
		IA_CSS_TRACE_0(RBM, INFO, "ia_css_rbm_print(): rbm: {\n");

		for (i = IA_CSS_RBM_NOF_ELEMS - 1; i >= 0 ; i--) {
			IA_CSS_TRACE_1(RBM, INFO, "\t%08x\n", data[i]);
		}
		IA_CSS_TRACE_0(RBM, INFO, "}\n");
	}

	return 0;
}
