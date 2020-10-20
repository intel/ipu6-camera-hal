/*
 * Copyright (C) 2016-2018 Intel Corporation.
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

#include "ia_aiq.h"

#include "LensHw.h"
#include "AiqSetting.h"

namespace icamera {

/*
 * \class LensManager
 * This class is used to control focus and aperture related controls.
 */
class LensManager {

public:
    LensManager(int cameraId, LensHw *lensHw);
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
     * \brief Set Lens results
     *
     * \param[in] ia_aiq_ae_results includes aperture result
     *            and ia_aiq_af_results includes focus result.
     *
     * \return OK if set successfully.
     */
    int setLensResult(const ia_aiq_ae_results &aeResults,
                      const ia_aiq_af_results &afResults);
    /**
     * \brief Get Lens info
     *
     * \param[out] aiqParam: updating lens related parameters.
     *
     */
    void getLensInfo(aiq_parameter_t &aiqParam);

private:
    DISALLOW_COPY_AND_ASSIGN(LensManager);

private:
    int mCameraId;
    LensHw *mLensHw;
    ia_aiq_aperture_control_dc_iris_command mDcIrisCommand;
    int mFocusPosition;

    // Guard for LensManager public API.
    Mutex mLock;
};

} /* namespace icamera */
