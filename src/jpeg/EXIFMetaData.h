/*
 * Copyright (C) 2016-2020 Intel Corporation
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

#include "Parameters.h"
#include "iutils/Errors.h"
namespace icamera {

#define MAX_NUM_GPS_PROCESSING_METHOD 64

/**
 * \class ExifMetaData
 *
 */
class ExifMetaData {
 public:
    ExifMetaData();
    virtual ~ExifMetaData();

    // jpeg info
    struct JpegSetting {
        uint8_t jpegQuality;
        uint8_t jpegThumbnailQuality;
        int thumbWidth;
        int thumbHeight;
        int orientation;
    };
    // GPS info
    struct GpsSetting {
        double latitude;
        double longitude;
        double altitude;
        char gpsProcessingMethod[MAX_NUM_GPS_PROCESSING_METHOD];
        long gpsTimeStamp;
    };
    // exif info
    JpegSetting mJpegSetting;
    GpsSetting mGpsSetting;
    camera_effect_mode_t effectMode;
    char* software;         /*!< software string from HAL */
    bool hdr;               /*!< whether hdr was used */
    bool flashFired;        /*!< whether flash was fired */
    int8_t v3AeMode;        /*!< v3 ae mode (e.g. for flash) */
    int8_t flashMode;       /*!< flash mode (e.g. TORCH,SINGLE,OFF) */
    bool saveMirrored;      /*!< whether to do mirroring */
    int cameraOrientation;  /*!< camera sensor orientation */
    int currentOrientation; /*!< Current orientation of the device */
    int zoomRatio;
    icamera::camera_ae_mode_t aeMode;
    icamera::camera_awb_mode_t awbMode;
    float currentFocusDistance;
};

}  // namespace icamera
