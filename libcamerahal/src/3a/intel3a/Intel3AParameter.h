/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#include "AiqSetting.h"
#include "AiqUtils.h"
#include "ia_aiq.h"

namespace icamera {

/*
 * \class Intel3AParameter
 * This class is used to prepare those parameters for
 * 3A running.
 */
class Intel3AParameter {
 public:
    explicit Intel3AParameter(int cameraId);
    ~Intel3AParameter();

    int init();
    int getCMCInfo(TuningMode tuningMode);
    int setSensorInfo(ia_aiq_exposure_sensor_descriptor descriptor);
    int updateParameter(aiq_parameter_t param);
    void updateAeResult(cca::cca_ae_results* aeResult);
    void updateAwbResult(cca::cca_awb_results* awbResult);
    void updatePaResult(cca::cca_pa_params* paResult);

    void fillAfTriggerResult(cca::cca_af_results* afResults);

 private:
    void initAeParameter();
    void initAfParameter();
    void initAwbParameter();

    void updateAeParameter(const aiq_parameter_t& param);
    void updateAwbParameter(const aiq_parameter_t& param);
    void updateAfParameter(const aiq_parameter_t& param);
    void updateAfParameterForAfTriggerStart();
    void updateAfParameterForAfTriggerCancel();

    float convertdBGainToISO(float sensitivityGain, int baseIso);
    void setManualExposure(const aiq_parameter_t& param);
    void setManualGain(const aiq_parameter_t& param);
    void setManualIso(const aiq_parameter_t& param);
    void setAeManualLimits(const aiq_parameter_t& param);

 public:
    int mCameraId;

    cca::cca_cmc mCMC;

    // aiq 3a parameters
    cca::cca_ae_input_params mAeParams;
    cca::cca_af_input_params mAfParams;
    cca::cca_awb_input_params mAwbParams;
    camera_test_pattern_mode_t mTestPatternMode;

    bool mUseManualAwbGain;
    bool mUseManualColorMatrix;

    camera_color_transform_t mColorMatrix;
    camera_color_gains_t mColorGains;
    camera_awb_gains_t mManualGains;
    camera_awb_gains_t mAwbGainShift;
    camera_weight_grid_mode_t mWeightGridMode;

    int mAePerTicks;
    int mAwbPerTicks;

    bool mAfForceLock;  // Lock AF to respond autofocus action triggered by user.
    float mManualFocusDistance;
    camera_ae_mode_t mAeMode;

 private:
    void dumpParameter();

    static const int MAX_FOCUS_DISTANCE = 5000;  // unit is mm

    camera_af_mode_t mAfMode;
    camera_af_trigger_t mAfTrigger;
    bool mDuringAfTriggerScan;

    camera_range_t mSensitivityRange;
};

} /* namespace icamera */
