/*
 * Copyright (C) 2019-2020 Intel Corporation.
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
#define LOG_TAG "PostProcessorBase"

#include <vector>
#include <hardware/camera3.h>
#include "stdlib.h"
#include "PostProcessorBase.h"
#include "iutils/CameraLog.h"

using std::shared_ptr;

namespace icamera {

PostProcessorBase::PostProcessorBase(std::string processName) :
    mName(processName),
    mProcessor(nullptr)
{
    LOG1("@%s PostProcessorBase created", __func__);
}

PostProcessorBase::~PostProcessorBase()
{
    LOG1("@%s PostProcessorBase destory", __func__);
}

ScaleProcess::ScaleProcess() :
    PostProcessorBase("Scaler")
{
    LOG1("@%s create scaler processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t ScaleProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                        shared_ptr<camera3::Camera3Buffer> &outBuf)
{
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->scaleFrame(inBuf, outBuf);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s", mName.c_str());

    return OK;
}

RotateProcess::RotateProcess(int angle) :
    PostProcessorBase("Rotate"),
    mAngle(angle)
{
    LOG1("@%s create rotate processor, degree: %d", __func__, mAngle);
    mProcessor = IImageProcessor::createImageProcessor();
};

status_t RotateProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                         shared_ptr<camera3::Camera3Buffer> &outBuf)
{
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);
    std::vector<uint8_t> rotateBuf;

    int ret = mProcessor->rotateFrame(inBuf, outBuf, mAngle, rotateBuf);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s", mName.c_str());

    return OK;
}

CropProcess::CropProcess() :
    PostProcessorBase("Crop")
{
    LOG1("@%s create crop processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
};

status_t CropProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                       shared_ptr<camera3::Camera3Buffer> &outBuf)
{
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->cropFrame(inBuf, outBuf);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s", mName.c_str());

    return OK;
}

ConvertProcess::ConvertProcess() :
    PostProcessorBase("Convert")
{
    LOG1("@%s create convert processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
};

status_t ConvertProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                          shared_ptr<camera3::Camera3Buffer> &outBuf)
{
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->convertFrame(inBuf, outBuf);
    CheckError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s", mName.c_str());

    return OK;
}

JpegProcess::JpegProcess(int cameraId) :
    PostProcessorBase("JpegEncode"),
    mCameraId(cameraId),
    mCropBuffer(nullptr),
    mScaleBuffer(nullptr),
    mThumbOutput(nullptr),
    mExifData(nullptr)
{
    LOG1("@%s create jpeg encode processor", __func__);

    mProcessor = IImageProcessor::createImageProcessor();
    mJpegEncoder = IJpegEncoder::createJpegEncoder();
    mJpegMaker = std::unique_ptr<JpegMaker>(new JpegMaker());
};

JpegProcess::~JpegProcess()
{
}

void JpegProcess::attachJpegBlob(const EncodePackage &package)
{
    LOG2("@%s, encoded data size: %d, exif data size: %d",
         __func__, package.encodedDataSize, package.exifDataSize);
    uint8_t *resultPtr = static_cast<uint8_t*>(package.outputData) +
                         package.outputSize - sizeof(struct camera3_jpeg_blob);

    // save jpeg size at the end of file
    auto *blob = reinterpret_cast<struct camera3_jpeg_blob*>(resultPtr);
    blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
    blob->jpeg_size = package.encodedDataSize + package.exifDataSize;
}

std::shared_ptr<camera3::Camera3Buffer> JpegProcess::cropAndDownscaleThumbnail(int thumbWidth, int thumbHeight,
                                                                               const shared_ptr<camera3::Camera3Buffer> &inBuf)
{
    LOG1("@%s, input size: %dx%d, thumbnail info: %dx%d",
         __func__, inBuf->width(), inBuf->height(), thumbWidth, thumbHeight);

    if (thumbWidth <= 0 || thumbHeight <= 0) {
        LOGW("@%s, skip, thumbWidth:%d, thumbHeight:%d", __func__, thumbWidth, thumbHeight);
        return nullptr;
    }

    int ret = OK;
    shared_ptr<camera3::Camera3Buffer> tempBuffer = inBuf;

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
            int size = CameraUtils::getFrameSize(inBuf->v4l2Fmt(), width, height);
            mCropBuffer = camera3::MemoryUtils::allocateHeapBuffer(width, height,
                                                                   width, inBuf->v4l2Fmt(),
                                                                   mCameraId, size);
            CheckError(!mCropBuffer, nullptr, "%s, Failed to allocate the internal crop buffer", __func__);
        }

        LOG2("@%s, Crop the main buffer from %dx%d to %dx%d",
             __func__, inBuf->width(), inBuf->height(), width, height);
        ret = mProcessor->cropFrame(inBuf, mCropBuffer);
        CheckError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mCropBuffer;
    }

    if (IImageProcessor::isProcessingTypeSupported(POST_PROCESS_SCALING)) {
        if (mScaleBuffer && (mScaleBuffer->width() != thumbWidth
            || mScaleBuffer->height() != thumbHeight))
            mScaleBuffer.reset();
        if (!mScaleBuffer) {
            int size = CameraUtils::getFrameSize(inBuf->v4l2Fmt(), thumbWidth, thumbHeight);
            mScaleBuffer = camera3::MemoryUtils::allocateHeapBuffer(thumbWidth, thumbHeight,
                                                                    thumbWidth, inBuf->v4l2Fmt(),
                                                                    mCameraId, size);
            CheckError(!mScaleBuffer, nullptr, "%s, Failed to allocate the internal scale buffer", __func__);
        }

        LOG2("@%s, Scale the buffer from %dx%d to %dx%d",
              __func__, inBuf->width(), inBuf->height(), thumbWidth, thumbHeight);
        ret = mProcessor->scaleFrame(tempBuffer, mScaleBuffer);
        CheckError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mScaleBuffer;
    }

    if (tempBuffer->width() != thumbWidth || tempBuffer->height() != thumbHeight) {
        LOGE("%s, Failed to crop & downscale the main buffer to thumbnail buffer", __func__);
        return nullptr;
    }

    return tempBuffer;
}

void JpegProcess::fillEncodeInfo(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                 const shared_ptr<camera3::Camera3Buffer> &outBuf,
                                 EncodePackage &package)
{
    package.inputWidth = inBuf->width();
    package.inputHeight = inBuf->height();
    package.inputStride = inBuf->stride();
    package.inputFormat = inBuf->v4l2Fmt();
    package.inputSize = inBuf->size();

    if (inBuf->getBufferType() == camera3::BUF_TYPE_HANDLE &&
        outBuf->getBufferType() == camera3::BUF_TYPE_HANDLE) {
        package.inputBufferHandle = static_cast<void*>(inBuf->getBufferHandle());
        package.outputBufferHandle = static_cast<void*>(outBuf->getBufferHandle());
    } else {
        package.inputData = inBuf->data();
        package.outputData = outBuf->data();
    }

    package.outputWidth = outBuf->width();
    package.outputHeight = outBuf->height();
    package.outputSize = outBuf->size();
}

status_t JpegProcess::doPostProcessing(const shared_ptr<camera3::Camera3Buffer> &inBuf,
                                       const icamera::Parameters &parameter,
                                       shared_ptr<camera3::Camera3Buffer> &outBuf)
{
    LOG1("@%s", __func__);
    bool isEncoded = false;

    icamera::ExifMetaData exifMetadata;
    status_t status = mJpegMaker->setupExifWithMetaData(inBuf->width(), inBuf->height(), parameter, &exifMetadata);
    CheckError(status != OK, UNKNOWN_ERROR, "@%s, Setup exif metadata failed.", __func__);
    LOG2("@%s: setting exif metadata done!", __func__);

    std::shared_ptr<camera3::Camera3Buffer> thumbInput =
        cropAndDownscaleThumbnail(exifMetadata.mJpegSetting.thumbWidth,
                                  exifMetadata.mJpegSetting.thumbHeight,
                                  inBuf);

    EncodePackage thumbnailPackage;
    if (thumbInput) {
        if (mThumbOutput == nullptr ||
            mThumbOutput->width() != exifMetadata.mJpegSetting.thumbWidth ||
            mThumbOutput->height() != exifMetadata.mJpegSetting.thumbHeight ||
            mThumbOutput->v4l2Fmt() != outBuf->v4l2Fmt()) {
            mThumbOutput = camera3::MemoryUtils::allocateHeapBuffer(
                               exifMetadata.mJpegSetting.thumbWidth, exifMetadata.mJpegSetting.thumbHeight,
                               exifMetadata.mJpegSetting.thumbWidth, outBuf->v4l2Fmt(),
                               mCameraId,
                               exifMetadata.mJpegSetting.thumbWidth * exifMetadata.mJpegSetting.thumbHeight * 3 / 2);
            CheckError(!mThumbOutput, NO_MEMORY, "%s, Failed to allocate the mThumbOutput", __func__);
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
            LOGW("Failed to generate thumbnail, isEncoded: %d, encoded thumbnail size: %d, quality:%d",
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
    CheckError(status != OK, status, "@%s, Failed to get Exif", __func__);
    LOG2("%s, exifBufSize %d, finalExifDataSize %d", __func__, exifBufSize, finalExifDataSize);

    // encode main image
    EncodePackage finalEncodePackage;
    fillEncodeInfo(inBuf, outBuf, finalEncodePackage);
    finalEncodePackage.quality = exifMetadata.mJpegSetting.jpegQuality;
    finalEncodePackage.exifData = finalExifDataPtr;
    finalEncodePackage.exifDataSize = finalExifDataSize;
    isEncoded = mJpegEncoder->doJpegEncode(&finalEncodePackage);
    CheckError(!isEncoded, UNKNOWN_ERROR, "@%s, Failed to encode main image", __func__);
    mJpegMaker->writeExifData(&finalEncodePackage);
    attachJpegBlob(finalEncodePackage);

    return OK;
}
} // namespace icamera
