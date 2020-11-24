/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2020 Intel Corporation.
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

#ifndef __PRINT_SUPPORT_H
#define __PRINT_SUPPORT_H

#if defined(_MSC_VER)
#ifdef _KERNEL_MODE

/* TODO: Windows driver team to provide tracing mechanism for kernel mode
 * e.g. DbgPrint and DbgPrintEx
 */
extern void FwTracePrintPWARN(const char *fmt, ...);
extern void FwTracePrintPRINT(const char *fmt, ...);
extern void FwTracePrintPERROR(const char *fmt, ...);
extern void FwTracePrintPDEBUG(const char *fmt, ...);
extern void FwTracePrintPFATAL(const char *fmt, ...);

#define PWARN(format, ...)	FwTracePrintPWARN(format, __VA_ARGS__)
#define PWARNING(format, ...)	FwTracePrintPWARN(format, __VA_ARGS__)
#define PRINT(format, ...)	FwTracePrintPRINT(format, __VA_ARGS__)
#define PINFO(format, ...)	FwTracePrintPRINT(format, __VA_ARGS__)
#define PERROR(format, ...)	FwTracePrintPERROR(format, __VA_ARGS__)
#define PDEBUG(format, ...)	FwTracePrintPDEBUG(format, __VA_ARGS__)
#define PVERBOSE(format, ...)	FwTracePrintPRINT(format, __VA_ARGS__)
#define PFATAL(format, ...)	FwTracePrintPFATAL(format, __VA_ARGS__)

#else
/* Windows usermode compilation */
#include <stdio.h>

/* To change the defines below, communicate with Windows team first
 * to ensure they will not get flooded with prints
 */
/* This is temporary workaround to avoid flooding userspace
 * Windows driver with prints
 */

#define PWARN(format, ...)
#define PWARNING(format, ...)
#define PRINT(format, ...)
#define PINFO(format, ...)
#define PERROR(format, ...)	printf("error: " format, __VA_ARGS__)
#define PDEBUG(format, ...)
#define PVERBOSE(format, ...)
#define PFATAL(format, ...)

#endif /* _KERNEL_MODE */
#elif defined(__HIVECC)
#include <hive/support.h>
/* To be revised

#define PWARN(format)
#define PRINT(format)				OP___printstring(format)
#define PERROR(variable)			OP___dump(9999, arguments)
#define PDEBUG(variable)			OP___dump(__LINE__, arguments)

*/

#define PRINTSTRING(str) OP___printstring(str)

#elif defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/printk.h>

#define PWARN(format, arguments...)	pr_debug(format, ##arguments)
#define PRINT(format, arguments...)	pr_debug(format, ##arguments)
#define PERROR(format, arguments...)	pr_debug(format, ##arguments)
#define PDEBUG(format, arguments...)	pr_debug(format, ##arguments)

#elif defined(__XTENSA_FW__)

#include "print_support_xtensa.h"

#elif defined(XTENSA_HOST)

#include "print_support_xtensa.h"

#else

#include <stdio.h>

#define PRINT_HELPER(prefix, format, ...) printf(prefix format "%s", __VA_ARGS__)

/** WA_1507432540 - Limit prints to PERROR only
 * Workaround to avoid flooding host with prints.
 */
#ifdef HOST_LOGLEVEL_ERROR_ONLY

/* The trailing "" allows the edge case of printing single string */
#define PWARN(...)
#define PRINT(...)
#define PERROR(...) PRINT_HELPER("error: ", __VA_ARGS__, "")
#define PDEBUG(...)

#else

/* The trailing "" allows the edge case of printing single string */
#define PWARN(...)  PRINT_HELPER("warning: ", __VA_ARGS__, "")
#define PRINT(...)  PRINT_HELPER("", __VA_ARGS__, "")
#define PERROR(...) PRINT_HELPER("error: ", __VA_ARGS__, "")
#define PDEBUG(...) PRINT_HELPER("debug: ", __VA_ARGS__, "")

#endif
#define PRINTSTRING(str) PRINT(str)

#endif
#endif /* __PRINT_SUPPORT_H */
