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

#include "src/iutils/PerfettoTrace.h"

#include <cstdlib>
#include <mutex>
#include <thread>

namespace icamera {
bool gPerfettoEnabled = false;
}

class PerfettoTrace : public perfetto::TrackEventSessionObserver {
 public:
    PerfettoTrace() { perfetto::TrackEvent::AddSessionObserver(this); }
    ~PerfettoTrace() override { perfetto::TrackEvent::RemoveSessionObserver(this); }

    void OnStart(const perfetto::DataSourceBase::StartArgs&) override {
        std::unique_lock<std::mutex> lock(mutex);
        cv.notify_one();
    }

    void ConnectToService() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [] { return perfetto::TrackEvent::IsEnabled(); });
    }

    std::mutex mutex;
    std::condition_variable cv;
};

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

static PerfettoTrace* gPerfettoAgent = nullptr;
static std::once_flag gPerfettoOnce;

static void uninitPerfettoTrace() {
    ::perfetto::TrackEvent::Flush();
    icamera::gPerfettoEnabled = false;
    delete gPerfettoAgent;
}

void initPerfettoTrace() {
    std::call_once(gPerfettoOnce, [&]() {
        perfetto::TracingInitArgs args;
        args.backends = perfetto::kSystemBackend;

        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();

        gPerfettoAgent = new PerfettoTrace;
        gPerfettoAgent->ConnectToService();

        icamera::gPerfettoEnabled = true;
        ::atexit(::uninitPerfettoTrace);
    });
}
