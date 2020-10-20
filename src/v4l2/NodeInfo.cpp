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

#include "src/v4l2/NodeInfo.h"

const VideoNodeInfo gVideoNodeInfos[] = {
    { VIDEO_GENERIC,             "VIDEO_GENERIC",              "Generic" },
    { VIDEO_GENERIC_MEDIUM_EXPO, "VIDEO_GENERIC_MEDIUM_EXPO",  "GenericMediumExpo" },
    { VIDEO_GENERIC_SHORT_EXPO,  "VIDEO_GENERIC_SHORT_EXPO",   "GenericShortExpo" },

    { VIDEO_PIXEL_ARRAY,         "VIDEO_PIXEL_ARRAY",          "PixelArray" },
    { VIDEO_PIXEL_BINNER,        "VIDEO_PIXEL_BINNER",         "PixelBinner" },
    { VIDEO_PIXEL_SCALER,        "VIDEO_PIXEL_SCALER",         "PixelScaler" },

    { VIDEO_ISYS_RECEIVER,       "VIDEO_ISYS_RECEIVER",        "ISysReceiver" },
    { VIDEO_ISYS_RECEIVER_BACKEND,  "VIDEO_ISYS_RECEIVER_BACKEND",  "CsiBE"},
};

const char* GetNodeName(VideoNodeType nodeType)
{
    int size = ARRAY_SIZE(gVideoNodeInfos);
    for (int i = 0; i < size; i++) {
        if (gVideoNodeInfos[i].type == nodeType) {
            return gVideoNodeInfos[i].shortName;
        }
    }
    return "InvalidNode";
}

VideoNodeType GetNodeType(const char* nodeName)
{
    int size = ARRAY_SIZE(gVideoNodeInfos);
    for (int i = 0; i < size; i++) {
        if (strcmp(gVideoNodeInfos[i].fullName, nodeName) == 0) {
            return gVideoNodeInfos[i].type;
        }
    }

    return VIDEO_GENERIC;
}
