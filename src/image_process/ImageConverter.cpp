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

#define LOG_TAG "ColorConverter"

#include <sys/types.h>
#include <linux/videodev2.h>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "iutils/Errors.h"
#include "ImageConverter.h"

namespace icamera {
namespace ImageConverter {

void YUV420ToRGB565(int width, int height, void *src, void *dst)
{
    int line, col, linewidth;
    int y, u, v, yy, vr, ug, vg, ub;
    int r, g, b;
    const unsigned char *py, *pu, *pv;
    unsigned short *rgbs = (unsigned short *) dst;

    linewidth = width >> 1;
    py = (unsigned char *) src;
    pu = py + (width * height);
    pv = pu + (width * height) / 4;

    y = *py++;
    yy = y << 8;
    u = *pu - 128;
    ug = 88 * u;
    ub = 454 * u;
    v = *pv - 128;
    vg = 183 * v;
    vr = 359 * v;

    for (line = 0; line < height; line++) {
        for (col = 0; col < width; col++) {
            r = (yy + vr) >> 8;
            g = (yy - ug - vg) >> 8;
            b = (yy + ub ) >> 8;
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            *rgbs++ = (((unsigned short)r>>3)<<11) | (((unsigned short)g>>2)<<5)
                   | (((unsigned short)b>>3)<<0);

            y = *py++;
            yy = y << 8;
            if (col & 1) {
                pu++;
                pv++;
                u = *pu - 128;
                ug = 88 * u;
                ub = 454 * u;
                v = *pv - 128;
                vg = 183 * v;
                vr = 359 * v;
            }
        }
        if ((line & 1) == 0) {
            pu -= linewidth;
            pv -= linewidth;
        }
    }
}

void trimConvertNV12ToRGB565(int width, int height, int srcStride, void *src, void *dst)
{

    unsigned char *yuvs = (unsigned char *) src;
    unsigned char *rgbs = (unsigned char *) dst;

    //the end of the luminance data
    int lumEnd = srcStride * height;
    int i = 0, j = 0;
    for( i=0; i < height; i++) {
        //points to the next luminance value pair
        int lumPtr = i * srcStride;
        //points to the next chromiance value pair
        int chrPtr = i / 2 * srcStride + lumEnd;
        for ( j=0; j < width; j+=2 ) {
            //read the luminance and chromiance values
            int Y1 = yuvs[lumPtr++] & 0xff;
            int Y2 = yuvs[lumPtr++] & 0xff;
            int Cb = (yuvs[chrPtr++] & 0xff) - 128;
            int Cr = (yuvs[chrPtr++] & 0xff) - 128;
            int R, G, B;

            //generate first RGB components
            B = Y1 + ((454 * Cb) >> 8);
            if(B < 0) B = 0; else if(B > 255) B = 255;
            G = Y1 - ((88 * Cb + 183 * Cr) >> 8);
            if(G < 0) G = 0; else if(G > 255) G = 255;
            R = Y1 + ((359 * Cr) >> 8);
            if(R < 0) R = 0; else if(R > 255) R = 255;
            //NOTE: this assume little-endian encoding
            *rgbs++ = (unsigned char) (((G & 0x3c) << 3) | (B >> 3));
            *rgbs++ = (unsigned char) ((R & 0xf8) | (G >> 5));

            //generate second RGB components
            B = Y2 + ((454 * Cb) >> 8);
            if(B < 0) B = 0; else if(B > 255) B = 255;
            G = Y2 - ((88 * Cb + 183 * Cr) >> 8);
            if(G < 0) G = 0; else if(G > 255) G = 255;
            R = Y2 + ((359 * Cr) >> 8);
            if(R < 0) R = 0; else if(R > 255) R = 255;
            //NOTE: this assume little-endian encoding
            *rgbs++ = (unsigned char) (((G & 0x3c) << 3) | (B >> 3));
            *rgbs++ = (unsigned char) ((R & 0xf8) | (G >> 5));
        }
    }
}

// covert YV12 (Y plane, V plane, U plane) to NV21 (Y plane, interlaced VU bytes)
void convertYV12ToNV21(int width, int height, int srcStride, int dstStride, void *src, void *dst)
{
    const int cStride = srcStride>>1;
    const int vuStride = dstStride;
    const int hhalf = height>>1;
    const int whalf = width>>1;

    // copy the entire Y plane
    unsigned char *srcPtr = (unsigned char *)src;
    unsigned char *dstPtr = (unsigned char *)dst;
    if (srcStride == dstStride) {
        MEMCPY_S(dstPtr, dstStride*height, srcPtr, dstStride*height);
    } else {
        for (int i = 0; i < height; i++) {
            MEMCPY_S(dstPtr, width, srcPtr, width);
            srcPtr += srcStride;
            dstPtr += dstStride;
        }
    }

    // interlace the VU data
    unsigned char *srcPtrV = (unsigned char *)src + height*srcStride;
    unsigned char *srcPtrU = srcPtrV + cStride*hhalf;
    dstPtr = (unsigned char *)dst + dstStride*height;
    for (int i = 0; i < hhalf; ++i) {
        unsigned char *pDstVU = dstPtr;
        unsigned char *pSrcV = srcPtrV;
        unsigned char *pSrcU = srcPtrU;
        for (int j = 0; j < whalf; ++j) {
            *pDstVU ++ = *pSrcV ++;
            *pDstVU ++ = *pSrcU ++;
        }
        dstPtr += vuStride;
        srcPtrV += cStride;
        srcPtrU += cStride;
    }
}

// copy YV12 to YV12 (Y plane, V plan, U plan) in case of different stride length
void copyYV12ToYV12(int width, int height, int srcStride, int dstStride, void *src, void *dst)
{
    // copy the entire Y plane
    if (srcStride == dstStride) {
        MEMCPY_S(dst, dstStride * height, src, dstStride * height);
    } else {
        unsigned char *srcPtrY = (unsigned char *)src;
        unsigned char *dstPtrY = (unsigned char *)dst;
        for (int i = 0; i < height; i ++) {
            MEMCPY_S(dstPtrY, width, srcPtrY, width);
            srcPtrY += srcStride;
            dstPtrY += dstStride;
        }
    }

    // copy VU plane
    const int scStride = srcStride >> 1;
    const int dcStride = ALIGN_16(dstStride >> 1); // Android CTS required: U/V plane needs 16 bytes aligned!
    if (dcStride == scStride) {
        unsigned char *srcPtrVU = (unsigned char *)src + height * srcStride;
        unsigned char *dstPtrVU = (unsigned char *)dst + height * dstStride;
        MEMCPY_S(dstPtrVU, height * dcStride, srcPtrVU, height * dcStride);
    } else {
        const int wHalf = width >> 1;
        const int hHalf = height >> 1;
        unsigned char *srcPtrV = (unsigned char *)src + height * srcStride;
        unsigned char *srcPtrU = srcPtrV + scStride * hHalf;
        unsigned char *dstPtrV = (unsigned char *)dst + height * dstStride;
        unsigned char *dstPtrU = dstPtrV + dcStride * hHalf;
        for (int i = 0; i < hHalf; i ++) {
            MEMCPY_S(dstPtrU, wHalf, srcPtrU, wHalf);
            MEMCPY_S(dstPtrV, wHalf, srcPtrV, wHalf);
            dstPtrU += dcStride, srcPtrU += scStride;
            dstPtrV += dcStride, srcPtrV += scStride;
        }
    }
}

// covert NV12 (Y plane, interlaced UV bytes) to
// NV21 (Y plane, interlaced VU bytes) and trim stride width to real width
void trimConvertNV12ToNV21(int width, int height, int srcStride, void *src, void *dst)
{
    const int ysize = width * height;
    unsigned const char *pSrc = (unsigned char *)src;
    unsigned char *pDst = (unsigned char *)dst;

    // Copy Y component
    if (srcStride == width) {
        MEMCPY_S(pDst, ysize, pSrc, ysize);
    } else if (srcStride > width) {
        int j = height;
        while(j--) {
            MEMCPY_S(pDst, width, pSrc, width);
            pSrc += srcStride;
            pDst += width;
        }
    } else {
        ALOGE("bad stride value");
        return;
    }

    // Convert UV to VU
    pSrc = (unsigned char *)src + srcStride * height;
    pDst = (unsigned char *)dst + width * height;
    for (int j = 0; j < height / 2; j++) {
        if (width >= 16) {
            const uint32_t *ptr0 = (const uint32_t *)(pSrc);
            uint32_t *ptr1 = (uint32_t *)(pDst);
            int bNotLastLine = ((j+1) == (height/2)) ? 0 : 1;
            int width_16 = (width + 15 * bNotLastLine) & ~0xf;
            if ((((uint64_t)(pSrc)) & 0xf) == 0 && (((uint64_t)(pDst)) & 0xf) == 0) { // 16 bytes aligned for both src and dest
                __asm__ volatile(\
                                 "movl       %0,  %%eax      \n\t"
                                 "movl       %1,  %%edx      \n\t"
                                 "movl       %2,  %%ecx      \n\t"
                                 "1:     \n\t"
                                 "movdqa (%%eax), %%xmm1     \n\t"
                                 "movdqa  %%xmm1, %%xmm0     \n\t"
                                 "psllw       $8, %%xmm1     \n\t"
                                 "psrlw       $8, %%xmm0     \n\t"
                                 "por     %%xmm0, %%xmm1     \n\t"
                                 "movdqa  %%xmm1, (%%edx)    \n\t"
                                 "add        $16, %%eax      \n\t"
                                 "add        $16, %%edx      \n\t"
                                 "sub        $16, %%ecx      \n\t"
                                 "jnz   1b \n\t"
                                 : "+m"(ptr0), "+m"(ptr1), "+m"(width_16)
                                 :
                                 : "eax", "ecx", "edx", "xmm0", "xmm1"
                                );
            }
            else { // either src or dest is not 16-bytes aligned
                __asm__ volatile(\
                                 "movl       %0,  %%eax      \n\t"
                                 "movl       %1,  %%edx      \n\t"
                                 "movl       %2,  %%ecx      \n\t"
                                 "1:     \n\t"
                                 "lddqu  (%%eax), %%xmm1     \n\t"
                                 "movdqa  %%xmm1, %%xmm0     \n\t"
                                 "psllw       $8, %%xmm1     \n\t"
                                 "psrlw       $8, %%xmm0     \n\t"
                                 "por     %%xmm0, %%xmm1     \n\t"
                                 "movdqu  %%xmm1, (%%edx)    \n\t"
                                 "add        $16, %%eax      \n\t"
                                 "add        $16, %%edx      \n\t"
                                 "sub        $16, %%ecx      \n\t"
                                 "jnz   1b \n\t"
                                 : "+m"(ptr0), "+m"(ptr1), "+m"(width_16)
                                 :
                                 : "eax", "ecx", "edx", "xmm0", "xmm1"
                                );
            }

            // process remaining data of less than 16 bytes of last row
            for (int i = width_16; i < width; i += 2) {
                pDst[i] = pSrc[i + 1];
                pDst[i + 1] = pSrc[i];
            }
        }
        else if ((((uint64_t)(pSrc)) & 0x3) == 0 && (((uint64_t)(pDst)) & 0x3) == 0){  // 4 bytes aligned for both src and dest
            const uint32_t *ptr0 = (const uint32_t *)(pSrc);
            uint32_t *ptr1 = (uint32_t *)(pDst);
            int width_4 = width & ~3;
            for (int i = 0; i < width_4; i += 4) {
                uint32_t data0 = *ptr0++;
                uint32_t data1 = (data0 >> 8) & 0x00ff00ff;
                uint32_t data2 = (data0 << 8) & 0xff00ff00;
                *ptr1++ = data1 | data2;
            }
            // process remaining data of less than 4 bytes at end of each row
            for (int i = width_4; i < width; i += 2) {
                pDst[i] = pSrc[i + 1];
                pDst[i + 1] = pSrc[i];
            }
        }
        else {
            unsigned const char *ptr0 = pSrc;
            unsigned char *ptr1 = pDst;
            for (int i = 0; i < width; i += 2) {
                *ptr1++ = ptr0[1];
                *ptr1++ = ptr0[0];
                ptr0 += 2;
            }
        }
        pDst += width;
        pSrc += srcStride;
    }
}

// convert NV12 (Y plane, interlaced UV bytes) to YV12 (Y plane, V plane, U plane)
// without Y and C 16 bytes aligned
void convertNV12ToYV12(int width, int height, int srcStride, void *src, void *dst)
{
    int yStride = width;
    size_t ySize = yStride * height;
    int cStride = yStride/2;
    size_t cSize = cStride * height/2;

    unsigned char *srcPtr = (unsigned char *) src;
    unsigned char *dstPtr = (unsigned char *) dst;
    unsigned char *dstPtrV = (unsigned char *) dst + ySize;
    unsigned char *dstPtrU = (unsigned char *) dst + ySize + cSize;

    // copy the entire Y plane
    if (srcStride == yStride) {
        MEMCPY_S(dstPtr, ySize, srcPtr, ySize);
        srcPtr += ySize;
    } else if (srcStride > width) {
        for (int i = 0; i < height; i++) {
            MEMCPY_S(dstPtr, width, srcPtr, width);
            srcPtr += srcStride;
            dstPtr += yStride;
        }
    } else {
        ALOGE("bad src stride value");
        return;
    }

    // deinterlace the UV data
    int halfHeight = height / 2;
    int halfWidth = width / 2;
    for ( int i = 0; i < halfHeight; ++i) {
        for ( int j = 0; j < halfWidth; ++j) {
            dstPtrV[j] = srcPtr[j * 2 + 1];
            dstPtrU[j] = srcPtr[j * 2];
        }
        srcPtr += srcStride;
        dstPtrV += cStride;
        dstPtrU += cStride;
    }
}

// convert NV12 (Y plane, interlaced UV bytes) to YV12 (Y plane, V plane, U plane)
// with Y and C 16 bytes aligned
void align16ConvertNV12ToYV12(int width, int height, int srcStride, void *src, void *dst)
{
    int yStride = ALIGN_16(width);
    size_t ySize = yStride * height;
    int cStride = ALIGN_16(yStride/2);
    size_t cSize = cStride * height/2;

    unsigned char *srcPtr = (unsigned char *) src;
    unsigned char *dstPtr = (unsigned char *) dst;
    unsigned char *dstPtrV = (unsigned char *) dst + ySize;
    unsigned char *dstPtrU = (unsigned char *) dst + ySize + cSize;

    // copy the entire Y plane
    if (srcStride == yStride) {
        MEMCPY_S(dstPtr, ySize, srcPtr, ySize);
        srcPtr += ySize;
    } else if (srcStride > width) {
        for (int i = 0; i < height; i++) {
            MEMCPY_S(dstPtr, width, srcPtr, width);
            srcPtr += srcStride;
            dstPtr += yStride;
        }
    } else {
        ALOGE("bad src stride value");
        return;
    }

    // deinterlace the UV data
    for ( int i = 0; i < height / 2; ++i) {
        for ( int j = 0; j < width / 2; ++j) {
            dstPtrV[j] = srcPtr[j * 2 + 1];
            dstPtrU[j] = srcPtr[j * 2];
        }
        srcPtr += srcStride;
        dstPtrV += cStride;
        dstPtrU += cStride;
    }
}

// P411's Y, U, V are seperated. But the YUY2's Y, U and V are interleaved.
void YUY2ToP411(int width, int height, int stride, void *src, void *dst)
{
    int ySize = width * height;
    int cSize = width * height / 4;
    int wHalf = width >> 1;

    unsigned char *srcPtr = (unsigned char *) src;
    unsigned char *dstPtr = (unsigned char *) dst;
    unsigned char *dstPtrU = (unsigned char *) dst + ySize;
    unsigned char *dstPtrV = (unsigned char *) dst + ySize + cSize;

    for (int i = 0; i < height; i++) {
        //The first line of the source
        //Copy first Y Plane first
        for (int j=0; j < width; j++) {
            dstPtr[j] = srcPtr[j*2];
        }

        if (i & 1) {
            //Copy the V plane
            for (int k = 0; k < wHalf; k++) {
                dstPtrV[k] = srcPtr[k * 4 + 3];
            }
            dstPtrV = dstPtrV + wHalf;
        } else {
            //Copy the U plane
            for (int k = 0; k< wHalf; k++) {
                dstPtrU[k] = srcPtr[k * 4 + 1];
            }
            dstPtrU = dstPtrU + wHalf;
        }

        srcPtr = srcPtr + stride * 2;
        dstPtr = dstPtr + width;
    }
}

// P411's Y, U, V are separated. But the NV12's U and V are interleaved.
void NV12ToP411Separate(int width, int height, int stride,
                                void *srcY, void *srcUV, void *dst)
{
    int i, j, p, q;
    unsigned char *psrcY = (unsigned char *) srcY;
    unsigned char *pdstY = (unsigned char *) dst;
    unsigned char *pdstU, *pdstV;
    unsigned char *psrcUV;

    // copy Y data
    for (i = 0; i < height; i++) {
        MEMCPY_S(pdstY, width, psrcY, width);
        pdstY += width;
        psrcY += stride;
    }

    // copy U data and V data
    psrcUV = (unsigned char *)srcUV;
    pdstU = (unsigned char *)dst + width * height;
    pdstV = pdstU + width * height / 4;
    p = q = 0;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width; j++) {
            if (j % 2 == 0) {
                pdstU[p] = (psrcUV[i * stride + j] & 0xFF);
                p++;
           } else {
                pdstV[q] = (psrcUV[i * stride + j] & 0xFF);
                q++;
            }
        }
    }
}

// P411's Y, U, V are seperated. But the NV12's U and V are interleaved.
void NV12ToP411(int width, int height, int stride, void *src, void *dst)
{
    NV12ToP411Separate(width, height, stride,
                    src, (void *)((unsigned char *)src + width * height), dst);
}

// P411's Y, U, V are separated. But the NV21's U and V are interleaved.
void NV21ToP411Separate(int width, int height, int stride,
                        void *srcY, void *srcUV, void *dst)
{
    int i, j, p, q;
    unsigned char *psrcY = (unsigned char *) srcY;
    unsigned char *pdstY = (unsigned char *) dst;
    unsigned char *pdstU, *pdstV;
    unsigned char *psrcUV;

    // copy Y data
    for (i = 0; i < height; i++) {
        MEMCPY_S(pdstY, width, psrcY, width);
        pdstY += width;
        psrcY += stride;
    }

    // copy U data and V data
    psrcUV = (unsigned char *)srcUV;
    pdstU = (unsigned char *)dst + width * height;
    pdstV = pdstU + width * height / 4;
    p = q = 0;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width; j++) {
            if ((j & 1) == 0) {
                pdstV[p] = (psrcUV[i * stride + j] & 0xFF);
                p++;
           } else {
                pdstU[q] = (psrcUV[i * stride + j] & 0xFF);
                q++;
            }
        }
    }
}

// P411's Y, U, V are seperated. But the NV21's U and V are interleaved.
void NV21ToP411(int width, int height, int stride, void *src, void *dst)
{
    NV21ToP411Separate(width, height, stride,
                       src, (void *)((unsigned char *)src + width * height), dst);
}

// IMC3 Y, U, V are separated,the stride for U/V is the same as Y.
// about IMC3 detail, please refer to http://www.fourcc.org/yuv.php
// But the NV12's U and V are interleaved.
void NV12ToIMC3(int width, int height, int stride, void *srcY, void *srcUV, void *dst)
{
    int i, j, p, q;
    unsigned char *pdstU, *pdstV;
    unsigned char *psrcUV;

    // copy Y data even with stride
    MEMCPY_S(dst, stride * height, srcY, stride * height);
    // copy U data and V data
    psrcUV = (unsigned char *)srcUV;
    pdstU = (unsigned char *)dst + stride * height;
    pdstV = pdstU + stride * height / 2;
    p = q = 0;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width; j++) {
            if (j % 2 == 0) {
                pdstU[p]= (psrcUV[i * stride + j] & 0xFF) ;
                p++;
           } else {
                pdstV[q]= (psrcUV[i * stride + j] & 0xFF);
                q++;
            }
        }
        p += stride - width/2;
        q += stride - width/2;
    }
}

// IMC1 Y, V,U are separated,the stride for U/V is the same as Y.
// IMC's V is before U
// But the NV12's U and V are interleaved.
void NV12ToIMC1(int width, int height, int stride, void *srcY, void *srcUV, void *dst)
{
    int i, j, p, q;
    unsigned char *pdstU, *pdstV;
    unsigned char *psrcUV;

    // copy Y data even with stride
    MEMCPY_S(dst, stride * height, srcY, stride * height);
    // copy U data and V data
    psrcUV = (unsigned char *)srcUV;
    pdstV = (unsigned char *)dst + stride * height;
    pdstU = pdstV + stride * height / 2;
    p = q = 0;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width; j++) {
            if (j % 2 == 0) {
                pdstU[p]= (psrcUV[i * stride + j] & 0xFF) ;
                p++;
           } else {
                pdstV[q]= (psrcUV[i * stride + j] & 0xFF);
                q++;
            }
        }
        p += stride - width/2;
        q += stride - width/2;
    }
}

// Re-pad YUV420 format image, the format can be YV12, YU12 or YUV420 planar.
// If buffer size: (height*dstStride*1.5) > (height*srcStride*1.5), src and dst
// buffer start addresses are same, the re-padding can be done inplace.
void repadYUV420(int width, int height, int srcStride, int dstStride, void *src, void *dst)
{
    unsigned char *dptr;
    unsigned char *sptr;
    void * (*myCopy)(void *dst, const void *src, size_t n);

    const int whalf = width >> 1;
    const int hhalf = height >> 1;
    const int scStride = srcStride >> 1;
    const int dcStride = dstStride >> 1;
    const int sySize = height * srcStride;
    const int dySize = height * dstStride;
    const int scSize = hhalf * scStride;
    const int dcSize = hhalf * dcStride;

    // directly copy, if (srcStride == dstStride)
    if (srcStride == dstStride) {
        MEMCPY_S(dst, dySize + 2*dcSize, src, dySize + 2*dcSize);
        return;
    }

    // copy V(YV12 case) or U(YU12 case) plane line by line
    sptr = (unsigned char *)src + sySize + 2*scSize - scStride;
    dptr = (unsigned char *)dst + dySize + 2*dcSize - dcStride;

    // try to avoid overlapped memcpy()
    myCopy = (abs(sptr -dptr) > dstStride) ? memcpy : memmove;

    for (int i = 0; i < hhalf; i ++) {
        myCopy(dptr, sptr, whalf);
        sptr -= scStride;
        dptr -= dcStride;
    }

    // copy  V(YV12 case) or U(YU12 case) U/V plane line by line
    sptr = (unsigned char *)src + sySize + scSize - scStride;
    dptr = (unsigned char *)dst + dySize + dcSize - dcStride;
    for (int i = 0; i < hhalf; i ++) {
        myCopy(dptr, sptr, whalf);
        sptr -= scStride;
        dptr -= dcStride;
    }

    // copy Y plane line by line
    sptr = (unsigned char *)src + sySize - srcStride;
    dptr = (unsigned char *)dst + dySize - dstStride;
    for (int i = 0; i < height; i ++) {
        myCopy(dptr, sptr, width);
        sptr -= srcStride;
        dptr -= dstStride;
    }
}

// covert YUYV(YUY2, YUV422 format) to YV12 (Y plane, V plane, U plane)
void convertYUYVToYV12(int width, int height, int srcStride, int dstStride, void *src, void *dst)
{
    int ySize = width * height;
    int cSize = ALIGN_16(dstStride/2) * height / 2;
    int wHalf = width >> 1;

    unsigned char *srcPtr = (unsigned char *) src;
    unsigned char *dstPtr = (unsigned char *) dst;
    unsigned char *dstPtrV = (unsigned char *) dst + ySize;
    unsigned char *dstPtrU = (unsigned char *) dst + ySize + cSize;

    for (int i = 0; i < height; i++) {
        //The first line of the source
        //Copy first Y Plane first
        for (int j=0; j < width; j++) {
            dstPtr[j] = srcPtr[j*2];
        }

        if (i & 1) {
            //Copy the V plane
            for (int k = 0; k< wHalf; k++) {
                dstPtrV[k] = srcPtr[k * 4 + 3];
            }
            dstPtrV = dstPtrV + ALIGN_16(dstStride>>1);
        } else {
            //Copy the U plane
            for (int k = 0; k< wHalf; k++) {
                dstPtrU[k] = srcPtr[k * 4 + 1];
            }
            dstPtrU = dstPtrU + ALIGN_16(dstStride>>1);
        }

        srcPtr = srcPtr + srcStride * 2;
        dstPtr = dstPtr + width;
    }
}

// covert YUYV(YUY2, YUV422 format) to NV21 (Y plane, interlaced VU bytes)
void convertYUYVToNV21(int width, int height, int srcStride, void *src, void *dst)
{
    int ySize = width * height;
    int u_counter=1, v_counter=0;

    unsigned char *srcPtr = (unsigned char *) src;
    unsigned char *dstPtr = (unsigned char *) dst;
    unsigned char *dstPtrUV = (unsigned char *) dst + ySize;

    for (int i=0; i < height; i++) {
        //The first line of the source
        //Copy first Y Plane first
        for (int j=0; j < width * 2; j++) {
            if (j % 2 == 0)
                dstPtr[j/2] = srcPtr[j];
            if (i%2) {
                if (( j % 4 ) == 3) {
                    dstPtrUV[v_counter] = srcPtr[j]; //V plane
                    v_counter += 2;
                }
                if (( j % 4 ) == 1) {
                    dstPtrUV[u_counter] = srcPtr[j]; //U plane
                    u_counter += 2;
                }
            }
        }

        srcPtr = srcPtr + srcStride * 2;
        dstPtr = dstPtr + width;
    }
}

void convertNV12ToYUYV(int srcWidth, int srcHeight, int srcStride, int dstStride, const void *src, void *dst)
{
    int y_counter = 0, u_counter = 1, v_counter = 3, uv_counter = 0;
    unsigned char *srcYPtr = (unsigned char *) src;
    unsigned char *srcUVPtr = (unsigned char *)src + srcWidth * srcHeight;
    unsigned char *dstPtr = (unsigned char *) dst;

    for (int i = 0; i < srcHeight; i++) {
        for (int k = 0; k < srcWidth; k++) {
                dstPtr[y_counter] = srcYPtr[k];
                y_counter += 2;
                dstPtr[u_counter] = srcUVPtr[uv_counter];
                u_counter += 4;
                dstPtr[v_counter] = srcUVPtr[uv_counter + 1];
                v_counter += 4;
                uv_counter += 2;
        }
        if ((i % 2) == 0) {
            srcUVPtr = srcUVPtr + srcStride;
        }

        dstPtr = dstPtr + 2 * dstStride;
        srcYPtr = srcYPtr + srcStride;
        u_counter = 1;
        v_counter = 3;
        y_counter = 0;
        uv_counter = 0;
    }
}

void convertBuftoYV12(int format, int width, int height, int srcStride,
                      int dstStride, void *src, void *dst, bool align16)
{
    switch (format) {
    case V4L2_PIX_FMT_NV12:
        align16 ? align16ConvertNV12ToYV12(width, height, srcStride, src, dst)
            : convertNV12ToYV12(width, height, srcStride, src, dst);
        break;
    case V4L2_PIX_FMT_YVU420:
        copyYV12ToYV12(width, height, srcStride, dstStride, src, dst);
        break;
    case V4L2_PIX_FMT_YUYV:
        convertYUYVToYV12(width, height, srcStride, dstStride, src, dst);
        break;
    default:
        ALOGE("%s: unsupported format %d", __func__, format);
        break;
    }
}

void convertBuftoNV21(int format, int width, int height, int srcStride,
                      int dstStride, void *src, void *dst)
{
    switch (format) {
    case V4L2_PIX_FMT_NV12:
        trimConvertNV12ToNV21(width, height, srcStride, src, dst);
        break;
    case V4L2_PIX_FMT_YVU420:
        convertYV12ToNV21(width, height, srcStride, dstStride, src, dst);
        break;
    case V4L2_PIX_FMT_YUYV:
        convertYUYVToNV21(width, height, srcStride, src, dst);
        break;
    default:
        ALOGE("%s: unsupported format %d", __func__, format);
        break;
    }
}

void convertBuftoYUYV(int format, int width, int height, int srcStride,
                      int dstStride, void *src, void *dst)
{
    switch (format) {
    case V4L2_PIX_FMT_NV12:
        convertNV12ToYUYV(width, height, srcStride, dstStride, src, dst);
        break;
    default:
        LOGE("%s: unsupported format %d", __func__, format);
        break;
    }
}
} // namespace ImageConverter
} // namespace icamera
