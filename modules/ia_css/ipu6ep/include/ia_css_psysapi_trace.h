/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2016 Intel Corporation.
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

#ifndef __IA_CSS_PSYSAPI_TRACE_H
#define __IA_CSS_PSYSAPI_TRACE_H

#include "ia_css_trace.h"

#define PSYSAPI_TRACE_LOG_LEVEL_OFF 0
#define PSYSAPI_TRACE_LOG_LEVEL_NORMAL 1
#define PSYSAPI_TRACE_LOG_LEVEL_DEBUG 2

/* PSYSAPI and all the submodules in PSYSAPI will have the default tracing
 * level set to the PSYSAPI_TRACE_CONFIG level. If not defined in the
 * psysapi.mk fill it will be set by default to no trace
 * (PSYSAPI_TRACE_LOG_LEVEL_OFF)
 */
#define PSYSAPI_TRACE_CONFIG_DEFAULT PSYSAPI_TRACE_LOG_LEVEL_OFF

#if !defined(PSYSAPI_TRACE_CONFIG)
	#define PSYSAPI_TRACE_CONFIG PSYSAPI_TRACE_CONFIG_DEFAULT
#endif

/* Module specific trace setting will be used if
 * the trace level is not specified from the module or
  PSYSAPI_TRACING_OVERRIDE is defined
 */
#if (defined(PSYSAPI_TRACE_CONFIG))
	/* Module specific trace setting */
	#if PSYSAPI_TRACE_CONFIG == PSYSAPI_TRACE_LOG_LEVEL_OFF
		/* PSYSAPI_TRACE_LOG_LEVEL_OFF */
		#define PSYSAPI_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_TRACE_LEVEL_ASSERT IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_ERROR IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_WARNING IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_INFO IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_DEBUG IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_VERBOSE IA_CSS_TRACE_LEVEL_DISABLED
	#elif PSYSAPI_TRACE_CONFIG == PSYSAPI_TRACE_LOG_LEVEL_NORMAL
		/* PSYSAPI_TRACE_LOG_LEVEL_NORMAL */
		#define PSYSAPI_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_TRACE_LEVEL_ASSERT IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_ERROR IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_WARNING IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_INFO IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_DEBUG IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_TRACE_LEVEL_VERBOSE IA_CSS_TRACE_LEVEL_DISABLED
	#elif PSYSAPI_TRACE_CONFIG == PSYSAPI_TRACE_LOG_LEVEL_DEBUG
		/* PSYSAPI_TRACE_LOG_LEVEL_DEBUG */
		#define PSYSAPI_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_TRACE_LEVEL_ASSERT IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_ERROR IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_WARNING IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_INFO IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_DEBUG IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_TRACE_LEVEL_VERBOSE IA_CSS_TRACE_LEVEL_ENABLED
	#else
		#error "No PSYSAPI_TRACE_CONFIG Tracing level defined"
	#endif
#else
	#error "PSYSAPI_TRACE_CONFIG not defined"
#endif

/* Overriding submodules in PSYSAPI with a specific tracing level */
/* #define PSYSAPI_DYNAMIC_TRACING_OVERRIDE TRACE_LOG_LEVEL_VERBOSE */

#endif /* __IA_CSS_PSYSAPI_TRACE_H */
