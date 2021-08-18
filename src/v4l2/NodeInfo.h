/*
 * Copyright (C) 2020 Intel Corporation
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

#include "iutils/Utils.h"

enum VideoNodeType {
    // video node device
    VIDEO_GENERIC,
    VIDEO_GENERIC_MEDIUM_EXPO,
    VIDEO_GENERIC_SHORT_EXPO,

    // sensor subdevice
    VIDEO_PIXEL_ARRAY,
    VIDEO_PIXEL_BINNER,
    VIDEO_PIXEL_SCALER,

    // ISP subdevice
    VIDEO_ISYS_RECEIVER,
    VIDEO_ISYS_RECEIVER_BACKEND,
};

struct VideoNodeInfo {
    VideoNodeType type;
    const char* fullName;
    const char* shortName;
};

enum EncodeBufferType {
    ENCODE_ISA_CONFIG  = 0,
    ENCODE_STATS = 1,
};

extern const VideoNodeInfo gVideoNodeInfos[];
extern const char* GetNodeName(VideoNodeType nodeType);
extern VideoNodeType GetNodeType(const char* nodeName);
