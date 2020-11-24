/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2015 - 2019 Intel Corporation.
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

/**
 * @file logger.h
 *
 * @brief Global objects needed for printing messages while doing logging
 */

#ifndef __LOGGER_H
#define __LOGGER_H

#include <string.h>
#include "print_support.h"
#include "misc_support.h"
#include "storage_class.h"
#include "assert_support.h"

/**
 * This variable represent the log message subtype
 */
enum loglevel {
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERROR
};

/**
 * This macro gives the short name of the input file, instead of providing the full path.
 * Note that the macro only might give a desired result on certain platforms.
 * In case no '/' is found, the input string is returned.
 */
#define __FILE_NO_PATH__ (__file_no_path__private(__FILE__))

STORAGE_CLASS_INLINE const char *__file_no_path__private(const char *file)
{
	const char *last_slash;

	assert(file != NULL); /* responsibility of caller to check for NULL */

	last_slash = strrchr(file, '/');
	/*
	 * If last_slash != NULL, then (last_slash + 1) is always a valid string.
	 * Note that *(last_slash + 1) is allowed to be and can be the string terminating character.
	 */
	return ((last_slash != NULL) ? (last_slash + 1) : file);
}

/**
 * Macro
 */

#ifdef HOST_DEBUG_ENABLE
#define PRINT_LOG_PRINT_DEBUG_LEVEL 1
#else
#define PRINT_LOG_PRINT_DEBUG_LEVEL 0
#endif

/** WA_1507432540 - Limit prints to LOGLEVEL_ERROR only
 * Workaround to avoid flooding host with prints.
 */
#ifdef HOST_LOGLEVEL_ERROR_ONLY
#define PRINT_LOG_PRINT_ERROR_LEVEL_ONLY 1
#else
#define PRINT_LOG_PRINT_ERROR_LEVEL_ONLY 0
#endif

/** A code-path without using logger.c is implemented in order to avoid
 * the issue with unsafe functions that are used in logger.c.
 * Certain functions, like strlen, vsnprintf, ... are considered unsafe and
 * are not allowed in the Windows user mode release package (which uses logger.h).
 * However, the safe versions like strlen_s are not available for the FW build (GCC).
 * See ticket #H1504581314.
 */
#ifdef HOST_LOGGER_USE_LOGGER_C_BACKEND

/**
 * This macro acts as a wrapper of function "print_log". It adds the filename,
 * function name and line number as permanent input arguments of "print_log"
 * function call
 */
#define PRINT_LOG(level, msg, ...)                                          \
do {                                                                        \
if (level != LOGLEVEL_DEBUG || PRINT_LOG_PRINT_DEBUG_LEVEL)                 \
	print_log(level, msg", file: %s, func: %s, line: %d",               \
		  ##__VA_ARGS__, __FILE_NO_PATH__, __FUNCTION__, __LINE__); \
} while (0)

/**
 * Macro
 */
/**
 * This macro acts as a wrapper of function "print_log". It adds the filename,
 * function name and line number as permanent input arguments of "print_log"
 * function call
 */
#define PRINT_LOG_AND_TIME(level, msg, ...)                                  \
do {                                                                         \
	print_log_and_time(level, msg", file: %s, func: %s, line: %d",       \
		  ##__VA_ARGS__, __FILE_NO_PATH__, __FUNCTION__, __LINE__);  \
} while (0)                                                                  \

#define PRINT_LOG_TIME_NS(msg, ...)               \
do {                                               \
	print_log_and_time_ns(msg", func: %s",        \
		  ##__VA_ARGS__, __FUNCTION__);            \
} while (0)

/**
 * @brief Print the message with given Level of the logging and argument list.
 *
 * This function directs the logging message to stdout
 * It calls generate_msg internally to construct message
 * in template format.
 * level argument refers to Info | Debug | Error | Warning
 * msg argument refers to variable message to be displayed.
 *
 * @param[in]   level           Level of the logging (Info/Debug/Warming/Error). Used for the prefix
 * @param[in]   msg             Base message.
 *
 * @return Generated message.
 */
void print_log(enum loglevel level, char *msg, ...);

/**
 * @brief Print the message with given Level of the logging and argument list (including timestamp).
 *
 * This function directs the logging message to stdout
 * It calls generate_msg internally to construct message
 * in template format.
 * level argument refers to Info | Debug | Error | Warning
 * msg argument refers to variable message to be displayed.
 *
 * @param[in]   level           Level of the logging (Info/Debug/Warming/Error). Used for the prefix
 * @param[in]   msg             Base message.
 *
 * @return Generated message.
 */
void print_log_and_time(enum loglevel level, char *msg, ...);

void print_log_and_time_ns(char *msg, ...);

#else /* HOST_LOGGER_USE_LOGGER_C_BACKEND */

#define PRINT_LOG(level, msg, ...)                                          \
do {                                                                        \
if (((level != LOGLEVEL_DEBUG) && !PRINT_LOG_PRINT_ERROR_LEVEL_ONLY) ||     \
    ((level == LOGLEVEL_ERROR) && PRINT_LOG_PRINT_ERROR_LEVEL_ONLY) ||      \
     PRINT_LOG_PRINT_DEBUG_LEVEL)                                           \
	PRINT(STR(level) " " msg ", file: %s, func: %s, line: %d\n",        \
		  ##__VA_ARGS__, __FILE_NO_PATH__, __FUNCTION__, __LINE__); \
} while (0)

#define PRINT_LOG_AND_TIME(level, msg, ...)  PRINT_LOG(level, msg, ##__VA_ARGS__)

#endif /* HOST_LOGGER_USE_LOGGER_C_BACKEND */
#endif /* __LOGGER_H */
