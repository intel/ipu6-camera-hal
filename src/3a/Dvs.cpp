/*
 * Copyright (C) 2017-2020 Intel Corporation.
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

#define LOG_TAG "Dvs"
#include "src/3a/Dvs.h"

#include <ia_cmc_parser.h>
#include <ia_pal_types_isp_ids_autogen.h>

#include <algorithm>
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Utils.h"
#include "PlatformData.h"
#include "AiqUtils.h"
#include "AiqResultStorage.h"
#include "IGraphConfigManager.h"
#include "IGraphConfig.h"

namespace icamera {

const int DVS_MIN_ENVELOPE = 12;
const float QUATERNION_CLIP_MAX_ANGLE = 3.5f;

const int DVS_OXDIM_Y = 128;
const int DVS_OYDIM_Y = 32;
const int DVS_OXDIM_UV  = 64;
const int DVS_OYDIM_UV  = 16;

Dvs::Dvs(int cameraId, AiqSetting *setting) :
    mDvsHandle(nullptr),
    mDvsEnabled(false),
    mLdcEnabled(false),
    mRscEnabled(false),
    mDigitalZoomRatio(1.0f),
    mCameraId(cameraId),
    mFps(30),
    mConfigMode(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL),
    mTuningMode(TUNING_MODE_VIDEO),
    mAiqSetting(setting),
    mKernelId(0),
    mMorphTable(nullptr),
    mStatistics(nullptr),
    mConfigured(false) {
    LOG1("@%s", __func__);

    CLEAR(mSrcResolution);
    CLEAR(mDstResolution);
    CLEAR(mEnvelopeResolution);
    CLEAR(mImage_transformation);

    mIntelDvs = new IntelDvs();
}

Dvs::~Dvs() {
    LOG1("@%s", __func__);
    delete mIntelDvs;
}

int Dvs::initDvsHandle(TuningMode tuningMode) {
    ia_binary_data aiqData;
    uintptr_t cmcHandle = reinterpret_cast<uintptr_t>(nullptr);
    int ret = PlatformData::getCpfAndCmc(mCameraId, nullptr, &aiqData, nullptr,
                                         &cmcHandle, tuningMode);
    CheckError(ret != OK, BAD_VALUE, "@%s, Get cpf data failed", __func__);

    ia_err err = mIntelDvs->init(aiqData, reinterpret_cast<ia_cmc_t*>(cmcHandle), &mDvsHandle);
    CheckError(err != ia_err_none, NO_INIT, "@%s, Failed to initilize IntelDvs", __func__);

    return OK;
}

int Dvs::deinitDvsHandle() {
    int status = deInitDVSTable();
    if (mDvsHandle) {
        mIntelDvs->deinit(mDvsHandle);
        mDvsHandle = nullptr;
    }

    return status;
}

int Dvs::init() {
    LOG1("@%s", __func__);

    return OK;
}

int Dvs::deinit() {
    LOG1("@%s", __func__);
    AutoMutex l(mLock);

    return deinitDvsHandle();
}

int Dvs::configure(const std::vector<ConfigMode>& configModes) {
    LOG1("@%s, isDvsSupported:%d", __func__, PlatformData::isDvsSupported(mCameraId));
    AutoMutex l(mLock);
    mConfigured = false;
    if (!PlatformData::isDvsSupported(mCameraId)) return OK;

    if (configModes.empty()) {
        return UNKNOWN_ERROR;
    }
    mConfigMode = configModes[0];

    TuningMode tuningMode;
    if (PlatformData::getTuningModeByConfigMode(mCameraId, mConfigMode, tuningMode) != OK) {
        return UNKNOWN_ERROR;
    }
    mTuningMode = tuningMode;

    mKernelId = 0;
    CLEAR(mSrcResolution);
    CLEAR(mDstResolution);

    return reconfigure();
}

int Dvs::configure(TuningMode tuningMode, uint32_t kernelId,
                   int srcWidth, int srcHeight, int dstWidth, int dstHeight) {
    LOG1("@%s, isDvsSupported:%d", __func__, PlatformData::isDvsSupported(mCameraId));
    AutoMutex l(mLock);
    mConfigured = false;
    if (!PlatformData::isDvsSupported(mCameraId)) return OK;

    mTuningMode = tuningMode;
    mKernelId = kernelId;
    mSrcResolution.width = srcWidth;
    mSrcResolution.height = srcHeight;
    mDstResolution.width = dstWidth;
    mDstResolution.height = dstHeight;

    return reconfigure();
}

int Dvs::setDVSConfiguration(uint32_t kernelId, ia_dvs_configuration *config) {
    config->num_axis = mDvsEnabled ? ia_dvs_algorithm_6_axis : ia_dvs_algorithm_0_axis;

    /* General setting for dvs */
    config->source_bq.width_bq = mSrcResolution.width / 2;
    config->source_bq.height_bq = mSrcResolution.height / 2;
    config->output_bq.width_bq = mDstResolution.width / 2;
    config->output_bq.height_bq = mDstResolution.height / 2;
    // if the DstResolution is valid, the output_bq from dstResolution.
    if (mDstResolution.width != 0 && mDstResolution.height != 0) {
        config->output_bq.width_bq = mDstResolution.width / 2;
        config->output_bq.height_bq = mDstResolution.height / 2;
    }
    config->ispfilter_bq.width_bq = DVS_MIN_ENVELOPE / 2;
    config->ispfilter_bq.height_bq = DVS_MIN_ENVELOPE / 2;

    // config->num_axis = ia_dvs_algorithm_0_axis;
    config->gdc_shift_x = 0;
    config->gdc_shift_y = 0;

    if (kernelId == ia_pal_uuid_isp_gdc3_1) {
        config->oxdim_y = DVS_OXDIM_Y;
        config->oydim_y = DVS_OYDIM_Y;
        config->oxdim_uv = DVS_OXDIM_UV;
        config->oydim_uv = DVS_OYDIM_UV;
    } else {
        config->oxdim_y = DVS_OXDIM_Y / 2;
        config->oydim_y = DVS_OYDIM_Y;
        config->oxdim_uv = DVS_OXDIM_UV;
        config->oydim_uv = DVS_OYDIM_UV;
    }

    config->hw_config.scan_mode = ia_dvs_gdc_scan_mode_stb;
    config->hw_config.interpolation = ia_dvs_gdc_interpolation_bci;
    config->hw_config.performance_point = ia_dvs_gdc_performance_point_1x1;

    config->gdc_buffer_config.x_offset = 0;
    config->gdc_buffer_config.y_offset = 0;
    config->gdc_buffer_config.width = config->source_bq.width_bq;
    config->gdc_buffer_config.height = config->source_bq.height_bq;
    config->frame_rate = mFps;
    config->validate_morph_table = false;

    /*
     * cropping from the active pixel array, needs to be coming from history
     */
    config->crop_params.horizontal_crop_offset = 0;
    config->crop_params.vertical_crop_offset = 0;
    config->crop_params.cropped_width = 0;
    config->crop_params.cropped_height = 0;
    config->quaternion_clip_max_angle = QUATERNION_CLIP_MAX_ANGLE;

    const float Max_Ratio = 1.45f;
    int bq_max_width = static_cast<int>(Max_Ratio * static_cast<float>(config->output_bq.width_bq));
    int bq_max_height =
                      static_cast<int>(Max_Ratio * static_cast<float>(config->output_bq.height_bq));

    config->envelope_bq.width_bq = mEnvelopeResolution.width / 2 - config->ispfilter_bq.width_bq;
    config->envelope_bq.height_bq = mEnvelopeResolution.height / 2 - config->ispfilter_bq.height_bq;
    // envelope should be larger than 0
    config->envelope_bq.width_bq = std::max(0, config->envelope_bq.width_bq);
    config->envelope_bq.height_bq = std::max(0, config->envelope_bq.height_bq);

    if (config->source_bq.width_bq - config->envelope_bq.width_bq -
        config->ispfilter_bq.width_bq > bq_max_width)
        config->envelope_bq.width_bq =
            config->source_bq.width_bq - config->ispfilter_bq.width_bq - bq_max_width;

    if (config->source_bq.height_bq - config->envelope_bq.height_bq -
        config->ispfilter_bq.height_bq > bq_max_height)
        config->envelope_bq.height_bq =
            config->source_bq.height_bq - config->ispfilter_bq.height_bq - bq_max_height;

    if (mLdcEnabled) {
        // The crop must be set in LDC function, or there is config dvs fail
        config->crop_params.cropped_width = mDstResolution.width / 2;
        config->crop_params.cropped_height = mDstResolution.height / 2;
        // envelope bq is only for stabilization and it has to be set as 0 when ldc enabled.
        // TODO: clear define the envelope_bq when ldc & video stabilization enabled together
        config->envelope_bq.width_bq = 0;
        config->envelope_bq.height_bq = 0;
        config->use_lens_distortion_correction = true;
    }

    if (mRscEnabled) {
        // TODO: set config.nonblanking_ratio to inputReadoutTime/framePeriod.
    }
    return 0;
}

int Dvs::reconfigure() {
    LOG1("@%s", __func__);

    int status = OK;
    uint32_t gdcKernelId = mKernelId;

    // If parameters are not valid, try to query them in GC.
    if (gdcKernelId == 0 || mSrcResolution.width == 0 || mSrcResolution.height == 0) {
        // update GC
        std::shared_ptr<IGraphConfig> gc = nullptr;

        if (PlatformData::getGraphConfigNodes(mCameraId)) {
            IGraphConfigManager *GCM = IGraphConfigManager::getInstance(mCameraId);
            if (GCM) {
                gc = GCM->getGraphConfig(mConfigMode);
            }
        }
        CheckWarning(gc == nullptr, OK, "Failed to get GC in DVS");

        // update resolution infomation
        ia_isp_bxt_resolution_info_t resolution;
        status = gc->getGdcKernelSetting(&gdcKernelId, &resolution);
        CheckWarning(status != OK, OK, "Failed to get GDC kernel setting, DVS disabled");
        mSrcResolution.width = resolution.input_width;
        mSrcResolution.height = resolution.input_height;
        mDstResolution.width = resolution.output_width;
        mDstResolution.height = resolution.output_height;
        mEnvelopeResolution.width = resolution.input_crop.left + resolution.input_crop.right;
        mEnvelopeResolution.height = resolution.input_crop.top + resolution.input_crop.bottom;
    }
    LOG1("%s, GDC kernel setting: id: %u, resolution:src: %dx%d, dst: %dx%d, envelope: %dx%d",
         __func__, gdcKernelId, mSrcResolution.width, mSrcResolution.height, mDstResolution.width,
         mDstResolution.height, mEnvelopeResolution.width, mEnvelopeResolution.height);

    if (mDvsHandle) {
        deinitDvsHandle();
    }
    status = initDvsHandle(mTuningMode);

    if (!mDvsHandle)
        return status;

    ia_dvs_configuration config;
    CLEAR(config);

    setDVSConfiguration(gdcKernelId, &config);
    dumpConfiguration(config);

    CheckError(mSrcResolution.width <= (config.envelope_bq.width_bq * 2), UNKNOWN_ERROR,
               "%s the mSrcResolution width: %d is too small", __func__, mSrcResolution.width);
    CheckError(mSrcResolution.height <= (config.envelope_bq.height_bq * 2), UNKNOWN_ERROR,
               "%s the mSrcResolution height: %d is too small", __func__, mSrcResolution.height);

    float zoomHRatio = mSrcResolution.width /
                       (mSrcResolution.width - config.envelope_bq.width_bq * 2);
    float zoomVRatio = mSrcResolution.height /
                       (mSrcResolution.height - config.envelope_bq.height_bq * 2);
    ia_err err = mIntelDvs->config(mDvsHandle, &config,
                                   (zoomHRatio > zoomVRatio) ? zoomHRatio : zoomVRatio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "Configure DVS failed %d", err);

    LOG2("Configure DVS success");
    err = mIntelDvs->setNonBlankRatio(mDvsHandle, config.nonblanking_ratio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "set non blank ratio failed %d", err);

    status = initDVSTable();
    CheckError(status != OK, UNKNOWN_ERROR, "Allocate dvs table failed");
    mConfigured = true;

    return status;
}

void Dvs::handleEvent(EventData eventData) {
    if (eventData.type != EVENT_PSYS_STATS_BUF_READY) return;
    if (!mConfigured) return;

    LOG3A("%s: handle EVENT_PSYS_STATS_BUF_READY", __func__);
    int64_t sequence = eventData.data.statsReady.sequence;

    AiqResultStorage* aiqResultStorage = AiqResultStorage::getInstance(mCameraId);
    DvsStatistics *dvsStatistics = aiqResultStorage->getDvsStatistics();
    if (dvsStatistics->sequence != sequence || dvsStatistics->dvsStats == nullptr) return;

    // Set dvs statistics
    {
    AutoMutex l(mLock);
    mStatistics = dvsStatistics->dvsStats;
    }

    // Run dvs
    if (mAiqSetting) {
        aiq_parameter_t aiqParam;
        mAiqSetting->getAiqParameter(aiqParam);
        updateParameter(aiqParam);
    }

    DvsResult *dvsResult = aiqResultStorage->acquireDvsResult();

    const AiqResult *feedback = aiqResultStorage->getAiqResult(sequence);
    if (feedback == nullptr) {
        LOGW("%s: no aiq result for sequence %ld! use the latest instead", __func__, sequence);
        feedback = aiqResultStorage->getAiqResult();
    }

    const ia_aiq_af_results *afResults = nullptr;
    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        afResults = &feedback->mAfResults;
    }
    int ret = run(&feedback->mAeResults, afResults, dvsResult, sequence);
    CheckError(ret != OK, VOID_VALUE, "Run DVS fail");

    aiqResultStorage->updateDvsResult(sequence);
}

int Dvs::run(const ia_aiq_ae_results *aeResults, const ia_aiq_af_results *afResults,
             DvsResult *result, int64_t sequence) {
    LOG2("@%s", __func__);
    if (!mConfigured) return OK;

    PERF_CAMERA_ATRACE_IMAGING();
    AutoMutex l(mLock);

    runImpl(aeResults, afResults);

    int dvsType = PlatformData::getDVSType(mCameraId);
    switch (dvsType) {
        case MORPH_TABLE: {
            return getMorphTable(sequence, result);
        }
        case IMG_TRANS:
            return getImageTrans(sequence, result);
        default:
            LOGE("not supportted dvs type");
            return UNKNOWN_ERROR;
    }
}

int Dvs::configureDigitalZoom(ia_dvs_zoom_mode zoom_mode, ia_rectangle *zoom_region,
                              ia_coordinate *zoom_coordinate) {
    LOG2("@%s zoom mode:%d", __func__, zoom_mode);
    AutoMutex l(mLock);

    ia_err err = mIntelDvs->setDigitalZoomMode(mDvsHandle, zoom_mode);
    CheckError(err != ia_err_none, BAD_VALUE, "set zoom mode error: %d", err);

    if (zoom_mode == ia_dvs_zoom_mode_region) {
        err = mIntelDvs->setDigitalZoomRegion(mDvsHandle, zoom_region);
    } else if (zoom_mode == ia_dvs_zoom_mode_coordinate) {
        err = mIntelDvs->setDigitalZoomCoordinate(mDvsHandle, zoom_coordinate);
    }

    int ret = AiqUtils::convertError(err);
    CheckError(ret != OK, ret, "Error config zoom: %d", ret);

    return OK;
}

int Dvs::setZoomRatio(float zoomRatio) {
    LOG2("@%s zoom:%4.2f", __func__, zoomRatio);
    AutoMutex l(mLock);

    ia_err err = mIntelDvs->setDigitalZoomMagnitude(mDvsHandle, zoomRatio);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "set digital zoom magnitude error: %d", err);

    return OK;
}

/**
 * Private function implementations. mLock is assumed to be held.
 */

int Dvs::initDVSTable() {
    LOG1("@%s", __func__);

    int dvsType = PlatformData::getDVSType(mCameraId);
    switch (dvsType) {
        case MORPH_TABLE:
            if (mMorphTable) {
                mIntelDvs->freeMorphTable(mDvsHandle, mMorphTable);
                mMorphTable = nullptr;
            }
            if (mDvsHandle) {
                mMorphTable = mIntelDvs->allocateMorphTalbe(mDvsHandle);
                CheckError(!mMorphTable, UNKNOWN_ERROR, "mMorphTable allocate failed");
            }
            break;
        case IMG_TRANS:
            LOG1("not need allocate MorphTable for image_transformation");
            break;
        default:
            LOGE("not supportted dvs type");
            return UNKNOWN_ERROR;
    }
    return OK;
}

int Dvs::deInitDVSTable() {
    int status = OK;
    if (mMorphTable) {
        mIntelDvs->freeMorphTable(mDvsHandle, mMorphTable);
        mMorphTable = nullptr;
    }

    return status;
}

int Dvs::runImpl(const ia_aiq_ae_results *aeResults, const ia_aiq_af_results *afResults) {
    LOG2("@%s", __func__);
    ia_err err = ia_err_none;
    int ret = OK;

    if (!mDvsHandle)
        return UNKNOWN_ERROR;

    if ((mDvsEnabled) && mStatistics && mStatistics->vector_count > 0) {
        err = mIntelDvs->setStatistics(mDvsHandle, mStatistics,
                                       aeResults, afResults, /*sensor events*/nullptr, 0, 0);
        ret = AiqUtils::convertError(err);
        CheckError(ret != OK, ret, "DVS set statistics failed: %d", ret);
    } else if ((mDvsEnabled) && !mStatistics) {
        return UNKNOWN_ERROR;
    }

    uint16_t focusPosition = 0;
    if (afResults) {
        focusPosition = static_cast<uint16_t>(afResults->next_lens_position);
    }

    err = mIntelDvs->execute(mDvsHandle, focusPosition);
    ret = AiqUtils::convertError(err);
    CheckError(ret != OK, ret, "DVS execution failed: %d", ret);

    return OK;
}

int Dvs::getMorphTable(int64_t sequence, DvsResult *result) {
    LOG2("@%s", __func__);
    CheckError(!result, UNKNOWN_ERROR, "@%s, result is null", __func__);

    int ret = mIntelDvs->getMorphTable(mDvsHandle, mMorphTable, result);
    CheckError(ret != OK, ret, "Error geting DVS result: %d", ret);
    return dumpDVSTable(&result->mMorphTable, sequence);
}

int Dvs::getImageTrans(int64_t sequence, DvsResult *result) {
    LOG2("@%s", __func__);

    ia_err err = mIntelDvs->getImageTransformation(mDvsHandle, &mImage_transformation);
    int ret = AiqUtils::convertError(err);
    CheckError(ret != OK, ret, "Error geting DVS result: %d", ret);
    dumpDVSTable(&mImage_transformation, sequence);
    return AiqUtils::deepCopyDvsResults(mImage_transformation, &result->mTransformation);
}

int Dvs::updateParameter(const aiq_parameter_t &param) {
    LOG2("@%s", __func__);
    if (!mConfigured) return OK;

    bool dvsEnabled = (param.videoStabilizationMode == VIDEO_STABILIZATION_MODE_ON);
    bool ldcEnabled = (param.ldcMode == LDC_MODE_ON);
    bool rscEnabled = (param.rscMode == RSC_MODE_ON);
    int digitalZoomRatio = param.digitalZoomRatio;

    if ((param.fps > 0.01 && param.fps != mFps)
        || param.tuningMode != mTuningMode
        || dvsEnabled != mDvsEnabled || ldcEnabled != mLdcEnabled
        || rscEnabled != mRscEnabled) {
        mFps = param.fps > 0.01 ? param.fps : mFps;
        mTuningMode = param.tuningMode;
        mDvsEnabled = dvsEnabled;
        mLdcEnabled = ldcEnabled;
        mRscEnabled = rscEnabled;

        LOG3A("%s: DVS fps = %f ", __func__, mFps);
        LOG3A("%s: DVS tuning Mode = %d ", __func__, mTuningMode);
        LOG3A("%s: DVS enabled = %d ", __func__, mDvsEnabled);
        LOG3A("%s: LDC enabled = %d ", __func__, mLdcEnabled);
        LOG3A("%s: RSC enabled = %d ", __func__, mRscEnabled);

        return reconfigure();
    }

    if (param.digitalZoomRatio > 0 && param.digitalZoomRatio!= mDigitalZoomRatio) {
        mDigitalZoomRatio = digitalZoomRatio;
        setZoomRatio(mDigitalZoomRatio);
        LOG3A("%s: digital zoom ratio = %f ", __func__, mDigitalZoomRatio);
    }

    return OK;
}

int Dvs::dumpDVSTable(ia_dvs_morph_table *table, int64_t sequence) {
    if (!CameraDump::isDumpTypeEnable(DUMP_AIQ_DVS_RESULT)) return OK;

    LOG3A("%s", __func__);

    CheckError(!table, BAD_VALUE, "%s: morph table is nullptr, and nothing to dump.", __func__);

    BinParam_t bParam;
    bParam.bType = BIN_TYPE_GENERAL;
    bParam.mType = M_PSYS;
    bParam.sequence = sequence;
    bParam.gParam.appendix = "dvs_morph_table_x_y";
    CameraDump::dumpBinary(0, table->xcoords_y,
                           table->width_y * table->height_y * sizeof(uint32_t), &bParam);
    bParam.gParam.appendix = "dvs_morph_table_y_y";
    CameraDump::dumpBinary(0, table->ycoords_y,
                           table->width_y * table->height_y * sizeof(uint32_t), &bParam);
    bParam.gParam.appendix = "dvs_morph_table_x_uv";
    CameraDump::dumpBinary(0, table->xcoords_uv,
                           table->width_uv * table->height_uv * sizeof(uint32_t), &bParam);
    bParam.gParam.appendix = "dvs_morph_table_y_uv";
    CameraDump::dumpBinary(0, table->ycoords_uv,
                           table->width_uv * table->height_uv * sizeof(uint32_t), &bParam);

    LOG3A("%s: DVS morph table y=[%d x %d], uv=[%d x %d] changed=%s", __func__,
          table->width_y, table->height_y,
          table->width_uv, table->height_uv,
          table->morph_table_changed == true ? "TRUE" : "FALSE");
    return OK;
}

int Dvs::dumpDVSTable(ia_dvs_image_transformation *trans, int64_t sequence) {
    if (!CameraDump::isDumpTypeEnable(DUMP_AIQ_DVS_RESULT)) return OK;

    LOG3A("%s", __func__);

    CheckError(!trans, BAD_VALUE, "%s: trans table is nullptr, and nothing to dump.", __func__);

    LOG3A("%s: DVS trans table num_homography_matrices=%d", __func__,
            trans->num_homography_matrices);

    BinParam_t bParam;
    bParam.bType = BIN_TYPE_GENERAL;
    bParam.mType = M_PSYS;
    bParam.sequence = sequence;
    for (int i = 0; i < DVS_HOMOGRAPHY_MATRIX_MAX_COUNT; i++) {
        LOG3A("%s: DVS trans table %d start_row=%d", __func__,
            i, trans->matrices[i].start_row);
        bParam.gParam.appendix = "matrices";
        CameraDump::dumpBinary(0, &trans->matrices[i].matrix, 3 * 3 * sizeof(float), &bParam);
    }

    return OK;
}

int Dvs::dumpConfiguration(ia_dvs_configuration config) {
    LOG3A("%s", __func__);

    LOG3A("config.num_axis %d", config.num_axis);
    LOG3A("config.nonblanking_ratio %f", config.nonblanking_ratio);
    LOG3A("config.source_bq.width_bq %d", config.source_bq.width_bq);
    LOG3A("config.source_bq.height_bq %d", config.source_bq.height_bq);
    LOG3A("config.output_bq.width_bq %d", config.output_bq.width_bq);
    LOG3A("config.output_bq.height_bq %d", config.output_bq.height_bq);
    LOG3A("config.envelope_bq.width_bq %d", config.envelope_bq.width_bq);
    LOG3A("config.envelope_bq.height_bq %d", config.envelope_bq.height_bq);
    LOG3A("config.ispfilter_bq.width_bq %d", config.ispfilter_bq.width_bq);
    LOG3A("config.ispfilter_bq.height_bq %d", config.ispfilter_bq.height_bq);
    LOG3A("config.gdc_shift_x %d", config.gdc_shift_x);
    LOG3A("config.gdc_shift_y %d", config.gdc_shift_y);
    LOG3A("config.oxdim_y %d", config.oxdim_y);
    LOG3A("config.oydim_y %d", config.oydim_y);
    LOG3A("config.oxdim_uv %d", config.oxdim_uv);
    LOG3A("config.oydim_uv %d", config.oydim_uv);
    LOG3A("config.hw_config.scan_mode %d", config.hw_config.scan_mode);
    LOG3A("config.hw_config.interpolation %d", config.hw_config.interpolation);
    LOG3A("config.hw_config.performance_point %d", config.hw_config.performance_point);
    LOG3A("config.validate_morph_table = %s",
          (config.validate_morph_table == true) ? "true" : "false");
    LOG3A("config.use_lens_distortion_correction = %s",
          (config.use_lens_distortion_correction == true) ? "true" : "false");

    return OK;
}

}  // namespace icamera
