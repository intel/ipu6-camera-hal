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

#pragma once

#include <map>

#include "ia_aiq.h"

#include "LensHw.h"
#include "AiqSetting.h"
#include "CameraEventType.h"

namespace icamera {

/*
 * \class LensManager
 * This class is used to control focus and aperture related controls.
 */
class LensManager {
 public:
    LensManager(int cameraId, LensHw* lensHw);
    ~LensManager();

    /**
     * \brief Called when AIQ engine is started
     */
    int start();

    /**
     * \brief Called when AIQ engine is stopped.
     */
    int stop();

    /**
     * \brief handle SOF event
     */
    void handleSofEvent(EventData eventData);

    /**
     * \brief Set Lens results
     *
     * \param[in] cca::cca_af_results includes focus result
     * \param[in] int64_t sequence id
     * \param[in] aiq_parameter_t includes focus settings
     *
     * \return OK if set successfully.
     */
    int setLensResult(const cca::cca_af_results& afResults, int64_t sequence,
                      const aiq_parameter_t& aiqParam);
    /**
     * \brief Get Lens info
     *
     * \param[out] aiqParam: updating lens related parameters.
     *
     */
    void getLensInfo(aiq_parameter_t& aiqParam);

 private:
    DISALLOW_COPY_AND_ASSIGN(LensManager);

    void setFocusPosition(int focusPostion);

 private:
    int mCameraId;
    LensHw* mLensHw;
    ia_aiq_aperture_control_dc_iris_command mDcIrisCommand;
    int mFocusPosition;

    // Guard for LensManager public API.
    Mutex mLock;
    // first: sequence id, second: focus position
    std::map<int64_t, int> mSeqToPositionMap;
    int64_t mLastSofSequence;
};

} /* namespace icamera */
