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

#define LOG_TAG "EXIFMetaData"

#include "EXIFMetaData.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

#define DEFAULT_ISO_SPEED 100

ExifMetaData::ExifMetaData()
        : effectMode(CAM_EFFECT_NONE),
          software(nullptr),
          hdr(false),
          flashFired(false),
          v3AeMode(BAD_VALUE),
          flashMode(BAD_VALUE),
          saveMirrored(false),
          cameraOrientation(0),
          currentOrientation(0),
          zoomRatio(1),
          aeMode(AE_MODE_AUTO),
          awbMode(AWB_MODE_AUTO),
          currentFocusDistance(0.0) {
    ALOGI("@%s", __func__);
    mJpegSetting.jpegQuality = 90;
    mJpegSetting.jpegThumbnailQuality = 90;
    mJpegSetting.orientation = 0;
    mJpegSetting.thumbWidth = 320;
    mJpegSetting.thumbHeight = 240;
    mGpsSetting.latitude = 0.0;
    mGpsSetting.longitude = 0.0;
    mGpsSetting.altitude = 0.0;
    CLEAR(mGpsSetting.gpsProcessingMethod);
    mGpsSetting.gpsTimeStamp = 0;
}

ExifMetaData::~ExifMetaData() {}

}  // namespace icamera
