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

#ifndef __ASSERT_SUPPORT_H
#define __ASSERT_SUPPORT_H

/* This file provides support for run-time assertions
 * and compile-time assertions.
 *
 * Run-time asstions are provided via the following syntax:
 *     assert(condition)
 * Run-time assertions are disabled using the NDEBUG flag.
 *
 * Compile time assertions are provided via the following syntax:
 *     COMPILATION_ERROR_IF(condition);
 * A compile-time assertion will fail to compile if the condition is false.
 * The condition must be constant, such that it can be evaluated
 * at compile time.
 *
 * OP___assert is deprecated.
 */

#define IA_CSS_ASSERT(expr) assert(expr)

#ifdef __KLOCWORK__
/* Klocwork does not see that assert will lead to abortion
 * as there is no good way to tell this to KW and the code
 * should not depend on assert to function (actually the assert
 * could be disabled in a release build) it was decided to
 * disable the assert for KW scans (by defining NDEBUG)
 * see also:
 * http://www.klocwork.com/products/documentation/current/
 * Tuning_C/C%2B%2B_analysis#Assertions
 */
#define NDEBUG
#endif /* __KLOCWORK__ */

/**
 * The following macro can help to test the size of a struct at compile
 * time rather than at run-time. It does not work for all compilers; see
 * below.
 *
 * Depending on the value of 'condition', the following macro is expanded to:
 * - condition==true:
 *     an expression containing an array declaration with negative size,
 *     usually resulting in a compilation error
 * - condition==false:
 *     (void) 1; // C statement with no effect
 *
 * example:
 *  COMPILATION_ERROR_IF( sizeof(struct host_sp_queues) !=
 *			SIZE_OF_HOST_SP_QUEUES_STRUCT);
 *
 * verify that the macro indeed triggers a compilation error with your compiler:
 *  COMPILATION_ERROR_IF( sizeof(struct host_sp_queues) !=
 *			(sizeof(struct host_sp_queues)+1) );
 *
 * Not all compilers will trigger an error with this macro;
 * use a search engine to search for BUILD_BUG_ON to find other methods.
 */
#ifdef __XTENSA_FW__

#include "assert_support_xtensa.h"

#else

#define COMPILATION_ERROR_IF(condition) \
((void)sizeof(char[1 - 2*!!(condition)]))

#endif
/* Compile time assertion */
#ifndef CT_ASSERT
#define CT_ASSERT(cnd) ((void)sizeof(char[(cnd)?1 :  -1]))
#endif /* CT_ASSERT */

#ifdef NDEBUG

#define assert(cnd) ((void)0)

#else

#include "storage_class.h"

#if defined(_MSC_VER)
#ifdef _KERNEL_MODE
/* Windows kernel mode compilation */
#include <ntddk.h>
#define assert(cnd) ASSERT(cnd)
#else
/* Windows usermode compilation */
#include <assert.h>
#endif

#elif defined(__XTENSA_FW__)
/* assert added in assert_support_xtensa*/
#elif defined(__HIVECC)

/*
 * target: assert disabled
 * sched: assert enabled only when SCHED_DEBUG is defined
 * unsched: assert enabled
 */
#if defined(HRT_HW)
#define assert(cnd) ((void)0)
#elif defined(HRT_SCHED) && !defined(DEBUG_SCHED)
#define assert(cnd) ((void)0)
#elif defined(PIPE_GENERATION)
#define assert(cnd) ((void)0)
#else
#include <hive/support.h>
#define assert(cnd) OP___csim_assert(cnd)
#endif

#elif defined(__KERNEL__)
#include <linux/bug.h>

#ifndef KERNEL_ASSERT_TO_BUG
#ifndef KERNEL_ASSERT_TO_BUG_ON
#ifndef KERNEL_ASSERT_TO_WARN_ON
#ifndef KERNEL_ASSERT_TO_WARN_ON_INF_LOOP
#ifndef KERNEL_ASSERT_UNDEFINED
/* Default */
#define KERNEL_ASSERT_TO_BUG
#endif /*KERNEL_ASSERT_UNDEFINED*/
#endif /*KERNEL_ASSERT_TO_WARN_ON_INF_LOOP*/
#endif /*KERNEL_ASSERT_TO_WARN_ON*/
#endif /*KERNEL_ASSERT_TO_BUG_ON*/
#endif /*KERNEL_ASSERT_TO_BUG*/

#ifdef KERNEL_ASSERT_TO_BUG
/* TODO: it would be cleaner to use this:
 * #define assert(cnd) BUG_ON(cnd)
 * but that causes many compiler warnings (==errors) under Android
 * because it seems that the BUG_ON() macro is not seen as a check by
 * gcc like the BUG() macro is. */
#define assert(cnd)							\
	do {								\
		if (!(cnd)) {						\
			BUG();						\
		}							\
	} while (0)
#endif /*KERNEL_ASSERT_TO_BUG*/

#ifdef KERNEL_ASSERT_TO_BUG_ON
#define assert(cnd) BUG_ON(!(cnd))
#endif /*KERNEL_ASSERT_TO_BUG_ON*/

#ifdef KERNEL_ASSERT_TO_WARN_ON
#define assert(cnd) WARN_ON(!(cnd))
#endif /*KERNEL_ASSERT_TO_WARN_ON*/

#ifdef KERNEL_ASSERT_TO_WARN_ON_INF_LOOP
#define assert(cnd)							\
	do {								\
		int not_cnd = !(cnd);					\
		WARN_ON(not_cnd);					\
		if (not_cnd) {						\
			for (;;) {					\
			}						\
		}							\
	} while (0)
#endif /*KERNEL_ASSERT_TO_WARN_ON_INF_LOOP*/

#ifdef KERNEL_ASSERT_UNDEFINED
#include KERNEL_ASSERT_DEFINITION_FILESTRING
#endif /*KERNEL_ASSERT_UNDEFINED*/

#elif defined(__FIST__) || defined(__GNUC__)

#include "assert.h"

#else /* default is for unknown environments */
#define assert(cnd) ((void)0)
#endif

#endif /* NDEBUG */
#ifndef PIPE_GENERATION
/* Deprecated OP___assert, this is still used in ~1000 places
 * in the code. This will be removed over time.
 * The implementation for the pipe generation tool is in see support.isp.h */
#define OP___assert(cnd) assert(cnd)

#ifdef C_RUN
#define compile_time_assert(cond) OP___assert(cond)
#else
#include "storage_class.h"
extern void _compile_time_assert(void);
STORAGE_CLASS_INLINE void compile_time_assert(unsigned cond)
{
	/* Call undefined function if cond is false */
	if (!cond)
		_compile_time_assert();
}
#endif
#endif /* PIPE_GENERATION */

#endif /* __ASSERT_SUPPORT_H */
