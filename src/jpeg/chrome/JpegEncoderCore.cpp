/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "JpegEncoderCore"

#include "JpegEncoderCore.h"

#include <cros-camera/camera_buffer_manager.h>
#include <linux/videodev2.h>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

JpegEncoderCore::JpegEncoderCore() : mJpegCompressor(cros::JpegCompressor::GetInstance()) {
    LOG1("@%s", __func__);
}

JpegEncoderCore::~JpegEncoderCore() {
    LOG1("@%s", __func__);
}

std::unique_ptr<IJpegEncoder> IJpegEncoder::createJpegEncoder() {
    return std::unique_ptr<JpegEncoderCore>(new JpegEncoderCore());
}

/**
 * doJpegEncode
 *
 * Do HW / SW JPEG encoding for the main buffer
 * Do SW JPEG encoding for the thumbnail buffer
 *
 * \param pa [IN/OUT] Information that should be encoded
 */
bool JpegEncoderCore::doJpegEncode(EncodePackage* pa) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    CheckError(pa == nullptr, false, "@%s, pa is nullptr", __func__);

    CheckError(pa->inputWidth != pa->outputWidth || pa->inputHeight != pa->outputHeight, false,
               "@%s, input size != output size", __func__);
    CheckError(pa->inputWidth <= 0 || pa->outputWidth <= 0, false,
               "@%s, inputWidth:%d, outputWidth:%d", __func__, pa->inputWidth, pa->outputWidth);
    CheckError(pa->inputHeight <= 0 || pa->outputHeight <= 0, false,
               "@%s, inputHeight:%d, outputHeight:%d", __func__, pa->inputHeight, pa->outputHeight);

    LOGI("@%s: Using Google encoding...", __func__);

    nsecs_t startTime = CameraUtils::systemTime();

    bool ret = false;
    if (pa->inputBufferHandle && pa->outputBufferHandle) {
        LOGI("%s, use buffer handle to do jpeg encode input: %p, output: %p", __func__,
             pa->inputBufferHandle, pa->outputBufferHandle);
        ret = mJpegCompressor->CompressImageFromHandle(
            *reinterpret_cast<buffer_handle_t*>(pa->inputBufferHandle),
            *reinterpret_cast<buffer_handle_t*>(pa->outputBufferHandle), pa->inputWidth,
            pa->inputHeight, pa->quality, pa->exifData, pa->exifDataSize, &pa->encodedDataSize);
    } else if (pa->inputData && pa->outputData) {
        LOGI("%s, use buffer address to do jpeg encode input: %p, output: %p", __func__,
             pa->inputData, pa->outputData);
        ret = mJpegCompressor->CompressImageFromMemory(
            pa->inputData, V4L2_PIX_FMT_NV12, pa->outputData, pa->outputSize, pa->inputWidth,
            pa->inputHeight, pa->quality, pa->exifData, pa->exifDataSize, &pa->encodedDataSize);
    } else {
        LOGE("%s: Google encode input type and output type does not match", __func__);
    }

    LOGI("@%s: Google encoding ret:%d, %dx%d need %u ms, jpeg size %u, quality %d)", __func__, ret,
         pa->outputWidth, pa->outputHeight, (CameraUtils::systemTime() - startTime) / 1000000,
         pa->encodedDataSize, pa->quality);
    return ret && pa->encodedDataSize > 0;
}
}  // namespace icamera
