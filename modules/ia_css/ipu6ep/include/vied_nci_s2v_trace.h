/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2016 Intel Corporation.
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

#ifndef __VIED_NCI_S2V_TRACE_H
#define __VIED_NCI_S2V_TRACE_H

#include "ia_css_trace.h"

/* Not using 0 to identify wrong configuration being passed from the .mk file outside.
 * Log levels not in the range below will cause a "No VIED_NCI_S2V_TRACE_CONFIG Tracing level defined"
 */
#define VIED_NCI_S2V_TRACE_LOG_LEVEL_OFF 1
#define VIED_NCI_S2V_TRACE_LOG_LEVEL_NORMAL 2
#define VIED_NCI_S2V_TRACE_LOG_LEVEL_DEBUG 3

/* VIED_NCI_S2V and all the submodules in VIED_NCI_S2V will have the default tracing level set to the VIED_NCI_S2V_TRACE_CONFIG
   level. If not defined in the VIED_NCI_S2V.mk fill it will be set by default to no trace (VIED_NCI_S2V_TRACE_LOG_LEVEL_NORMAL)
 */
#define VIED_NCI_S2V_TRACE_CONFIG_DEFAULT VIED_NCI_S2V_TRACE_LOG_LEVEL_NORMAL

#if !defined(VIED_NCI_S2V_TRACE_CONFIG)
	#define VIED_NCI_S2V_TRACE_CONFIG VIED_NCI_S2V_TRACE_CONFIG_DEFAULT
#endif

/* VIED_NCI_S2V Module tracing backend is mapped to TUNIT tracing for target platforms */
#ifdef IA_CSS_TRACE_PLATFORM_CELL
	#ifndef HRT_CSIM
		#define VIED_NCI_S2V_TRACE_METHOD IA_CSS_TRACE_METHOD_TRACE
	#else
		#define VIED_NCI_S2V_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
	#endif
#else
	#define VIED_NCI_S2V_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
#endif

/* Module specific trace setting will be used if
 * the trace level is not specified from the module or
  VIED_NCI_S2V_TRACING_OVERRIDE is defined
 */
#if (defined(VIED_NCI_S2V_TRACE_CONFIG))
	/* Module specific trace setting */
	#if VIED_NCI_S2V_TRACE_CONFIG == VIED_NCI_S2V_TRACE_LOG_LEVEL_OFF
		/* VIED_NCI_S2V_TRACE_LOG_LEVEL_OFF */
		#define VIED_NCI_S2V_TRACE_LEVEL_ASSERT		IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_ERROR		IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_WARNING 	IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_INFO		IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_DEBUG		IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_VERBOSE 	IA_CSS_TRACE_LEVEL_DISABLED
	#elif VIED_NCI_S2V_TRACE_CONFIG == VIED_NCI_S2V_TRACE_LOG_LEVEL_NORMAL
		/* VIED_NCI_S2V_TRACE_LOG_LEVEL_NORMAL */
		#define VIED_NCI_S2V_TRACE_LEVEL_ASSERT		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_ERROR		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_WARNING 	IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_INFO		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_DEBUG		IA_CSS_TRACE_LEVEL_DISABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_VERBOSE 	IA_CSS_TRACE_LEVEL_DISABLED
	#elif VIED_NCI_S2V_TRACE_CONFIG == VIED_NCI_S2V_TRACE_LOG_LEVEL_DEBUG
		/* VIED_NCI_S2V_TRACE_LOG_LEVEL_DEBUG */
		#define VIED_NCI_S2V_TRACE_LEVEL_ASSERT		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_ERROR		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_WARNING 	IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_INFO		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_DEBUG		IA_CSS_TRACE_LEVEL_ENABLED
		#define VIED_NCI_S2V_TRACE_LEVEL_VERBOSE 	IA_CSS_TRACE_LEVEL_ENABLED
	#else
		#error "No VIED_NCI_S2V_TRACE_CONFIG Tracing level defined"
	#endif
#else
	#error "VIED_NCI_S2V_TRACE_CONFIG not defined"
#endif

#endif /* __VIED_NCI_S2V_TRACE_H */
