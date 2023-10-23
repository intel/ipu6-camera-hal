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

#ifndef __IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
#define __IA_CSS_PSYS_STATIC_STORAGE_CLASS_H

#include "storage_class.h"

#ifndef __IA_CSS_PSYS_STATIC_INLINE__
#define IA_CSS_PSYS_STATIC_STORAGE_CLASS_H STORAGE_CLASS_EXTERN
#define IA_CSS_PSYS_STATIC_STORAGE_CLASS_C
#else
#define IA_CSS_PSYS_STATIC_STORAGE_CLASS_H STORAGE_CLASS_INLINE
#define IA_CSS_PSYS_STATIC_STORAGE_CLASS_C STORAGE_CLASS_INLINE
#endif

#endif /* __IA_CSS_PSYS_STATIC_STORAGE_CLASS_H */
