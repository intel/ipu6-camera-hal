/*
 * Copyright (C) 2016-2020 Intel Corporation
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

#define LOG_TAG "JpegMaker"

#include "JpegMaker.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

JpegMaker::JpegMaker() {
    LOG2("@%s", __func__);
    mExifMaker = std::unique_ptr<EXIFMaker>(new EXIFMaker());
}

JpegMaker::~JpegMaker() {
    LOG2("@%s", __func__);
}

status_t JpegMaker::setupExifWithMetaData(int bufWidth, int bufHeight, const Parameters& parameter,
                                          ExifMetaData* metaData) {
    LOG2("@%s", __func__);

    status_t status = OK;

    status = processJpegSettings(parameter, metaData);
    CheckError(status != OK, status, "@%s: Process settngs for JPEG failed!", __func__);

    mExifMaker->initialize(bufWidth, bufHeight);
    mExifMaker->pictureTaken(metaData);

    mExifMaker->enableFlash(metaData->flashFired, metaData->v3AeMode, metaData->flashMode);
    mExifMaker->updateSensorInfo(parameter);
    mExifMaker->saveMakernote(parameter);

    status = processExifSettings(parameter, metaData);
    if (status != OK) {
        LOGE("@%s: Process settngs for Exif! %d", __func__, status);
        return status;
    }

    mExifMaker->initializeLocation(metaData);
    mExifMaker->setSensorAeConfig(parameter);

    if (metaData->software) mExifMaker->setSoftware(metaData->software);

    return status;
}

status_t JpegMaker::getExif(const EncodePackage& thumbnailPackage, uint8_t* exifPtr,
                            uint32_t* exifSize) {
    if (thumbnailPackage.encodedDataSize > 0 && thumbnailPackage.quality > 0) {
        mExifMaker->setThumbnail(static_cast<unsigned char*>(thumbnailPackage.outputData),
                                 thumbnailPackage.encodedDataSize, thumbnailPackage.outputWidth,
                                 thumbnailPackage.outputHeight);
    }
    *exifSize = mExifMaker->makeExif(exifPtr);
    return *exifSize > 0 ? OK : UNKNOWN_ERROR;
}

status_t JpegMaker::processExifSettings(const Parameters& params, ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    status = processGpsSettings(params, metaData);
    status |= processColoreffectSettings(params, metaData);
    status |= processScalerCropSettings(params, metaData);

    return status;
}

/* copy exif data into output buffer */
void JpegMaker::writeExifData(EncodePackage* package) {
    CheckError(package == nullptr, VOID_VALUE, "@%s, package is nullptr", __func__);

    if (package->exifDataSize == 0) return;

    CheckError(!package->outputData, VOID_VALUE, "@%s, outputData is nullptr", __func__);
    CheckError(!package->exifData, VOID_VALUE, "@%s, exifData is nullptr", __func__);

    unsigned int jSOISize = sizeof(mJpegMarkerSOI);
    unsigned char* jpegOut = reinterpret_cast<unsigned char*>(package->outputData);
    MEMCPY_S(jpegOut, jSOISize, mJpegMarkerSOI, jSOISize);
    jpegOut += jSOISize;

    MEMCPY_S(jpegOut, package->exifDataSize, reinterpret_cast<unsigned char*>(package->exifData),
             package->exifDataSize);
}

/**
 * processJpegSettings
 *
 * Store JPEG settings to the exif metadata
 *
 * \param [IN] jpeg parameters
 * \ metaData [out] metadata of the request
 *
 */
status_t JpegMaker::processJpegSettings(const Parameters& params, ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    CheckError(!metaData, UNKNOWN_ERROR, "MetaData struct not intialized");

    // make jpeg with thumbnail or not
    camera_resolution_t thumbSize = {0};
    params.getJpegThumbnailSize(thumbSize);
    LOG2("%s request thumbname size %dx%d", __func__, thumbSize.width, thumbSize.height);

    uint8_t new_jpeg_quality = DEFAULT_JPEG_QUALITY;
    int ret = params.getJpegQuality(&new_jpeg_quality);
    if (ret != icamera::OK) {
        LOGW("cannot find jpeg quality, use default");
    }
    metaData->mJpegSetting.jpegQuality = new_jpeg_quality;

    uint8_t new_jpeg_thumb_quality = DEFAULT_JPEG_QUALITY;
    params.getJpegThumbnailQuality(&new_jpeg_thumb_quality);
    metaData->mJpegSetting.jpegThumbnailQuality = new_jpeg_thumb_quality;
    metaData->mJpegSetting.thumbWidth = thumbSize.width;
    metaData->mJpegSetting.thumbHeight = thumbSize.height;

    int new_rotation = 0;
    params.getJpegRotation(new_rotation);
    metaData->mJpegSetting.orientation = new_rotation;

    LOG1("jpegQuality=%d,thumbQuality=%d,thumbW=%d,thumbH=%d,orientation=%d",
         metaData->mJpegSetting.jpegQuality, metaData->mJpegSetting.jpegThumbnailQuality,
         metaData->mJpegSetting.thumbWidth, metaData->mJpegSetting.thumbHeight,
         metaData->mJpegSetting.orientation);

    params.getAeMode(metaData->aeMode);
    params.getAwbMode(metaData->awbMode);

    metaData->currentFocusDistance = 0.0;
    float focusDistance = 0.0;
    params.getFocusDistance(focusDistance);
    if (focusDistance != 0) {
        metaData->currentFocusDistance = ceil(1000.0 / focusDistance);
    }
    LOG2("aeMode=%d, awbMode=%d, currentFocusDistance=%f", metaData->aeMode, metaData->awbMode,
         metaData->currentFocusDistance);

    return status;
}

/**
 * This function will get GPS metadata from request setting
 *
 * \param[in] settings The Anroid metadata to process GPS settings from
 * \param[out] metadata The EXIF data where the GPS setting are written to
 */
status_t JpegMaker::processGpsSettings(const Parameters& param, ExifMetaData* metadata) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    // gps latitude
    double new_gps_latitude = 0.0;
    param.getJpegGpsLatitude(new_gps_latitude);
    metadata->mGpsSetting.latitude = new_gps_latitude;

    double new_gps_longitude = 0.0;
    param.getJpegGpsLongitude(new_gps_longitude);
    metadata->mGpsSetting.longitude = new_gps_longitude;

    double new_gps_altitude = 0.0;
    param.getJpegGpsAltitude(new_gps_altitude);
    metadata->mGpsSetting.altitude = new_gps_altitude;

    // gps timestamp
    int64_t new_gps_timestamp = 0;
    param.getJpegGpsTimeStamp(new_gps_timestamp);
    metadata->mGpsSetting.gpsTimeStamp = new_gps_timestamp;

    // gps processing method
    char new_gps_processing_method[MAX_NUM_GPS_PROCESSING_METHOD + 1];
    CLEAR(new_gps_processing_method);
    param.getJpegGpsProcessingMethod(MAX_NUM_GPS_PROCESSING_METHOD, new_gps_processing_method);
    if (strlen(new_gps_processing_method) != 0) {
        snprintf(metadata->mGpsSetting.gpsProcessingMethod,
                 sizeof(metadata->mGpsSetting.gpsProcessingMethod), "%s",
                 new_gps_processing_method);
    }

    return status;
}

status_t JpegMaker::processColoreffectSettings(const Parameters& param, ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    camera_effect_mode_t new_image_effect = CAM_EFFECT_NONE;
    param.getImageEffect(new_image_effect);
    metaData->effectMode = new_image_effect;
    LOG2("effect mode=%d", metaData->effectMode);

    return status;
}

status_t JpegMaker::processScalerCropSettings(const Parameters& param, ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    return status;
}

}  // namespace icamera
