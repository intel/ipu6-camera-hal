/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2013 - 2017 Intel Corporation.
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

#ifndef __ERROR_SUPPORT_H
#define __ERROR_SUPPORT_H

#if defined(__KERNEL__)
#include <linux/errno.h>
#else
#include <errno.h>
#endif
#include <assert_support.h>

/* OS-independent definition of IA_CSS errno values */
/* #define IA_CSS_EINVAL 1 */
/* #define IA_CSS_EFAULT 2 */

#ifdef __HIVECC
#define ERR_EMBEDDED 1
#else
#define ERR_EMBEDDED 0
#endif

#if ERR_EMBEDDED
#define DECLARE_ERRVAL
#else
#define DECLARE_ERRVAL \
	int _errval = 0;
#endif

/* Use "owl" in while to prevent compiler warnings in Windows */
#define ALWAYS_FALSE ((void)0, 0)

#define verifret(cond, error_type) \
do {                               \
	if (!(cond)) {             \
		return error_type; \
	}                          \
} while (ALWAYS_FALSE)

#define verifjmp(cond, error_tag)    \
do {                                \
	if (!(cond)) {                   \
		goto error_tag;             \
	}                               \
} while (ALWAYS_FALSE)

#define verifexit(cond)  \
do {                               \
	if (!(cond)) {              \
		goto EXIT;         \
	}                          \
} while (ALWAYS_FALSE)

#if ERR_EMBEDDED
#define verifexitval(cond, error_tag) \
do {                               \
	assert(cond);		   \
} while (ALWAYS_FALSE)
#else
#define verifexitval(cond, error_tag) \
do {                               \
	if (!(cond)) {              \
		_errval = (error_tag); \
		goto EXIT;         \
	}                          \
} while (ALWAYS_FALSE)
#endif

#if ERR_EMBEDDED
#define haserror(error_tag) (0)
#else
#define haserror(error_tag) \
	(_errval == (error_tag))
#endif

#if ERR_EMBEDDED
#define noerror() (1)
#else
#define noerror() \
	(_errval == 0)
#endif

#define verifjmpexit(cond)         \
do {                               \
	if (!(cond)) {             \
		goto EXIT;         \
	}                          \
} while (ALWAYS_FALSE)

#define verifjmpexitsetretval(cond, retval)         \
do {                               \
	if (!(cond)) {              \
		retval = -1;	   \
		goto EXIT;         \
	}                          \
} while (ALWAYS_FALSE)

#endif /* __ERROR_SUPPORT_H */
