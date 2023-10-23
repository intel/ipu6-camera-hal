/*
 * Copyright (C) 2021-2022 Intel Corporation.
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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef CAL_BUILD
#include <base/logging.h>
#endif

#include <sys/time.h>
#include <time.h>
#include "iutils/LogSink.h"
#include "iutils/Utils.h"

#ifdef CAMERA_SYS_LOG
#include <stdarg.h>
#include <syslog.h>
#include <map>
#include <iostream>
#endif
namespace icamera {
extern const char* cameraDebugLogToString(int level);
#define CAMERA_DEBUG_LOG_ERR (1 << 5)
#define CAMERA_DEBUG_LOG_WARNING (1 << 3)

#ifdef CAL_BUILD
const char* GLogSink::getName() const {
    return "Google gLOG";
}

void GLogSink::sendOffLog(LogItem logItem) {
    char prefix[32];
    ::snprintf(prefix, sizeof(prefix),
               "CamHAL[%s]: ", icamera::cameraDebugLogToString(logItem.level));

    switch (logItem.level) {
        case CAMERA_DEBUG_LOG_ERR:
            ::logging::LogMessage(prefix, 0, -::logging::LOGGING_ERROR).stream()
                << logItem.logTags << ':' << logItem.logEntry;
            break;
        case CAMERA_DEBUG_LOG_WARNING:
            ::logging::LogMessage(prefix, 0, -::logging::LOGGING_WARNING).stream()
                << logItem.logTags << ':' << logItem.logEntry;
            break;
        default:
            ::logging::LogMessage(prefix, 0, -::logging::LOGGING_INFO).stream()
                << logItem.logTags << ':' << logItem.logEntry;
            break;
    }
}
#endif

const char* StdconLogSink::getName() const {
    return "Stdcon LOG";
}

void StdconLogSink::sendOffLog(LogItem logItem) {
#define TIME_BUF_SIZE 128
    char timeInfo[TIME_BUF_SIZE];
    setLogTime(timeInfo);
    fprintf(stdout, "[%s] CamHAL[%s] %s\n", timeInfo,
            icamera::cameraDebugLogToString(logItem.level), logItem.logEntry);
}

void LogOutputSink::setLogTime(char* buf) {
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

#ifdef CAMERA_TRACE
FtraceLogSink::FtraceLogSink() {
    mFtraceFD = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    if (mFtraceFD == -1) {
        fprintf(stderr, "[WAR] Cannot init ftrace sink, [%s] self killing...", strerror(errno));
        raise(SIGABRT);
    }
}
const char* FtraceLogSink::getName() const {
    return "Ftrace LOG";
}

void FtraceLogSink::sendOffLog(LogItem logItem) {
#define TIME_BUF_SIZE 128
    char timeInfo[TIME_BUF_SIZE];
    setLogTime(timeInfo);
    dprintf(mFtraceFD, "%s CamHAL[%s] %s\n", timeInfo, cameraDebugLogToString(logItem.level),
            logItem.logEntry);
}
#endif

#define DEFALUT_PATH "/run/camera/hal_logs.txt"
FileLogSink::FileLogSink() {
    static const char* filePath = ::getenv("FILE_LOG_PATH");

    if (!filePath) filePath = DEFALUT_PATH;

    mFp = fopen(filePath, "w");
}

const char* FileLogSink::getName() const {
    return "File LOG";
}

void FileLogSink::sendOffLog(LogItem logItem) {
    if (mFp == nullptr) return;

    char timeInfo[TIME_BUF_SIZE];
    setLogTime(timeInfo);
    fprintf(mFp, "[%s] CamHAL[%s] %s:%s\n", timeInfo,
            icamera::cameraDebugLogToString(logItem.level), logItem.logTags, logItem.logEntry);
    fflush(mFp);
}

FileLogSink::~FileLogSink() {
    if (mFp) fclose(mFp);
}

#ifdef CAMERA_SYS_LOG
SysLogSink::SysLogSink() {}

SysLogSink::~SysLogSink() {}

const char* SysLogSink::getName() const {
    return "SYS LOG";
}

void SysLogSink::sendOffLog(LogItem logItem) {
#define TIME_BUF_SIZE 128
    char logMsg[500] = {0};
    char timeInfo[TIME_BUF_SIZE] = {0};
    setLogTime(timeInfo);
    const char* levelStr = icamera::cameraDebugLogToString(logItem.level);
    snprintf(logMsg, sizeof(logMsg), "[%s] CamHAL[%s] %s\n", timeInfo, levelStr, logItem.logEntry);
    std::map<const char*, int> levelMap{
        {"LV1", LOG_DEBUG}, {"LV2", LOG_DEBUG},   {"LV3", LOG_DEBUG}, {"INF", LOG_INFO},
        {"ERR", LOG_ERR},   {"WAR", LOG_WARNING}, {"UKN", LOG_DEBUG}};

    openlog("cameraHal", LOG_PID | LOG_CONS, LOG_USER);
    syslog(levelMap[levelStr], "%s", logMsg);
    closelog();
}
#endif

};  // namespace icamera
