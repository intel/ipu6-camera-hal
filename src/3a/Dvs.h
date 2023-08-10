/*
 * Copyright (C) 2017-2023 Intel Corporation
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

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelCcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include <ia_dvs.h>
#include <ia_dvs_types.h>
#include <ia_isp_bxt.h>

#include <memory>
#include <vector>
#include <unordered_map>

#include "AiqSetting.h"
#include "CameraEvent.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "Parameters.h"

namespace icamera {

typedef struct DvsConfig {
    cca::cca_gdc_configurations gdcConfigs;
    float zoomRatio;
    cca::CCADVSOutputType outputType;
    bool enableDvs;
} DvsConfig;

class Dvs : public EventListener {
 public:
    explicit Dvs(int cameraId);
    ~Dvs();

    int configure(const ConfigMode configMode, DvsConfig* cfg);
    void handleEvent(EventData eventData);

 private:
    int configCcaDvsData(int32_t streamId, const ConfigMode configMode, DvsConfig* cfg);
    void dumpDvsConfiguration(const DvsConfig& config);
    void setParameter(const camera_zoom_region_t& region);

 private:
    int mCameraId;
    TuningMode mTuningMode;
    camera_zoom_region_t mZoomRegion;
    struct ZoomParam {
        camera_zoom_region_t ptzRegion;
        camera_zoom_region_t gdcRegion;
    };
    // first: stream id, second: ZoomParam
    std::unordered_map<int32_t, ZoomParam> mZoomParamMap;
    std::mutex mLock;

    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(Dvs);
};

} /* namespace icamera */
