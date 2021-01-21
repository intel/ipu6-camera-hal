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
#include <unordered_map>

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
#include "modules/sandboxing/client/IntelCca.h"
#else
#include "modules/algowrapper/IntelCca.h"
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
    int setStatsParams(const cca::cca_stats_params &statsParams);

    /**
     * \brief run AE
     *
     * \return OK if succeed, other value indicates failed
     */
    int runAe(long requestId, AiqResult* aiqResult);

    /**
     * \brief run AWB, AF, SA, PA and GBCE
     *
     * \return OK if succeed, other value indicates failed
     */
    int runAiq(long requestId, AiqResult *aiqResult);

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

    int runAEC(long requestId, cca::cca_ae_results* aeResults);
    void focusDistanceResult(const cca::cca_af_results *afResults,
                             float *afDistanceDiopters,
                             camera_range_t *focusRange);
    int processSAResults(cca::cca_sa_results *saResults, float *lensShadingMap);
    int checkColorOrder(cmc_bayer_order bayerOrder, ColorOrder *colorOrder);
    int storeLensShadingMap(const LSCGrid &inputLscGrid,
                            const LSCGrid &resizeLscGrid, float *dstLscGridRGGB);
    int reFormatLensShadingMap(const LSCGrid &inputLscGrid, float *dstLscGridRGGB);

    int calculateHyperfocalDistance(TuningMode mode);
    int calculateDepthOfField(const cca::cca_af_results &afResults, camera_range_t *focusRange);
    int initAiqPlusParams();
    // debug dumpers
    int dumpPaParams(const cca::cca_pa_params& pa);
    int dumpSaResult(const cca::cca_sa_results* saResult);

    IntelCca* getIntelCca(TuningMode tuningMode);

private:
    int mCameraId;
    unsigned long long mTimestamp;  // Latest statistics timestamp
    float mSensorPixelClock;

    bool mAeForceLock;
    bool mAwbForceLock;
    bool mAfForceLock;

    std::unique_ptr<Intel3AParameter> mIntel3AParameter;

    // Original AeResult class arrays are kept in 3a engine which is safely used here.
    cca::cca_ae_results mLastAeResult;
    cca::cca_awb_results mLastAwbResult;
    cca::cca_af_results mLastAfResult;

    uint64_t mAeRunTime;
    uint64_t mAwbRunTime;
    uint64_t mAiqRunTime;

    std::unordered_map<TuningMode, IntelCca*> mIntelCcaHandles;

    enum AiqState {
        AIQ_NOT_INIT = 0,
        AIQ_INIT,
        AIQ_CONFIGURED,
        AIQ_MAX
    } mAiqState;

    ia_aiq_frame_params mFrameParams;

    cca::cca_gbce_input_params mGbceParams;
    cca::cca_pa_input_params mPaParams;
    ia_aiq_color_channels mPaColorGains;
    cca::cca_sa_input_params mSaParams;

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

    camera_tonemap_mode_t mTonemapMode;

    std::unique_ptr<cca::cca_aiq_params> mAiqParams;
    std::unique_ptr<cca::cca_aiq_results> mAiqResults;

    int32_t mTonemapMaxCurvePoints;
};

} /* namespace icamera */
