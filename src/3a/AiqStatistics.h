/*
 * Copyright (C) 2018 Intel Corporation.
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

#include "ia_aiq_types.h"
#include "ia_isp_bxt_types.h"
#include "ia_dvs_types.h"
#include "ia_isp_bxt_statistics_types.h"

#include "AiqUtils.h"

namespace icamera {

/*
 * \class AiqStatistics
 *
 * This class is used to envelop AIQ statistics.
 */
class AiqStatistics {
public:
    AiqStatistics();
    ~AiqStatistics();

    AiqStatistics& operator=(const AiqStatistics& other);

    int saveRgbsGridData(const ia_aiq_rgbs_grid* const *rgbsGrid, int exposureNum);
    int saveAfGridData(const ia_aiq_af_grid *afGrid);

private:
    int copyRgbsGridData(const ia_aiq_rgbs_grid *src, ia_aiq_rgbs_grid *dst);

public:
    long mSequence = -1;
    unsigned long long mTimestamp = 0;
    TuningMode mTuningMode = TUNING_MODE_MAX;
    bool mInUse = false;

    int mExposureNum = 0;
    ia_aiq_rgbs_grid mRgbsGridArray[MAX_EXPOSURES_NUM];
    ia_aiq_af_grid mAfGridArray[MAX_EXPOSURES_NUM];
};

} /* namespace icamera */

