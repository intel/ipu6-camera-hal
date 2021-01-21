/*
 * Copyright (C) 2015-2018 Intel Corporation.
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

#include <stdarg.h>

#include "utils/ScopedAtrace.h"
#ifdef HAVE_ANDROID_OS
#include <log/log.h>
#endif

#ifdef CAMERA_TRACE
#include "CameraTrace.h"
#endif

namespace icamera {
/**
 * global log level
 * This global variable is set from system properties
 * It is used to control the level of verbosity of the traces in logcat
 * It is also used to store the status of certain RD features
 */
extern int gLogLevel;
extern int gPerfLevel;
extern int gEnforceDvs;
extern int gSlowlyRunRatio;

/**
 * LOG levels
 *
 * LEVEL 1 is used to track events in the HAL that are relevant during
 * the operation of the camera, but are not happening on a per frame basis.
 * this ensures that the level of logging is not too verbose
 *
 * LEVEL 2 is used to track information on a per request basis
 *
 * REQ_STATE is used to track the state of each request. By state we mean a one
 * of the following request properties:
 *  - metadata result
 *  - buffer
 *  - shutter
 *  - error
 *
 * PERF TRACES enable only traces that provide performance metrics on the opera
 * tion of the HAL
 *
 * PERF TRACES BREAKDOWN provides further level of detail on the performance
 * metrics
 */
enum  {
    /* verbosity level of general traces */
    CAMERA_DEBUG_LOG_LEVEL1 = 1,
    CAMERA_DEBUG_LOG_LEVEL2 = 1 << 1,

    /* Bitmask to enable a concrete set of traces */
    CAMERA_DEBUG_LOG_REQ_STATE = 1 << 2,
    CAMERA_DEBUG_LOG_AIQ = 1 << 3,
    CAMERA_DEBUG_LOG_XML = 1 << 4,
    CAMERA_DEBUG_LOG_VC_SYNC = 1 << 5,
    CAMERA_DEBUG_LOG_FPS = 1 << 6,
    CAMERA_DEBUG_LOG_KERNEL_TOGGLE = 1 << 8,
    CAMERA_DEBUG_LOG_SANDBOXING = 1 << 9,

    /* Make logs persistent, retrying if logcat is busy */
    CAMERA_DEBUG_LOG_PERSISTENT = 1 << 12, /* 4096 */

    /* reserved for any components */
    CAMERA_DEBUG_LOG_GRAPH = 1 << 13,

    CAMERA_DEBUG_LOG_DBG = 1 <<16,
    CAMERA_DEBUG_LOG_INFO = 1 <<17,
    CAMERA_DEBUG_LOG_ERR = 1 <<18,
    CAMERA_DEBUG_LOG_WARNING = 1 <<19,
    CAMERA_DEBUG_LOG_VERBOSE = 1 <<20
};

enum  {
    /* Emit well-formed performance traces */
    CAMERA_DEBUG_LOG_PERF_TRACES = 1,

    /* Print out detailed timing analysis */
    CAMERA_DEBUG_LOG_PERF_TRACES_BREAKDOWN = 2,

    /* Print out detailed timing analysis for IOCTL */
    CAMERA_DEBUG_LOG_PERF_IOCTL_BREAKDOWN = 1<<2,

    /* Print out detailed memory information analysis for IOCTL */
    CAMERA_DEBUG_LOG_PERF_MEMORY = 1<<3,

    /*enable camera atrace level 0 for camtune-record*/
    CAMERA_DEBUG_LOG_ATRACE_LEVEL0 = 1<<4,

    /*enable media topology dump*/
    CAMERA_DEBUG_LOG_MEDIA_TOPO_LEVEL = 1<<5,

    /*enable media controller info dump*/
    CAMERA_DEBUG_LOG_MEDIA_CONTROLLER_LEVEL = 1<<6,

    /*enable camera imaging atrace level 1 for camtune-record*/
    CAMERA_DEBUG_LOG_ATRACE_LEVEL1 = 1<<7,
};

enum {
    CAMERA_POWERBREAKDOWN_DISABLE_PREVIEW = 1<<0,
    CAMERA_POWERBREAKDOWN_DISABLE_FDFR = 1<<1,
    CAMERA_POWERBREAKDOWN_DISABLE_3A = 1<<2,
};

namespace Log {
void setDebugLevel(void);
void print_log(bool enable, const char *module, const int level, const char *format, ...);
bool isDebugLevelEnable(int level);
bool isModulePrintable(const char *module);
bool isDumpMediaTopo(void);
bool isDumpMediaInfo(void);
void ccaPrintError(const char *fmt, va_list ap);
void ccaPrintInfo(const char *fmt, va_list ap);
void ccaPrintDebug(const char *fmt, va_list ap);
};

#define SLOWLY_MULTIPLIER (icamera::gSlowlyRunRatio ? icamera::gSlowlyRunRatio : 1)

#ifdef HAVE_LINUX_OS //Linux OS

#define LOG1(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_LEVEL1, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_LEVEL1, format, ##args)
#define LOG2(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_LEVEL2, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_LEVEL2, format, ##args)
#define LOGR(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_REQ_STATE, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_REQ_STATE, format, ##args)
#define LOG3A(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_AIQ, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_AIQ, format, ##args)
#define LOGXML(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_XML, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_XML, format, ##args)
#define LOGVCSYNC(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_VC_SYNC, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_VC_SYNC, format, ##args)
#define LOGG(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_GRAPH, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_GRAPH, format, ##args)
#define LOGIPC(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_SANDBOXING, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_SANDBOXING, format, ##args)
#define LOGFPS(format, args...) \
    icamera::Log::print_log(icamera::gLogLevel& icamera::CAMERA_DEBUG_LOG_FPS, LOG_TAG, \
                            icamera::CAMERA_DEBUG_LOG_FPS, format, ##args)

#define LOGD(format, args...) \
    icamera::Log::print_log(true, LOG_TAG, icamera::CAMERA_DEBUG_LOG_DBG, format, ##args)
#define LOGI(format, args...) \
    icamera::Log::print_log(true, LOG_TAG, icamera::CAMERA_DEBUG_LOG_INFO, format, ##args)
#define LOGE(format, args...) \
    icamera::Log::print_log(true, LOG_TAG, icamera::CAMERA_DEBUG_LOG_ERR, format, ##args)
#define LOGW(format, args...) \
    icamera::Log::print_log(true, LOG_TAG, icamera::CAMERA_DEBUG_LOG_WARNING, format, ##args)
#define LOGV(format, args...) \
    icamera::Log::print_log(true, LOG_TAG, icamera::CAMERA_DEBUG_LOG_VERBOSE, format, ##args)

#define ALOGE LOGE
#define ALOGD LOGD
#define ALOGI LOGI
#define ALOGW LOGW
#define ALOGV LOGV
#define ALOGW_IF
#define LOG_ALWAYS_FATAL_IF
#define LOG_FATAL_IF

#else //Android OS

void __camera_hal_log(bool condition, int prio, const char *tag, const char *fmt, ...);

#define LOG1(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_LEVEL1, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG2(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_LEVEL2, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGR(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_REQ_STATE, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG3A(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_AIQ, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGXML(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_XML, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGVCSYNC(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_VC_SYNC, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGG(...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_GRAPH, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGIPC(format, args...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_SANDBOXING, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGFPS(format, args...) \
    icamera::__camera_hal_log(icamera::gLogLevel & icamera::CAMERA_DEBUG_LOG_FPS, \
                              ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define LOGE(...) icamera::__camera_hal_log(true, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) icamera::__camera_hal_log(true, ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) icamera::__camera_hal_log(true, ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) icamera::__camera_hal_log(true, ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGV(...) icamera::__camera_hal_log(true, ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#endif
#define HAL_TRACE_NAME(level, name) ScopedTrace ___tracer(level, name )
#define HAL_TRACE_CALL(level) HAL_TRACE_NAME(level, __PRETTY_FUNCTION__)

#ifdef CAMERA_TRACE
#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
#define MAKE_COLOR(data) (icamera::CameraTrace::setColor(data))
#define TRACE_LOG_TYPE(type, ...) icamera::CameraTrace CONCAT(_Trace_, __LINE__)(type, __VA_ARGS__)

#define TRACE_LOG_PROCESS(...) TRACE_LOG_TYPE(icamera::TraceEventStart, __VA_ARGS__)
#define TRACE_STRUCT_PROCESS(name, struct_name, ...) \
    TRACE_LOG_TYPE(icamera::TraceEventStart, name, #struct_name, sizeof(struct_name), __VA_ARGS__)
#define TRACE_LOG_POINT(...) TRACE_LOG_TYPE(icamera::TraceEventPoint, __VA_ARGS__)
#define TRACE_STRUCT_POINT(name, struct_name, ...) \
    TRACE_LOG_TYPE(icamera::TraceEventPoint, name, #struct_name, sizeof(struct_name), __VA_ARGS__)

#else
#define MAKE_COLOR(data) (data)
#define TRACE_LOG_PROCESS(...)
#define TRACE_STRUCT_PROCESS(name, struct_name, ...)
#define TRACE_LOG_POINT(...)
#define TRACE_STRUCT_POINT(name, struct_name, ...)
#endif

class ScopedTrace {
    public:
        inline ScopedTrace(int level, const char* name) :
            mLevel(level),
            mName(name) {
                if ((mLevel <= gLogLevel) && !(gLogLevel & CAMERA_DEBUG_LOG_VC_SYNC))
                    LOGD("ENTER-%s",name);
            }

        inline ~ScopedTrace() {
                if ((mLevel <= gLogLevel)  && !(gLogLevel & CAMERA_DEBUG_LOG_VC_SYNC))
                    LOGD("EXIT-%s", mName);
        }

    private:
        int mLevel;
        const char* mName;
};

} //namespace icamera
