/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#include "iutils/Utils.h"
#include "Parameters.h"

namespace icamera {

class CameraBuffer;

enum EventType
{
    EVENT_ISYS_SOF = 0,
    EVENT_PSYS_STATS_BUF_READY,
    EVENT_PSYS_STATS_SIS_BUF_READY,
    EVENT_ISYS_FRAME,
    EVENT_PSYS_FRAME,
    EVENT_PROCESS_REQUEST,
    EVENT_DEVICE_RECONFIGURE,
    EVENT_FRAME_AVAILABLE,
    EVENT_PSYS_REQUEST_BUF_READY,
};

struct EventDataStatsReady
{
    timeval timestamp;
    long sequence;
};

struct EventDataSync
{
    timeval timestamp;
    long sequence;
};

struct EventDataFrame
{
    timeval timestamp;
    long sequence;
};

struct EventDataMeta
{
    timeval timestamp;
    long sequence;
};

struct EventRequestData
{
    int bufferNum;
    camera_buffer_t** buffer;
    Parameters* param;

    long settingSeq;
};

struct EventConfigData
{
    stream_config_t *streamList;
};

struct EventFrameAvailable
{
    int streamId;
};

struct EventRequestBufferReady
{
    int64_t timestamp;
    long sequence;
};

struct EventData
{
    EventData() : type(EVENT_ISYS_SOF) {
        CLEAR(data);
    }

    EventType type;
    std::shared_ptr<CameraBuffer> buffer;
    union
    {
        EventDataStatsReady statsReady;
        EventDataSync sync;
        EventDataFrame frame;
        EventDataMeta meta;
        EventRequestData request;
        EventConfigData config;
        EventFrameAvailable frameDone;
        EventRequestBufferReady requestReady;
    } data;
};

}
