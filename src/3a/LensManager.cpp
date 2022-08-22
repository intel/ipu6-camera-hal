/*
 * Copyright (C) 2016-2021 Intel Corporation.
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

#define LOG_TAG LensManager

#include "LensManager.h"
#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "PlatformData.h"

namespace icamera {

LensManager::LensManager(int cameraId, LensHw *lensHw) :
    mCameraId(cameraId),
    mLensHw(lensHw),
    mDcIrisCommand(ia_aiq_aperture_control_dc_iris_close),
    mFocusPosition(-1),
    mLastSofSequence(-1) {
}

LensManager::~LensManager() {
}

int LensManager::start() {
    AutoMutex l(mLock);

    mDcIrisCommand = ia_aiq_aperture_control_dc_iris_close;
    mFocusPosition = -1;
    mLastSofSequence = -1;

    return OK;
}

int LensManager::stop() {
    AutoMutex l(mLock);

    if (!mLensHw->isLensSubdevAvailable()) {
        return OK;
    }

    return OK;
}

void LensManager::handleSofEvent(EventData eventData) {
    AutoMutex l(mLock);
    if (eventData.type == EVENT_ISYS_SOF) {
        mLastSofSequence = eventData.data.sync.sequence;

        if (mSeqToPositionMap.find(mLastSofSequence) != mSeqToPositionMap.end()) {
            setFocusPosition(static_cast<int>(mSeqToPositionMap[mLastSofSequence]));
            mSeqToPositionMap.erase(mLastSofSequence);
        }

        // remove previous focus result for just SOF missing
        for (auto it = mSeqToPositionMap.begin(); it != mSeqToPositionMap.end(); it++) {
            if (it->second <= mLastSofSequence) {
                mSeqToPositionMap.erase(it->second);
            } else {
                break;
            }
        }
    }
}

int LensManager::setLensResult(const cca::cca_af_results &afResults,
                               int64_t sequence, const aiq_parameter_t &aiqParam) {
    AutoMutex l(mLock);

    if (!mLensHw->isLensSubdevAvailable() || afResults.next_lens_position == 0) {
        return OK;
    }

    int ret = OK;

    int lensHwType = PlatformData::getLensHwType(mCameraId);
    switch(lensHwType) {
        case LENS_VCM_HW:
            if (aiqParam.afMode == AF_MODE_OFF && aiqParam.focusDistance > 0.0f) {
                // The manual focus setting requires perframe control
                mSeqToPositionMap[sequence] = afResults.next_lens_position;
            } else {
                // Ignore auto focus result if there is manual settings before.
                if (!mSeqToPositionMap.empty()) return OK;

                setFocusPosition(static_cast<int>(afResults.next_lens_position));
            }
            break;
        default:
            LOGW("Not supported Lens HW type, lensHwType = %d", lensHwType);
            break;
    }

    return ret;
}

void LensManager::setFocusPosition(int focusPosition) {
    if (mFocusPosition != focusPosition) {
        int ret = mLensHw->setFocusPosition(focusPosition);
        if (ret == OK) {
            mFocusPosition = focusPosition;
            LOG2("SENSORCTRLINFO: vcm_step=%d", mFocusPosition);
        }
    }
}

void LensManager::getLensInfo(aiq_parameter_t &aiqParam) {
    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        mLensHw->getLatestPosition(aiqParam.lensPosition, aiqParam.lensMovementStartTimestamp);
    }
}

} /* namespace icamera */
