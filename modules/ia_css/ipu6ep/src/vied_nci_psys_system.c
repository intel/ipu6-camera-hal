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

#include "ia_css_psys_sim_storage_class.h"

/*
 * Functions to possibly inline
 */

#ifdef __IA_CSS_PSYS_SIM_INLINE__
STORAGE_CLASS_INLINE int
__ia_css_psys_system_global_avoid_warning_on_empty_file(void) { return 0; }
#else /* __IA_CSS_PSYS_SIM_INLINE__ */
#include "psys_system_global_impl.h"
#endif /* __IA_CSS_PSYS_SIM_INLINE__ */
