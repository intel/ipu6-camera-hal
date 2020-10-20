/*
 * Copyright (C) 2015-2020 Intel Corporation.
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

#include "iutils/Utils.h"
#include "iutils/RWLock.h"
#include "Parameters.h"

#include "AiqUtils.h"

namespace icamera {

#define DEFAULT_LSC_GRID_SIZE (64 * 64)
#define DEFAULT_TONEMAP_CURVE_POINT_NUM 2048

// Imaging algorithms that are supported
typedef enum {
    IMAGING_ALGO_NONE = 0,
    IMAGING_ALGO_AE   = 1,
    IMAGING_ALGO_AWB  = 1 << 1,
    IMAGING_ALGO_AF   = 1 << 2,
    IMAGING_ALGO_GBCE = 1 << 3,
    IMAGING_ALGO_PA   = 1 << 4,
    IMAGING_ALGO_SA   = 1 << 5
} imaging_algorithm_t;

typedef struct {
    char data[MAX_CUSTOM_CONTROLS_PARAM_SIZE];
    unsigned int length;
} custom_aic_param_t;

typedef enum {
    FRAME_USAGE_PREVIEW,
    FRAME_USAGE_VIDEO,
    FRAME_USAGE_STILL,
    FRAME_USAGE_CONTINUOUS,
} frame_usage_mode_t;

/*
 * aiq related parameters
 */
struct aiq_parameter_t {
    frame_usage_mode_t frameUsage;
    camera_ae_mode_t aeMode;
    bool aeForceLock;
    camera_awb_mode_t awbMode;
    bool awbForceLock;
    camera_af_mode_t afMode;
    camera_af_trigger_t afTrigger;
    camera_scene_mode_t sceneMode;
    int64_t manualExpTimeUs;
    float manualGain;
    int32_t manualIso;
    float evShift;
    float fps;
    camera_range_t aeFpsRange;
    camera_antibanding_mode_t antibandingMode;
    camera_range_t cctRange;
    camera_coordinate_t whitePoint;
    camera_awb_gains_t awbManualGain;
    camera_awb_gains_t awbGainShift;
    camera_color_transform_t manualColorMatrix;
    camera_color_gains_t manualColorGains;
    camera_window_list_t aeRegions;
    camera_window_list_t afRegions;
    camera_blc_area_mode_t blcAreaMode;
    camera_converge_speed_mode_t aeConvergeSpeedMode;
    camera_converge_speed_mode_t awbConvergeSpeedMode;
    camera_converge_speed_t aeConvergeSpeed;
    camera_converge_speed_t awbConvergeSpeed;
    int run3ACadence;
    uint8_t ltmStrength;
    camera_weight_grid_mode_t weightGridMode;
    camera_ae_distribution_priority_t aeDistributionPriority;
    custom_aic_param_t customAicParam;
    camera_yuv_color_range_mode_t yuvColorRangeMode;
    camera_range_t exposureTimeRange;
    camera_range_t sensitivityGainRange;
    camera_video_stabilization_mode_t videoStabilizationMode;
    camera_resolution_t resolution;
    camera_ldc_mode_t ldcMode;
    camera_rsc_mode_t rscMode;
    camera_flip_mode_t flipMode;
    float digitalZoomRatio;
    camera_range_t evRange;
    camera_rational_t evStep;

    TuningMode tuningMode;

    int lensPosition;
    unsigned long long lensMovementStartTimestamp;
    camera_makernote_mode_t makernoteMode;
    float minFocusDistance;
    float focusDistance;
    camera_shading_mode_t shadingMode;
    camera_lens_shading_map_mode_type_t lensShadingMapMode;
    camera_coordinate_t lensShadingMapSize;

    camera_tonemap_mode_t tonemapMode;
    camera_tonemap_preset_curve_t tonemapPresetCurve;
    float tonemapGamma;
    camera_tonemap_curves_t tonemapCurves;
    float tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM * 3];  // r, g, b

    aiq_parameter_t() { reset(); }
    void reset();
    void dump();
};

/*
 * \class AiqSetting
 * This class is used for setting parameters to other aiq class
 * and return some useful status of aiq results
 */
class AiqSetting {

public:
    AiqSetting(int cameraId);
    ~AiqSetting();

    int init(void);
    int deinit(void);
    int configure(const stream_config_t *streamList);

    int setParameters(const Parameters& params);

    int getAiqParameter(aiq_parameter_t &param);

private:
    void updateFrameUsage(const stream_config_t *streamList);

public:
    int mCameraId;

private:
    std::vector<TuningMode> mTuningModes;
    unsigned int mPipeSwitchFrameCount;
    aiq_parameter_t mAiqParam;

    RWLock mParamLock;
};

} /* namespace icamera */
