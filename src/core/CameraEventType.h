/*
 * Copyright (C) 2015-2022 Intel Corporation.
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

#include <memory>

#include "Parameters.h"
#include "iutils/Utils.h"

namespace icamera {

class CameraBuffer;

enum EventType {
    EVENT_ISYS_SOF = 0,
    EVENT_PSYS_STATS_BUF_READY,
    EVENT_PSYS_STATS_SIS_BUF_READY,
    EVENT_ISYS_FRAME,
    EVENT_PSYS_FRAME,
    EVENT_PROCESS_REQUEST,
    EVENT_FRAME_AVAILABLE,
    EVENT_PSYS_REQUEST_BUF_READY,
    EVENT_REQUEST_METADATA_READY,
    // INTEL_DVS_S
    EVENT_DVS_READY,
    // INTEL_DVS_E
    EVENT_ISYS_ERROR,
};

struct EventDataStatsReady {
    timeval timestamp;
    int64_t sequence;
};

struct EventDataSync {
    timeval timestamp;
    int64_t sequence;
};

struct EventDataFrame {
    timeval timestamp;
    int64_t sequence;
};

struct EventDataMeta {
    timeval timestamp;
    int64_t sequence;
};

struct EventRequestData {
    int bufferNum;
    camera_buffer_t** buffer;
    Parameters* param;

    int64_t settingSeq;
};

struct EventConfigData {
    stream_config_t* streamList;
};

struct EventFrameAvailable {
    int streamId;
};

struct EventRequestReady {
    int64_t timestamp;
    int64_t sequence;
    uint32_t requestId;
};

// INTEL_DVS_S
struct EventDVSRunReady {
    int streamId;
};
// INTEL_DVS_E

struct EventData {
    EventData() : type(EVENT_ISYS_SOF), pipeType(-1) { CLEAR(data); }

    EventType type;
    int pipeType;  // if you don't care it, you can ignore it.
    std::shared_ptr<CameraBuffer> buffer;
    union {
        EventDataStatsReady statsReady;
        EventDataSync sync;
        EventDataFrame frame;
        EventDataMeta meta;
        EventRequestData request;
        EventConfigData config;
        EventFrameAvailable frameDone;
        EventRequestReady requestReady;  // use for returning metadata and shutter event
// INTEL_DVS_S
        EventDVSRunReady dvsRunReady;
// INTEL_DVS_E
    } data;
};

}  // namespace icamera
