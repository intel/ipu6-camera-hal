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

#ifndef __MATH_SUPPORT_H
#define __MATH_SUPPORT_H

#include "storage_class.h" /* for STORAGE_CLASS_INLINE */
#include "type_support.h"
#include "assert_support.h"

/* in case we have min/max/MIN/MAX macro's undefine them */
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef MIN /* also defined in include/hrt/numeric.h from SDK */
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

#ifndef INT8_MIN
#define INT8_MIN        (-127 - 1)
#endif

#ifndef INT8_MAX
#define INT8_MAX        (127)
#endif

#ifndef UINT8_MAX
#define UINT8_MAX       (0xffUL)
#endif

#ifndef UINT16_MAX
#define UINT16_MAX       (0xffffUL)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX       (0xffffffffUL)
#endif

#define IS_ODD(a) ((a) & 0x1)
#define IS_EVEN(a) (!IS_ODD(a))
#define IS_POWER2(a) (!((a)&((a)-1)))
#define IS_MULTIPLE_OF(a, b) ((a % b) == 0)
#define IS_MASK_BITS_SET(a, b)	((a & b) != 0)

/*To Find next power of 2 number from x */
#define bit2(x)            ((x)      | ((x) >> 1))
#define bit4(x)            (bit2(x)  | (bit2(x) >> 2))
#define bit8(x)            (bit4(x)  | (bit4(x) >> 4))
#define bit16(x)           (bit8(x)  | (bit8(x) >> 8))
#define bit32(x)           (bit16(x) | (bit16(x) >> 16))
#define NEXT_POWER_OF_2(x) (bit32(x-1) + 1)

/* force a value to a lower even value */
#define EVEN_FLOOR(x)	((x) & ~1UL)

/* A => B */
#define IMPLIES(a, b) (!(a) || (b))

/* The ORIG_BITS th bit is the sign bit */
/* Sign extends a ORIG_BITS bits long signed number to a 64-bit signed number */
/* By type casting it can relimited to any valid type-size
 * (32-bit signed or 16-bit or 8-bit)
 */
/* By masking it can be transformed to any arbitrary bit size */
#define SIGN_EXTEND(VAL, ORIG_BITS) \
((~(((VAL)&(1ULL<<((ORIG_BITS)-1)))-1))|(VAL))

#define EXTRACT_BIT(a, b)	((a >> b) & 1)

/* for preprocessor and array sizing use MIN and MAX
   otherwise use min and max */
#define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define CLIP(a, b, c)		MIN((MAX((a), (b))), (c))
/* Integer round-down division of a with b */
#define FLOOR_DIV(a, b)		((b) ? ((a) / (b)) : 0)
/* Align a to the lower multiple of b */
#define FLOOR_MUL(a, b)		(FLOOR_DIV(a, b) * (b))
/* Integer round-up division of a with b */
#define CEIL_DIV(a, b)		((b) ? (((a) + (b) - 1) / (b)) : 0)
/* Align a to the upper multiple of b */
#define CEIL_MUL(a, b)		(CEIL_DIV(a, b) * (b))
/* Align a to the upper multiple of b - fast implementation
 * for cases when b=pow(2,n)
 */
#define CEIL_MUL2(a, b)		(((a) + (b) - 1) & ~((b) - 1))
/* integer round-up division of a with pow(2,b) */
#define CEIL_SHIFT(a, b)	(((a) + (1UL << (b)) - 1) >> (b))
/* Align a to the upper multiple of pow(2,b) */
#define CEIL_SHIFT_MUL(a, b)	(CEIL_SHIFT(a, b) << (b))
/* Absolute difference of a and b */
#define ABS_DIF(a, b)		(((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define ABS(a) ABS_DIF(a, 0)
/* Square of x */
#define SQR(x)			((x)*(x))
/* Integer round-half-down division of a nad b */
#define ROUND_HALF_DOWN_DIV(a, b)	((b) ? ((a) + (b / 2) - 1) / (b) : 0)
/* Align a to the round-half-down multiple of b */
#define ROUND_HALF_DOWN_MUL(a, b)	(ROUND_HALF_DOWN_DIV(a, b) * (b))

#define MAX3(a, b, c)		MAX((a), MAX((b), (c)))
#define MIN3(a, b, c)		MIN((a), MIN((b), (c)))
#define MAX4(a, b, c, d)	MAX((MAX((a), (b))), (MAX((c), (d))))
#define MIN4(a, b, c, d)	MIN((MIN((a), (b))), (MIN((c), (d))))

/* min and max should not be macros as they will evaluate their arguments twice.
   if you really need a macro (e.g. for CPP or for initializing an array)
   use MIN() and MAX(), otherwise use min() and max() */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(*(a))))
#endif

#ifndef BYTES
#define BYTES(bit) (((bit)+7)/8)
#endif

#if !defined(PIPE_GENERATION)
STORAGE_CLASS_INLINE unsigned int max_value_bits(unsigned int bits)
{
	return (bits == 0) ? 0 : ((2 * ((1 << ((bits) - 1)) - 1)) + 1);
}
STORAGE_CLASS_INLINE unsigned int max_value_bytes(unsigned int bytes)
{
	return max_value_bits(IA_CSS_UINT8_T_BITS * bytes);
}
STORAGE_CLASS_INLINE int max(int a, int b)
{
	return MAX(a, b);
}

STORAGE_CLASS_INLINE int min(int a, int b)
{
	return MIN(a, b);
}

STORAGE_CLASS_INLINE int clip(int a, int b, int c)
{
	return min(max(a, b), c);
}

STORAGE_CLASS_INLINE unsigned int umax(unsigned int a, unsigned int b)
{
	return MAX(a, b);
}

STORAGE_CLASS_INLINE unsigned int umin(unsigned int a, unsigned int b)
{
	return MIN(a, b);
}

STORAGE_CLASS_INLINE unsigned int uclip(unsigned int a, unsigned int b,
					unsigned int c)
{
	return umin(umax(a, b), c);
}

STORAGE_CLASS_INLINE unsigned int ceil_div(unsigned int a, unsigned int b)
{
	return CEIL_DIV(a, b);
}

STORAGE_CLASS_INLINE unsigned int ceil_mul(unsigned int a, unsigned int b)
{
	return CEIL_MUL(a, b);
}

STORAGE_CLASS_INLINE unsigned int ceil_mul2(unsigned int a, unsigned int b)
{
	return CEIL_MUL2(a, b);
}

STORAGE_CLASS_INLINE unsigned int ceil_shift(unsigned int a, unsigned int b)
{
	return CEIL_SHIFT(a, b);
}

STORAGE_CLASS_INLINE unsigned int ceil_shift_mul(unsigned int a, unsigned int b)
{
	return CEIL_SHIFT_MUL(a, b);
}

STORAGE_CLASS_INLINE int abs_dif(int a, int b)
{
	return ABS_DIF(a, b);
}

STORAGE_CLASS_INLINE unsigned int uabs_dif(unsigned int a, unsigned int b)
{
	return ABS_DIF(a, b);
}

STORAGE_CLASS_INLINE unsigned int round_half_down_div(unsigned int a,
		unsigned int b)
{
	return ROUND_HALF_DOWN_DIV(a, b);
}

STORAGE_CLASS_INLINE unsigned int round_half_down_mul(unsigned int a,
		unsigned int b)
{
	return ROUND_HALF_DOWN_MUL(a, b);
}

STORAGE_CLASS_INLINE unsigned int ceil_pow2(uint32_t a)
{
	unsigned int retval = 0;

	if (IS_POWER2(a)) {
		retval =  (unsigned int)a;
	} else {
		unsigned int v = a;

		v |= v>>1;
		v |= v>>2;
		v |= v>>4;
		v |= v>>8;
		v |= v>>16;
		retval =  (unsigned int)(v+1);
	}
	return retval;
}

STORAGE_CLASS_INLINE unsigned int floor_log2(uint32_t a)
{
	static const uint8_t de_bruijn[] = {
		0,  9,  1, 10, 13, 21,  2, 29, 11, 14, 16, 18, 22, 25,  3, 30,
		8, 12, 20, 28, 15, 17, 24,  7, 19, 27, 23,  6, 26,  5,  4, 31
	};
	uint32_t v = a;

	v |= v>>1;
	v |= v>>2;
	v |= v>>4;
	v |= v>>8;
	v |= v>>16;
	return (unsigned int)de_bruijn[(v*0x07C4ACDDU)>>27];
}

/* Divide by small power of two */
STORAGE_CLASS_INLINE unsigned int
udiv2_small_i(uint32_t a, uint32_t b)
{
	assert(b <= 2);
	return a >> (b-1);
}

/* optimized divide for small results
 * a will be divided by b
 * outbits is the number of bits needed for the result
 * the smaller the cheaper the function will be.
 * if the result doesn't fit in the number of output bits
 * the result is incorrect and the function will assert
 */
STORAGE_CLASS_INLINE unsigned int
udiv_medium(uint32_t a, uint32_t b, unsigned outbits)
{
	int bit;
	unsigned res = 0;
	unsigned mask;

#ifdef VOLCANO
#pragma ipu unroll
#endif
	for (bit = outbits-1 ; bit >= 0; bit--) {
		mask = 1<<bit;
		if (a >= (b<<bit)) {
			res |= mask; /* set the bit */
			a = a - (b<<bit);
		}
#ifdef __HIVECC
#pragma hivecc unroll
#endif
	}
	/* check if the remainder is smaller than the divisor.
	 * if not we didn't produce enough output bits
	 */
	assert(a < b);
	return res;
}

#if !defined(__VIED_CELL)
/*
 * For SP and ISP, SDK provides the definition of OP_std_modadd.
 * We need it only for host
 */
STORAGE_CLASS_INLINE unsigned int OP_std_modadd(int a, int b, int c)
{
	return (a+b < 0 ? a+b+c : a+b >= c ? a+b-c : a+b);
}

/*
 * For SP and ISP, SDK provides the definition of OP_asp_slor.
 * We need it only for host
 */
STORAGE_CLASS_INLINE unsigned int OP_asp_slor(int a, int b, int c)
{
	return ((a << c) | b);
}
#else
#include "hive/customops.h"
#endif /* !defined(__VIED_CELL) */

#endif /* !defined(PIPE_GENERATION) */
#if !defined(__KERNEL__)
#define clamp(a, min_val, max_val) MIN(MAX((a), (min_val)), (max_val))
#endif /* !defined(__KERNEL__) */

#endif /* __MATH_SUPPORT_H */
