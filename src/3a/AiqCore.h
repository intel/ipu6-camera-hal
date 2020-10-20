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

#include <memory>

#include "ia_aiq.h"
#include "ia_ltm.h"
#include "ia_cmc_types.h"
#ifndef ENABLE_SANDBOXING
#include "ia_log.h"
#endif

#include "AiqSetting.h"
#include "AiqResult.h"

#include "Intel3AParameter.h"

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelAiq.h"
#else
#include "modules/algowrapper/IntelAiq.h"
#endif

namespace icamera {

/*
 * \class AiqCore
 * This class is used to set parameter, statistics and run Ae,
 * Af, Awb, Gbce, Pa, Sa.
 */
class AiqCore {

public:
    AiqCore(int cameraId);
    ~AiqCore();

    /**
     * \brief AiqCore init
     *
     * Init AiqPlus and AAAObject
     */
    int init();

    /**
     * \brief AiqCore deinit
     *
     * Deinit AiqPlus and AAAObject
     */
    int deinit();

    /**
     * \brief AiqCore configure
     *
     * Configure AiqPlus ConfigMode
     */
    int configure(const std::vector<ConfigMode>& configModes);

    /**
     * \brief Set sensor and frame info
     *
     * \param frameParams: the frame info parameter
     * \param descriptor: the sensor info parameter
     */
    int setSensorInfo(const ia_aiq_frame_params &frameParams,
                      const ia_aiq_exposure_sensor_descriptor &descriptor);

    /**
     * \brief update param and set converge speed
     *
     * \param param: the parameter update to AiqPlus and Aiq3A or custom 3A
     */
    int updateParameter(const aiq_parameter_t &param);

    /**
     * \brief Set ispStatistics to AiqPlus and Aiq3A or custom 3A
     */
    int setStatistics(const ia_aiq_statistics_input_params_v4 *ispStatistics);

    /**
     * \brief run 3A and AiqPlus
     *
     * \return OK if succeed, other value indicates failed
     */
    int runAiq(AiqResult *aiqResult);

private:
    DISALLOW_COPY_AND_ASSIGN(AiqCore);

    // LSC data
    typedef struct ColorOrder {
        uint8_t r[2];
        uint8_t gr[2];
        uint8_t gb[2];
        uint8_t b[2];
    } ColorOrder;

    class LSCGrid {
     public: /* this was a struct: class just to satisfy a static code scanner */
        uint16_t width;
        uint16_t height;
        uint16_t *gridR;
        uint16_t *gridGr;
        uint16_t *gridGb;
        uint16_t *gridB;

        bool isBad() const {
            return (gridB == NULL || gridGb == NULL || gridR == NULL ||
                    gridGr == NULL || width == 0 || height == 0);
        }
        LSCGrid(): width(0), height(0), gridR(NULL), gridGr(NULL),
            gridGb(NULL), gridB(NULL) {}
    };

    int run3A(AiqResult *aiqResult);
    int runAiqPlus(AiqResult *aiqResult);
    int runAe(ia_aiq_ae_results* aeResults);
    int runAf(AiqResult *aiqResult);
    void focusDistanceResult(const ia_aiq_af_results *afResults,
                             float *afDistanceDiopters,
                             camera_range_t *focusRange);
    int runAwb(ia_aiq_awb_results* awbResults);
    int runGbce(ia_aiq_gbce_results *gbceResults);
    int runPa(ia_aiq_pa_results_v1 *paResults,
              ia_aiq_awb_results *awbResults,
              ia_aiq_exposure_parameters *exposureParams,
              ia_aiq_advanced_ccm_t *preferredAcm);
    int runSa(ia_aiq_sa_results_v1 *saResults,
              ia_aiq_awb_results *awbResults,
              float *lensShadingMap);
    int processSAResults(ia_aiq_sa_results_v1 *saResults, float *lensShadingMap);
    int checkColorOrder(cmc_bayer_order bayerOrder, ColorOrder *colorOrder);
    int storeLensShadingMap(const LSCGrid &inputLscGrid,
                            const LSCGrid &resizeLscGrid, float *dstLscGridRGGB);
    int reFormatLensShadingMap(const LSCGrid &inputLscGrid, float *dstLscGridRGGB);

    int calculateHyperfocalDistance(TuningMode mode);
    int calculateDepthOfField(const ia_aiq_af_results &afResults, camera_range_t *focusRange);
    int initAiqPlusParams();
    int initIntelAiqHandle(const std::vector<TuningMode>& tuningModes);
    void deinitIntelAiqHandle(void);
    // debug dumpers
    int dumpPaResult(const ia_aiq_pa_results_v1* paResult);
    int dumpSaResult(const ia_aiq_sa_results_v1* saResult);

private:
    int mCameraId;
    unsigned long long mTimestamp;  // Latest statistics timestamp
    float mSensorPixelClock;

    bool mAeForceLock;
    bool mAwbForceLock;
    bool mAfForceLock;

    std::unique_ptr<Intel3AParameter> mIntel3AParameter;

    // Original AeResult class arrays are kept in 3a engine which is safely used here.
    ia_aiq_ae_results *mLastAeResult;
    ia_aiq_awb_results *mLastAwbResult;
    ia_aiq_af_results *mLastAfResult;

    int mAeRunTime;
    int mAwbRunTime;

    IntelAiq *mIntelAiqHandle[TUNING_MODE_MAX];
    bool mIntelAiqHandleStatus[TUNING_MODE_MAX];

    enum AiqState {
        AIQ_NOT_INIT = 0,
        AIQ_INIT,
        AIQ_CONFIGURED,
        AIQ_MAX
    } mAiqState;

    ia_aiq_frame_params mFrameParams;

    ia_aiq_gbce_input_params mGbceParams;
    ia_aiq_pa_input_params mPaParams;
    ia_aiq_color_channels mPaColorGains;
    ia_aiq_sa_input_params_v1 mSaParams;
    float mHyperFocalDistance;  // in millimeters

    TuningMode mTuningMode;
    camera_shading_mode_t mShadingMode;
    camera_lens_shading_map_mode_type_t mLensShadingMapMode;
    camera_coordinate_t mLensShadingMapSize;
    uint16_t mResizeLscGridR[DEFAULT_LSC_GRID_SIZE];
    uint16_t mResizeLscGridGr[DEFAULT_LSC_GRID_SIZE];
    uint16_t mResizeLscGridGb[DEFAULT_LSC_GRID_SIZE];
    uint16_t mResizeLscGridB[DEFAULT_LSC_GRID_SIZE];
    float mLscOffGrid[DEFAULT_LSC_GRID_SIZE * 4];
    float mLscGridRGGB[DEFAULT_LSC_GRID_SIZE * 4];
    size_t mLscGridRGGBLen;
    float mLastEvShift;

};

} /* namespace icamera */
