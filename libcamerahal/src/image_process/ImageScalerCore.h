/*
 * Copyright (C) 2012-2020 Intel Corporation
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

namespace icamera {
/**
 * \class ImageScalerCore
 *
 */
class ImageScalerCore {
public:
    static void downScaleImage(void *src, void *dest,
                               int dest_w, int dest_h, int dest_stride,
                               int src_w, int src_h, int src_stride,
                               int format, int src_skip_lines_top = 0,
                               int src_skip_lines_bottom = 0);
    static int cropCompose(void *src, unsigned int srcW, unsigned int srcH, unsigned int srcStride, int srcFormat,
                           void *dst, unsigned int dstW, unsigned int dstH, unsigned int dstStride, int dstFormat,
                           unsigned int srcCropW, unsigned int srcCropH, unsigned int srcCropLeft, unsigned int srcCropTop,
                           unsigned int dstCropW, unsigned int dstCropH, unsigned int dstCropLeft, unsigned int dstCropTop);
    static int cropComposeZoom(void *src, void *dst,
                               unsigned int width, unsigned int height, unsigned int stride, int format,
                               unsigned int srcCropW, unsigned int srcCropH, unsigned int srcCropLeft, unsigned int srcCropTop);

protected:
    static void downScaleYUY2Image(unsigned char *dest, const unsigned char *src,
                                   const int dest_w, const int dest_h, const int dest_stride,
                                   const int src_w, const int src_h, const int src_stride);

    static void downScaleAndCropNv12Image(unsigned char *dest, const unsigned char *src,
                                          const int dest_w, const int dest_h, const int dest_stride,
                                          const int src_w, const int src_h, const int src_stride,
                                          const int src_skip_lines_top = 0,
                                          const int src_skip_lines_bottom = 0);

    static void trimNv12Image(unsigned char *dest, const unsigned char *src,
                              const int dest_w, const int dest_h, const int dest_stride,
                              const int src_w, const int src_h, const int src_stride,
                              const int src_skip_lines_top = 0,
                              const int src_skip_lines_bottom = 0);

    static void downScaleAndCropNv12ImageQvga(unsigned char *dest, const unsigned char *src,
                                              const int dest_stride, const int src_stride);

    static void downScaleAndCropNv12ImageQcif(unsigned char *dest, const unsigned char *src,
                                              const int dest_stride, const int src_stride);

    static void downScaleNv12ImageFrom800x600ToQvga(unsigned char *dest, const unsigned char *src,
                                                    const int dest_stride, const int src_stride);

private:
    static const int MFP = 16;            // Fractional bits for fixed point calculations

private:
    static void cropComposeCopy(void *src, void *dst, unsigned int size);
    static void cropComposeUpscaleNV12_bl(
        void *src, unsigned int srcH, unsigned int srcStride,
        unsigned int srcCropLeft, unsigned int srcCropTop,
        unsigned int srcCropW, unsigned int srcCropH,
        void *dst, unsigned int dstH, unsigned int dstStride,
        unsigned int dstCropLeft, unsigned int dstCropTop,
        unsigned int dstCropW, unsigned int dstCropH);

};
} // namespace icamera
