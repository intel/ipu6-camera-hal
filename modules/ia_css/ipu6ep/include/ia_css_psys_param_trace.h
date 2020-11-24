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

#ifndef __IA_CSS_PSYS_PARAM_TRACE_H
#define __IA_CSS_PSYS_PARAM_TRACE_H

#include "ia_css_psysapi_trace.h"

#define PSYS_PARAM_TRACE_LEVEL_CONFIG_DEFAULT	PSYSAPI_TRACE_LOG_LEVEL_OFF

/* Default sub-module tracing config */
#if (!defined(PSYSAPI_PARAM_TRACING_OVERRIDE))
     #define PSYS_PARAM_TRACE_LEVEL_CONFIG PSYS_PARAM_TRACE_LEVEL_CONFIG_DEFAULT
#endif

/* Module/sub-module specific trace setting will be used if
 * the trace level is not specified from the module or
  PSYSAPI_PARAM_TRACING_OVERRIDE is defined
 */
#if (defined(PSYSAPI_PARAM_TRACING_OVERRIDE))
	/* Module/sub-module specific trace setting */
	#if PSYSAPI_PARAM_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_OFF
		/* PSYSAPI_TRACE_LOG_LEVEL_OFF */
		#define PSYSAPI_PARAM_TRACE_METHOD \
			IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_PARAM_TRACE_LEVEL_ASSERT \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_ERROR \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_WARNING \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_INFO \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_DEBUG \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_VERBOSE \
			IA_CSS_TRACE_LEVEL_DISABLED
	#elif PSYSAPI_PARAM_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_NORMAL
		/* PSYSAPI_TRACE_LOG_LEVEL_NORMAL */
		#define PSYSAPI_PARAM_TRACE_METHOD \
			IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_PARAM_TRACE_LEVEL_ASSERT \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_ERROR \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_WARNING \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_INFO \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_DEBUG \
			IA_CSS_TRACE_LEVEL_DISABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_VERBOSE \
			IA_CSS_TRACE_LEVEL_DISABLED
	#elif PSYSAPI_PARAM_TRACING_OVERRIDE == PSYSAPI_TRACE_LOG_LEVEL_DEBUG
		/* PSYSAPI_TRACE_LOG_LEVEL_DEBUG */
		#define PSYSAPI_PARAM_TRACE_METHOD \
			IA_CSS_TRACE_METHOD_NATIVE
		#define PSYSAPI_PARAM_TRACE_LEVEL_ASSERT \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_ERROR \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_WARNING \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_INFO \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_DEBUG \
			IA_CSS_TRACE_LEVEL_ENABLED
		#define PSYSAPI_PARAM_TRACE_LEVEL_VERBOSE \
			IA_CSS_TRACE_LEVEL_ENABLED
	#else
		#error "No PSYSAPI_DATA Tracing level defined"
	#endif
#else
	/* Inherit Module trace setting */
	#define PSYSAPI_PARAM_TRACE_METHOD \
		PSYSAPI_TRACE_METHOD
	#define PSYSAPI_PARAM_TRACE_LEVEL_ASSERT \
		PSYSAPI_TRACE_LEVEL_ASSERT
	#define PSYSAPI_PARAM_TRACE_LEVEL_ERROR \
		PSYSAPI_TRACE_LEVEL_ERROR
	#define PSYSAPI_PARAM_TRACE_LEVEL_WARNING \
		PSYSAPI_TRACE_LEVEL_WARNING
	#define PSYSAPI_PARAM_TRACE_LEVEL_INFO \
		PSYSAPI_TRACE_LEVEL_INFO
	#define PSYSAPI_PARAM_TRACE_LEVEL_DEBUG \
		PSYSAPI_TRACE_LEVEL_DEBUG
	#define PSYSAPI_PARAM_TRACE_LEVEL_VERBOSE \
		PSYSAPI_TRACE_LEVEL_VERBOSE
#endif

#endif /* __IA_CSS_PSYSAPI_PARAM_TRACE_H */
