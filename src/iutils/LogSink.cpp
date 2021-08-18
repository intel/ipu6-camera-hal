/*
 * Copyright (C) 2021 Intel Corporation.
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

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef CAL_BUILD
#include <base/logging.h>
#endif

#include <sys/time.h>
#include <time.h>

#include "src/iutils/LogSink.h"

namespace icamera {
extern const char* cameraDebugLogToString(int level);
#define CAMERA_DEBUG_LOG_ERR (1 << 18)
#define CAMERA_DEBUG_LOG_WARNING (1 << 19)
#define CAMERA_DEBUG_LOG_VERBOSE (1 << 20)

#ifdef CAL_BUILD
const char* gLogSink::getName() const { return "Google gLOG"; }

void gLogSink::sendOffLog(const char* prefix, const char* logEntry,
                               int level) {
    switch (level) {
        case CAMERA_DEBUG_LOG_ERR:
            ::logging::LogMessage("<CAMHAL>", 0, -::logging::LOGGING_ERROR)
                    .stream()
                << prefix << logEntry;
            break;
        case CAMERA_DEBUG_LOG_WARNING:
            ::logging::LogMessage("<CAMHAL>", 0, -::logging::LOGGING_WARNING)
                    .stream()
                << prefix << logEntry;
            break;
        default:
            ::logging::LogMessage("<CAMHAL>", 0, -::logging::LOGGING_INFO)
                    .stream()
                << prefix << logEntry;
            break;
    }
}

#endif
const char* StdconLogSink::getName() const { return "Stdcon LOG"; }

void StdconLogSink::sendOffLog(const char* prefix, const char* logEntry,
                               int level) {
#define TIME_BUF_SIZE 128
    char timeInfo[TIME_BUF_SIZE];
    setLogTime(timeInfo);
    fprintf(stdout, "[%s] [%s] %s %s\n", timeInfo,
            cameraDebugLogToString(level), prefix, logEntry);
}

void StdconLogSink::setLogTime(char* buf) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t nowtime = tv.tv_sec;
    struct tm local_tm;

    struct tm* nowtm = localtime_r(&nowtime, &local_tm);
    if (nowtm) {
        char tmbuf[TIME_BUF_SIZE];
        strftime(tmbuf, TIME_BUF_SIZE, "%m-%d %H:%M:%S", nowtm);
        snprintf(buf, TIME_BUF_SIZE, "%.96s.%d", tmbuf,
                 static_cast<int>((tv.tv_usec / 1000) % 1000));
    }
}

};  // namespace icamera
