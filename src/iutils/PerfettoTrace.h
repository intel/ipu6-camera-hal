/*
 * Copyright (C) 2023 Intel Corporation.
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

#include <unistd.h>

#include <mutex>
#include <condition_variable>

#include <perfetto/perfetto.h>

namespace icamera {
extern bool gPerfettoEnabled;
}

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("libcamhal").SetDescription("libcamhal"));

extern void initPerfettoTrace();

constexpr std::string_view DECODE_TO_NEW_NAME(const char* fn) {
    std::string_view sv(fn);
    auto paren = sv.rfind('(');
    auto space = sv.rfind(' ', paren + 1);
    return sv.substr(space + 1, paren - space - 1);
}

#define TRACE_PERFETTO_EVENT_NAME DECODE_TO_NEW_NAME(__PRETTY_FUNCTION__)

#define PERFETTO_TRACE_EVENT(...)                                                           \
    static const std::string event_##__LINE__(TRACE_PERFETTO_EVENT_NAME);             \
    do {                                                                               \
        if (gPerfettoEnabled)                                                         \
            TRACE_EVENT("libcamhal", perfetto::StaticString{event_##__LINE__.c_str()}, \
                        ##__VA_ARGS__);                                                \
    } while (0)

#define PERF_CAMERA_ATRACE(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM1(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM2(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM3(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_IMAGING(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM1_IMAGING(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM2_IMAGING(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM3_IMAGING(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);
