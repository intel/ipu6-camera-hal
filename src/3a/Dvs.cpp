/*
 * Copyright (C) 2017-2022 Intel Corporation.
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

#define LOG_TAG Dvs
#include "src/3a/Dvs.h"

#include <ia_cmc_parser.h>
#include <ia_pal_types_isp_ids_autogen.h>

#include <algorithm>

#include "AiqResultStorage.h"
#include "AiqUtils.h"
#include "IGraphConfig.h"
#include "IGraphConfigManager.h"
#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

const int DVS_OXDIM_Y = 128;
const int DVS_OYDIM_Y = 32;
const int DVS_OXDIM_UV = 64;
const int DVS_OYDIM_UV = 16;
const int DVS_MIN_ENVELOPE = 12;

Dvs::Dvs(int cameraId) : mCameraId(cameraId), mTuningMode(TUNING_MODE_VIDEO) {}

Dvs::~Dvs() {}

int Dvs::configure(const ConfigMode configMode, cca::cca_init_params* params) {
    CheckAndLogError(!params, BAD_VALUE, "params is nullptr");
    LOG2("@%s", __func__);

    for (uint8_t i = 0; i < params->dvs_ids.count; i++) {
        int ret = configCcaDvsData(params->dvs_ids.ids[i], configMode, params);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, configure DVS data error", __func__);
    }

    TuningMode tuningMode;
    if (PlatformData::getTuningModeByConfigMode(mCameraId, configMode, tuningMode) != OK) {
        return UNKNOWN_ERROR;
    }
    mTuningMode = tuningMode;

    return OK;
}

int Dvs::configCcaDvsData(int32_t streamId, const ConfigMode configMode,
                          cca::cca_init_params* params) {
    // update GC
    std::shared_ptr<IGraphConfig> gc = nullptr;
    if (PlatformData::getGraphConfigNodes(mCameraId)) {
        IGraphConfigManager* GCM = IGraphConfigManager::getInstance(mCameraId);
        if (GCM) {
            gc = GCM->getGraphConfig(configMode);
        }
    }
    CheckWarning(gc == nullptr, BAD_VALUE, "Failed to get GC in DVS");

    ia_isp_bxt_resolution_info_t resolution;
    uint32_t gdcKernelId;
    int status = gc->getGdcKernelSetting(&gdcKernelId, &resolution, streamId);
    CheckWarning(status != OK, UNKNOWN_ERROR, "Failed to get GDC kernel setting, DVS disabled");

    LOG2("%s, GDC kernel setting: id: %u, resolution:src: %dx%d, dst: %dx%d", __func__, gdcKernelId,
         resolution.input_width, resolution.input_height, resolution.output_width,
         resolution.output_height);

    cca::cca_gdc_configuration* gdcConfig = &params->gdcConfig;
    CLEAR(*gdcConfig);
    gdcConfig->gdc_filter_width = DVS_MIN_ENVELOPE / 2;
    gdcConfig->gdc_filter_height = DVS_MIN_ENVELOPE / 2;
    MEMCPY_S(&gdcConfig->gdc_resolution_info, sizeof(ia_isp_bxt_resolution_info_t), &resolution,
             sizeof(ia_isp_bxt_resolution_info_t));
    gdcConfig->pre_gdc_top_padding = 0;
    gdcConfig->pre_gdc_bottom_padding = 0;

    if (gdcKernelId == ia_pal_uuid_isp_gdc3_1) {
        gdcConfig->splitMetadata[0] = DVS_OYDIM_UV;
        gdcConfig->splitMetadata[1] = DVS_OXDIM_UV;
        gdcConfig->splitMetadata[2] = DVS_OYDIM_Y;
        gdcConfig->splitMetadata[3] = DVS_OXDIM_Y;
    } else {
        gdcConfig->splitMetadata[0] = DVS_OYDIM_UV;
        gdcConfig->splitMetadata[1] = DVS_OXDIM_UV;
        gdcConfig->splitMetadata[2] = DVS_OYDIM_Y;
        gdcConfig->splitMetadata[3] = DVS_OXDIM_Y / 2;
    }

    camera_resolution_t envelopeResolution;
    camera_resolution_t envelope_bq;
    envelopeResolution.width = resolution.input_crop.left + resolution.input_crop.right;
    envelopeResolution.height = resolution.input_crop.top + resolution.input_crop.bottom;
    envelope_bq.width = envelopeResolution.width / 2 - gdcConfig->gdc_filter_width;
    envelope_bq.height = envelopeResolution.height / 2 - gdcConfig->gdc_filter_height;
    // envelope should be larger than 0
    envelope_bq.width = std::max(0, envelope_bq.width);
    envelope_bq.height = std::max(0, envelope_bq.height);

    const float Max_Ratio = 1.45f;
    int bq_max_width =
        static_cast<int>(Max_Ratio * static_cast<float>(resolution.output_width / 2));
    int bq_max_height =
        static_cast<int>(Max_Ratio * static_cast<float>(resolution.output_height / 2));
    if (resolution.input_width / 2 - envelope_bq.width - gdcConfig->gdc_filter_width > bq_max_width)
        envelope_bq.width = resolution.input_width / 2 - gdcConfig->gdc_filter_width - bq_max_width;

    if (resolution.input_height / 2 - envelope_bq.height - gdcConfig->gdc_filter_height >
        bq_max_height)
        envelope_bq.height =
            resolution.input_height / 2 - gdcConfig->gdc_filter_height - bq_max_height;

    float zoomHRatio = resolution.input_width / (resolution.input_width - envelope_bq.width * 2);
    float zoomVRatio = resolution.input_height / (resolution.input_height - envelope_bq.height * 2);
    params->dvsZoomRatio = (zoomHRatio > zoomVRatio) ? zoomHRatio : zoomVRatio;
    params->enableVideoStablization = VIDEO_STABILIZATION_MODE_OFF;
    int dvsType = PlatformData::getDVSType(mCameraId);
    if (dvsType == IMG_TRANS) {
        params->dvsOutputType = cca::CCA_DVS_IMAGE_TRANSFORM;
    } else {
        params->dvsOutputType = cca::CCA_DVS_MORPH_TABLE;
    }

    gdcConfig->gdc_resolution_history = gdcConfig->gdc_resolution_info;

    ZoomParam zoomParam;
    CLEAR(zoomParam);
    zoomParam.gdcRegion.left = 0;
    zoomParam.gdcRegion.top = 0;
    zoomParam.gdcRegion.right = resolution.input_width / 2;
    zoomParam.gdcRegion.bottom = resolution.input_height / 2;

    {
        std::lock_guard<std::mutex> l(mLock);
        mZoomParamMap[streamId] = zoomParam;
    }

    dumpDvsConfiguration(*params);
    return OK;
}

void Dvs::setParameter(const Parameters& p) {
    camera_zoom_region_t region;
    if (p.getZoomRegion(&region) != OK) return;

    // Convert active pixel array system to GDC system.
    camera_coordinate_system_t srcSystem = PlatformData::getActivePixelArray(mCameraId);
    std::lock_guard<std::mutex> l(mLock);
    for (auto& it : mZoomParamMap) {
        auto& gdcRegion = it.second.gdcRegion;
        auto& ptzRegion = it.second.ptzRegion;

        camera_coordinate_system_t dstSystem = {gdcRegion.left, gdcRegion.top, gdcRegion.right,
                                                gdcRegion.bottom};
        LOG2("%s, dstSystem [%d, %d, %d, %d]", __func__, gdcRegion.left, gdcRegion.top,
             gdcRegion.right, gdcRegion.bottom);

        camera_coordinate_t srcCoordinate = {region.left, region.top};
        camera_coordinate_t dstCoordinate
            = AiqUtils::convertCoordinateSystem(srcSystem, dstSystem, srcCoordinate);
        ptzRegion.left = dstCoordinate.x;
        ptzRegion.top = dstCoordinate.y;
        srcCoordinate = {region.right, region.bottom};
        dstCoordinate = AiqUtils::convertCoordinateSystem(srcSystem, dstSystem, srcCoordinate);
        ptzRegion.right = dstCoordinate.x;
        ptzRegion.bottom = dstCoordinate.y;
        LOG2("%s, Ptz [%d, %d, %d, %d]", __func__, ptzRegion.left, ptzRegion.top,
             ptzRegion.right, ptzRegion.bottom);
    }
}

void Dvs::handleEvent(EventData eventData) {
    LOG2("@%s: eventData.type:%d", __func__, eventData.type);

    if (eventData.type != EVENT_DVS_READY) return;
    int streamId = eventData.data.dvsRunReady.streamId;

    IntelCca* intelCcaHandle = IntelCca::getInstance(mCameraId, mTuningMode);
    CheckAndLogError(!intelCcaHandle, VOID_VALUE, "@%s, Failed to get IntelCca instance", __func__);

    camera_zoom_region_t gdcRegion;
    camera_zoom_region_t ptzRegion;
    {
        std::lock_guard<std::mutex> l(mLock);
        if (mZoomParamMap.find(streamId) == mZoomParamMap.end()) return;

        gdcRegion = mZoomParamMap[streamId].gdcRegion;
        ptzRegion = mZoomParamMap[streamId].ptzRegion;
    }

    // Run DVS
    LOG2("%s: Ready to run DVS", __func__);

    cca::cca_dvs_zoom zp;
    memset(&zp, 0, sizeof(zp));
    zp.digital_zoom_ratio = 1.0f;
    zp.digital_zoom_factor = 1.0f;
    zp.zoom_mode = ia_dvs_zoom_mode_region;
    if (!ptzRegion.left && !ptzRegion.top && !ptzRegion.right && !ptzRegion.bottom) {
        zp.zoom_region = {gdcRegion.left, gdcRegion.top, gdcRegion.right, gdcRegion.bottom};
    } else {
        /*
            SCALER_CROP_REGION can adjust to a small crop region if the aspect of active
            pixel array is not same as the crop region aspect. Crop can only on either
            horizontally or veritacl but never both.
            If active pixel array's aspect ratio is wider than the crop region, the region
            should be further cropped vertically.
        */
        auto coord = PlatformData::getActivePixelArray(mCameraId);
        int wpa = coord.right - coord.left;
        int hpa = coord.bottom - coord.top;

        int width = ptzRegion.right - ptzRegion.left;
        int height = ptzRegion.bottom - ptzRegion.top;

        float aspect0 = static_cast<float>(wpa) / hpa;
        float aspect1 = static_cast<float>(width) / height;

        if (std::fabs(aspect0 - aspect1) < 0.00001) {
            zp.zoom_region = {ptzRegion.left, ptzRegion.top, ptzRegion.right, ptzRegion.bottom};
        } else if (aspect0 > aspect1) {
            auto croppedHeight = width / aspect0;
            int diff = std::abs(height - croppedHeight) / 2;
            zp.zoom_region = {ptzRegion.left, ptzRegion.top + diff, ptzRegion.right,
                              ptzRegion.bottom - diff};
        } else {
            auto croppedWidth = height * aspect0;
            int diff = std::abs(width - croppedWidth) / 2;
            zp.zoom_region = {ptzRegion.left + diff, ptzRegion.top, ptzRegion.right - diff,
                              ptzRegion.bottom};
        }
    }
    intelCcaHandle->updateZoom(streamId, zp);

    ia_err iaErr = intelCcaHandle->runDVS(streamId, eventData.data.statsReady.sequence);
    int ret = AiqUtils::convertError(iaErr);
    CheckAndLogError(ret != OK, VOID_VALUE, "Error running DVS: %d", ret);
    return;
}

void Dvs::dumpDvsConfiguration(const cca::cca_init_params& config) {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(Dvs), CAMERA_DEBUG_LOG_LEVEL3)) return;

    LOG3("config.dvsOutputType %d", config.dvsOutputType);
    LOG3("config.enableVideoStablization %d", config.enableVideoStablization);
    LOG3("config.dvsZoomRatio %f", config.dvsZoomRatio);
    LOG3("config.gdcConfig.pre_gdc_top_padding %d", config.gdcConfig.pre_gdc_top_padding);
    LOG3("config.gdcConfig.pre_gdc_bottom_padding %d", config.gdcConfig.pre_gdc_bottom_padding);
    LOG3("config.gdcConfig.gdc_filter_width %d", config.gdcConfig.gdc_filter_width);
    LOG3("config.gdcConfig.gdc_filter_height %d", config.gdcConfig.gdc_filter_height);
    LOG3("config.gdcConfig.splitMetadata[0](oydim_uv) %d", config.gdcConfig.splitMetadata[0]);
    LOG3("config.gdcConfig.splitMetadata[1](oxdim_uv) %d", config.gdcConfig.splitMetadata[1]);
    LOG3("config.gdcConfig.splitMetadata[2](oydim_y) %d", config.gdcConfig.splitMetadata[2]);
    LOG3("config.gdcConfig.splitMetadata[3](oxdim_y) %d", config.gdcConfig.splitMetadata[3]);
    LOG3("config.gdcConfig.gdc_resolution_info.input_width %d, input_height %d",
         config.gdcConfig.gdc_resolution_info.input_width,
         config.gdcConfig.gdc_resolution_info.input_height);
    LOG3("config.gdcConfig.gdc_resolution_info.output_width %d, output_height %d",
         config.gdcConfig.gdc_resolution_info.output_width,
         config.gdcConfig.gdc_resolution_info.output_height);
    LOG3("config.gdcConfig.gdc_resolution_info.input_crop.left %d, top %d, right %d, bottom %d",
         config.gdcConfig.gdc_resolution_info.input_crop.left,
         config.gdcConfig.gdc_resolution_info.input_crop.top,
         config.gdcConfig.gdc_resolution_info.input_crop.right,
         config.gdcConfig.gdc_resolution_info.input_crop.bottom);
    LOG3("config.gdcConfig.gdc_resolution_history.input_width %d, input_height %d",
         config.gdcConfig.gdc_resolution_history.input_width,
         config.gdcConfig.gdc_resolution_history.input_height);
    LOG3("config.gdcConfig.gdc_resolution_history.output_width %d, output_height %d",
         config.gdcConfig.gdc_resolution_history.output_width,
         config.gdcConfig.gdc_resolution_history.output_height);
    LOG3("config.gdcConfig.gdc_resolution_history.input_crop.left %d, top %d, right %d, bottom %d",
         config.gdcConfig.gdc_resolution_history.input_crop.left,
         config.gdcConfig.gdc_resolution_history.input_crop.top,
         config.gdcConfig.gdc_resolution_history.input_crop.right,
         config.gdcConfig.gdc_resolution_history.input_crop.bottom);
}

}  // namespace icamera
