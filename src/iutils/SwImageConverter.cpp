/*
 * Copyright (C) 2016-2020 Intel Corporation.
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

#define LOG_TAG "SwImageConverter"

#include "Errors.h"
#include "Utils.h"
#include "CameraLog.h"
#include "SwImageConverter.h"

namespace icamera {

void SwImageConverter::RGB2YUV(unsigned short R, unsigned short G, unsigned short B,
     unsigned char *Y, unsigned char *U, unsigned char *V)
{
    int Rp, Gp, Bp;
    int oY, oU, oV;
    Rp = R; Gp = G; Bp = B;
    oY = (257 * Rp + 504 * Gp + 98 * Bp) / 4000 + 16;
    oU = (-148 * Rp - 291 * Gp + 439 * Bp) / 4000 + 128;
    oV = (439 * Rp - 368 * Gp - 71 * Bp) / 4000 + 128;
    if (oY > 255) oY = 255;
    if (oY < 0) oY = 0;
    if (oU > 255) oU = 255;
    if (oU < 0) oU = 0;
    if (oV > 255) oV = 255;
    if (oV < 0) oV = 0;
    *Y = (unsigned char)oY;
    *U = (unsigned char)oU;
    *V = (unsigned char)oV;
}

void SwImageConverter::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V,
    unsigned short *R, unsigned short *G, unsigned short *B)
{
    int Yp, Up, Vp, Ypp;
    int oR, oG, oB;
    Yp = Y - 16;
    Up = (U - 128);
    Vp = (V - 128);
    Ypp = 9535 * Yp;

    oB = (Ypp + 16531 * Up) >> 11;
    oG = (Ypp - 6660 * Vp - 3203 * Up) >> 11;
    oR = (Ypp + 13074 * Vp) >> 11;
    if (oR > 1023) oR = 1023;
    if (oR < 0) oR = 0;
    if (oG > 1023) oG = 1023;
    if (oG < 0) oG = 0;
    if (oB > 1023) oB = 1023;
    if (oB < 0) oB = 0;
    *R = (unsigned short)oR;
    *G = (unsigned short)oG;
    *B = (unsigned short)oB;
}

void SwImageConverter::convertBayerBlock(unsigned int x, unsigned int y,
    unsigned int width, unsigned int height,
    unsigned short bayer_data[4], unsigned char *out_buf,
    unsigned int src_fmt, unsigned int dst_fmt)
{
    unsigned char *Ybase;
    unsigned char *UVbase;
    unsigned char Y, U, V;
    unsigned short R, Gr, Gb, B;
    switch (src_fmt) {
        case V4L2_PIX_FMT_SRGGB8: R = bayer_data[0] << 2; Gr = bayer_data[1] << 2; Gb = bayer_data[2] << 2; B = bayer_data[3] << 2; break;
        case V4L2_PIX_FMT_SGRBG8: Gr = bayer_data[0] << 2; R = bayer_data[1] << 2; B = bayer_data[2] << 2; Gb = bayer_data[3] << 2; break;
        case V4L2_PIX_FMT_SGBRG8: Gb = bayer_data[0] << 2; B = bayer_data[1] << 2; R = bayer_data[2] << 2; Gr = bayer_data[3] << 2; break;
        case V4L2_PIX_FMT_SBGGR8: B = bayer_data[0] << 2; Gb = bayer_data[1] << 2; Gr = bayer_data[2] << 2; R = bayer_data[3] << 2; break;
        case V4L2_PIX_FMT_SRGGB10: R = bayer_data[0]; Gr = bayer_data[1]; Gb = bayer_data[2]; B = bayer_data[3]; break;
        case V4L2_PIX_FMT_SGRBG10: Gr = bayer_data[0]; R = bayer_data[1]; B = bayer_data[2]; Gb = bayer_data[3]; break;
        case V4L2_PIX_FMT_SGBRG10: Gb = bayer_data[0]; B = bayer_data[1]; R = bayer_data[2]; Gr = bayer_data[3]; break;
        case V4L2_PIX_FMT_SBGGR10: B = bayer_data[0]; Gb = bayer_data[1]; Gr = bayer_data[2]; R = bayer_data[3]; break;
        case V4L2_PIX_FMT_SRGGB12: R = bayer_data[0] >> 2; Gr = bayer_data[1] >> 2; Gb = bayer_data[2] >> 2; B = bayer_data[3] >> 2; break;
        case V4L2_PIX_FMT_SGRBG12: Gr = bayer_data[0] >> 2; R = bayer_data[1] >> 2; B = bayer_data[2] >> 2; Gb = bayer_data[3] >> 2; break;
        case V4L2_PIX_FMT_SGBRG12: Gb = bayer_data[0] >> 2; B = bayer_data[1] >> 2; R = bayer_data[2] >> 2; Gr = bayer_data[3] >> 2; break;
        case V4L2_PIX_FMT_SBGGR12: B = bayer_data[0] >> 2; Gb = bayer_data[1] >> 2; Gr = bayer_data[2] >> 2; R = bayer_data[3] >> 2; break;
        default: return;
    }

    int dstStride = CameraUtils::getStride(dst_fmt, width);
    switch(dst_fmt) {
        case V4L2_PIX_FMT_SRGGB8:
            out_buf[y * dstStride + x] = (R >> 2);
            out_buf[y * dstStride + x + 1] = (Gr >> 2);
            out_buf[(y + 1) * dstStride + x] = (Gb >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (B >> 2);
            break;
        case V4L2_PIX_FMT_SGRBG8:
            out_buf[y * dstStride + x] = (Gr >> 2);
            out_buf[y * dstStride + x + 1] = (R >> 2);
            out_buf[(y + 1) * dstStride + x] = (B >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (Gb >> 2);
            break;
        case V4L2_PIX_FMT_SGBRG8:
            out_buf[y * dstStride + x] = (Gb >> 2);
            out_buf[y * dstStride + x + 1] = (B >> 2);
            out_buf[(y + 1) * dstStride + x] = (R >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (Gr >> 2);
            break;
        case V4L2_PIX_FMT_SBGGR8:
            out_buf[y * dstStride + x] = (B >> 2);
            out_buf[y * dstStride + x + 1] = (Gb >> 2);
            out_buf[(y + 1) * dstStride + x] = (Gr >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (R >> 2);
            break;
        case V4L2_PIX_FMT_SRGGB10:
            *((unsigned short *) out_buf + y * dstStride + x) = R;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = Gr;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = Gb;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = B;
            break;
        case V4L2_PIX_FMT_SGRBG10:
            *((unsigned short *) out_buf + y * dstStride + x) = Gr;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = R;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = B;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = Gb;
            break;
        case V4L2_PIX_FMT_SGBRG10:
            *((unsigned short *) out_buf + y * dstStride + x) = Gb;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = B;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = R;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = Gr;
            break;
        case V4L2_PIX_FMT_SBGGR10:
            *((unsigned short *) out_buf + y * dstStride + x) = B;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = Gb;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = Gr;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = R;
            break;
        case V4L2_PIX_FMT_NV12:
            Ybase = out_buf;
            UVbase = Ybase + dstStride * height;
            RGB2YUV(R, (Gr + Gb) / 2, B, &Y, &U, &V);
            Ybase[y * dstStride + x] = Ybase[y * dstStride + x + 1] =
                Ybase[(y + 1) * dstStride + x] = Ybase[(y + 1) * dstStride + x + 1] = Y;
            UVbase[y / 2 * dstStride + x / 2 * 2] = U;
            UVbase[y / 2 * dstStride + x / 2 * 2 + 1] = V;
            break;
        case V4L2_PIX_FMT_UYVY:
            RGB2YUV(R, (Gr + Gb) / 2, B, &Y, &U, &V);
            out_buf[y * dstStride + x * 2] = U;
            out_buf[y * dstStride + x * 2 + 1] = Y;
            out_buf[y * dstStride + x * 2 + 2] = V;
            out_buf[y * dstStride + x * 2 + 3] = Y;
            out_buf[(y + 1) * dstStride + x * 2] = U;
            out_buf[(y + 1) * dstStride + x * 2 + 1] = Y;
            out_buf[(y + 1) * dstStride + x * 2 + 2] = V;
            out_buf[(y + 1) * dstStride + x * 2 + 3] = Y;
            break;
        case V4L2_PIX_FMT_YUYV:
            RGB2YUV(R, (Gr + Gb) / 2, B, &Y, &U, &V);
            out_buf[y * dstStride + x * 2] = Y;
            out_buf[y * dstStride + x * 2 + 1] = U;
            out_buf[y * dstStride + x * 2 + 2] = Y;
            out_buf[y * dstStride + x * 2 + 3] = V;
            out_buf[(y + 1) * dstStride + x * 2] = Y;
            out_buf[(y + 1) * dstStride + x * 2 + 1] = U;
            out_buf[(y + 1) * dstStride + x * 2 + 2] = Y;
            out_buf[(y + 1) * dstStride + x * 2 + 3] = V;
            break;
        case V4L2_PIX_FMT_YUV420:
        {
            RGB2YUV(R, (Gr + Gb) / 2, B, &Y, &U, &V);
            Ybase = out_buf;
            uint8_t* UBase = out_buf + dstStride * height;
            uint8_t* VBase = out_buf + dstStride * (height + height / 4);
            Ybase[y * dstStride + x] = Y;
            Ybase[y * dstStride + x + 1] = Y;
            Ybase[(y + 1) * dstStride + x] = Y;
            Ybase[(y + 1) * dstStride + x + 1] = Y;
            if (y % 4 == 0) {
                UBase[y / 4 * dstStride + x / 2] = U;
                VBase[y / 4 * dstStride + x / 2] = V;
            } else {
                UBase[y / 4 * dstStride + width / 2 + x / 2] = U;
                VBase[y / 4 * dstStride + width / 2 + x / 2] = V;
            }
            break;
        }
        default:
            break;
    }
}

void SwImageConverter::convertYuvBlock(unsigned int x, unsigned int y,
    unsigned int width, unsigned int height,
    unsigned char *in_buf, unsigned char *out_buf,
    unsigned int src_fmt, unsigned int dst_fmt)
{
    unsigned char *YBase;
    unsigned char *UVBase;
    unsigned char Y[4];
    unsigned char U[4];
    unsigned char V[4];
    unsigned short R, G, B;
    int srcStride = CameraUtils::getStride(src_fmt, width);

    switch(src_fmt) {
        case V4L2_PIX_FMT_NV12:
            YBase = in_buf;
            UVBase = in_buf + srcStride * height;
            Y[0] = YBase[y * srcStride + x];
            Y[1] = YBase[y * srcStride + x + 1];
            Y[2] = YBase[(y + 1) * srcStride + x];
            Y[3] = YBase[(y + 1) * srcStride + x + 1];
            U[0] = U[1] = U[2] = U[3] = UVBase[y / 2 * srcStride + x / 2 * 2];
            V[0] = V[1] = V[2] = V[3] = UVBase[y / 2 * srcStride + x / 2 * 2 + 1];
            break;
        case V4L2_PIX_FMT_UYVY:
            Y[0] = in_buf[y * srcStride + x * 2 + 1];
            Y[1] = in_buf[y * srcStride + x * 2 + 3];
            Y[2] = in_buf[(y + 1) * srcStride + x * 2 + 1];
            Y[3] = in_buf[(y + 1) * srcStride + x * 2 + 3];
            U[0] = U[1] = in_buf[y * srcStride + x * 2];
            U[2] = U[3] = in_buf[(y + 1) * srcStride + x * 2];
            V[0] = V[1] = in_buf[y * srcStride + x * 2 + 2];
            V[2] = V[3] = in_buf[(y + 1) * srcStride + x * 2 + 2];
            break;
        case V4L2_PIX_FMT_YUYV:
            Y[0] = in_buf[y * srcStride + x * 2];
            Y[1] = in_buf[y * srcStride + x * 2 + 2];
            Y[2] = in_buf[(y + 1) * srcStride + x * 2];
            Y[3] = in_buf[(y + 1) * srcStride + x * 2 + 2];
            U[0] = U[1] = in_buf[y * srcStride + x * 2 + 1];
            U[2] = U[3] = in_buf[(y + 1) * srcStride + x * 2 + 1];
            V[0] = V[1] = in_buf[y * srcStride + x * 2 + 3];
            V[2] = V[3] = in_buf[(y + 1) * srcStride + x * 2 + 3];
            break;
        default:
            return;
    }

    int dstStride = CameraUtils::getStride(dst_fmt, width);
    switch(dst_fmt) {
        case V4L2_PIX_FMT_NV12:
            YBase = out_buf;
            UVBase = out_buf + dstStride * height;
            YBase[y * dstStride + x] = Y[0];
            YBase[y * dstStride + x + 1] = Y[1];
            YBase[(y + 1) * dstStride + x] = Y[2];
            YBase[(y + 1) * dstStride + x + 1] = Y[3];
            UVBase[y / 2 * dstStride + x / 2 * 2] = U[0];
            UVBase[y / 2 * dstStride + x / 2 * 2 + 1] = V[0];
            break;
        case V4L2_PIX_FMT_UYVY:
            out_buf[y * dstStride + x * 2 + 1] = Y[0];
            out_buf[y * dstStride + x * 2 + 3] = Y[1];
            out_buf[(y + 1) * dstStride + x * 2 + 1] = Y[2];
            out_buf[(y + 1) * dstStride + x * 2 + 3] = Y[3];
            out_buf[y * dstStride + x * 2] = U[0];
            out_buf[(y + 1) * dstStride + x * 2] = U[2];
            out_buf[y * dstStride + x * 2 + 2] = V[0];
            out_buf[(y + 1) * dstStride + x * 2 + 2] = V[2];
            break;
        case V4L2_PIX_FMT_YUYV:
            out_buf[y * dstStride + x * 2] = Y[0];
            out_buf[y * dstStride + x * 2 + 2] = Y[1];
            out_buf[(y + 1) * dstStride + x * 2] = Y[2];
            out_buf[(y + 1) * dstStride + x * 2 + 2] = Y[3];
            out_buf[y * dstStride + x * 2 + 1] = U[0];
            out_buf[(y + 1) * dstStride + x * 2 + 1] = U[2];
            out_buf[y * dstStride + x * 2 + 3] = V[0];
            out_buf[(y + 1) * dstStride + x * 2 + 3] = V[2];
            break;
        case V4L2_PIX_FMT_YUV420:
        {
            YBase = out_buf;
            uint8_t* UBase = out_buf + dstStride * height;
            uint8_t* VBase = out_buf + dstStride * (height + height / 4);
            YBase[y * dstStride + x] = Y[0];
            YBase[y * dstStride + x + 1] = Y[1];
            YBase[(y + 1) * dstStride + x] = Y[2];
            YBase[(y + 1) * dstStride + x + 1] = Y[3];
            if (y % 4 == 0) {
                UBase[y / 4 * dstStride + x / 2] = (U[0] + U[2]) / 2;
                VBase[y / 4 * dstStride + x / 2] = (V[0] + V[2]) / 2;
            } else {
                UBase[y / 4 * dstStride + width / 2 + x / 2] = (U[0] + U[2]) / 2;
                VBase[y / 4 * dstStride + width / 2 + x / 2] = (V[0] + V[2]) / 2;
            }
            break;
        }
        case V4L2_PIX_FMT_SRGGB8:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            out_buf[y * dstStride + x] = (R >> 2);
            out_buf[y * dstStride + x + 1] = (G >> 2);
            out_buf[(y + 1) * dstStride + x] = (G >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (B >> 2);
            break;
        case V4L2_PIX_FMT_SGRBG8:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            out_buf[y * dstStride + x] = (G >> 2);
            out_buf[y * dstStride + x + 1] = (R >> 2);
            out_buf[(y + 1) * dstStride + x] = (B >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (G >> 2);
            break;
        case V4L2_PIX_FMT_SGBRG8:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            out_buf[y * dstStride + x] = (G >> 2);
            out_buf[y * dstStride + x + 1] = (B >> 2);
            out_buf[(y + 1) * dstStride + x] = (R >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (G >> 2);
            break;
        case V4L2_PIX_FMT_SBGGR8:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            out_buf[y * dstStride + x] = (B >> 2);
            out_buf[y * dstStride + x + 1] = (G >> 2);
            out_buf[(y + 1) * dstStride + x] = (G >> 2);
            out_buf[(y + 1) * dstStride + x + 1] = (R >> 2);
            break;
        case V4L2_PIX_FMT_SRGGB10:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            *((unsigned short *) out_buf + y * dstStride + x) = R;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = G;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = G;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = B;
            break;
        case V4L2_PIX_FMT_SGRBG10:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            *((unsigned short *) out_buf + y * dstStride + x) = G;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = R;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = B;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = G;
            break;
        case V4L2_PIX_FMT_SGBRG10:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            *((unsigned short *) out_buf + y * dstStride + x) = G;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = B;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = R;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = G;
            break;
        case V4L2_PIX_FMT_SBGGR10:
            YUV2RGB(Y[0], U[0], V[0], &R, &G, &B);
            *((unsigned short *) out_buf + y * dstStride + x) = B;
            *((unsigned short *) out_buf + y * dstStride + x + 1) = G;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x) = G;
            *((unsigned short *) out_buf + (y + 1) * dstStride + x + 1) = R;
            break;
        default:
            return;
    }
}

int SwImageConverter::convertFormat(unsigned int width, unsigned int height,
                        unsigned char *inBuf, unsigned int inLength, unsigned int srcFmt,
                        unsigned char *outBuf, unsigned int outLength, unsigned int dstFmt)
{
    CheckError((inBuf == nullptr || outBuf == nullptr), BAD_VALUE, "Invalid input(%p) or output buffer(%p)", inBuf, outBuf);

    unsigned int x, y;
    unsigned short bayer_data[4];

    LOG2("%s srcFmt %s => dstFmt %s %dx%d", __func__,
         CameraUtils::format2string(srcFmt).c_str(),
         CameraUtils::format2string(dstFmt).c_str(), width, height);

    if (dstFmt == srcFmt) {
        // No need do format convertion.
        LOG2("No conversion needed");
        MEMCPY_S(outBuf, outLength, inBuf, inLength);
        return 0;
    }

    // for not vector raw
    int srcStride = CameraUtils::getStride(srcFmt, width);
    for(y = 0; y < height; y += 2) {
        for(x = 0; x < width; x += 2) {
            if(CameraUtils::isRaw(srcFmt)) {
                if(CameraUtils::getBpp(srcFmt) == 8) {
                    bayer_data[0] = inBuf[y * srcStride + x];
                    bayer_data[1] = inBuf[y * srcStride + x + 1];
                    bayer_data[2] = inBuf[(y + 1) * srcStride + x];
                    bayer_data[3] = inBuf[(y + 1) * srcStride + x + 1];
                } else {
                    int offset = srcStride / (CameraUtils::getBpp(srcFmt) / 8);
                    bayer_data[0] = *((unsigned short *) inBuf + y * offset + x);
                    bayer_data[1] = *((unsigned short *) inBuf + y * offset + x + 1);
                    bayer_data[2] = *((unsigned short *) inBuf + (y + 1) * offset + x);
                    bayer_data[3] =
                        *((unsigned short *) inBuf + (y + 1) * offset + x + 1);
                }
                convertBayerBlock(x, y, width, height, bayer_data, outBuf, srcFmt, dstFmt);
            } else {
                convertYuvBlock(x, y, width, height, inBuf, outBuf, srcFmt, dstFmt);
            }
        }
    }
    return 0;
}

} //namespace icamera
