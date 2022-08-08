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

#ifndef __STORAGE_CLASS_H
#define __STORAGE_CLASS_H

#define STORAGE_CLASS_EXTERN \
extern

#if defined(_MSC_VER)
#define STORAGE_CLASS_INLINE \
static __inline
#elif defined(__HIVECC)
#define STORAGE_CLASS_INLINE \
static inline
#elif defined(__XTENSA_FW__)
#include "storage_class_xtensa.h"
#else
#define STORAGE_CLASS_INLINE \
static inline
#endif

/* Register struct */
#ifndef __register
#if defined(__HIVECC) && !defined(PIPE_GENERATION)
#define __register register
#else
#define __register
#endif
#endif

/* Memory attribute */
#ifndef MEM
#ifdef PIPE_GENERATION
#elif defined(__HIVECC)
#include <hive/attributes.h>
#else
#define MEM(any_mem)
#endif
#endif

#endif /* __STORAGE_CLASS_H */
