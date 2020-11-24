/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 - 2020 Intel Corporation.
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

#ifndef __BIT_SUPPORT_H
#define __BIT_SUPPORT_H

#include "storage_class.h"
#include "type_support.h"

/**
 * @brief Validate value has specified number of bits, for 8 bit values.
 *
 * @param[in]	val				The numeric value to be checked.
 *
 * @param[in]	num_of_bits		Allowed bit range.
 *
 * @return 0 - if value fits into the given range, non-zero value otherwise.
 */
#define IA_CSS_VALIDATE_8_BIT_RANGE(val, num_of_bits) \
		((val) & (0xff<<(num_of_bits)))

/**
 * @brief Validate value has specified number of bits, for 16 bit values.
 *
 * @param[in]	val				The numeric value to be checked.
 *
 * @param[in]	num_of_bits		Allowed bit range.
 *
 * @return 0 - if value fits into the given range, non-zero value otherwise.
 */
#define IA_CSS_VALIDATE_16_BIT_RANGE(val, num_of_bits) \
		((val) & (0xffff<<(num_of_bits)))

/**
 * @brief Simple OR-Set of value, with offset.
 *
 * @param[in]	val			The numeric value to set.
 *
 * @param[in]	offset		Field offset.
 *
 * @param[in]	num			Number resulting from OR-Set operation.
 */
#define IA_CSS_SHIFT_OR_FIELD(val, offset, num) ((num) | ((val)<<(offset)))

/**
 * @brief Create a slice mask from data size.
 *
 * @param[in]	size			Size of data to be masked.
 *
 * @return Data mask.
 */
#define IA_CSS_MASK_FROM_SIZE_32(size) (((uint64_t)0x1<<(size))-1)

/**
 * @brief Simple OR-Set of value AND size, with offset.
 *
 * @param[in]	val		The numeric value to set.
 *
 * @param[in]	offset		Field offset.
 *
 * @param[in]	num		Number resulting from OR-Set operation.
 */
#define IA_CSS_CREATE_FIELD_32(val, size, offset) (((val)&(IA_CSS_MASK_FROM_SIZE_32(size)))<<(offset))

/**
 * @brief Macro version of bitslice to avoid type mismatches
 *
 * @param[in]	val		The numeric value to slice.
 *
 * @param[in]	lsb		Beginning of the requested bits from val.
 *
 * @param[in]	numbits		Number of bits starting from lsb to slice.
 */
#define IA_CSS_BIT_SLICE(val, lsb, numbits) ((val & IA_CSS_MASK_FROM_SIZE_32(numbits+lsb)) >> lsb)

/**
 * @brief Compute the minimum number of bits to encode a value.
 *
 * @param[in]	value	The numeric value to be computed.
 *
 * @return number of bits.
 */
STORAGE_CLASS_INLINE unsigned int minimum_bits(unsigned int value)
{
	int result = 0;

	if (value == 0) {
		return 0;
	}

	value -= 1;
	while (value) {
		result++;
		value >>= 1;
	}

	return result;
}

/**
 * @brief Compute the byte index containing a certain bit where
 * the LSB is in byte 0.
 *
 * @param[in] bitpos	Bit position.
 *
 * @return the byte index.
 */
STORAGE_CLASS_INLINE unsigned int bitpos_to_bytepos(unsigned int bitpos)
{
	return (bitpos + 7) / 8;
}

/**
 * @brief Compute the least significant bit position of a certain
 * byte where the LSB is in byte 0.
 *
 * @param[in] bytepose	Byte position.
 *
 * @return the bit position.
 */
STORAGE_CLASS_INLINE unsigned int bytepos_to_bitpos(unsigned int bytepos)
{
	return bytepos * 8;
}

/**
 * @brief Get a slice of bits from a value.
 *
 * @param[in] value	Numberic value to be sliced.
 * @param[in] lsb	Least significant bit in the slice.
 * @param[in] numbits	Number of bits in the slice.
 *
 * @return a slice of bits.
 */
STORAGE_CLASS_INLINE unsigned int bit_slice(unsigned int value,
		unsigned int lsb, unsigned int numbits)
{
	unsigned int mask = (1 << (numbits + lsb)) - 1;
	/* below operation is translated to asp_andlsr customop*/
	return (value & mask) >> lsb;
}

/**
 * @brief Get a slice of bits from a 64 bit value.
 *
 * @param[in] value	Numberic value to be sliced.
 * @param[in] lsb	Least significant bit in the slice.
 * @param[in] numbits	Number of bits in the slice.
 *
 * @return a slice of bits.
 */
STORAGE_CLASS_INLINE uint64_t bit_slice_64bit(uint64_t value,
		unsigned int lsb, unsigned int numbits)
{

	int offset = (numbits + lsb);
	uint64_t mask;

	if (offset >= 64) {
		mask = -1;
	} else {
		mask = (1ULL << offset) - 1;
	}
	/* below operation is translated to asp_andlsr customop*/
	return (value & mask) >> lsb;
}

/**
 * @brief Get a slice of bits from a value, [,)
 *
 * @param[in] value	Numberic value to be sliced.
 * @param[in] first_bit	first bit index.
 * @param[in] last_bit last bit index (not included).
 *
 * @return a slice of bits.
 */
STORAGE_CLASS_INLINE uint32_t bit_slice_f_bit_l_bit(uint32_t value,
		uint32_t first_bit, uint32_t last_bit)
{
	uint32_t mask = (1 << (last_bit - first_bit)) - 1;

	return (value >> first_bit) & mask;
}

/**
 * @brief Get a slice of bytes from a value.
 *
 * @param[in] value	Numberic value to be sliced.
 * @param[in] byte0	Position of the least significiant byte.
 * @param[in] numbytes	Number of bytes in the slice.
 *
 * @return a slice of bytes.
 */
STORAGE_CLASS_INLINE unsigned int byte_slice(unsigned int value,
		unsigned int byte0, unsigned int numbytes)
{
	unsigned int mask = (1 << (numbytes * 8)) - 1;
	return (value >> (byte0 * 8)) & mask;
}

/**
 * @brief Left-shift a value and perform bitwise OR operation with
 * another value.
 *
 * @param[in] val1	Value to be left-shifted.
 * @param[in] numbits	Number of bits to be left-shifted.
 * @param[in] val2	Input to bitwise OR operation.
 *
 * @return the operation result.
 */
STORAGE_CLASS_INLINE unsigned int bit_lshift_OR(unsigned int val1,
		unsigned int numbits, unsigned int val2)
{
	/* below operation is translated to asp_slor customop*/
	return (val1 << numbits) | bit_slice(val2, 0, numbits);
}

/**
 * @brief Set a bit to "1" in a value.
 *
 * @param[in] value	Numeric value.
 * @param[in] bitpo	Bit position.
 *
 * @return the operation result.
 */
STORAGE_CLASS_INLINE unsigned int bit_set(unsigned int value,
		unsigned int bitpos)
{
	return value | (1 << bitpos);
}

/**
 * @brief Clear a bit in a value.
 *
 * @param[in] value	Numeric value.
 * @param[in] bitpos	Bit position.
 *
 * @return the operation result.
 */
STORAGE_CLASS_INLINE unsigned int bit_clear(unsigned int value,
		unsigned int bitpos)
{
	return value & (~(1 << bitpos));
}

/**
 * @brief Get a bit from a value.
 *
 * @param[in] value	Numeric value.
 * @param[in] bitpos	Bit position.
 *
 * @return the value of the bit.
 */
STORAGE_CLASS_INLINE unsigned int bit_get(unsigned int value,
		unsigned int bitpos)
{
	return bit_slice(value, bitpos, 1);
}

#endif /* __BIT_SUPPORT_H */
