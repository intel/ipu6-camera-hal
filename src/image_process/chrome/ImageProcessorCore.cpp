/*
 * Copyright (C) 2019 Intel Corporation
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

#define LOG_TAG "ImageProcessorCore"

#include <libyuv.h>
#include "iutils/CameraLog.h"
#include "ImageProcessorCore.h"
#include "ImageConverter.h"

namespace icamera {

ImageProcessorCore::ImageProcessorCore()
{
    LOG2("enter %s", __func__);
    mRotationMode = {
        {0, libyuv::RotationMode::kRotate0},
        {90, libyuv::RotationMode::kRotate90},
        {180, libyuv::RotationMode::kRotate180},
        {270, libyuv::RotationMode::kRotate270}
    };
}

ImageProcessorCore::~ImageProcessorCore()
{
    LOG2("enter %s", __func__);
}

std::unique_ptr<IImageProcessor> IImageProcessor::createImageProcessor()
{
    return std::unique_ptr<ImageProcessorCore>(new ImageProcessorCore());
}

//If support this kind of post process type in current OS
bool IImageProcessor::isProcessingTypeSupported(PostProcessType type)
{
    int supportedType = POST_PROCESS_ROTATE |
                        POST_PROCESS_SCALING |
                        POST_PROCESS_CROP |
                        POST_PROCESS_CONVERT |
                        POST_PROCESS_JPEG_ENCODING;

    return supportedType & type;
}

status_t ImageProcessorCore::cropFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                       std::shared_ptr<camera3::Camera3Buffer> &output)
{
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x",
         __func__, input->width(), input->height(), input->v4l2Fmt(),
         output->width(), output->height(), output->v4l2Fmt());

    int srcW = input->stride();
    int srcH = input->height();
    int dstW = output->stride();
    int dstH = output->height();

    std::unique_ptr<uint8_t[]> srcI420Buf;
    unsigned int srcI420BufSize = srcW * srcH * 3 / 2;
    srcI420Buf.reset(new uint8_t[srcI420BufSize]);

    const uint8_t* srcBufY = static_cast<uint8_t*>(input->data());
    const uint8_t* srcBufUV = srcBufY + srcW * srcH;
    uint8_t* srcI420BufY = static_cast<uint8_t*>(srcI420Buf.get());
    uint8_t* srcI420BufU = srcI420BufY + srcW * srcH;
    uint8_t* srcI420BufV = srcI420BufU + srcW * srcH / 4;
    int ret = libyuv::NV12ToI420(srcBufY, srcW,
                                 srcBufUV, srcW,
                                 srcI420BufY, srcW,
                                 srcI420BufU, srcW / 2,
                                 srcI420BufV, srcW / 2,
                                 srcW, srcH);
    CheckError((ret != 0), UNKNOWN_ERROR, "@%s, NV12ToI420 fails", __func__);

    std::unique_ptr<uint8_t[]> dstI420BufUV;
    unsigned int dstI420BufUVSize = dstW * dstH / 2;
    dstI420BufUV.reset(new uint8_t[dstI420BufUVSize]);

    uint8_t* dstI420BufU = static_cast<uint8_t*>(dstI420BufUV.get());
    uint8_t* dstI420BufV = dstI420BufU + dstW * dstH / 4;
    ret = libyuv::ConvertToI420(static_cast<uint8_t*>(srcI420Buf.get()), srcI420BufSize,
                                static_cast<uint8_t*>(output->data()), dstW,
                                dstI420BufU, (dstW + 1) / 2,
                                dstI420BufV, (dstW + 1) / 2,
                                (srcW - dstW) / 2, (srcH - dstH) / 2,
                                srcW, srcH, dstW, dstH,
                                libyuv::RotationMode::kRotate0, libyuv::FourCC::FOURCC_I420);
    CheckError(ret != 0, UNKNOWN_ERROR, "@%s, ConvertToI420 fails", __func__);

    uint8_t* dstBufUV = static_cast<uint8_t*>(output->data()) + dstW * dstH;
    libyuv::MergeUVPlane(dstI420BufU, (dstW + 1) / 2,
                         dstI420BufV, (dstW + 1) / 2,
                         dstBufUV, dstW,
                         (dstW + 1) / 2, (dstH + 1) / 2);

    return OK;
}

status_t ImageProcessorCore::scaleFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                        std::shared_ptr<camera3::Camera3Buffer> &output)
{
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x",
         __func__, input->width(), input->height(), input->v4l2Fmt(),
         output->width(), output->height(), output->v4l2Fmt());

    // Y plane
    libyuv::ScalePlane(static_cast<uint8_t*>(input->data()),
                       input->stride(),
                       input->width(),
                       input->height(),
                       static_cast<uint8_t*>(output->data()),
                       output->stride(),
                       output->width(),
                       output->height(),
                       libyuv::kFilterNone);

    // UV plane
    int inUVOffsetByte = input->stride() * input->height();
    int outUVOffsetByte = output->stride() * output->height();
    libyuv::ScalePlane_16(static_cast<uint16_t*>(input->data()) + inUVOffsetByte / sizeof(uint16_t),
                          input->stride() / 2,
                          input->width() / 2,
                          input->height() / 2,
                          static_cast<uint16_t*>(output->data()) + outUVOffsetByte / sizeof(uint16_t),
                          output->stride() / 2,
                          output->width() / 2,
                          output->height() / 2,
                          libyuv::kFilterNone);

    return OK;
}

status_t ImageProcessorCore::rotateFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                         std::shared_ptr<camera3::Camera3Buffer> &output,
                                         int angle, std::vector<uint8_t> &rotateBuf)
{
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x",
         __func__, input->width(), input->height(), input->v4l2Fmt(),
         output->width(), output->height(), output->v4l2Fmt());

    CheckError(output->width() != input->height() || output->height() != input->width(),
          BAD_VALUE, "output resolution mis-match [%d x %d] -> [%d x %d]",
          input->width(), input->height(), output->width(), output->height());
    CheckError((angle != 90 && angle != 270), BAD_VALUE, "angle value:%d is wrong", angle);

    const uint8_t* inBuffer = static_cast<uint8_t*>(input->data());
    uint8_t* outBuffer = static_cast<uint8_t*>(output->data());
    int outW = output->width();
    int outH = output->height();
    int outStride = output->stride();
    int inW = input->width();
    int inH = input->height();
    int inStride = input->stride();
    if (rotateBuf.size() < input->size()) {
        rotateBuf.resize(input->size());
    }

    // TODO: find a way to rotate NV12 directly.
    uint8_t* I420Buffer = rotateBuf.data();

    if (mRotationMode[angle] == libyuv::RotationMode::kRotate0) {
        libyuv::CopyPlane(inBuffer, inStride, outBuffer, outStride, inW, inH);
        libyuv::CopyPlane(inBuffer + inH * inStride, inStride,
                          outBuffer + outH * outStride, outStride,
                          inW, inH / 2);
    } else {
        int ret = libyuv::NV12ToI420Rotate(
                      inBuffer, inStride, inBuffer + inH * inStride, inStride,
                      I420Buffer, outW,
                      I420Buffer + outW * outH, outW / 2,
                      I420Buffer + outW * outH * 5 / 4, outW / 2,
                      inW, inH, mRotationMode[angle]);
        CheckError((ret < 0), UNKNOWN_ERROR, "@%s, rotate fail [%d]!", __func__, ret);

        ret = libyuv::I420ToNV12(I420Buffer, outW,
                                 I420Buffer + outW * outH, outW / 2,
                                 I420Buffer + outW * outH * 5 / 4, outW / 2,
                                 outBuffer, outStride,
                                 outBuffer +  outStride * outH, outStride,
                                 outW, outH);
        CheckError((ret < 0), UNKNOWN_ERROR, "@%s, convert fail [%d]!", __func__, ret);
    }

    return OK;
}

status_t ImageProcessorCore::convertFrame(const std::shared_ptr<camera3::Camera3Buffer> &input,
                                          std::shared_ptr<camera3::Camera3Buffer> &output)
{
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x",
         __func__, input->width(), input->height(), input->v4l2Fmt(),
         output->width(), output->height(), output->v4l2Fmt());

    switch (output->v4l2Fmt()) {
        case V4L2_PIX_FMT_YVU420:
            // XXX -> YV12
            ImageConverter::convertBuftoYV12(input->v4l2Fmt(), input->width(),
                                             input->height(), input->stride(),
                                             output->stride(), input->data(), output->data());
            break;
        case V4L2_PIX_FMT_NV21:
            // XXX -> NV21
            ImageConverter::convertBuftoNV21(input->v4l2Fmt(), input->width(),
                                             input->height(), input->stride(),
                                             output->stride(), input->data(), output->data());
            break;
        case V4L2_PIX_FMT_YUYV:
            // XXX -> YUYV
            ImageConverter::convertBuftoYUYV(input->v4l2Fmt(), input->width(),
                                             input->height(), input->stride(),
                                             output->stride(), input->data(), output->data());
            break;
        default:
            LOGE("%s: not implement for color conversion 0x%x -> 0x%x!",
                 __func__, input->v4l2Fmt(), output->v4l2Fmt());
            return UNKNOWN_ERROR;
    }

    return OK;
}
} /* namespace icamera */
