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

#include <ia_aiq_types.h>
#include <ia_bcomp_types.h>
#include <ia_dvs_types.h>
#include <ia_isp_bxt.h>
#include <ia_isp_bxt_statistics_types.h>
#include <ia_isp_bxt_types.h>

#include "Parameters.h"

namespace icamera {

#define MAX_EXPOSURES_COUNT 3

struct ConvertInputParam {
    bool multiExpo;
    ia_binary_data* statsBuffer;
    camera_resolution_t* dvsReso;
    const ia_aiq_ae_results* aeResults;
    ia_bcomp_results* bcompResult;
};

struct ConvertResult {
    ia_isp_bxt_statistics_query_results_t* queryResults;
    ia_aiq_rgbs_grid* rgbsGrid[MAX_EXPOSURES_COUNT];
    ia_aiq_af_grid* afGrid;
    ia_dvs_statistics* dvsStats;
};
}  // namespace icamera
