/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#define LOG_TAG CameraLog

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <sstream>
#include <string>

#ifdef USE_VSYS_LOG
#include <base/logging.h>
#endif

#ifdef CAMERA_SYS_LOG
#include <syslog.h>
#endif

#include "CameraLog.h"
#include "Trace.h"
#include "iutils/Utils.h"
#include "CameraDump.h"

icamera::LogOutputSink* globalLogSink;
extern const char* tagNames[];

GroupDesc globalGroupsDescp[TAGS_MAX_NUM];

namespace icamera {
int gLogLevel = 0;
int gPerfLevel = 0;
int gSlowlyRunRatio = 0;
// DUMP_ENTITY_TOPOLOGY_S
bool gIsDumpMediaTopo = false;
// DUMP_ENTITY_TOPOLOGY_E
bool gIsDumpMediaInfo = false;

const char* cameraDebugLogToString(int level) {
    switch (level) {
        case CAMERA_DEBUG_LOG_LEVEL1:
            return "LV1";
        case CAMERA_DEBUG_LOG_LEVEL2:
            return "LV2";
        case CAMERA_DEBUG_LOG_LEVEL3:
            return "LV3";
        case CAMERA_DEBUG_LOG_INFO:
            return "INF";
        case CAMERA_DEBUG_LOG_ERR:
            return "ERR";
        case CAMERA_DEBUG_LOG_WARNING:
            return "WAR";
        default:
            return "UKN";
    }
}

#ifdef USE_VSYS_LOG
__attribute__((__format__(__printf__, 3, 0))) static void printLog(const char* module, int level,
                                                                   const char* fmt, va_list ap) {
    char prefix[64] = {};
    snprintf(prefix, sizeof(prefix), "[%s]: CamHAL_%s:", cameraDebugLogToString(level), module);

    char message[256] = {};
    vsnprintf(message, sizeof(message), fmt, ap);

    switch (level) {
        case CAMERA_DEBUG_LOG_ERR:
            LOG(ERROR) << prefix << message;
            break;
        case CAMERA_DEBUG_LOG_WARNING:
            LOG(WARNING) << prefix << message;
            break;
        default:
            LOG(INFO) << prefix << message;
            break;
    }
}
#endif

#ifdef CAMERA_SYS_LOG
__attribute__((__format__(__printf__, 3, 0))) static void printLog(const char* module, int level,
                                                                   const char* fmt, va_list ap) {
    const char* levelStr = nullptr;
    int priority;

    switch (level) {
        case CAMERA_DEBUG_LOG_LEVEL1:
            levelStr = "LV1";
            priority = LOG_DEBUG;
            break;
        case CAMERA_DEBUG_LOG_LEVEL2:
            levelStr = "LV2";
            priority = LOG_DEBUG;
            break;
        case CAMERA_DEBUG_LOG_LEVEL3:
            levelStr = "LV3";
            priority = LOG_DEBUG;
            break;
        case CAMERA_DEBUG_LOG_INFO:
            levelStr = "INF";
            priority = LOG_INFO;
            break;
        case CAMERA_DEBUG_LOG_ERR:
            levelStr = "ERR";
            priority = LOG_ERR;
            break;
        case CAMERA_DEBUG_LOG_WARNING:
            levelStr = "WAR";
            priority = LOG_WARNING;
            break;
        default:
            levelStr = "UKN";
            priority = LOG_DEBUG;
            break;
    }

    char format[1024] = {0};
    snprintf(format, sizeof(format), "[%s]: CamHAL_%s: %s", levelStr, module, fmt);
    openlog("cameraHal", LOG_PID | LOG_CONS, LOG_USER);
    vsyslog(priority, format, ap);
    closelog();
}
#endif

static void getLogTime(char* timeBuf, int bufLen) {
    // The format of time is: 01-22 15:24:53.071
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t nowtime = tv.tv_sec;
    struct tm* nowtm = localtime(&nowtime);
    if (nowtm) {  // If nowtm is nullptr, simply print nothing for time info
        char tmbuf[bufLen];
        CLEAR(tmbuf);
        strftime(tmbuf, bufLen, "%m-%d %H:%M:%S", nowtm);
        snprintf(timeBuf, bufLen, "%s.%03ld", tmbuf, tv.tv_usec / 1000);
    }
}

__attribute__((__format__(__printf__, 3, 0))) static void printLog(const char* module, int level,
                                                                   const char* fmt, va_list ap) {
    // Add time into beginning of the log.
    const int BUF_LEN = 64;
    char timeBuf[BUF_LEN] = {'\0'};

    getLogTime(timeBuf, BUF_LEN);

    fprintf(stdout, "%s: [%s]: CamHAL_%s:", timeBuf, cameraDebugLogToString(level), module);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
}

void doLogBody(int logTag, int level, int grpPosition, const char* fmt, ...) {
    if (!(level & globalGroupsDescp[grpPosition].level)) return;

    char message[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);

    globalLogSink->sendOffLog({message, level, tagNames[grpPosition]});
}

void doLogBody(int logTag, int level, const char* fmt, ...) {
    if (!(level & globalGroupsDescp[logTag].level)) return;

    char message[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);

    globalLogSink->sendOffLog({message, level, tagNames[logTag]});
}

namespace Log {

#define DEFAULT_LOG_SINK "GLOG"
#define FILELOG_SINK "FILELOG"

static void initLogSinks() {
#ifdef CAL_BUILD
    const char* sinkName = ::getenv("logSink");

    if (!sinkName) {
        sinkName = DEFAULT_LOG_SINK;
    }

    if (!::strcmp(sinkName, DEFAULT_LOG_SINK)) {
        globalLogSink = new GLogSink();
    } else if (!::strcmp(sinkName, FILELOG_SINK)) {
        globalLogSink = new FileLogSink;
    } else {
        globalLogSink = new StdconLogSink();
    }
#endif

#ifdef CAMERA_SYS_LOG
    const char* sinkName = ::getenv("logSink");

    if (!sinkName) {
        sinkName = DEFAULT_LOG_SINK;
    }

    if (!::strcmp(sinkName, DEFAULT_LOG_SINK)) {
        globalLogSink = new SysLogSink();
    } else if (!::strcmp(sinkName, FILELOG_SINK)) {
        globalLogSink = new FileLogSink;
    } else {
        globalLogSink = new StdconLogSink();
    }
#endif

    globalLogSink = new StdconLogSink();
}

static void setLogTagLevel() {
    static const char* LOG_FILE_TAG = "cameraTags";
    char* logFileTag = ::getenv(LOG_FILE_TAG);

    if (!logFileTag) return;
    std::string s = logFileTag;
    std::istringstream is(s);
    std::string token;

    while (std::getline(is, token, ':')) {
        auto pos = token.find_first_of('-');

        std::string name;
        std::string levelStr;
        if (pos != std::string::npos) {
            name = token.substr(0, pos);
            levelStr = token.substr(pos + 1);
        } else {
            name = token;
        }

        for (int itemIdx = 0; itemIdx < TAGS_MAX_NUM; ++itemIdx) {
            if (name != tagNames[itemIdx]) continue;

            if (!levelStr.empty())
                globalGroupsDescp[itemIdx].level = strtoul(levelStr.c_str(), nullptr, 0);
        }
    }
}

void setDebugLevel(void) {
    const char* PROP_CAMERA_HAL_DEBUG = "cameraDebug";
    const char* PROP_CAMERA_HAL_PERF = "cameraPerf";
    const char* PROP_CAMERA_RUN_RATIO = "cameraRunRatio";

    initLogSinks();

    // debug
    char* dbgLevel = getenv(PROP_CAMERA_HAL_DEBUG);
    gLogLevel = CAMERA_DEBUG_LOG_ERR | CAMERA_DEBUG_LOG_WARNING | CAMERA_DEBUG_LOG_INFO;

    if (dbgLevel) {
        gLogLevel = strtoul(dbgLevel, nullptr, 0);
        LOG1("Debug level is 0x%x", gLogLevel);
    }

    if (gLogLevel & CAMERA_DEBUG_LOG_DYNAMIC_DUMP) {
        CameraDump::setDumpThread();
        LOGI("Dynamic dump is enabled.");
    }

    for (size_t i = 0; i < TAGS_MAX_NUM; ++i) {
        globalGroupsDescp[i].level = gLogLevel;
    }

    setLogTagLevel();

    char* slowlyRunRatio = getenv(PROP_CAMERA_RUN_RATIO);
    if (slowlyRunRatio) {
        gSlowlyRunRatio = strtoul(slowlyRunRatio, nullptr, 0);
        LOG1("Slow run ratio is 0x%x", gSlowlyRunRatio);
    }

    // performance
    char* perfLevel = getenv(PROP_CAMERA_HAL_PERF);
    if (perfLevel) {
        gPerfLevel = strtoul(perfLevel, nullptr, 0);
        LOGI("Performance level is 0x%x", gPerfLevel);

        // bitmask of tracing categories
        if (gPerfLevel & CAMERA_DEBUG_LOG_PERF_TRACES) {
            LOG1("Perf KPI start/end trace is not yet supported");
        }
        if (gPerfLevel & CAMERA_DEBUG_LOG_PERF_TRACES_BREAKDOWN) {
            LOG1("Perf KPI breakdown trace is not yet supported");
        }
        if (gPerfLevel & CAMERA_DEBUG_LOG_PERF_IOCTL_BREAKDOWN) {
            LOG1("Perf IOCTL breakdown trace is not yet supported");
        }
        if (gPerfLevel & CAMERA_DEBUG_LOG_PERF_MEMORY) {
            LOG1("Perf memory breakdown trace is not yet supported");
        }
        // DUMP_ENTITY_TOPOLOGY_S
        if (gPerfLevel & CAMERA_DEBUG_LOG_MEDIA_TOPO_LEVEL) {
            gIsDumpMediaTopo = true;
        }
        // DUMP_ENTITY_TOPOLOGY_E
        if (gPerfLevel & CAMERA_DEBUG_LOG_MEDIA_CONTROLLER_LEVEL) {
            gIsDumpMediaInfo = true;
        }
        ScopedAtrace::setTraceLevel(gPerfLevel);
    }
}

bool isDebugLevelEnable(int level) {
    return gLogLevel & level;
}

bool isLogTagEnabled(int tag, int level) {
    if (tag < 0 || tag >= TAGS_MAX_NUM) return false;
    return level ? (globalGroupsDescp[tag].level & level) : (globalGroupsDescp[tag].level > 0);
}

// DUMP_ENTITY_TOPOLOGY_S
bool isDumpMediaTopo(void) {
    return gIsDumpMediaTopo;
}
// DUMP_ENTITY_TOPOLOGY_E

bool isDumpMediaInfo(void) {
    return gIsDumpMediaInfo;
}

__attribute__((__format__(__printf__, 4, 0))) void print_log(bool enable, const char* module,
                                                             const int level, const char* format,
                                                             ...) {
    if (!enable && (level != CAMERA_DEBUG_LOG_ERR)) return;

    va_list arg;
    va_start(arg, format);

    printLog(module, level, format, arg);

    va_end(arg);
}

__attribute__((__format__(__printf__, 1, 0))) void ccaPrintError(const char* fmt, va_list ap) {
    if (gLogLevel & CAMERA_DEBUG_LOG_CCA) {
        printLog("CCA_DEBUG", CAMERA_DEBUG_LOG_ERR, fmt, ap);
    }
}

__attribute__((__format__(__printf__, 1, 0))) void ccaPrintInfo(const char* fmt, va_list ap) {
    if (gLogLevel & CAMERA_DEBUG_LOG_CCA) {
        printLog("CCA_DEBUG", CAMERA_DEBUG_LOG_INFO, fmt, ap);
    }
}

}  // namespace Log

#ifdef HAVE_ANDROID_OS

void __camera_hal_log(bool condition, int prio, const char* tag, const char* fmt, ...) {
    if (condition) {
        va_list ap;
        va_start(ap, fmt);
        if (gLogLevel & CAMERA_DEBUG_LOG_PERSISTENT) {
            int errnoCopy;
            unsigned int maxTries = 20;
            do {
                errno = 0;
                __android_log_vprint(prio, tag, fmt, ap);
                errnoCopy = errno;
                if (errnoCopy == EAGAIN) usleep(2000); /* sleep 2ms */
            } while (errnoCopy == EAGAIN && maxTries--);
        } else {
            __android_log_vprint(prio, tag, fmt, ap);
        }
    }
}

#endif  // HAVE_ANDROID_OS
}  // namespace icamera
