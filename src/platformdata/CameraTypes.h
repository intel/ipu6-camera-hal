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

#include <string>
#include <vector>
#include <map>
#include <utility>
#include "Parameters.h"

namespace icamera {

/**
 * Use to link buffer producers and consumers
 */
enum Port {
    MAIN_PORT = 0,
    SECOND_PORT,
    THIRD_PORT,
    FORTH_PORT,
    INVALID_PORT
};

enum {
    FACING_BACK = 0,
    FACING_FRONT = 1,
};

enum {
    ORIENTATION_0 = 0,
    ORIENTATION_90 = 90,
    ORIENTATION_180 = 180,
    ORIENTATION_270 = 270,
};

enum {
    LENS_VCM_HW = 0,
    LENS_NONE_HW
};

enum {
    AUTO_SWITCH_FULL = 0,
    AUTO_SWITCH_PSYS
};

enum {
    SENSOR_EXPOSURE_SINGLE = 0,        /* sensor is single exposure */
    SENSOR_FIX_EXPOSURE_RATIO,         /* Fix exposure ratio between long and short exposure */
    SENSOR_RELATIVE_MULTI_EXPOSURES,   /* AE output exposures are converted to Shutter and
                                          Readout time, then set to sensor driver */
    SENSOR_MULTI_EXPOSURES,            /* Multi-exposures are set to sensor driver directly */
    SENSOR_DUAL_EXPOSURES_DCG_AND_VS   /* Dual-exposure and multiple gains, i.e. DCG + VS */
};

enum {
    SENSOR_GAIN_NONE = 0,
    SENSOR_MULTI_DG_AND_CONVERTION_AG,  /* Multi-DigitalGain and convertion AnalogGain are set
                                           to sensor driver */
    ISP_DG_AND_SENSOR_DIRECT_AG,        /* All digital gain is passed to ISP */
    SENSOR_MULTI_DG_AND_DIRECT_AG       /* Multi analog and digital gains, i.e. DCG */
};

/**
 * This definition is used to distinguish different camera running mode, like video or still.
 */
typedef enum {
    TUNING_MODE_VIDEO,
    TUNING_MODE_VIDEO_ULL,
    TUNING_MODE_VIDEO_CUSTOM_AIC,
    TUNING_MODE_VIDEO_LL,
    TUNING_MODE_VIDEO_REAR_VIEW,
    TUNING_MODE_VIDEO_HITCH_VIEW,
    TUNING_MODE_STILL_CAPTURE,
    TUNING_MODE_MAX
} TuningMode;

/*
 * The mapping algorithm for sensor digital gain
 */
typedef enum {
    SENSOR_DG_TYPE_NONE,
    SENSOR_DG_TYPE_X,           //linear relationship, gain = n*value (value: register value, n: ratio)
    SENSOR_DG_TYPE_2_X,         //exponential relationship, gain = 2 ^ value (value: register value)
} SensorDgType;

typedef enum {
    MORPH_TABLE = 0,
    IMG_TRANS
} DvsType;

// Note AUTO is not real config mode in the HAL.
typedef camera_stream_configuration_mode_t ConfigMode;

typedef struct TuningConfig {
    ConfigMode configMode;                 /*!< configMode is internal usage to select AIQ and
                                                Pipeline. AUTO is not real config mode. */
    TuningMode tuningMode;                 /*!< tuningMode is used to define user cases,
                                                like video or still. */
    std::string aiqbName;                       /*!< special aiqb name corresponding with TuningMode */
} TuningConfig;

typedef struct {
    /*!< tuningMode is used to define user cases, like video or still. */
    TuningMode tuningMode;
    unsigned int cmcTag;
    unsigned int aiqTag;
    unsigned int ispTag;
    unsigned int othersTag;
} LardTagConfig;

typedef struct {
    uint32_t horizontal_crop_offset;
    uint32_t vertical_crop_offset;
    uint32_t cropped_image_width;
    uint32_t cropped_image_height;
    uint32_t horizontal_scaling_numerator;
    uint32_t horizontal_scaling_denominator;
    uint32_t vertical_scaling_numerator;
    uint32_t vertical_scaling_denominator;
} SensorFrameParams;

enum ExecutorNotifyPolicy {
    POLICY_FRAME_FIRST = 0,
    POLICY_STATS_FIRST,
    POLICY_INVALID,
};

struct ExecutorPolicy {
    std::string exeName;
    ExecutorNotifyPolicy notifyPolicy;
    std::vector<std::string> pgList;
    std::vector<int> opModeList;
    std::vector<int> cyclicFeedbackRoutineList;
    std::vector<int> cyclicFeedbackDelayList;
    ExecutorPolicy() : notifyPolicy(POLICY_FRAME_FIRST) {}
};

struct ExecutorDepth {
    std::vector<std::string> bundledExecutors;
    std::vector<int> depths;
};

// <pgname, port of input refer terminal>
typedef std::pair<std::string, int32_t> ShareReferIdDesc;

struct PolicyConfig {
    int graphId;
    std::string policyDescription;
    std::vector<ExecutorPolicy> pipeExecutorVec;
    std::vector<std::string> exclusivePgs;
    std::vector<ExecutorDepth> bundledExecutorDepths;
    std::vector<ShareReferIdDesc> shareReferPairList;  // i: producer; i+1: consumer
    bool enableBundleInSdv;

    PolicyConfig() { graphId = -1; enableBundleInSdv = true; }
};

struct CommonConfig {
    float xmlVersion;
    std::string ipuName;
    std::vector<std::string> availableSensors;
    bool isGpuTnrEnabled;
    bool isStillTnrPrior;
    bool isTnrParamForceUpdate;
    int cameraNumber;

    CommonConfig() {
        xmlVersion = 1.0;
        isGpuTnrEnabled = false;
        isStillTnrPrior = true;
        isTnrParamForceUpdate = false;
        cameraNumber = -1;
    }
};

struct OBSetting {
    ConfigMode configMode;
    int top;
    int left;
    int sectionHeight;
    int interleaveStep;
};

struct ExpRange {
    int min;
    int max;
    int step;
    int lowerBound;
    int upperBound;
};

/**
 * Multi exposure range information
*/
struct MultiExpRange {
    camera_resolution_t Resolution;
    ExpRange SHS1;
    ExpRange RHS1;
    ExpRange SHS2;
    ExpRange RHS2;
    ExpRange SHS3;
};

struct UserToPslOutputMap {
    camera_resolution_t User;
    camera_resolution_t Psl;
};

struct FrameInfo {
    FrameInfo() {}
    int mWidth = 0;
    int mHeight = 0;
    int mFormat = 0;
    int mStride = 0;
    int mBpp = 0;
};
typedef std::map<Port, FrameInfo> FrameInfoPortMap;

/**
 * Indicate if the graph setting for video and still is coupled or dispersed
 */
typedef enum {
    COUPLED,
    DISPERSED,
} GraphSettingType;

} /* namespace icamera */
