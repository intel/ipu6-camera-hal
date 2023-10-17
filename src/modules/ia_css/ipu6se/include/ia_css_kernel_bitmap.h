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

#ifndef __IA_CSS_KERNEL_BITMAP_H
#define __IA_CSS_KERNEL_BITMAP_H

/*! \file */

/** @file ia_css_kernel_bitmap.h
 *
 * The types and operations to make logic decisions given kernel bitmaps
 * "ia_css_kernel_bitmap_t" can be larger than native types
 */

#include <type_support.h>
#include "vied_nci_psys_resource_model.h"
#include "ia_css_kernel_bitmap_storage_class.h"

/**
 * @addtogroup group_psysapi
 * @{
 */

#define IA_CSS_KERNEL_BITMAP_ELEM_TYPE uint32_t
#define IA_CSS_KERNEL_BITMAP_ELEM_BITS \
	(sizeof(IA_CSS_KERNEL_BITMAP_ELEM_TYPE)*8)
#define IA_CSS_KERNEL_BITMAP_NOF_ELEMS \
	((IA_CSS_KERNEL_BITMAP_BITS) / (IA_CSS_KERNEL_BITMAP_ELEM_BITS))

/** An element is a 32 bit unsigned integer. 64 bit integers might cause
 * problems in the compiler.
 */
typedef struct {
	IA_CSS_KERNEL_BITMAP_ELEM_TYPE data[IA_CSS_KERNEL_BITMAP_NOF_ELEMS];
} ia_css_kernel_bitmap_elems_t;

/** Users should make no assumption about the actual type of
 * ia_css_kernel_bitmap_t.
 * Users should use IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS in
 * case they erroneously assume that this type is uint64_t and they
 * cannot change their implementation.
 */
#ifndef IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS
typedef ia_css_kernel_bitmap_elems_t ia_css_kernel_bitmap_t;
#else
typedef uint64_t ia_css_kernel_bitmap_t;
#if IA_CSS_KERNEL_BITMAP_BITS > 64
#error IA_CSS_KERNEL_BITMAP_BITS > 64 not supported \
	with IA_CSS_KERNEL_BITMAP_DO_NOT_USE_ELEMS
#endif
#endif

/*! Print the bits of a kernel bitmap

 @return < 0 on error
 */
extern int ia_css_kernel_bitmap_print(
	const ia_css_kernel_bitmap_t	bitmap,
	void				*fid);

/*! Create an empty kernel bitmap

 @return bitmap = 0
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_clear(void);

/*! Creates the complement of a kernel bitmap
 * @param	[in] bitmap kernel bitmap
 * @return ~bitmap
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_complement(
	const ia_css_kernel_bitmap_t bitmap);

/*! Create the union of two kernel bitmaps

 @param	[in] bitmap0					kernel bitmap 0
 @param	[in] bitmap1					kernel bitmap 1

 @return bitmap0 | bitmap1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_union(
	const ia_css_kernel_bitmap_t			bitmap0,
	const ia_css_kernel_bitmap_t			bitmap1);

/*! Create the intersection of two kernel bitmaps

 @param	[in] bitmap0					kernel bitmap 0
 @param	[in] bitmap1					kernel bitmap 1

 @return bitmap0 & bitmap1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_intersection(
	const ia_css_kernel_bitmap_t			bitmap0,
	const ia_css_kernel_bitmap_t			bitmap1);

/*! Check if the kernel bitmaps is empty

 @param	[in] bitmap					kernel bitmap

 @return bitmap == 0
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
bool ia_css_is_kernel_bitmap_empty(
	const ia_css_kernel_bitmap_t			bitmap);

/*! Check if the intersection of two kernel bitmaps is empty

 @param	[in] bitmap0					kernel bitmap 0
 @param	[in] bitmap1					kernel bitmap 1

 @return (bitmap0 & bitmap1) == 0
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
bool ia_css_is_kernel_bitmap_intersection_empty(
	const ia_css_kernel_bitmap_t			bitmap0,
	const ia_css_kernel_bitmap_t			bitmap1);

/*! Check if the second kernel bitmap is a subset of the first (or equal)

 @param	[in] bitmap0					kernel bitmap 0
 @param	[in] bitmap1					kernel bitmap 1

 Note: An empty set is always a subset, this function
 returns true if bitmap 1 is empty

 @return (bitmap0 & bitmap1) == bitmap1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
bool ia_css_is_kernel_bitmap_subset(
	const ia_css_kernel_bitmap_t			bitmap0,
	const ia_css_kernel_bitmap_t			bitmap1);

/*! Check if the kernel bitmaps are equal

 @param	[in] bitmap0					kernel bitmap 0
 @param	[in] bitmap1					kernel bitmap 1

 @return bitmap0 == bitmap1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
bool ia_css_is_kernel_bitmap_equal(
	const ia_css_kernel_bitmap_t			bitmap0,
	const ia_css_kernel_bitmap_t			bitmap1);

/*! Right shift kernel bitmap

 @param	[in] bitmap					kernel bitmap 0

 @return bitmap >> 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_shift(
	const ia_css_kernel_bitmap_t			bitmap);

/*! Check if the kernel bitmaps contains only a single element

 @param	[in] bitmap					kernel bitmap

 @return weight(bitmap) == 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
bool ia_css_is_kernel_bitmap_onehot(
	const ia_css_kernel_bitmap_t			bitmap);

/*! Checks whether a specific kernel bit is set
 * @return bitmap[index] == 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
int ia_css_is_kernel_bitmap_set(
	const ia_css_kernel_bitmap_t	bitmap,
	const unsigned int		index);

/*! Create the union of a kernel bitmap with a onehot bitmap
 * with a bit set at index

 @return bitmap[index] |= 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_set(
	const ia_css_kernel_bitmap_t			bitmap,
	const unsigned int						index);

/*! Set elem_index-th 32-bit element of the bitmap
 * to elem_value

 @return updated bitmap
 */
extern ia_css_kernel_bitmap_t ia_css_kernel_bitmap_set_element_from_uint32(
	ia_css_kernel_bitmap_t			bitmap,
	const unsigned int			elem_index,
	const uint32_t				elem_value);

/*! Creates kernel bitmap using a uint64 value.
 * @return bitmap with the same bits set as in value (provided that width of bitmap is sufficient).
 */
extern ia_css_kernel_bitmap_t ia_css_kernel_bitmap_create_from_uint64(
	const uint64_t	value);

/*! Converts an ia_css_kernel_bitmap_t type to uint64_t. Note that if
 *  ia_css_kernel_bitmap_t contains more then 64 bits, only the lowest 64 bits
 *  are returned.
 *  @return uint64_t representation of value
*/
extern uint64_t ia_css_kernel_bitmap_to_uint64(
	const ia_css_kernel_bitmap_t value);

/*! Creates a kernel bitmap with the bit at index 'index' removed.
 * @return ~(1 << index) & bitmap
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_unset(
	const ia_css_kernel_bitmap_t	bitmap,
	const unsigned int		index);

/*! Set a previously clear field of a kernel bitmap at index

 @return if bitmap[index] == 0, bitmap[index] -> 1, else 0
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bitmap_set_unique(
	const ia_css_kernel_bitmap_t			bitmap,
	const unsigned int						index);

/*! Create a onehot kernel bitmap with a bit set at index

 @return bitmap[index] = 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_bit_mask(
	const unsigned int						index);

/*! Create a random bitmap

 @return bitmap[index] = 1
 */
IA_CSS_KERNEL_BITMAP_STORAGE_CLASS_H
ia_css_kernel_bitmap_t ia_css_kernel_ran_bitmap(void);

/*! @} */

#ifdef __IA_CSS_KERNEL_BITMAP_INLINE__
#include "ia_css_kernel_bitmap_impl.h"
#endif /* __IA_CSS_KERNEL_BITMAP_INLINE__ */

#endif /* __IA_CSS_KERNEL_BITMAP_H */
