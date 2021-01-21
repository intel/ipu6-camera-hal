/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#define LOG_TAG "Dvs"
#include "src/3a/Dvs.h"

#include <ia_cmc_parser.h>
#include <ia_pal_types_isp_ids_autogen.h>

#include <algorithm>

#include "AiqResultStorage.h"
#include "AiqUtils.h"
#include "IGraphConfig.h"
#include "IGraphConfigManager.h"
#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

Dvs::Dvs(int cameraId)
        : mCameraId(cameraId),
          mTuningMode(TUNING_MODE_VIDEO) {
    LOG2("@%s", __func__);
}

Dvs::~Dvs() {
    LOG2("@%s", __func__);
}

int Dvs::configure(const std::vector<ConfigMode>& configModes) {
    LOG2("@%s", __func__);

    if (configModes.empty()) {
        return UNKNOWN_ERROR;
    }

    TuningMode tuningMode;
    if (PlatformData::getTuningModeByConfigMode(mCameraId, configModes[0], tuningMode) != OK) {
        return UNKNOWN_ERROR;
    }
    mTuningMode = tuningMode;

    return OK;
}

void Dvs::handleEvent(EventData eventData) {
    LOG2("@%s: eventData.type:%d", __func__, eventData.type);

    if (eventData.type != EVENT_PSYS_STATS_BUF_READY) return;

    IntelCca* intelCcaHandle = IntelCca::getInstance(mCameraId, mTuningMode);
    CheckError(!intelCcaHandle, VOID_VALUE, "@%s, Failed to get IntelCca instance", __func__);

    // Run DVS
    LOG2("%s: handle EVENT_PSYS_STATS_BUF_READY", __func__);
    ia_err iaErr = intelCcaHandle->runDVS(eventData.data.statsReady.sequence);
    int ret = AiqUtils::convertError(iaErr);
    CheckError(ret != OK, VOID_VALUE, "Error running DVS: %d", ret);
    return;
}
}  // namespace icamera
