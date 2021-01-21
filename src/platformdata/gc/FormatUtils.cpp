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

#define LOG_TAG "FormatUtils"

#include <stdint.h>
#include <math.h>
#include <linux/v4l2-mediabus.h>
#include <linux/ipu-isys.h>
#include "ia_cipf/ia_cipf_types.h"
#include "FormatUtils.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using std::string;
using namespace icamera;

/**
 * Utilities to query information about V4L2 types in graph config
 */

namespace graphconfig {
namespace utils {

enum FormatType {
    FORMAT_RAW,
    FORMAT_RAW_VEC,
    FORMAT_YUV,
    FORMAT_YUV_VEC,
    FORMAT_RGB,
    FORMAT_MBUS_BAYER,
    FORMAT_MBUS_YUV,
    FORMAT_JPEG,
    FORMAT_FOURCC
};

struct FormatInfo {
    int32_t pixelCode;  // OS specific pixel code, in this case V4L2 or Media bus
    int32_t commonPixelCode;  // Common pixel code used by CIPF and GCSS in settings
    string fullName;
    string shortName;
    int32_t bpp;
    FormatType type;
};

/**
 * gFormatMapping
 *
 * Table for mapping OS agnostic formats defined in CIPF and OS specific ones
 * (in this case V4L2, or media bus).
 * The table also helps provide textual representation and bits per pixel.
 * CIPF does not define most of the formats, only the ones it needs, that is why
 * most of the entries have 0 on the common pixel format.
 * Conversely there are some new formats introduce by CIPF that do not have
 * V4L2 representation.
 */
static const FormatInfo gFormatMapping[] = {
    { V4L2_PIX_FMT_SBGGR8, 0, "V4L2_PIX_FMT_SBGGR8", "BGGR8", 8, FORMAT_RAW },
    { V4L2_PIX_FMT_SGBRG8, 0, "V4L2_PIX_FMT_SGBRG8", "GBRG8", 8, FORMAT_RAW },
    { V4L2_PIX_FMT_SRGGB8, 0, "V4L2_PIX_FMT_SRGGB8", "RGGB8", 8, FORMAT_RAW },

    { V4L2_PIX_FMT_SGRBG8, ia_cipf_frame_fourcc_grbg, "V4L2_PIX_FMT_SGRBG8", "GRBG8", 8, FORMAT_RAW },
    { V4L2_PIX_FMT_SBGGR10, ia_cipf_frame_fourcc_bg10, "V4L2_PIX_FMT_SBGGR10", "BGGR10", 16, FORMAT_RAW },
    { V4L2_PIX_FMT_SGBRG10, ia_cipf_frame_fourcc_gb10, "V4L2_PIX_FMT_SGBRG10", "GBRG10", 16, FORMAT_RAW },
    { V4L2_PIX_FMT_SGRBG10, ia_cipf_frame_fourcc_ba10, "V4L2_PIX_FMT_SGRBG10", "GRBG10", 16, FORMAT_RAW },
    { V4L2_PIX_FMT_SRGGB10, ia_cipf_frame_fourcc_rg10, "V4L2_PIX_FMT_SRGGB10", "RGGB10", 16, FORMAT_RAW },
    // align to xos definition and css format: IA_CSS_DATA_FORMAT_BAYER_GRBG
    // it's different with ia_cipf_frame_fourcc_ba10 which align to css format:
    // IA_CSS_DATA_FORMAT_RAW
    { V4L2_PIX_FMT_SGRBG10, ia_cipf_frame_fourcc_gr10, "V4L2_PIX_FMT_SGRBG10", "GRBG10", 16, FORMAT_RAW },
    { 0, css_fourcc_grbg_12_li, "css_fourcc_grbg_12_li", "CSL6", 15, FORMAT_RAW},
    { V4L2_PIX_FMT_SGRBG12, ia_cipf_frame_fourcc_ba12, "V4L2_PIX_FMT_SGRBG12", "GRBG12", 16, FORMAT_RAW },

    { V4L2_PIX_FMT_NV12, ia_cipf_frame_fourcc_nv12, "V4L2_PIX_FMT_NV12", "NV12", 12, FORMAT_YUV },
    { V4L2_PIX_FMT_NV21, ia_cipf_frame_fourcc_nv21, "V4L2_PIX_FMT_NV21", "NV21", 12, FORMAT_YUV },
    { V4L2_PIX_FMT_UYVY, ia_cipf_frame_fourcc_uyvy, "V4L2_PIX_FMT_UYVY", "UYVY", 16, FORMAT_YUV },

    { V4L2_PIX_FMT_YUV420, ia_cipf_frame_fourcc_iyuv, "YUV420_8_PL", "YUV420", 12, FORMAT_YUV },
    // Packed formats No V4L2 equivalent exists
    // Normal YUV420 planar but with each sample of 12bits stored in 16bits
    { 0, ia_cipf_frame_fourcc_i420, "YUV420-12-16p", "YUV420", 24, FORMAT_YUV },
    // Normal YUV420 planar but with each sample of 12-bit DMA-packed
    // (42 pixels and 8 bits of padding in a 64-byte DMA word)
    { 0, css_fourcc_yuv420_12_p64, "YUV420-12-64p", "YUV420", 18, FORMAT_YUV },
    //  Normal YUV420 planar but with each sample of 10-bit DMA-packed
    // (51 pixels and 2 bits of padding in a 64-byte DMA word)
    { 0, css_fourcc_yuv420_10_p64, "YUV420-10-64p", "YUV420", 15, FORMAT_YUV },

    { V4L2_MBUS_FMT_SBGGR10_1X10, ia_cipf_frame_fourcc_bg10, "V4L2_MBUS_FMT_SBGGR10_1X10", "SBGGR10_1X10", 10, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGBRG10_1X10, ia_cipf_frame_fourcc_gb10, "V4L2_MBUS_FMT_SGBRG10_1X10", "SGBRG10_1X10", 10, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGRBG10_1X10, ia_cipf_frame_fourcc_ba10, "V4L2_MBUS_FMT_SGRBG10_1X10", "SGRBG10_1X10", 10, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SRGGB10_1X10, ia_cipf_frame_fourcc_rg10, "V4L2_MBUS_FMT_SRGGB10_1X10", "SRGGB10_1X10", 10, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_UYVY8_1X16, ia_cipf_frame_fourcc_uyvy, "V4L2_MBUS_FMT_UYVY8_1X16", "UYVY8_1X16", 16, FORMAT_MBUS_YUV },
    { V4L2_PIX_FMT_SBGGR12, 0, "V4L2_PIX_FMT_SBGGR12", "BGGR12", 16, FORMAT_RAW },
    { V4L2_PIX_FMT_SGBRG12, 0, "V4L2_PIX_FMT_SGBRG12", "GBRG12", 16, FORMAT_RAW },
    { V4L2_PIX_FMT_SRGGB12, 0, "V4L2_PIX_FMT_SRGGB12", "RGGB12", 16, FORMAT_RAW },

    { V4L2_PIX_FMT_SBGGR10P, 0, "V4L2_PIX_FMT_SBGGR10P", "BGGR10P", 10, FORMAT_RAW },
    { V4L2_PIX_FMT_SGBRG10P, 0, "V4L2_PIX_FMT_SGBRG10P", "GBRG10P", 10, FORMAT_RAW },
    { V4L2_PIX_FMT_SGRBG10P, 0, "V4L2_PIX_FMT_SGRBG10P", "GRBG10P", 10, FORMAT_RAW },
    { V4L2_PIX_FMT_SRGGB10P, 0, "V4L2_PIX_FMT_SRGGB10P", "RGGB10P", 10, FORMAT_RAW },

    { V4L2_PIX_FMT_NV16, 0, "V4L2_PIX_FMT_NV16", "NV16", 16, FORMAT_YUV },
    { V4L2_PIX_FMT_YUYV, ia_cipf_frame_fourcc_yuy2, "V4L2_PIX_FMT_YUYV", "YUYV", 16, FORMAT_YUV },
    { V4L2_PIX_FMT_YVU420, 0, "V4L2_PIX_FMT_YVU420", "YVU420", 12, FORMAT_YUV },
    { V4L2_PIX_FMT_YUV422P, 0, "V4L2_PIX_FMT_YUV422P", "YUV422P", 16, FORMAT_YUV },

    { V4L2_PIX_FMT_BGR24, 0, "V4L2_PIX_FMT_BGR24", "BGR24", 24, FORMAT_RGB },
    { V4L2_PIX_FMT_BGR32, 0, "V4L2_PIX_FMT_BGR32", "BGR32", 32, FORMAT_RGB },
    { V4L2_PIX_FMT_XBGR32, 0, "V4L2_PIX_FMT_XBGR32", "XBGR32", 32, FORMAT_RGB },
    { V4L2_PIX_FMT_XRGB32, 0, "V4L2_PIX_FMT_XRGB32", "XRGB32", 32, FORMAT_RGB },
    { V4L2_PIX_FMT_RGB565, 0, "V4L2_PIX_FMT_RGB565", "RGB565", 16, FORMAT_RGB },

    { V4L2_PIX_FMT_JPEG, 0, "V4L2_PIX_FMT_JPEG", "JPEG", 0, FORMAT_JPEG },

    { V4L2_MBUS_FMT_SBGGR12_1X12, 0, "V4L2_MBUS_FMT_SBGGR12_1X12", "SBGGR12_1X12", 12, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGBRG12_1X12, 0, "V4L2_MBUS_FMT_SGBRG12_1X12", "SGBRG12_1X12", 12, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGRBG12_1X12, 0, "V4L2_MBUS_FMT_SGRBG12_1X12", "SGRBG12_1X12", 12, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SRGGB12_1X12, 0, "V4L2_MBUS_FMT_SRGGB12_1X12", "SRGGB12_1X12", 12, FORMAT_MBUS_BAYER },

    { V4L2_MBUS_FMT_SBGGR8_1X8, 0, "V4L2_MBUS_FMT_SBGGR8_1X8", "SBGGR8_1X8", 8, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGBRG8_1X8, 0, "V4L2_MBUS_FMT_SGBRG8_1X8", "SGBRG8_1X8", 8, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SGRBG8_1X8, 0, "V4L2_MBUS_FMT_SGRBG8_1X8", "SGRBG8_1X8", 8, FORMAT_MBUS_BAYER },
    { V4L2_MBUS_FMT_SRGGB8_1X8, 0, "V4L2_MBUS_FMT_SRGGB8_1X8", "SRGGB8_1X8", 8, FORMAT_MBUS_BAYER },

    { V4L2_MBUS_FMT_YUYV8_1X16, 0, "V4L2_MBUS_FMT_YUYV8_1X16", "YUYV8_1X16", 16, FORMAT_MBUS_YUV },
    { V4L2_MBUS_FMT_UYVY8_2X8, 0, "V4L2_MBUS_FMT_UYVY8_2X8","UYVY8_2X8", 8, FORMAT_MBUS_YUV},

};

const string pixelCode2String(int32_t code)
{

    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].pixelCode == code) {
            return gFormatMapping[i].fullName;
        }
    }
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].commonPixelCode == code) {
            return gFormatMapping[i].fullName;
        }
    }

    LOGE("Invalid Pixel Format: 0x%x", code);
    return "INVALID FORMAT";
}

int32_t string2PixelCode(const string &code)
{
    if (code.empty()) {
        LOGE("Invalid Pixel Format: %s", code.c_str());
        return -1;
    }

    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].fullName == code) {
            return gFormatMapping[i].pixelCode;
        }
    }

    LOGE("Invalid Pixel Format: %s", code.c_str());
    return -1;
}

const string format2string(int32_t format)
{
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].pixelCode == format) {
            return gFormatMapping[i].shortName;
        }
    }

    LOGW("Not in our format list :%x", format);
    return "INVALID-FORMAT";
}

bool isPlanarFormat(int32_t format)
{
    return (format == V4L2_PIX_FMT_NV12 ||
            format == V4L2_PIX_FMT_NV21 ||
            format == V4L2_PIX_FMT_YUV420 ||
            format == V4L2_PIX_FMT_YVU420 ||
            format == V4L2_PIX_FMT_YUV422P);
}

bool isRaw(int32_t format)
{
    int32_t size = ARRAY_SIZE(gFormatMapping);
    for (int32_t i = 0; i < size; i++) {
        if (gFormatMapping[i].pixelCode == format) {
            // Both normal raw and vector raw treated as raw here.
            return gFormatMapping[i].type == FORMAT_RAW_VEC || gFormatMapping[i].type == FORMAT_RAW;
        }
    }

    return false;
}

bool isVectorizedRaw(int32_t format)
{
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].pixelCode == format) {
            return gFormatMapping[i].type == FORMAT_RAW_VEC;
        }
    }

    return false;
}

/**
 * Calculate bytes per line(bpl) based on fourcc format.
 *
 * \param[in] format 4CC code in OS specific format
 * \return bpl bytes per line
 */
int32_t getBpl(int32_t format, int32_t width)
{
    int32_t bpl = 0;
    switch (format) {
        case css_fourcc_yuv420_12_p64:      // YUV
            /*
             * Align based on UV planes, which have half the strides compared to y plane.
             * 42 whole pixels in each 64 byte word, rest 8 bits per word is padding.
             * The total bpl is double the UV-plane strides.
             */
            bpl = ceil(((double)width / 2) / 42) * 64 * 2;
            break;
        case css_fourcc_yyuv420_v32:        // Y032
            bpl = width * 6;
            break;
        case css_fourcc_grbg_12_li:            // CSL6
            bpl = width * 4;
            break;
        case css_fourcc_grbg_10_v32:        // BV0G
        case ia_cipf_frame_fourcc_i420:     // V420
        case css_fourcc_raw_interleaved:    // BV0K
        case ia_cipf_frame_fourcc_ba10:     // BA10
        case ia_cipf_frame_fourcc_gr10:     // GR10
        case ia_cipf_frame_fourcc_ba12:
        case css_fourcc_p010:               //YUV 10bit serial
        case css_fourcc_p010_lsb:
        case css_fourcc_p010_msb_tile_y:
        case css_fourcc_p010_msb_cile_y:
        case ia_cipf_frame_fourcc_yuy2:
            bpl = width * 2;
            break;
        case ia_cipf_frame_fourcc_nv12:     // NV12
        case ia_cipf_frame_fourcc_grbg:     // GRBG
            bpl = width;
            break;
        default:
            bpl = width;
            LOGW("bpl defaulting to width for format:%s", CameraUtils::format2string(format).c_str());
            break;
    }
    return bpl;
}

/**
 *  Retrieve the bits per pixel  from the OS specific pixel code.
 *  This is ususally used for buffer allocation calculations
 *
 *  \param [in] format 4CC code in OS specific format
 *  \return bits per pixel
 */
int32_t getBpp(int32_t format)
{
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].pixelCode == format) {
            return gFormatMapping[i].bpp;
        }
    }

    LOGE("There is no bpp supplied for format %s",
            pixelCode2String(format).c_str());
    return -1;
}

/**
 *  Retrieve the bits per pixel from the common pixel code format (CIPF)
 *  This is usually used for buffer allocation calculations
 *
 *  \param [in] format 4CC code in Common format
 *  \return bits per pixel
 */
int32_t getBppFromCommon(int32_t format)
{
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].commonPixelCode == format) {
            return gFormatMapping[i].bpp;
        }
    }

    LOGE("There is no bpp supplied for format %s",
            pixelCode2String(format).c_str());
    return -1;
}

int32_t getNumOfPlanes(int32_t format)
{
    switch(format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_FMT_IPU_ISYS_META:
            return 1;
        //Add more when needed...
        default:
            return 1;
    }
}

int32_t getV4L2Format(const int32_t commonPixelFormat)
{
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].commonPixelCode == commonPixelFormat)
            return gFormatMapping[i].pixelCode;
    }

    LOGE("Failed to find any V4L2 format with format %s",
            pixelCode2String(commonPixelFormat).c_str());
    return -1;
}

} // namespace utils
} // namespace graphconfig
