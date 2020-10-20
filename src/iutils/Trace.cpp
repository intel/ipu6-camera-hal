/*
 * Copyright (C) 2014-2020 Intel Corporation
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

#define LOG_TAG "Trace"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "Trace.h"
#include "iutils/CameraLog.h"

namespace icamera {

std::atomic<int>        atrace_is_ready(0);
uint64_t                atrace_enabled_tags  = ATRACE_TAG_NOT_READY;
int                     atrace_marker_fd     = -1;
static pthread_once_t   atrace_once_control  = PTHREAD_ONCE_INIT;

static void atrace_init_once()
{
    atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    if (atrace_marker_fd == -1) {
        ATRACE_LOGE("atrace %s open error: %s!\n", __func__, strerror(errno));
        return;
    }
    atrace_enabled_tags = ATRACE_TAG_ALWAYS;
    atrace_is_ready = 1;
}

void atrace_setup()
{
    pthread_once(&atrace_once_control, atrace_init_once);
}

} //namespace icamera
