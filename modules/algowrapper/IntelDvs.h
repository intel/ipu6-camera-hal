/*
 * Copyright (C) 2019-2020 Intel Corporation.
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

#include <ia_dvs.h>
#include <ia_dvs_types.h>
#include <ia_isp_bxt.h>

#include "CameraEvent.h"
#include "DvsResult.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"

namespace icamera {
class IntelDvs {
 public:
    IntelDvs();
    ~IntelDvs();

    ia_err init(const ia_binary_data& aiqTuningBinary, const ia_cmc_t* cmc,
                ia_dvs_state** dvsHandle);
    void deinit(ia_dvs_state* dvsHandle);
    ia_err config(ia_dvs_state* dvsHandle, ia_dvs_configuration* config, float zoomRatio);
    ia_err setNonBlankRatio(ia_dvs_state* dvsHandle, float nonBlankingRatio);
    ia_err setDigitalZoomMode(ia_dvs_state* dvsHandle, ia_dvs_zoom_mode zoomMode);
    ia_err setDigitalZoomRegion(ia_dvs_state* dvsHandle, ia_rectangle* zoomRegion);
    ia_err setDigitalZoomCoordinate(ia_dvs_state* dvsHandle, ia_coordinate* zoomCoordinate);
    ia_err setDigitalZoomMagnitude(ia_dvs_state* dvsHandle, float zoomRatio);
    void freeMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable);
    ia_dvs_morph_table* allocateMorphTalbe(ia_dvs_state* dvsHandle);
    int getMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable);
    int getMorphTable(ia_dvs_state* dvsHandle, ia_dvs_morph_table* morphTable, DvsResult* result);
    ia_err setStatistics(ia_dvs_state* dvsHandle, const ia_dvs_statistics* statistics,
                         const ia_aiq_ae_results* aeResults, const ia_aiq_af_results* afResults,
                         const ia_aiq_sensor_events* sensorEvents, uint64_t frameReadoutStart,
                         uint64_t frameReadoutEnd);
    ia_err execute(ia_dvs_state* dvsHandle, uint16_t focusPosition);
    ia_err getImageTransformation(ia_dvs_state* dvsHandle,
                                  ia_dvs_image_transformation* imageTransformation);
};
} /* namespace icamera */
