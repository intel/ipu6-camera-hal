/*
 * Copyright (C) 2018-2024 Intel Corporation.
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

#include "ia_isp_types.h"
#include "iutils/Utils.h"
#include "Parameters.h"

namespace icamera {

struct IspImageEnhancement {
    char manualSharpness;
    char manualBrightness;
    char manualContrast;
    char manualHue;
    char manualSaturation;
};

struct IspSettings {
    ia_isp_feature_setting nrSetting;
    ia_isp_feature_setting eeSetting;
    ia_isp_feature_setting nrStillSetting;
    ia_isp_feature_setting eeStillSetting;
    ia_isp_effect effects;
    bool videoStabilization;
    IspImageEnhancement manualSettings;
    ia_binary_data* palOverride;
    // DOL_FEATURE_S
    short vbp;  // Used for DOL camera
    // DOL_FEATURE_E
    float zoom;
    camera_mount_type_t sensorMountType;
    IspSettings() {
        CLEAR(*this);
        zoom = 1.0f;
    }
};

}  // namespace icamera
