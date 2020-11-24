/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __IA_CSS_RBM_TRACE_H
#define __IA_CSS_RBM_TRACE_H

#include "ia_css_trace.h"

/* Not using 0 to identify wrong configuration being passed from the .mk file outside.
* Log levels not in the range below will cause a "No RBM_TRACE_CONFIG Tracing level defined"
*/
#define RBM_TRACE_LOG_LEVEL_OFF 1
#define RBM_TRACE_LOG_LEVEL_NORMAL 2
#define RBM_TRACE_LOG_LEVEL_DEBUG 3

#define RBM_TRACE_CONFIG_DEFAULT RBM_TRACE_LOG_LEVEL_NORMAL

#if !defined(RBM_TRACE_CONFIG)
#	define RBM_TRACE_CONFIG RBM_TRACE_CONFIG_DEFAULT
#endif

/* IPU_RESOURCE Module tracing backend is mapped to TUNIT tracing for target platforms */
#ifdef __HIVECC
#	ifndef HRT_CSIM
#		define RBM_TRACE_METHOD IA_CSS_TRACE_METHOD_TRACE
#	else
#		define RBM_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
#	endif
#else
#	define RBM_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
#endif

#if (defined(RBM_TRACE_CONFIG))
/* Module specific trace setting */
#	if RBM_TRACE_CONFIG == RBM_TRACE_LOG_LEVEL_OFF
/* RBM_TRACE_LOG_LEVEL_OFF */
#		define RBM_TRACE_LEVEL_ASSERT	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_ERROR	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_WARNING	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_INFO	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_DEBUG	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_VERBOSE	IA_CSS_TRACE_LEVEL_DISABLED
#	elif RBM_TRACE_CONFIG == RBM_TRACE_LOG_LEVEL_NORMAL
/* RBM_TRACE_LOG_LEVEL_NORMAL */
#		define RBM_TRACE_LEVEL_ASSERT	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_ERROR	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_WARNING	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_INFO	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_DEBUG	IA_CSS_TRACE_LEVEL_DISABLED
#		define RBM_TRACE_LEVEL_VERBOSE	IA_CSS_TRACE_LEVEL_DISABLED
#	elif RBM_TRACE_CONFIG == RBM_TRACE_LOG_LEVEL_DEBUG
/* RBM_TRACE_LOG_LEVEL_DEBUG */
#		define RBM_TRACE_LEVEL_ASSERT	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_ERROR	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_WARNING	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_INFO	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_DEBUG	IA_CSS_TRACE_LEVEL_ENABLED
#		define RBM_TRACE_LEVEL_VERBOSE	IA_CSS_TRACE_LEVEL_ENABLED
#	else
#		error "No RBM_TRACE_CONFIG Tracing level defined"
#	endif
#else
#	error "RBM_TRACE_CONFIG not defined"
#endif

#endif /* __RBM_TRACE_H */
