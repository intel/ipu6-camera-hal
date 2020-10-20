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

#define LOG_TAG "ImageScalerCore"

#include <memory>
#include <linux/videodev2.h>
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "ImageScalerCore.h"

#define RESOLUTION_VGA_WIDTH    640
#define RESOLUTION_VGA_HEIGHT   480
#define RESOLUTION_QVGA_WIDTH   320
#define RESOLUTION_QVGA_HEIGHT  240
#define RESOLUTION_QCIF_WIDTH   176
#define RESOLUTION_QCIF_HEIGHT  144
#define MIN(a,b) ((a)<(b)?(a):(b))

namespace icamera {

void ImageScalerCore::downScaleImage(void *src, void *dest,
    int dest_w, int dest_h, int dest_stride,
    int src_w, int src_h, int src_stride,
    int format, int src_skip_lines_top, // number of lines that are skipped from src image start pointer
    int src_skip_lines_bottom) // number of lines that are skipped after reading src_h (should be set always to reach full image height)
{
    unsigned char *m_dest = (unsigned char *)dest;
    const unsigned char * m_src = (const unsigned char *)src;
    switch (format) {
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12: {
            if ((dest_w == src_w && dest_h <= src_h) || (dest_w <= src_w && dest_h == src_h)) {
                // trim only if only src_h is larger than dest_h or src_w is larger than dest_w
                ImageScalerCore::trimNv12Image(m_dest, m_src,
                                               dest_w, dest_h, dest_stride,
                                               src_w, src_h, src_stride,
                                               src_skip_lines_top, src_skip_lines_bottom);
            } else {
                // downscale & crop
                ImageScalerCore::downScaleAndCropNv12Image(m_dest, m_src,
                                                           dest_w, dest_h, dest_stride,
                                                           src_w, src_h, src_stride,
                                                           src_skip_lines_top, src_skip_lines_bottom);
            }
            break;
        }
        case V4L2_PIX_FMT_YUYV: {
            ImageScalerCore::downScaleYUY2Image(m_dest, m_src,
                                                dest_w, dest_h, dest_stride,
                                                src_w, src_h, src_stride);
            break;
        }
        default: {
            LOGE("no downscale support for format = %d", format);
            break;
        }
    }
}

void ImageScalerCore::downScaleYUY2Image(unsigned char *dest, const unsigned char *src,
                                         const int dest_w, const int dest_h, const int dest_stride,
                                         const int src_w, const int src_h, const int src_stride)
{
    if (dest==NULL || dest_w <=0 || dest_h <=0 || src==NULL || src_w <=0 || src_h <= 0 )
        return;

    if (dest_w%2 != 0) // if the dest_w is not an even number, exit
        return;

    const int scale_w = (src_w<<8) / dest_w; // scale factors
    const int scale_h = (src_h<<8) / dest_h;
    int macro_pixel_width = dest_w >> 1;
    unsigned int val_1, val_2; // for bi-linear-interpolation
    int i,j,k;

    for(i=0; i < dest_h; ++i) {
        int src_i = i * scale_h;
        int dy = src_i & 0xff;
        src_i >>= 8;
        for(j=0; j < macro_pixel_width; ++j) {
            int src_j = j * scale_w;
            int dx = src_j & 0xff;
            src_j = src_j >> 8;
            for(k = 0; k < 4; ++k) {
                // bi-linear-interpolation
                if(dx == 0 && dy == 0) {
                    dest[i * 2 * dest_stride + 4 * j + k] = src[src_i * 2 * src_stride + src_j * 4 + k];
                } else if(dx == 0 && dy != 0){
                    val_1 = (unsigned int)src[src_i * 2 * src_stride + src_j * 4 + k];
                    val_2 = (unsigned int)src[(src_i + 1) * 2 * src_stride + src_j * 4 + k];
                    val_1 = (val_1 * (256 - dy) + val_2 * dy) >> 8;
                    dest[i * 2 * dest_stride + 4 * j + k] = ((val_1 <= 255) ? val_1: 255);
                } else if(dx != 0 && dy == 0) {
                    val_1 = ((unsigned int)src[src_i * 2 * src_stride + src_j * 4 + k] * (256 - dx)
                        + (unsigned int)src[src_i * 2 * src_stride + (src_j +1) * 4 + k] * dx) >> 8;
                    dest[i * 2 * dest_stride + 4 * j + k] = ((val_1 <= 255) ? val_1: 255);
                } else {
                    val_1 = ((unsigned int)src[src_i * 2 * src_stride + src_j * 4 + k] * (256 - dx)
                        + (unsigned int)src[src_i * 2 * src_stride + (src_j +1) * 4 + k] * dx) >> 8;
                    val_2 = ((unsigned int)src[(src_i + 1) * 2 * src_stride + src_j * 4 + k] * (256 - dx)
                        + (unsigned int)src[(src_i + 1) * 2 * src_stride + (src_j+1) * 4 + k] * dx) >> 8;
                    val_1 = (val_1 * (256 - dy) + val_2 * dy) >> 8;
                    dest[i * 2 * dest_stride + 4 * j + k] = ((val_1 <= 255) ? val_1: 255);
                }
            }
        }
    }
}

void ImageScalerCore::trimNv12Image(unsigned char *dest, const unsigned char *src,
                                    const int dest_w, const int dest_h, const int dest_stride,
                                    const int src_w, const int src_h, const int src_stride,
                                    const int src_skip_lines_top, // number of lines that are skipped from src image start pointer
                                    const int src_skip_lines_bottom) // number of lines that are skipped after reading src_h (should be set always to reach full image height)
{
    LOG1("@%s: dest_w: %d, dest_h: %d, dest_stride:%d, src_w: %d, src_h: %d, src_stride: %d, skip_top: %d, skip_bottom: %d",
         __func__, dest_w,dest_h,dest_stride,src_w,src_h,src_stride,src_skip_lines_top,src_skip_lines_bottom);

    const unsigned char *y = src;
    const unsigned char *uv = src + src_h * src_stride;
    if (dest_w < src_w) {
        /*
         *                     src_w
         *  y    ---------------------------------
         *       -    -        dest_w       -    -
         *       -    -                     -    -
         *       -    -                     -    -
         * src_h -    - dest_h              -    -
         *       -    -                     -    -
         *       -    -                     -    -
         *       -    -                     -    -
         *       ---------------------------------
         *
         *                     src_w
         *  uv   ---------------------------------
         *       -    -        dest_w       -    -
         * src_h -    - dest_h              -    -
         *       -    -                     -    -
         *       ---------------------------------
         */
        y += (src_w - dest_w) / 2;
        uv += (src_w - dest_w) / 2;
    } else if (dest_h < src_h) {
        /*
         *                     src_w
         *  y    ---------------------------------
         *       -             dest_w            -
         *       ---------------------------------
         *       -                               -
         * src_h - dest_h                        -
         *       -                               -
         *       ---------------------------------
         *       -                               -
         *       ---------------------------------
         *
         *                     src_w
         *  uv   ---------------------------------
         *       ---------------------------------
         * src_h - dest_h      dest_w            -
         *       ---------------------------------
         *       ---------------------------------
         */
        y += (src_h - dest_h) * src_stride / 2;
        uv += (src_h - dest_h) * src_stride / 4;
    }

    // Y
    for (int i = 0; i < dest_h; i++) {
        MEMCPY_S(dest, src_stride, y, dest_stride);
        dest += dest_stride;
        y += src_stride;
    }

    // UV
    for (int i = 0; i < dest_h / 2; i++) {
        MEMCPY_S(dest, src_stride, uv, dest_stride);
        dest += dest_stride;
        uv += src_stride;
    }
}

// VGA-QCIF begin (Enzo specific)
void ImageScalerCore::downScaleAndCropNv12Image(unsigned char *dest, const unsigned char *src,
                                                const int dest_w, const int dest_h, const int dest_stride,
                                                const int src_w, const int src_h, const int src_stride,
                                                const int src_skip_lines_top, // number of lines that are skipped from src image start pointer
                                                const int src_skip_lines_bottom) // number of lines that are skipped after reading src_h (should be set always to reach full image height)
{
    LOG1("@%s: dest_w: %d, dest_h: %d, dest_stride: %d, src_w: %d, src_h: %d, src_stride: %d, skip_top: %d, skip_bottom: %d, dest: %p, src: %p",
         __func__, dest_w, dest_h, dest_stride, src_w, src_h, src_stride, src_skip_lines_top, src_skip_lines_bottom, dest, src);

    if (src_w == 800 && src_h == 600 && src_skip_lines_top == 0 && src_skip_lines_bottom == 0
        && dest_w == RESOLUTION_QVGA_WIDTH && dest_h == RESOLUTION_QVGA_HEIGHT) {
        downScaleNv12ImageFrom800x600ToQvga(dest, src, dest_stride, src_stride);
        return;
    }
    if (src_w == RESOLUTION_VGA_WIDTH && src_h == RESOLUTION_VGA_HEIGHT
        && src_skip_lines_top == 0 && src_skip_lines_bottom == 0
        && dest_w == RESOLUTION_QVGA_WIDTH && dest_h == RESOLUTION_QVGA_HEIGHT) {
        downScaleAndCropNv12ImageQvga(dest, src, dest_stride, src_stride);
        return;
    }
    if (src_w == RESOLUTION_VGA_WIDTH && src_h == RESOLUTION_VGA_HEIGHT
        && src_skip_lines_top == 0 && src_skip_lines_bottom == 0
        && dest_w == RESOLUTION_QCIF_WIDTH && dest_h == RESOLUTION_QCIF_WIDTH) {
        downScaleAndCropNv12ImageQcif(dest, src, dest_stride, src_stride);
        return;
    }

    // skip lines from top
    if (src_skip_lines_top > 0)
        src += src_skip_lines_top * src_stride;

    // Correct aspect ratio is defined by destination buffer
    long int aspect_ratio = (dest_w << 16) / dest_h;
    // Then, we calculate what should be the width of source image
    // (should be multiple by four)
    int proper_source_width = (aspect_ratio * (long int)(src_h) + 0x8000L) >> 16;
    proper_source_width = (proper_source_width + 2) & ~0x3;
    // Now, the source image should have some surplus width
    if (src_w < proper_source_width) {
        LOGE("%s: source image too narrow", __func__);
    }
    // Let's divide the surplus to both sides
    int l_skip = src_w < proper_source_width ? 0 : ((src_w - proper_source_width) >> 1);
    int r_skip = src_w < proper_source_width ? 0 : (src_w - proper_source_width - l_skip);
    int skip = l_skip + r_skip;

    int i, j, x1, y1, x2, y2;
    unsigned int val_1, val_2;
    int dx, dy;
    int src_Y_data = src_stride * (src_h + src_skip_lines_bottom + (src_skip_lines_top >> 1));
    int dest_Y_data = dest_stride * dest_h;
    int width, height;
    if (0 == dest_w || 0 == dest_h) {
        LOGE("%s,dest_w or dest_h should not be 0", __func__);
        return;
    }
    const int scaling_w = ((src_w - skip) << 8) / dest_w;
    const int scaling_h = (src_h << 8) / dest_h;
    dx = 0;
    dy = 0;
    // get Y data
    for (i = 0; i < dest_h; i++) {
        y1 = i * scaling_h;
        dy = y1 & 0xff;
        y2 = y1 >> 8;
        for (j = 0; j < dest_w; j++) {
            x1 = j * scaling_w;
            dx = x1 & 0xff;
            x2 = (x1 >> 8) + l_skip;
            val_1 = ((unsigned int)src[y2 * src_stride + x2] * (256 - dx)
                    + (unsigned int)src[y2 * src_stride + x2 + 1] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_stride + x2] * (256 - dx)
                    + (unsigned int)src[(y2 + 1) * src_stride + x2 + 1] * dx) >> 8;
            dest[i * dest_stride + j] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
        }
    }
    i = 0;
    j = 0;
    width = dest_w >> 1;
    height = dest_h >> 1;
    //get UV data
    for (i = 0; i < height; i++) {
        y1 = i * scaling_h;
        dy = y1 & 0xff;
        y2 = y1 >> 8;
        for (j = 0; j < width; j++) {
            x1 = j * scaling_w;
            dx = x1 & 0xff;
            x2 = (x1 >> 8) + l_skip / 2;
            //fill U data
            val_1 = ((unsigned int)src[y2 * src_stride + (x2 << 1) + src_Y_data] * (256 - dx)
                     + (unsigned int)src[y2 * src_stride + ((x2 + 1) << 1) + src_Y_data] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_stride + (x2 << 1) + src_Y_data] * (256 -dx)
                     + (unsigned int)src[(y2 +1) * src_stride + ((x2 + 1) << 1) + src_Y_data] * dx) >> 8;
            dest[i * dest_stride + (j << 1) + dest_Y_data] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
            //fill V data
            val_1 = ((unsigned int)src[y2 * src_stride + (x2 << 1) + 1 + src_Y_data] * (256 - dx)
                     + (unsigned int)src[y2 * src_stride + ((x2 + 1) << 1) + 1 + src_Y_data] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_stride + (x2 << 1) + 1 + src_Y_data] * (256 -dx)
                     + (unsigned int)src[(y2 +1) * src_stride + ((x2 + 1) << 1) + 1 + src_Y_data] * dx) >> 8;
            dest[i * dest_stride + (j << 1) + 1 + dest_Y_data] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
        }
    }
}

void ImageScalerCore::downScaleAndCropNv12ImageQvga(unsigned char *dest, const unsigned char *src,
                                                    const int dest_stride, const int src_stride)
{
    LOG1("@%s", __func__);
    const int dest_w = RESOLUTION_QVGA_WIDTH;
    const int dest_h = RESOLUTION_QVGA_HEIGHT;
    const int src_h = RESOLUTION_VGA_HEIGHT;
    const int scale = 2;

    // Y component
    for (int i = 0; i < dest_h; i++) {
        u_int32_t *s1 = (u_int32_t *)(&src[(i * scale + 0) * src_stride]);
        u_int32_t *s2 = (u_int32_t *)(&src[(i * scale + 1) * src_stride]);
        u_int32_t  *d = (u_int32_t *)(&dest[i * dest_stride]);
        // This processes 4 dest pixels at a time
        for (int j = 0; j < dest_w; j+=4) {
            u_int32_t a1; // Input data upper row
            u_int32_t a2; // Input data lower row
            u_int32_t b;  // Output data
            a1 = *s1++;
            a2 = *s2++;
            b  = ((a1 & 0xff) + ((a1 >> 8) & 0xff) + (a2 & 0xff) + ((a2 >> 8) & 0xff) + 2) / 4;
            b |= ((((a1 >> 16) & 0xff) + ((a1 >> 24) & 0xff) + ((a2 >> 16) & 0xff) + ((a2 >> 24) & 0xff) + 2) / 4) << 8;
            a1 = *s1++;
            a2 = *s2++;
            b |= (((a1 & 0xff) + ((a1 >> 8) & 0xff) + (a2 & 0xff) + ((a2 >> 8) & 0xff) + 2) / 4) << 16;
            b |= ((((a1 >> 16) & 0xff) + ((a1 >> 24) & 0xff) + ((a2 >> 16) & 0xff) + ((a2 >> 24) & 0xff) + 2) / 4) << 24;
            *d++ = b;
        }
    }

    // UV components
    src = &src[src_stride * src_h];
    dest = &dest[dest_stride * dest_h];

    for (int i = 0; i < dest_h/2; i++) {
        u_int32_t *s1 = (u_int32_t *)(&src[(i * scale + 0) * src_stride]);
        u_int32_t *s2 = (u_int32_t *)(&src[(i * scale + 1) * src_stride]);
        u_int32_t  *d = (u_int32_t *)(&dest[i * dest_stride]);
        // This processes 2 dest UV pairs at a time
        for (int j = 0; j < dest_w/2; j+=2) {
            u_int32_t a1; // Input data upper row
            u_int32_t a2; // Input data lower row
            u_int32_t b;  // Output data
            a1 = *s1++;
            a2 = *s2++;
            b  = ((a1 & 0xff) + ((a1 >> 16) & 0xff) + (a2 & 0xff) + ((a2 >> 16) & 0xff) + 2) / 4;
            b |= ((((a1 >> 8) & 0xff) + ((a1 >> 24) & 0xff) + ((a2 >> 8) & 0xff) + ((a2 >> 24) & 0xff) + 2) / 4) << 8;
            a1 = *s1++;
            a2 = *s2++;
            b |= (((a1 & 0xff) + ((a1 >> 16) & 0xff) + (a2 & 0xff) + ((a2 >> 16) & 0xff) + 2) / 4) << 16;
            b |= ((((a1 >> 8) & 0xff) + ((a1 >> 24) & 0xff) + ((a2 >> 8) & 0xff) + ((a2 >> 24) & 0xff) + 2) / 4) << 24;
            *d++ = b;
        }
    }
}

void ImageScalerCore::downScaleAndCropNv12ImageQcif(unsigned char *dest, const unsigned char *src,
                                                    const int dest_stride, const int src_stride)
{
    LOG1("@%s", __func__);
    const int dest_w = RESOLUTION_QCIF_WIDTH;
    const int dest_h = RESOLUTION_QCIF_HEIGHT;
    const int src_w = RESOLUTION_VGA_WIDTH;
    const int src_h = RESOLUTION_VGA_HEIGHT;

    // Correct aspect ratio is defined by destination buffer
    long int aspect_ratio = (dest_w << 16) / dest_h;
    // Then, we calculate what should be the width of source image
    // (should be multiple by four)
    int proper_source_width = (aspect_ratio * (long int)(src_h) + 0x8000L) >> 16;
    proper_source_width = (proper_source_width + 2) & ~0x3;
    // Now, the source image should have some surplus width
    if (src_w < proper_source_width) {
        LOGE("%s: source image too narrow", __func__);
        return;
    }
    // Let's divide the surplus to both sides
    int l_skip = (src_w - proper_source_width) >> 1;
    int r_skip = src_w - proper_source_width - l_skip;
    int skip = l_skip + r_skip;

    int i, j, x1, y1, x2, y2;
    unsigned int val_1, val_2;
    int dx, dy;
    int src_Y_data = src_stride * src_h;
    int dest_Y_data = dest_stride * dest_h;
    int width, height;
    const int scaling_w = ((src_w - skip) << 8) / dest_w;
    const int scaling_h = (src_h << 8) / dest_h;
    dx = 0;
    dy = 0;
    // get Y data
    for (i = 0; i < dest_h; i++) {
        y1 = i * scaling_h;
        dy = y1 & 0xff;
        y2 = y1 >> 8;
        for (j = 0; j < dest_w; j++) {
            x1 = j * scaling_w;
            dx = x1 & 0xff;
            x2 = (x1 >> 8) + l_skip;
            val_1 = ((unsigned int)src[y2 * src_stride + x2] * (256 - dx)
                    + (unsigned int)src[y2 * src_stride + x2 + 1] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_stride + x2] * (256 - dx)
                    + (unsigned int)src[(y2 + 1) * src_stride + x2 + 1] * dx) >> 8;
            dest[i * dest_stride + j] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
        }
    }
    i = 0;
    j = 0;
    width = dest_w >> 1;
    height = dest_h >> 1;
    //get UV data
    for (i = 0; i < height; i++) {
        y1 = i * scaling_h;
        dy = y1 & 0xff;
        y2 = y1 >> 8;
        for (j = 0; j < width; j++) {
            x1 = j * scaling_w;
            dx = x1 & 0xff;
            x2 = (x1 >> 8) + l_skip / 2;
            //fill U data
            val_1 = ((unsigned int)src[y2 * src_stride + (x2 << 1) + src_Y_data] * (256 - dx)
                     + (unsigned int)src[y2 * src_stride + ((x2 + 1) << 1) + src_Y_data] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_stride + (x2 << 1) + src_Y_data] * (256 -dx)
                     + (unsigned int)src[(y2 +1) * src_stride + ((x2 + 1) << 1) + src_Y_data] * dx) >> 8;
            dest[i * dest_stride + (j << 1) + dest_Y_data] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
            //fill V data
            val_1 = ((unsigned int)src[y2 * src_w + (x2 << 1) + 1 + src_Y_data] * (256 - dx)
                     + (unsigned int)src[y2 * src_w + ((x2 + 1) << 1) + 1 + src_Y_data] * dx) >> 8;
            val_2 = ((unsigned int)src[(y2 + 1) * src_w + (x2 << 1) + 1 + src_Y_data] * (256 -dx)
                     + (unsigned int)src[(y2 +1) * src_w + ((x2 + 1) << 1) + 1 + src_Y_data] * dx) >> 8;
            dest[i * dest_stride + (j << 1) + 1 + dest_Y_data] = MIN(((val_1 * (256 - dy) + val_2 * dy) >> 8), 0xff);
        }
    }
}

void ImageScalerCore::downScaleNv12ImageFrom800x600ToQvga(unsigned char *dest, const unsigned char *src,
                                                          const int dest_stride, const int src_stride)
{
    LOG1("@%s", __func__);
    const int dest_w = RESOLUTION_QVGA_WIDTH;
    const int dest_h = RESOLUTION_QVGA_HEIGHT;
    const int src_h = 600;

    // Y component

    // Processing 2 dest rows and 5 src rows at a time
    for (int i = 0; i < dest_h / 2; i++) {
        u_int32_t *s1 = (u_int32_t *)(&src[(i * 5 + 0) * src_stride]);
        u_int32_t *s2 = (u_int32_t *)(&src[(i * 5 + 1) * src_stride]);
        u_int32_t *s3 = (u_int32_t *)(&src[(i * 5 + 2) * src_stride]);
        u_int32_t *d = (u_int32_t *)(&dest[(i * 2 + 0) * dest_stride]);
        // This processes 8 dest pixels at a time
        for (int j = 0; j < dest_w; j+=8) {
            u_int32_t a1; // Input data upper row
            u_int32_t a2; // Input data middle row
            u_int32_t a3; // Input data lower row
            u_int32_t t;  // Temp data (for constructing the output)
            u_int32_t b;  // Output data
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b = t; // First pixel
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 8; // Second pixel
            t = (0 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 1 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 16; // Third pixel
            t = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 24; // Fourth pixel
            *d++ = b;
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b = t; // Fifth pixel
            t = (2 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 8; // Sixth pixel
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 16; // Seventh pixel
            t = (0 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 24; // Eigth pixel
            *d++ = b;
        }
        s1 = (u_int32_t *)(&src[(i * 5 + 4) * src_stride]);
        s2 = (u_int32_t *)(&src[(i * 5 + 3) * src_stride]);
        s3 = (u_int32_t *)(&src[(i * 5 + 2) * src_stride]);
        d = (u_int32_t *)(&dest[(i * 2 + 1) * dest_stride]);
        // This processes 8 dest pixels at a time
        for (int j = 0; j < dest_w; j+=8) {
            u_int32_t a1; // Input data lower row
            u_int32_t a2; // Input data middle row
            u_int32_t a3; // Input data upper row
            u_int32_t t;  // Temp data (for constructing the output)
            u_int32_t b;  // Output data
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b = t; // First pixel
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 8; // Second pixel
            t = (0 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 1 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 16; // Third pixel
            t = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 24; // Fourth pixel
            *d++ = b;
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b = t; // Fifth pixel
            t = (2 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 8; // Sixth pixel
            t = (0 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            t = (4 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 16) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 16) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 16) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ t;
            t = (t + 12) / 25;
            b |= t << 16; // Seventh pixel
            t = (0 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 16) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 16) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 16) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            t = (t + 12) / 25;
            b |= t << 24; // Eigth pixel
            *d++ = b;
        }
    }

    // UV components
    src = &src[src_stride * src_h];
    dest = &dest[dest_stride * dest_h];

    // Processing 2 dest rows and 5 src rows at a time
    for (int i = 0; i < (dest_h/2) / 2; i++) {
        u_int32_t *s1 = (u_int32_t *)(&src[(i * 5 + 0) * src_stride]);
        u_int32_t *s2 = (u_int32_t *)(&src[(i * 5 + 1) * src_stride]);
        u_int32_t *s3 = (u_int32_t *)(&src[(i * 5 + 2) * src_stride]);
        u_int16_t *d = (u_int16_t *)(&dest[(i * 2 + 0) * dest_stride]);
        // This processes 4 dest UV pairs at a time
        for (int j = 0; j < dest_w/2; j+=4) {
            u_int32_t a1; // Input data upper row
            u_int32_t a2; // Input data middle row
            u_int32_t a3; // Input data lower row
            u_int32_t u;  // Temp data (for constructing the output)
            u_int32_t v;  // Temp data (for constructing the output)
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 16) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 16) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 16) & 0xff))+ u;
            v = (2 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // First uv pair;
            u = (2 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (2 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Second uv pair;
            u = (0 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (0 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Third uv pair;
            u = (0 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 16) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 16) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 16) & 0xff));
            v = (0 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Fourth uv pair;
        }
        s1 = (u_int32_t *)(&src[(i * 5 + 4) * src_stride]);
        s2 = (u_int32_t *)(&src[(i * 5 + 3) * src_stride]);
        s3 = (u_int32_t *)(&src[(i * 5 + 2) * src_stride]);
        d = (u_int16_t *)(&dest[(i * 2 + 1) * dest_stride]);
        // This processes 4 dest UV pairs at a time
        for (int j = 0; j < dest_w/2; j+=4) {
            u_int32_t a1; // Input data lower row
            u_int32_t a2; // Input data middle row
            u_int32_t a3; // Input data upper row
            u_int32_t u;  // Temp data (for constructing the output)
            u_int32_t v;  // Temp data (for constructing the output)
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (2 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 16) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 16) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 16) & 0xff))+ u;
            v = (2 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // First uv pair;
            u = (2 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 2 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 1 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (2 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 2 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 1 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 0 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 0 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 0 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 0 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 0 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 0 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Second uv pair;
            u = (0 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff));
            v = (0 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Third uv pair;
            u = (0 * ((a1 >> 0) & 0xff) + 2 * ((a1 >> 16) & 0xff) +
                 0 * ((a2 >> 0) & 0xff) + 2 * ((a2 >> 16) & 0xff) +
                 0 * ((a3 >> 0) & 0xff) + 1 * ((a3 >> 16) & 0xff));
            v = (0 * ((a1 >> 8) & 0xff) + 2 * ((a1 >> 24) & 0xff) +
                 0 * ((a2 >> 8) & 0xff) + 2 * ((a2 >> 24) & 0xff) +
                 0 * ((a3 >> 8) & 0xff) + 1 * ((a3 >> 24) & 0xff));
            a1 = *s1++;
            a2 = *s2++;
            a3 = *s3++;
            u = (4 * ((a1 >> 0) & 0xff) + 4 * ((a1 >> 16) & 0xff) +
                 4 * ((a2 >> 0) & 0xff) + 4 * ((a2 >> 16) & 0xff) +
                 2 * ((a3 >> 0) & 0xff) + 2 * ((a3 >> 16) & 0xff))+ u;
            v = (4 * ((a1 >> 8) & 0xff) + 4 * ((a1 >> 24) & 0xff) +
                 4 * ((a2 >> 8) & 0xff) + 4 * ((a2 >> 24) & 0xff) +
                 2 * ((a3 >> 8) & 0xff) + 2 * ((a3 >> 24) & 0xff))+ v;
            u = (u + 12) / 25;
            v = (v + 12) / 25;
            *d++ = u | (v << 8); // Fourth uv pair;
        }
    }

}
// VGA-QCIF end

int ImageScalerCore::cropCompose(
    void *src, unsigned int srcW, unsigned int srcH, unsigned int srcStride, int srcFormat,
    void *dst, unsigned int dstW, unsigned int dstH, unsigned int dstStride, int dstFormat,
    unsigned int srcCropW, unsigned int srcCropH, unsigned int srcCropLeft, unsigned int srcCropTop,
    unsigned int dstCropW, unsigned int dstCropH, unsigned int dstCropLeft, unsigned int dstCropTop)
{
    static const unsigned int MAXVAL = 65536;
    static const int ALLOW_DOWNSCALING = 1;

    // Check that we support the formats
    if ((srcFormat != V4L2_PIX_FMT_NV12 &&
         srcFormat != V4L2_PIX_FMT_NV21) ||
        srcFormat != dstFormat) {
        LOGE("Format conversion is not yet supported");
        return UNKNOWN_ERROR;
    }

    if (srcW >= MAXVAL || srcH >= MAXVAL || srcCropLeft >= MAXVAL || srcCropH >= MAXVAL ||
        dstW >= MAXVAL || dstH >= MAXVAL || dstCropLeft >= MAXVAL || dstCropH >= MAXVAL) {
        LOGE("Values out of range");
        return UNKNOWN_ERROR;
    }

    // Check that the windows are within limits
    if (srcCropLeft + srcCropW > srcW || srcCropTop + srcCropH > srcH ||
        dstCropLeft + dstCropW > dstW || dstCropTop + dstCropH > dstH) {
        LOGE("Crop region is outside of the image");
        return UNKNOWN_ERROR;
    }

    // Check that widths are even
    if ((srcW & 1) || (dstW & 1)) {
        LOGE("Image width must be even");
        return UNKNOWN_ERROR;
    }

    if (srcStride == dstStride && srcCropW == srcW && srcW == dstW &&
        srcCropH == srcH && srcH == dstH &&
        dstCropW == dstW && dstCropH == dstH) {
        // If no cropping/scaling is requested, just copy data
        cropComposeCopy(src, dst, srcStride * srcH * 3 / 2);
        return 0;
    }

    if (!ALLOW_DOWNSCALING &&
        (dstCropH < srcCropH || dstCropW < srcCropW)) {
        LOGE("Trying to downscale when it is disabled");
        return UNKNOWN_ERROR;
    }

    if (srcStride == srcW && dstStride == dstW) {
        // Upscaling both horizontally and vertically
        cropComposeUpscaleNV12_bl(
            src, srcH, srcStride, srcCropLeft, srcCropTop, srcCropW, srcCropH,
            dst, dstH, dstStride, dstCropLeft, dstCropTop, dstCropW, dstCropH);
        return 0;
    }

    LOGE("Unsupported scaling parameters");
    return UNKNOWN_ERROR;
}

/**
 * CropComposeZoom
 *
 * Crop and compose algorithm is need to perform digital zooming.
 * Both source and destination images are in same format and resolution,
 * the crop rectangle in the source image is scaled to fill whole image
 * in the destination image.
 */
int ImageScalerCore::cropComposeZoom(void *src, void *dst,
                                     unsigned int width, unsigned int height, unsigned int stride, int format,
                                     unsigned int srcCropW, unsigned int srcCropH, unsigned int srcCropLeft, unsigned int srcCropTop)
{
    return cropCompose(src, width, height, stride, format,
                       dst, width, height, stride, format,
                       srcCropW, srcCropH, srcCropLeft, srcCropTop,
                       width, height, 0, 0);
}

void ImageScalerCore::cropComposeCopy(void *src, void *dst, unsigned int size)
{
    MEMCPY_S((int8_t *) dst, size, (int8_t *) src, size);
}

// Bilinear scaling, chrominance with nearest neighbor
void ImageScalerCore::cropComposeUpscaleNV12_bl(
    void *src, unsigned int srcH, unsigned int srcStride,
    unsigned int srcCropLeft, unsigned int srcCropTop,
    unsigned int srcCropW, unsigned int srcCropH,
    void *dst, unsigned int dstH, unsigned int dstStride,
    unsigned int dstCropLeft, unsigned int dstCropTop,
    unsigned int dstCropW, unsigned int dstCropH)
{
    static const int BILINEAR = 1;
    static const unsigned int FP_1  = 1 << MFP;       // Fixed point 1.0
    static const unsigned int FRACT = (1 << MFP) - 1; // Fractional part mask
    unsigned int dx, dy, sx, sy;
    unsigned char *s = (unsigned char *)src;
    unsigned char *d = (unsigned char *)dst;
    unsigned int sx0, sy0, dx0, dy0, dx1, dy1;

    unsigned int sxd = ((srcCropW<<MFP) + (dstCropW>>1)) / dstCropW;
    unsigned int syd = ((srcCropH<<MFP) + (dstCropH>>1)) / dstCropH;

    if (!src || !dst) {
        LOGE("buffer pointer is NULL");
        return;
    }

    // Upscale luminance
    sx0 = srcCropLeft << MFP;
    sy0 = srcCropTop << MFP;
    dx0 = dstCropLeft;
    dy0 = dstCropTop;
    dx1 = dstCropLeft + dstCropW;
    dy1 = dstCropTop + dstCropH;
    for (dy = dy0, sy = sy0; dy < dy1; dy++, sy += syd) {
        for (dx = dx0, sx = sx0; dx < dx1; dx++, sx += sxd) {
            unsigned int sxi = sx >> MFP;
            unsigned int syi = sy >> MFP;
            unsigned int s0 = s[srcStride*syi+sxi];
            if (BILINEAR) {
                unsigned int fx = sx & FRACT;             // Fractional part
                unsigned int fy = sy & FRACT;
                unsigned int fx1 = FP_1 - fx;               // 1 - fractional part
                unsigned int fy1 = FP_1 - fy;
                unsigned int s1 = s[srcStride*syi+sxi+1];
                unsigned int s2 = s[srcStride*(syi+1)+sxi];
                unsigned int s3 = s[srcStride*(syi+1)+sxi+1];
                unsigned int s4 = (s0 * fx1 + s1 * fx) >> MFP;
                unsigned int s5 = (s2 * fx1 + s3 * fx) >> MFP;
                s0 = (s4 * fy1 + s5 * fy) >> MFP;
            }
            d[dstStride*dy+dx] = s0;
        }
    }

    // Upscale chrominance
    s = (unsigned char *)src + srcStride*srcH;
    d = (unsigned char *)dst + dstStride*dstH;
    sx0 = srcCropLeft << (MFP - 1);
    sy0 = srcCropTop << (MFP - 1);
    dx0 = dstCropLeft >> 1;
    dy0 = dstCropTop >> 1;
    dx1 = (dstCropLeft + dstCropW) >> 1;
    dy1 = (dstCropTop + dstCropH) >> 1;
    for (dy = dy0, sy = sy0; dy < dy1; dy++, sy += syd) {
        for (dx = dx0, sx = sx0; dx < dx1; dx++, sx += sxd) {
            unsigned int sxi = sx >> MFP;
            unsigned int syi = sy >> MFP;
            d[dstStride*dy+dx*2+0] = s[srcStride*syi+sxi*2+0];
            d[dstStride*dy+dx*2+1] = s[srcStride*syi+sxi*2+1];
        }
    }
}

} // namespace icamera

