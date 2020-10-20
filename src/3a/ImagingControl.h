/*
 * Copyright (C) 2018-2020 Intel Corporation.
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

#include "iutils/Errors.h"
#include "AiqSetting.h"
#include "AiqResult.h"

namespace icamera {

/*
 * \interface class ImagingControl
 * This is an interface class for imaging algorithm controllers.
 */
class ImagingControl {
public:
    virtual ~ImagingControl() {}

    virtual int init() = 0;
    virtual int deinit() = 0;
    virtual int run(AiqResult *aiqResult, int algoType) = 0;

    virtual int configure(const std::vector<ConfigMode>& configModes) { return OK; }
    virtual int updateParameter(const aiq_parameter_t &param) { return OK; }
    virtual int setFrameInfo(const ia_aiq_frame_params &frameParams) { return OK; }
    virtual int setSensorInfo(const ia_aiq_exposure_sensor_descriptor &descriptor) { return OK; }
    virtual int setStatistics(const ia_aiq_statistics_input_params_v4 *ispStatistics) { return OK; }
    virtual int getSupportedAlgoType() { return IMAGING_ALGO_NONE; }

};

} //namespace icamera
