/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __NCI_MBR_TRACE_H
#define __NCI_MBR_TRACE_H

#include "ia_css_trace.h"

#define NCI_MBR_TRACE_LOG_LEVEL_DEFAULT	1
#define NCI_MBR_TRACE_LOG_LEVEL_DEBUG	2

/* Set to default level if no level is defined */
#ifndef NCI_MBR_TRACE_LEVEL
#define NCI_MBR_TRACE_LEVEL	NCI_MBR_TRACE_LOG_LEVEL_DEFAULT
#endif /* NCI_MBR_TRACE_LEVEL */

/* NCI_MBR Module tracing backend is mapped to TUNIT tracing for target platforms */
#ifdef __HIVECC
#	ifndef HRT_CSIM
#		define NCI_MBR_TRACE_METHOD IA_CSS_TRACE_METHOD_TRACE
#	else
#		define NCI_MBR_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
#	endif
#else
#	define NCI_MBR_TRACE_METHOD IA_CSS_TRACE_METHOD_NATIVE
#endif

#define NCI_MBR_TRACE_LEVEL_INFO	IA_CSS_TRACE_LEVEL_ENABLED
#define NCI_MBR_TRACE_LEVEL_ERROR	IA_CSS_TRACE_LEVEL_ENABLED

#if (NCI_MBR_TRACE_LEVEL == NCI_MBR_TRACE_LOG_LEVEL_DEFAULT)
#	define NCI_MBR_TRACE_LEVEL_VERBOSE		IA_CSS_TRACE_LEVEL_DISABLED
#	define NCI_MBR_TRACE_LEVEL_DEBUG		IA_CSS_TRACE_LEVEL_DISABLED
#	define NCI_MBR_TRACE_LEVEL_WARNING		IA_CSS_TRACE_LEVEL_DISABLED
#elif (NCI_MBR_TRACE_LEVEL == NCI_MBR_TRACE_LOG_LEVEL_DEBUG)
#	define NCI_MBR_TRACE_LEVEL_VERBOSE		IA_CSS_TRACE_LEVEL_ENABLED
#	define NCI_MBR_TRACE_LEVEL_DEBUG		IA_CSS_TRACE_LEVEL_ENABLED
#	define NCI_MBR_TRACE_LEVEL_WARNING		IA_CSS_TRACE_LEVEL_ENABLED
#else
#	error "NCI_MBR trace level not defined!"
#endif /* NCI_MBR_TRACE_LEVEL */

#endif /* __NCI_MBR_TRACE_H */
