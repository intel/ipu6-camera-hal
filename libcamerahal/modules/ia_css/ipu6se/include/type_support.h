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

#ifndef __TYPE_SUPPORT_H
#define __TYPE_SUPPORT_H

/* Per the DLI spec, types are in "type_support.h" and
 * "platform_support.h" is for unclassified/to be refactored
 * platform specific definitions.
 */
#define IA_CSS_UINT8_T_BITS	8
#define IA_CSS_UINT16_T_BITS	16
#define IA_CSS_UINT32_T_BITS	32
#define IA_CSS_INT32_T_BITS	32
#define IA_CSS_UINT64_T_BITS	64

#if defined(_MSC_VER)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#if defined(_M_X64)
#define HOST_ADDRESS(x) (unsigned long long)(x)
#else
#define HOST_ADDRESS(x) (unsigned long)(x)
#endif

#elif defined(PARAM_GENERATION)
/* Nothing */
#elif defined(__HIVECC)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#define HOST_ADDRESS(x) (unsigned long)(x)

typedef long long int64_t;
typedef unsigned long long uint64_t;

#elif defined(__XTENSA_FW__)
#include "type_support_xtensa.h"
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/limits.h>

#define CHAR_BIT (8)
#define HOST_ADDRESS(x) (unsigned long)(x)

#elif defined(__GNUC__)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#define HOST_ADDRESS(x) (unsigned long)(x)

#else /* default is for the FIST environment */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#define HOST_ADDRESS(x) (unsigned long)(x)

#endif
#if !defined(PIPE_GENERATION) && !defined(IO_GENERATION)
/* genpipe cannot handle the void* syntax */
typedef void *HANDLE;
#endif

#endif /* __TYPE_SUPPORT_H */
