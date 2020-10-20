/*
 * Copyright (C) 2016-2020 Intel Corporation.
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

#define LOG_TAG "LensManager"

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
    mFocusPosition(-1)
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);
}

LensManager::~LensManager()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);
}

int LensManager::start()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);
    AutoMutex l(mLock);

    mDcIrisCommand = ia_aiq_aperture_control_dc_iris_close;
    mFocusPosition = -1;

    return OK;
}

int LensManager::stop()
{
    LOG1("%s, mCameraId = %d", __func__, mCameraId);
    AutoMutex l(mLock);

    if (!mLensHw->isLensSubdevAvailable()) {
        return OK;
    }

    return OK;
}

int LensManager::setLensResult(const ia_aiq_ae_results &aeResults,
                               const ia_aiq_af_results &afResults)
{
    LOG3A("%s, mCameraId = %d", __func__, mCameraId);
    AutoMutex l(mLock);

    if (!mLensHw->isLensSubdevAvailable()) {
        return OK;
    }

    int ret = OK;

    int lensHwType = PlatformData::getLensHwType(mCameraId);
    switch(lensHwType) {
        case LENS_VCM_HW:
            if (mFocusPosition != afResults.next_lens_position) {
                ret = mLensHw->setFocusPosition(afResults.next_lens_position);
                mFocusPosition = afResults.next_lens_position;
                LOG3A("mFocusPosition = %d, camera id %d", mFocusPosition, mCameraId);
                LOG2("SENSORCTRLINFO: vcm_step=%d", mFocusPosition);
            }
            break;
// IPU4_FEATURE_S
        case LENS_PWM_HW:
            mDcIrisCommand = aeResults.aperture_control->dc_iris_command;
            ret = mLensHw->setPwmDuty(aeResults.aperture_control->code);
            LOG3A("lens duty = %d, mDcIrisCommand = %d", aeResults.aperture_control->code, mDcIrisCommand);
            break;
// IPU4_FEATURE_E
        default:
            LOGW("Not supported Lens HW type, lensHwType = %d", lensHwType);
            break;
    }

    return ret;
}

void LensManager::getLensInfo(aiq_parameter_t &aiqParam) {

    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        mLensHw->getLatestPosition(aiqParam.lensPosition, aiqParam.lensMovementStartTimestamp);
    }
}

} /* namespace icamera */
