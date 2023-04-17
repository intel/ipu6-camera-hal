/*
 * Copyright (C) 2019-2023 Intel Corporation.
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
#define LOG_TAG PostProcessorBase

#include "PostProcessorBase.h"

#include <hardware/camera3.h>

#include <vector>

#include "HALv3Utils.h"
#include "iutils/CameraLog.h"
#include "stdlib.h"

using std::shared_ptr;

namespace icamera {

PostProcessorBase::PostProcessorBase(std::string processName)
        : mName(processName),
          mProcessor(nullptr) {}

ScaleProcess::ScaleProcess() : PostProcessorBase("Scaler") {
    LOG1("@%s create scaler processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t ScaleProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                        shared_ptr<camera3::Camera3Buffer>& outBuf) {
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->scaleFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

RotateProcess::RotateProcess(int angle) : PostProcessorBase("Rotate"), mAngle(angle) {
    LOG1("@%s create rotate processor, degree: %d", __func__, mAngle);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t RotateProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                         shared_ptr<camera3::Camera3Buffer>& outBuf) {
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);
    std::vector<uint8_t> rotateBuf;

    int ret = mProcessor->rotateFrame(inBuf, outBuf, mAngle, rotateBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

CropProcess::CropProcess() : PostProcessorBase("Crop") {
    LOG1("@%s create crop processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t CropProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                       shared_ptr<camera3::Camera3Buffer>& outBuf) {
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->cropFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

ConvertProcess::ConvertProcess() : PostProcessorBase("Convert") {
    LOG1("@%s create convert processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t ConvertProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                          shared_ptr<camera3::Camera3Buffer>& outBuf) {
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->convertFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

JpegProcess::JpegProcess(int cameraId)
        : PostProcessorBase("JpegEncode"),
          mCameraId(cameraId),
          mCropBuffer(nullptr),
          mScaleBuffer(nullptr),
          mThumbOutput(nullptr),
          mExifData(nullptr) {
    LOG1("@%s create jpeg encode processor", __func__);

    mProcessor = IImageProcessor::createImageProcessor();
    mJpegEncoder = IJpegEncoder::createJpegEncoder();
    mJpegMaker = std::unique_ptr<JpegMaker>(new JpegMaker());
}

void JpegProcess::attachJpegBlob(const EncodePackage& package) {
    LOG2("@%s, encoded data size: %d, exif data size: %d", __func__, package.encodedDataSize,
         package.exifDataSize);
    uint8_t* resultPtr = static_cast<uint8_t*>(package.outputData) + package.outputSize -
                         sizeof(struct camera3_jpeg_blob);

    // save jpeg size at the end of file
    auto* blob = reinterpret_cast<struct camera3_jpeg_blob*>(resultPtr);
    blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
    blob->jpeg_size = package.encodedDataSize + package.exifDataSize;
}

std::shared_ptr<camera3::Camera3Buffer> JpegProcess::cropAndDownscaleThumbnail(
    int thumbWidth, int thumbHeight, const shared_ptr<camera3::Camera3Buffer>& inBuf) {
    LOG2("@%s, input size: %dx%d, thumbnail info: %dx%d", __func__, inBuf->width(), inBuf->height(),
         thumbWidth, thumbHeight);

    if (thumbWidth <= 0 || thumbHeight <= 0) {
        LOGW("@%s, skip, thumbWidth:%d, thumbHeight:%d", __func__, thumbWidth, thumbHeight);
        return nullptr;
    }

    int ret = OK;
    shared_ptr<camera3::Camera3Buffer> tempBuffer = inBuf;

    int format = camera3::HalV3Utils::V4l2FormatToHALFormat(inBuf->v4l2Fmt());
    int usage = inBuf->usage();
    LOG2("%s, inputbuffer format:%s(%d), usage:%d", __func__,
         CameraUtils::format2string(inBuf->v4l2Fmt()).c_str(), format, usage);

    // Do crop first if needed
    if (IImageProcessor::isProcessingTypeSupported(POST_PROCESS_CROP) &&
        inBuf->width() * thumbHeight != inBuf->height() * thumbWidth) {
        int width = 0, height = 0;
        if (inBuf->width() * thumbHeight < inBuf->height() * thumbWidth) {
            width = inBuf->width();
            height = inBuf->width() * thumbHeight / thumbWidth;
        } else {
            width = inBuf->height() * thumbWidth / thumbHeight;
            height = inBuf->height();
        }

        if (mCropBuffer && (mCropBuffer->width() != width || mCropBuffer->height() != height))
            mCropBuffer.reset();
        if (!mCropBuffer) {
            mCropBuffer =
                camera3::MemoryUtils::allocateHandleBuffer(width, height, format, usage, mCameraId);
            if (!mCropBuffer || mCropBuffer->lock() != icamera::OK) {
                mCropBuffer = nullptr;
                LOGE("%s, Failed to allocate the internal crop buffer", __func__);
                return nullptr;
            }
        }

        LOG2("@%s, Crop the main buffer from %dx%d to %dx%d", __func__, inBuf->width(),
             inBuf->height(), width, height);
        ret = mProcessor->cropFrame(inBuf, mCropBuffer);
        CheckAndLogError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mCropBuffer;
    }

    if (IImageProcessor::isProcessingTypeSupported(POST_PROCESS_SCALING)) {
        if (mScaleBuffer &&
            (mScaleBuffer->width() != thumbWidth || mScaleBuffer->height() != thumbHeight))
            mScaleBuffer.reset();
        if (!mScaleBuffer) {
            mScaleBuffer = camera3::MemoryUtils::allocateHandleBuffer(thumbWidth, thumbHeight,
                                                                      format, usage, mCameraId);
            if (!mScaleBuffer || mScaleBuffer->lock() != icamera::OK) {
                mScaleBuffer = nullptr;
                LOGE("%s, Failed to allocate the internal scale buffer", __func__);
                return nullptr;
            }
        }

        LOG2("@%s, Scale the buffer from %dx%d to %dx%d", __func__, inBuf->width(), inBuf->height(),
             thumbWidth, thumbHeight);
        ret = mProcessor->scaleFrame(tempBuffer, mScaleBuffer);
        CheckAndLogError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mScaleBuffer;
    }

    if (tempBuffer->width() != thumbWidth || tempBuffer->height() != thumbHeight) {
        LOGE("%s, Failed to crop & downscale the main buffer to thumbnail buffer", __func__);
        return nullptr;
    }

    return tempBuffer;
}

void JpegProcess::fillEncodeInfo(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                 const shared_ptr<camera3::Camera3Buffer>& outBuf,
                                 EncodePackage& package) {
    package.inputWidth = inBuf->width();
    package.inputHeight = inBuf->height();
    package.inputStride = inBuf->stride();
    package.inputFormat = inBuf->v4l2Fmt();
    package.inputSize = inBuf->size();

    if (inBuf->getBufferType() == camera3::BUF_TYPE_HANDLE &&
        outBuf->getBufferType() == camera3::BUF_TYPE_HANDLE) {
        package.inputBufferHandle = static_cast<void*>(inBuf->getBufferHandle());
        package.outputBufferHandle = static_cast<void*>(outBuf->getBufferHandle());
    }
    package.inputData = inBuf->data();
    package.outputData = outBuf->data();

    package.outputWidth = outBuf->width();
    package.outputHeight = outBuf->height();
    package.outputSize = outBuf->size();
}

status_t JpegProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer>& inBuf,
                                       const icamera::Parameters& parameter,
                                       shared_ptr<camera3::Camera3Buffer>& outBuf) {
    LOG1("@%s processor name: %s", __func__, mName.c_str());

    bool isEncoded = false;

    icamera::ExifMetaData exifMetadata;
    status_t status = mJpegMaker->setupExifWithMetaData(inBuf->width(), inBuf->height(), parameter,
                                                        &exifMetadata);
    CheckAndLogError(status != OK, UNKNOWN_ERROR, "@%s, Setup exif metadata failed.", __func__);
    LOG2("@%s: setting exif metadata done!", __func__);

    std::shared_ptr<camera3::Camera3Buffer> thumbInput = cropAndDownscaleThumbnail(
        exifMetadata.mJpegSetting.thumbWidth, exifMetadata.mJpegSetting.thumbHeight, inBuf);

    EncodePackage thumbnailPackage;
    if (thumbInput) {
        if (mThumbOutput == nullptr ||
            mThumbOutput->width() != exifMetadata.mJpegSetting.thumbWidth ||
            mThumbOutput->height() != exifMetadata.mJpegSetting.thumbHeight ||
            mThumbOutput->v4l2Fmt() != outBuf->v4l2Fmt()) {
            mThumbOutput = camera3::MemoryUtils::allocateHandleBuffer(
                exifMetadata.mJpegSetting.thumbWidth, exifMetadata.mJpegSetting.thumbHeight,
                outBuf->format(), outBuf->usage(), mCameraId);
            if (!mThumbOutput || mThumbOutput->lock() != icamera::OK) {
                mThumbOutput = nullptr;
                LOGE("%s, Failed to allocate the mThumbOutput", __func__);
                return NO_MEMORY;
            }
        }

        // encode thumbnail image
        fillEncodeInfo(thumbInput, mThumbOutput, thumbnailPackage);
        thumbnailPackage.quality = exifMetadata.mJpegSetting.jpegThumbnailQuality;
        // the exifDataSize should be 0 for encoding thumbnail
        thumbnailPackage.exifData = nullptr;
        thumbnailPackage.exifDataSize = 0;

        do {
            isEncoded = mJpegEncoder->doJpegEncode(&thumbnailPackage);
            thumbnailPackage.quality -= 5;
        } while (thumbnailPackage.encodedDataSize > THUMBNAIL_SIZE_LIMITATION &&
                 thumbnailPackage.quality > 0);

        if (!isEncoded || thumbnailPackage.quality < 0) {
            LOGW("Failed to generate thumbnail, isEncoded: %d, encoded thumbnail size: %d, "
                 "quality:%d",
                 isEncoded, thumbnailPackage.encodedDataSize, thumbnailPackage.quality);
        }
    }

    // save exif data
    uint32_t exifBufSize = ENABLE_APP2_MARKER ? EXIF_SIZE_LIMITATION * 2 : EXIF_SIZE_LIMITATION;
    if (mExifData == nullptr) {
        mExifData = std::unique_ptr<unsigned char[]>(new unsigned char[exifBufSize]);
    }
    uint8_t* finalExifDataPtr = static_cast<uint8_t*>(mExifData.get());
    uint32_t finalExifDataSize = 0;
    status = mJpegMaker->getExif(thumbnailPackage, finalExifDataPtr, &finalExifDataSize);
    CheckAndLogError(status != OK, status, "@%s, Failed to get Exif", __func__);
    LOG2("%s, exifBufSize %d, finalExifDataSize %d", __func__, exifBufSize, finalExifDataSize);

    // encode main image
    EncodePackage finalEncodePackage;
    fillEncodeInfo(inBuf, outBuf, finalEncodePackage);
    finalEncodePackage.quality = exifMetadata.mJpegSetting.jpegQuality;
#ifdef SW_JPEG_ENCODE
    finalEncodePackage.exifData = finalExifDataPtr;
    finalEncodePackage.exifDataSize = finalExifDataSize;
    isEncoded = mJpegEncoder->doJpegEncode(&finalEncodePackage);
    CheckAndLogError(!isEncoded, UNKNOWN_ERROR, "@%s, Failed to encode main image", __func__);
    mJpegMaker->writeExifData(&finalEncodePackage);
#else
    finalEncodePackage.exifData = nullptr;
    finalEncodePackage.exifDataSize = 0;
    isEncoded = mJpegEncoder->doJpegEncode(&finalEncodePackage);
    CheckAndLogError(!isEncoded, UNKNOWN_ERROR, "@%s, Failed to encode main image", __func__);
    finalEncodePackage.outputData = outBuf->data();
    finalEncodePackage.exifData = finalExifDataPtr;
    finalEncodePackage.exifDataSize = finalExifDataSize;
    memmove(reinterpret_cast<uint8_t*>(finalEncodePackage.outputData) + finalExifDataSize,
            finalEncodePackage.outputData, finalEncodePackage.encodedDataSize);
    mJpegMaker->writeExifData(&finalEncodePackage);
#endif
    attachJpegBlob(finalEncodePackage);

    return OK;
}
}  // namespace icamera
