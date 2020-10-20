/*
 * Copyright (C) 2015-2018 Intel Corporation.
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

#include "AiqSetting.h"
#include "AiqUtils.h"

namespace icamera {

/*
 * \class Intel3AParameter
 * This class is used to prepare those parameters for
 * 3A running.
 */
class Intel3AParameter {

public:
    Intel3AParameter(int cameraId);
    ~Intel3AParameter();

    int init();
    int setSensorInfo(ia_aiq_exposure_sensor_descriptor descriptor);
    int updateParameter(aiq_parameter_t param);
    void updateAeResult(ia_aiq_ae_results* aeResult);
    void updateAwbResult(ia_aiq_awb_results* awbResult);
    void updatePaResult(ia_aiq_pa_results_v1* paResult);

    void fillAfTriggerResult(ia_aiq_af_results *afResults);

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
    // aiq 3a parameters
    ia_aiq_ae_input_params  mAeParams;
    ia_aiq_af_input_params  mAfParams;
    ia_aiq_awb_input_params mAwbParams;

    bool mUseManualAwbGain;
    bool mUseManualColorMatrix;

    camera_color_transform_t mColorMatrix;
    camera_color_gains_t mColorGains;
    camera_awb_gains_t mManualGains;
    camera_awb_gains_t mAwbGainShift;
    camera_weight_grid_mode_t mWeightGridMode;

    int mAePerTicks;
    int mAwbPerTicks;

    bool mAfForceLock; // Lock AF to respond autofocus action triggered by user.
private:
    static const int MAX_FOCUS_DISTANCE = 5000; // unit is mm

    /*!< ia_aiq_ae_input_params pointer contents */
    ia_aiq_exposure_sensor_descriptor mSensorDescriptor;
    ia_rectangle mExposureWindow;
    ia_coordinate mExposureCoordinate;
    ia_aiq_ae_features mAeFeatures;
    ia_aiq_ae_manual_limits mAeManualLimits;

    /*!< ia_aiq_af_input_params pointer contents */
    ia_aiq_manual_focus_parameters mManualFocusParams;
    ia_rectangle mFocusRect;

    /*!< ia_aiq_awb_input_params pointer contents */
    ia_aiq_awb_manual_cct_range mManualCctRange;
    ia_coordinate mManualWhiteCoordinate;

    long mManualExposureTimeUs[MAX_EXPOSURES_NUM];
    float mManualAnalogGain[MAX_EXPOSURES_NUM];
    short mManualIso[MAX_EXPOSURES_NUM];

    camera_af_mode_t mAfMode;
    camera_af_trigger_t mAfTrigger;
    bool mDuringAfTriggerScan;
};

} /* namespace icamera */
