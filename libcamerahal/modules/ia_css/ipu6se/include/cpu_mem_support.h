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

#ifndef __CPU_MEM_SUPPORT_H
#define __CPU_MEM_SUPPORT_H

#include "storage_class.h"
#include "assert_support.h"
#include "type_support.h"

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_copy(void *dst, const void *src, unsigned int size)
{
	/* memcpy cannot be used in in Windows (function is not allowed),
	 * and the safer function memcpy_s is not available on other platforms.
	 * Because usage of ia_css_cpu_mem_copy is minimal, we implement it here in an easy,
	 * but sub-optimal way.
	 */
	unsigned int i;

	assert(dst != NULL && src != NULL);

	if (!(dst != NULL && src != NULL)) {
		return NULL;
	}
	for (i = 0; i < size; i++) {
		((char *)dst)[i] = ((char *)src)[i];
	}
	return dst;
}

#if defined(__KERNEL__)

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <asm/cacheflush.h>

/* TODO: remove, workaround for issue in hrt file ibuf_ctrl_2600_config.c
 * error checking code added to SDK that uses calls to exit function
 */
#define exit(a) return

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc(unsigned int size)
{
	return kmalloc(size, GFP_KERNEL);
}

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc_page_aligned(unsigned int size)
{
	return ia_css_cpu_mem_alloc(size); /* todo: align to page size */
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_protect(void *ptr, unsigned int size, int prot)
{
	/* nothing here yet */
}

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_set_zero(void *dst, unsigned int size)
{
	return memset(dst, 0, size); /* available in kernel in linux/string.h */
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_free(void *ptr)
{
	kfree(ptr);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_flush(void *ptr, unsigned int size)
{
	/* parameter check here */
	if (ptr == NULL)
		return;

	clflush_cache_range(ptr, size);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_invalidate(void *ptr, unsigned int size)
{
	/* for now same as flush */
	ia_css_cpu_mem_cache_flush(ptr, size);
}

#elif defined(_MSC_VER)

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

extern void *hrt_malloc(size_t bytes, int zero_mem);
extern void *hrt_free(void *ptr);
extern void hrt_mem_cache_flush(void *ptr, unsigned int size);
extern void hrt_mem_cache_invalidate(void *ptr, unsigned int size);

#define malloc(a)	hrt_malloc(a, 1)
#define free(a)		hrt_free(a)

#define CSS_PAGE_SIZE	(1<<12)

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc(unsigned int size)
{
	return malloc(size);
}

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc_page_aligned(unsigned int size)
{
	unsigned int buffer_size = size;

	/* Currently hrt_malloc calls Windows ExAllocatePoolWithTag() routine
	 * to request system memory. If the number of bytes is equal or bigger
	 * than the page size, then the returned address is page aligned,
	 * but if it's smaller it's not necessarily page-aligned We agreed
	 * with Windows team that we allocate a full page
	 * if it's less than page size
	*/
	if (buffer_size < CSS_PAGE_SIZE)
		buffer_size = CSS_PAGE_SIZE;

	return ia_css_cpu_mem_alloc(buffer_size);
}

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_set_zero(void *dst, unsigned int size)
{
	return memset(dst, 0, size);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_free(void *ptr)
{
	free(ptr);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_flush(void *ptr, unsigned int size)
{
#ifdef _KERNEL_MODE
	hrt_mem_cache_flush(ptr, size);
#else
	(void)ptr;
	(void)size;
#endif
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_invalidate(void *ptr, unsigned int size)
{
#ifdef _KERNEL_MODE
	hrt_mem_cache_invalidate(ptr, size);
#else
	(void)ptr;
	(void)size;
#endif
}

#else

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
/* Needed for the MPROTECT */
#include <unistd.h>
#include <sys/types.h>

/* For host only, not fw */
#ifndef __XTENSA_FW__
#include <sys/mman.h>
#include <malloc.h>
#endif

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc(unsigned int size)
{
	return malloc(size);
}

#ifdef _SC_PAGE_SIZE
STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_alloc_page_aligned(unsigned int size)
{
	int pagesize;

	pagesize = sysconf(_SC_PAGE_SIZE);
	return memalign(pagesize, size);
}
#endif /*_SC_PAGE_SIZE*/

STORAGE_CLASS_INLINE void*
ia_css_cpu_mem_set_zero(void *dst, unsigned int size)
{
	return memset(dst, 0, size);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_free(void *ptr)
{
	free(ptr);
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_flush(void *ptr, unsigned int size)
{
	/* not needed in simulation */
	(void)ptr;
	(void)size;
}

STORAGE_CLASS_INLINE void
ia_css_cpu_mem_cache_invalidate(void *ptr, unsigned int size)
{
	/* not needed in simulation */
	(void)ptr;
	(void)size;
}

#endif
#endif /* __CPU_MEM_SUPPORT_H */
