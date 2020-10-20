/*
 * Copyright (C) 2020 Intel Corporation.
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

#ifndef __PLATFORM_SUPPORT_H
#define __PLATFORM_SUPPORT_H

#include "storage_class.h"

#define MSEC_IN_SEC 1000
#define NSEC_IN_MSEC 1000000

#define NUM_SLEEP_MSEC_WA_XTENSA 1

#if defined(_MSC_VER)
#include <string.h>

#define IA_CSS_EXTERN
#define SYNC_WITH(x)
#define CSS_ALIGN(d, a) _declspec(align(a)) d

STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    /* Placeholder for driver team*/
}

STORAGE_CLASS_INLINE void ia_css_sleep_msec(unsigned int long delay_time_ms)
{
    /* Placeholder for driver team*/
    (void)delay_time_ms;
}

#elif defined(__HIVECC)
#include <string.h>
#include <hive/support.h>
#include <hive/attributes.h>

#define IA_CSS_EXTERN extern
#define CSS_ALIGN(d, a) d __attribute__((aligned(a)))
STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    OP___schedule();
}

#elif defined(__KERNEL__)
#include <linux/string.h>
#include <linux/delay.h>

#define IA_CSS_EXTERN
#define CSS_ALIGN(d, a) d __aligned(a)

STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    usleep_range(1, 50);
}
#elif defined(__XTENSA_FW__)

#include <hive/support.h>

#define ALIAS_ATTR(target) __attribute__((alias(#target)))
#define WEAK_ALIAS_ATTR(target) __attribute__((weak, alias(#target)))

#elif defined(__GNUC__)
#include <string.h>

#define IA_CSS_EXTERN
#define CSS_ALIGN(d, a) d __attribute__((aligned(a)))

/* Define some __HIVECC specific macros to nothing to allow host code compilation */
#ifndef NO_ALIAS
#define NO_ALIAS
#endif

#ifndef SYNC_WITH
#define SYNC_WITH(x)
#endif

#if defined(HRT_CSIM)
#include "hrt/host.h" /* Using hrt_sleep from hrt/host.h */
STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    /* For the SDK still using hrt_sleep */
    hrt_sleep();
}
STORAGE_CLASS_INLINE void ia_css_sleep_msec(unsigned int long delay_time_ms)
{
    /* For the SDK still using hrt_sleep */
    unsigned int long i = 0;

    for (i = 0; i < delay_time_ms; i++) {
        hrt_sleep();
    }
}
#elif defined(__XTENSA_FW__) /* XTENSA FW */
STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    /* do nothing */
}
#elif defined(IPUSIM_SLEEP)
#include "hrt/host.h"
#include "misc_support.h" /* for NOT_USED macro */
STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    hrt_sleep();
}
STORAGE_CLASS_INLINE void ia_css_sleep_msec(unsigned int long delay_time_ms)
{
    /* if hrt_sleep_ns(delay_time_ms) is used here, it slows tests dramatically.
     * hrt_sleep_ns(delay_time_ms * 1000000) causes an effective hang.
     * This is because ia_css_sleep_msec is used thousands of times (BUG).
     * hrt_sleep_ns(1) is fine (NUM_SLEEP_MSEC_WA_XTENSA = 1).
     */
    NOT_USED(delay_time_ms);
    hrt_sleep_ns(NUM_SLEEP_MSEC_WA_XTENSA);
}
#else
#include <time.h>
STORAGE_CLASS_INLINE void ia_css_sleep(void)
{
    struct timespec delay_time;

    delay_time.tv_sec = 0;
    delay_time.tv_nsec = 10;
    nanosleep(&delay_time, NULL);
}
STORAGE_CLASS_INLINE void ia_css_sleep_msec(unsigned int long delay_time_ms)
{
    struct timespec delay_time;

    if (delay_time_ms >= MSEC_IN_SEC) {
        delay_time.tv_sec = delay_time_ms / MSEC_IN_SEC;
        delay_time.tv_nsec = (delay_time_ms % MSEC_IN_SEC) * NSEC_IN_MSEC;
    } else {
        delay_time.tv_sec = 0;
        delay_time.tv_nsec = delay_time_ms * NSEC_IN_MSEC;
    }
    nanosleep(&delay_time, NULL);
}
#endif

#else

#if defined(__XTENSA_FW__)

#define CSS_ALIGN(d, a) d

#define ia_css_sleep()

#if defined(__XTENSA_EL__)
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif

#endif
#include <string.h>
#endif

/*needed for the include in stdint.h for various environments */
#include "type_support.h"
#include "storage_class.h"

#define MAX_ALIGNMENT            8
#define aligned_uint8(type, obj)    CSS_ALIGN(uint8_t obj, 1)
#define aligned_int8(type, obj)        CSS_ALIGN(int8_t obj, 1)
#define aligned_uint16(type, obj)    CSS_ALIGN(uint16_t obj, 2)
#define aligned_int16(type, obj)    CSS_ALIGN(int16_t obj, 2)
#define aligned_uint32(type, obj)    CSS_ALIGN(uint32_t obj, 4)
#define aligned_int32(type, obj)    CSS_ALIGN(int32_t obj, 4)

/* needed as long as hivecc does not define the type (u)int64_t */
#if defined(__HIVECC)
#define aligned_uint64(type, obj)    CSS_ALIGN(unsigned long long obj, 8)
#define aligned_int64(type, obj)    CSS_ALIGN(signed long long obj, 8)
#else
#define aligned_uint64(type, obj)    CSS_ALIGN(uint64_t obj, 8)
#define aligned_int64(type, obj)    CSS_ALIGN(int64_t obj, 8)
#endif
#define aligned_enum(enum_type, obj)    CSS_ALIGN(uint32_t obj, 4)
#define aligned_struct(struct_type, obj)    struct_type obj

#endif /* __PLATFORM_SUPPORT_H */
