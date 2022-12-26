/*
 * Copyright (C) 2014-2018 Intel Corporation
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

#pragma once

#include <atomic>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

namespace icamera {

#ifndef ATRACE_NO_LOG
#define ATRACE_LOGE(format, args...) printf(format, ##args)
#else
#define ATRACE_LOGE()
#endif

#define CAMERA_PRId32 "d"
#define CAMERA_PRId64 "I64d"

#define ATRACE_MESSAGE_LENGTH 1024
#define PROPERTY_VALUE_MAX 1024
/**
 * The ATRACE_TAG macro can be defined before including this header to trace
 * using one of the tags defined below.  It must be defined to one of the
 * following ATRACE_TAG_* macros.  The trace tag is used to filter tracing in
 * userland to avoid some of the runtime cost of tracing when it is not desired.
 *
 * Defining ATRACE_TAG to be ATRACE_TAG_ALWAYS will result in the tracing always
 * being enabled - this should ONLY be done for debug code, as userland tracing
 * has a performance cost even when the trace is not being recorded.  Defining
 * ATRACE_TAG to be ATRACE_TAG_NEVER or leaving ATRACE_TAG undefined will result
 * in the tracing always being disabled.
 */
#define ATRACE_TAG_NEVER            0       // This tag is never enabled.
#define ATRACE_TAG_ALWAYS           (1<<0)  // This tag is always enabled.
#define ATRACE_TAG_LAST             ATRACE_TAG_ALWAYS

// Reserved for initialization.
#define ATRACE_TAG_NOT_READY        (1LL<<63)

#define ATRACE_TAG_VALID_MASK ((ATRACE_TAG_LAST - 1) | ATRACE_TAG_LAST)

// define the ATRACE_TAG to ALWAYS for nestdemo.
#define ATRACE_TAG ATRACE_TAG_ALWAYS

/**
 * Opens the trace file for writing and reads the property for initial tags.
 * The atrace.tags.enableflags property sets the tags to trace.
 * This function should not be explicitly called, the first call to any normal
 * trace function will cause it to be run safely.
 */
void atrace_setup();

/**
 * Flag indicating whether setup has been completed, initialized to 0.
 * Nonzero indicates setup has completed.
 * Note: This does NOT indicate whether or not setup was successful.
 */
extern std::atomic<int> atrace_is_ready;

/**
 * Set of ATRACE_TAG flags to trace for, initialized to ATRACE_TAG_NOT_READY.
 * A value of zero indicates setup has failed.
 * Any other nonzero value indicates setup has succeeded, and tracing is on.
 */
extern uint64_t atrace_enabled_tags;

/**
 * Handle to the kernel's trace buffer, initialized to -1.
 * Any other value indicates setup has succeeded, and is a valid fd for tracing.
 */
extern int atrace_marker_fd;

/**
 * atrace_init readies the process for tracing by opening the trace_marker file.
 * Calling any trace function causes this to be run, so calling it is optional.
 * This can be explicitly run to avoid setup delay on first trace function.
 */
#define ATRACE_INIT() atrace_init()
static inline void atrace_init()
{
    if (!atrace_is_ready.load()) {
        atrace_setup();
    }
}

/**
 * Get the mask of all tags currently enabled.
 * It can be used as a guard condition around more expensive trace calculations.
 * Every trace function calls this, which ensures atrace_init is run.
 */
#define ATRACE_GET_ENABLED_TAGS() atrace_get_enabled_tags()
static inline uint64_t atrace_get_enabled_tags()
{
    atrace_init();
    return atrace_enabled_tags;
}

/**
 * Test if a given tag is currently enabled.
 * Returns nonzero if the tag is enabled, otherwise zero.
 * It can be used as a guard condition around more expensive trace calculations.
 */
#define ATRACE_ENABLED() atrace_is_tag_enabled(ATRACE_TAG)
static inline uint64_t atrace_is_tag_enabled(uint64_t tag)
{
    return atrace_get_enabled_tags() & tag;
}

/**
 * Trace the beginning of a context.  name is used to identify the context.
 * This is often used to time function execution.
 */
#define ATRACE_BEGIN(name) atrace_begin(ATRACE_TAG, name)
static inline void atrace_begin(uint64_t tag, const char* name)
{
    if (atrace_is_tag_enabled(tag)) {
        char buf[ATRACE_MESSAGE_LENGTH];
        ssize_t len;

        len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "B|%d|%s", static_cast<int>(getpid()), name);
        if (write(atrace_marker_fd, buf, len) != len)
            ATRACE_LOGE("atrace %s write %s error: %s!\n", __func__, buf, strerror(errno));
    }
}

/**
 * Trace the end of a context.
 * This should match up (and occur after) a corresponding ATRACE_BEGIN.
 */
#define ATRACE_END() atrace_end(ATRACE_TAG)
static inline void atrace_end(uint64_t tag)
{
    if (atrace_is_tag_enabled(tag)) {
        char c = 'E';
        if (write(atrace_marker_fd, &c, 1) !=1)
            ATRACE_LOGE("atrace %s write error: %s!\n", __func__, strerror(errno));
    }
}

/**
 * Trace the beginning of an asynchronous event. Unlike ATRACE_BEGIN/ATRACE_END
 * contexts, asynchronous events do not need to be nested. The name describes
 * the event, and the cookie provides a unique identifier for distinguishing
 * simultaneous events. The name and cookie used to begin an event must be
 * used to end it.
 */
#define ATRACE_ASYNC_BEGIN(name, cookie) \
    atrace_async_begin(ATRACE_TAG, name, cookie)
static inline void atrace_async_begin(uint64_t tag, const char* name,
        int32_t cookie)
{
    if (atrace_is_tag_enabled(tag)) {
        char buf[ATRACE_MESSAGE_LENGTH];
        ssize_t len;

        len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "S|%d|%s|%" CAMERA_PRId32,
                       static_cast<int>(getpid()), name, cookie);
        if (write(atrace_marker_fd, buf, len) != len)
            ATRACE_LOGE("atrace %s write %s error: %s!\n", __func__, buf, strerror(errno));
    }
}

/**
 * Trace the end of an asynchronous event.
 * This should have a corresponding ATRACE_ASYNC_BEGIN.
 */
#define ATRACE_ASYNC_END(name, cookie) atrace_async_end(ATRACE_TAG, name, cookie)
static inline void atrace_async_end(uint64_t tag, const char* name,
        int32_t cookie)
{
    if (atrace_is_tag_enabled(tag)) {
        char buf[ATRACE_MESSAGE_LENGTH];
        ssize_t len;

        len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "F|%d|%s|%" CAMERA_PRId32,
                       static_cast<int>(getpid()), name, cookie);
        if (write(atrace_marker_fd, buf, len) != len)
            ATRACE_LOGE("atrace %s write %s error: %s!\n", __func__, buf, strerror(errno));
    }
}

/**
 * Traces an integer counter value.  name is used to identify the counter.
 * This can be used to track how a value changes over time.
 */
#define ATRACE_INT(name, value) atrace_int(ATRACE_TAG, name, value)
static inline void atrace_int(uint64_t tag, const char* name, int32_t value)
{
    if (atrace_is_tag_enabled(tag)) {
        char buf[ATRACE_MESSAGE_LENGTH];
        ssize_t len;

        len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "C|%d|%s|%" CAMERA_PRId32,
                       static_cast<int>(getpid()), name, value);
        if (write(atrace_marker_fd, buf, len) != len)
            ATRACE_LOGE("atrace %s write %s error: %s!\n", __func__, buf, strerror(errno));
    }
}

/**
 * Traces a 64-bit integer counter value.  name is used to identify the
 * counter. This can be used to track how a value changes over time.
 */
#define ATRACE_INT64(name, value) atrace_int64(ATRACE_TAG, name, value)
static inline void atrace_int64(uint64_t tag, const char* name, int64_t value)
{
    if (atrace_is_tag_enabled(tag)) {
        char buf[ATRACE_MESSAGE_LENGTH];
        ssize_t len;

        len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "C|%d|%s|%" PRId64 " " CAMERA_PRId64,
                       static_cast<int>(getpid()), name, value);
        if (write(atrace_marker_fd, buf, len) != len)
            ATRACE_LOGE("atrace %s write %s error: %s!\n", __func__, buf, strerror(errno));
    }
}

} //namespace icamera
