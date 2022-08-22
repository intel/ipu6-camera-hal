/*
 * Copyright (C) 2017-2021 Intel Corporation
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

#include "AiqSetting.h"
#include "CameraEvent.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "Parameters.h"

namespace icamera {
class Dvs : public EventListener {
 public:
    explicit Dvs(int cameraId);
    ~Dvs();

    int configure(const ConfigMode configMode, cca::cca_init_params *params);
    void handleEvent(EventData eventData);
    void setParameter(const Parameters& p);

 private:
    int configCcaDvsData(const ConfigMode configMode, cca::cca_init_params *params);
    void dumpDvsConfiguration(const cca::cca_init_params &config);

 private:
    int mCameraId;
    TuningMode mTuningMode;
    camera_zoom_region_t mPtzRegion;
    camera_zoom_region_t mGDCRegion;

    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(Dvs);
};

} /* namespace icamera */
