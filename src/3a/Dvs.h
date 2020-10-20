/*
 * Copyright (C) 2017-2020 Intel Corporation
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

#include <ia_dvs.h>
#include <ia_dvs_types.h>
#include <ia_isp_bxt.h>
#include <memory>
#include <vector>

#include "iutils/Thread.h"
#include "iutils/Errors.h"

#include "AiqSetting.h"
#include "CameraEvent.h"
#include "DvsResult.h"

#ifdef ENABLE_SANDBOXING
#include "modules/sandboxing/client/IntelDvs.h"
#else
#include "modules/algowrapper/IntelDvs.h"
#endif

namespace icamera {
struct DvsStatistics {
    explicit DvsStatistics(ia_dvs_statistics *dvs = nullptr, int64_t seq = -1) {
        dvsStats = dvs;
        sequence = seq;
    }
    ia_dvs_statistics *dvsStats;
    int64_t sequence;
};

/**
 * \class Dvs
 * Wrapper of the DVSx, provide 2 basic functionalities in video mode:
 * 1. zoom (including center and freeform)
 * 2. DVS
 * The algorithm should generate the morph table to support the
 * above functionalities.
 */
class Dvs : public EventListener {
 public:
    explicit Dvs(int cameraId, AiqSetting *setting = nullptr);
    ~Dvs();

    int init();
    int deinit();
    int configure(const std::vector<ConfigMode>& configMode);
    int configure(TuningMode tuningMode, uint32_t kernelId,
                  int srcWidth, int srcHeight, int dstWidth, int dstHeight);
    int updateParameter(const aiq_parameter_t &param);
    void handleEvent(EventData eventData);

    int run(const ia_aiq_ae_results *aeResults, const ia_aiq_af_results *afResults,
            DvsResult *result, int64_t sequence = 0);

 private:
    int configureDigitalZoom(ia_dvs_zoom_mode zoom_mode, ia_rectangle *zoom_region,
                             ia_coordinate *zoom_coordinate);
    int setZoomRatio(float zoom);

    int initDvsHandle(TuningMode tuningMode);
    int deinitDvsHandle();
    int initDVSTable();
    int deInitDVSTable();
    int reconfigure();
    int runImpl(const ia_aiq_ae_results *aeResults, const ia_aiq_af_results *afResults);
    int getMorphTable(int64_t sequence, DvsResult *result);
    int getImageTrans(int64_t sequence, DvsResult *result);
    int setDVSConfiguration(uint32_t kernelId, ia_dvs_configuration *config);
    int dumpDVSTable(ia_dvs_morph_table *table, int64_t sequence);
    int dumpDVSTable(ia_dvs_image_transformation *trans, int64_t sequence);
    int dumpConfiguration(ia_dvs_configuration config);

 private:
    // Guard for class Dvs public API
    IntelDvs *mIntelDvs;
    Mutex mLock;
    ia_dvs_state *mDvsHandle;
    bool mDvsEnabled;
    bool mLdcEnabled;
    bool mRscEnabled;
    float mDigitalZoomRatio;
    int mCameraId;
    float mFps;
    ConfigMode mConfigMode;
    TuningMode mTuningMode;
    AiqSetting *mAiqSetting;

    uint32_t mKernelId;
    camera_resolution_t mSrcResolution;
    camera_resolution_t mDstResolution;
    camera_resolution_t mEnvelopeResolution;

    ia_dvs_morph_table *mMorphTable;
    ia_dvs_image_transformation mImage_transformation;
    ia_dvs_statistics *mStatistics;
    bool mConfigured;

    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(Dvs);
};

}  /* namespace icamera */
