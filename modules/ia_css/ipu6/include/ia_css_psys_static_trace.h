/*
 * Copyright (C) 2021 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_STATIC_TRACE_H
#define __IA_CSS_PSYS_STATIC_TRACE_H

#include "ia_css_psysapi_trace.h"

#define PSYS_STATIC_TRACE_LEVEL_CONFIG_DEFAULT    PSYSAPI_TRACE_LOG_LEVEL_OFF

/* Default sub-module tracing config */
#if (!defined(PSYSAPI_STATIC_TRACING_OVERRIDE))
     #define PSYS_STATIC_TRACE_LEVEL_CONFIG \
    PSYS_STATIC_TRACE_LEVEL_CONFIG_DEFAULT
#endif

/* Module/sub-module specific trace setting will be used if
 * the trace level is not specified from the module or
  PSYSAPI_STATIC_TRACING_OVERRIDE is defined
 */
#if (defined(PSYSAPI_STATIC_TRACING_OVERRIDE))
    /* Module/sub-module specific trace setting */
    #if PSYSAPI_STATIC_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_OFF
        /* PSYSAPI_TRACE_LOG_LEVEL_OFF */
        #define PSYSAPI_STATIC_TRACE_METHOD \
            IA_CSS_TRACE_METHOD_NATIVE
        #define PSYSAPI_STATIC_TRACE_LEVEL_ASSERT \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_ERROR \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_WARNING \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_INFO \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_DEBUG \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_VERBOSE \
            IA_CSS_TRACE_LEVEL_DISABLED
    #elif PSYSAPI_STATIC_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_NORMAL
        /* PSYSAPI_TRACE_LOG_LEVEL_NORMAL */
        #define PSYSAPI_STATIC_TRACE_METHOD \
            IA_CSS_TRACE_METHOD_NATIVE
        #define PSYSAPI_STATIC_TRACE_LEVEL_ASSERT \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_ERROR \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_WARNING \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_INFO \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_DEBUG \
            IA_CSS_TRACE_LEVEL_DISABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_VERBOSE \
            IA_CSS_TRACE_LEVEL_DISABLED
    #elif PSYSAPI_STATIC_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_DEBUG
        /* PSYSAPI_TRACE_LOG_LEVEL_DEBUG */
        #define PSYSAPI_STATIC_TRACE_METHOD \
            IA_CSS_TRACE_METHOD_NATIVE
        #define PSYSAPI_STATIC_TRACE_LEVEL_ASSERT \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_ERROR \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_WARNING \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_INFO \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_DEBUG \
            IA_CSS_TRACE_LEVEL_ENABLED
        #define PSYSAPI_STATIC_TRACE_LEVEL_VERBOSE \
            IA_CSS_TRACE_LEVEL_ENABLED
    #else
        #error "No PSYSAPI_DATA Tracing level defined"
    #endif
#else
    /* Inherit Module trace setting */
    #define PSYSAPI_STATIC_TRACE_METHOD \
        PSYSAPI_TRACE_METHOD
    #define PSYSAPI_STATIC_TRACE_LEVEL_ASSERT \
        PSYSAPI_TRACE_LEVEL_ASSERT
    #define PSYSAPI_STATIC_TRACE_LEVEL_ERROR \
        PSYSAPI_TRACE_LEVEL_ERROR
    #define PSYSAPI_STATIC_TRACE_LEVEL_WARNING \
        PSYSAPI_TRACE_LEVEL_WARNING
    #define PSYSAPI_STATIC_TRACE_LEVEL_INFO \
        PSYSAPI_TRACE_LEVEL_INFO
    #define PSYSAPI_STATIC_TRACE_LEVEL_DEBUG \
        PSYSAPI_TRACE_LEVEL_DEBUG
    #define PSYSAPI_STATIC_TRACE_LEVEL_VERBOSE \
        PSYSAPI_TRACE_LEVEL_VERBOSE
#endif

#endif /* __IA_CSS_PSYSAPI_STATIC_TRACE_H */
