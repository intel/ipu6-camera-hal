/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

#define LOG_TAG Utils

#include "iutils/Utils.h"

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "linux/ipu-isys.h"
#include "linux/media-bus-format.h"

using std::string;

namespace icamera {

int CameraUtils::getFileContent(const char* filename, char* buffer, int maxSize) {
    std::ifstream stream(filename);

    stream.seekg(0, std::ios::end);
    long copyLength = stream.tellg();
    stream.seekg(0, std::ios::beg);

    if (copyLength > maxSize) {
        copyLength = maxSize;
    }

    stream.read(buffer, copyLength);
    return copyLength;
}

#define GET_FOURCC_FMT(a, b, c, d) \
    ((uint32_t)(d) | ((uint32_t)(c) << 8) | ((uint32_t)(b) << 16) | ((uint32_t)(a) << 24))

enum FormatType {
    FORMAT_RAW,
    FORMAT_RAW_VEC,
    FORMAT_YUV,
    FORMAT_YUV_VEC,
    FORMAT_RGB,
    FORMAT_MBUS,
    FORMAT_JPEG,
    FORMAT_FOURCC,
};

struct FormatInfo {
    int v4l2Fmt;
    int iaFourcc;
    const char* fullName;
    const char* shortName;
    int bpp;
    FormatType type;
};

static const FormatInfo gFormatMapping[] = {
    {V4L2_PIX_FMT_GREY, 0, "V4L2_PIX_FMT_GREY", "GREY", 8, FORMAT_RAW},

    {V4L2_PIX_FMT_SBGGR8, 0, "V4L2_PIX_FMT_SBGGR8", "BGGR8", 8, FORMAT_RAW},
    {V4L2_PIX_FMT_SGBRG8, 0, "V4L2_PIX_FMT_SGBRG8", "GBRG8", 8, FORMAT_RAW},
    {V4L2_PIX_FMT_SGRBG8, 0, "V4L2_PIX_FMT_SGRBG8", "GRBG8", 8, FORMAT_RAW},
    {V4L2_PIX_FMT_SRGGB8, 0, "V4L2_PIX_FMT_SRGGB8", "RGGB8", 8, FORMAT_RAW},

    {V4L2_PIX_FMT_SBGGR10, 0, "V4L2_PIX_FMT_SBGGR10", "BGGR10", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SGBRG10, 0, "V4L2_PIX_FMT_SGBRG10", "GBRG10", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SGRBG10, 0, "V4L2_PIX_FMT_SGRBG10", "GRBG10", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SRGGB10, 0, "V4L2_PIX_FMT_SRGGB10", "RGGB10", 16, FORMAT_RAW},

    {V4L2_PIX_FMT_SBGGR12, 0, "V4L2_PIX_FMT_SBGGR12", "BGGR12", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SGBRG12, 0, "V4L2_PIX_FMT_SGBRG12", "GBRG12", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SGRBG12, 0, "V4L2_PIX_FMT_SGRBG12", "GRBG12", 16, FORMAT_RAW},
    {V4L2_PIX_FMT_SRGGB12, 0, "V4L2_PIX_FMT_SRGGB12", "RGGB12", 16, FORMAT_RAW},

    {V4L2_PIX_FMT_SBGGR10P, 0, "V4L2_PIX_FMT_SBGGR10P", "BGGR10P", 10, FORMAT_RAW},
    {V4L2_PIX_FMT_SGBRG10P, 0, "V4L2_PIX_FMT_SGBRG10P", "GBRG10P", 10, FORMAT_RAW},
    {V4L2_PIX_FMT_SGRBG10P, 0, "V4L2_PIX_FMT_SGRBG10P", "GRBG10P", 10, FORMAT_RAW},
    {V4L2_PIX_FMT_SRGGB10P, 0, "V4L2_PIX_FMT_SRGGB10P", "RGGB10P", 10, FORMAT_RAW},

    {V4L2_PIX_FMT_NV12, 0, "V4L2_PIX_FMT_NV12", "NV12", 12, FORMAT_YUV},
    {V4L2_PIX_FMT_NV21, 0, "V4L2_PIX_FMT_NV21", "NV21", 12, FORMAT_YUV},
    {V4L2_PIX_FMT_NV16, 0, "V4L2_PIX_FMT_NV16", "NV16", 16, FORMAT_YUV},
    {V4L2_PIX_FMT_YUYV, 0, "V4L2_PIX_FMT_YUYV", "YUYV", 16, FORMAT_YUV},
    {V4L2_PIX_FMT_UYVY, 0, "V4L2_PIX_FMT_UYVY", "UYVY", 16, FORMAT_YUV},
    {V4L2_PIX_FMT_P010, 0, "V4L2_PIX_FMT_P010", "P010", 24, FORMAT_YUV},

    {V4L2_PIX_FMT_YUV420, 0, "V4L2_PIX_FMT_YUV420", "YUV420", 12, FORMAT_YUV},
    {V4L2_PIX_FMT_YVU420, 0, "V4L2_PIX_FMT_YVU420", "YVU420", 12, FORMAT_YUV},
    {V4L2_PIX_FMT_YUV422P, 0, "V4L2_PIX_FMT_YUV422P", "YUV422P", 16, FORMAT_YUV},

    {V4L2_PIX_FMT_BGR24, 0, "V4L2_PIX_FMT_BGR24", "BGR24", 24, FORMAT_RGB},
    {V4L2_PIX_FMT_BGR32, 0, "V4L2_PIX_FMT_BGR32", "BGR32", 32, FORMAT_RGB},
    {V4L2_PIX_FMT_RGB24, 0, "V4L2_PIX_FMT_RGB24", "RGB24", 24, FORMAT_RGB},
    {V4L2_PIX_FMT_RGB32, 0, "V4L2_PIX_FMT_RGB32", "RGB32", 32, FORMAT_RGB},
    {V4L2_PIX_FMT_XBGR32, 0, "V4L2_PIX_FMT_XBGR32", "XBGR32", 32, FORMAT_RGB},
    {V4L2_PIX_FMT_XRGB32, 0, "V4L2_PIX_FMT_XRGB32", "XRGB32", 32, FORMAT_RGB},
    {V4L2_PIX_FMT_RGB565, 0, "V4L2_PIX_FMT_RGB565", "RGB565", 16, FORMAT_RGB},

    {V4L2_PIX_FMT_JPEG, 0, "V4L2_PIX_FMT_JPEG", "JPG", 0, FORMAT_JPEG},

    {V4L2_MBUS_FMT_SBGGR12_1X12, 0, "V4L2_MBUS_FMT_SBGGR12_1X12", "SBGGR12_1X12", 12, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGBRG12_1X12, 0, "V4L2_MBUS_FMT_SGBRG12_1X12", "SGBRG12_1X12", 12, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGRBG12_1X12, 0, "V4L2_MBUS_FMT_SGRBG12_1X12", "SGRBG12_1X12", 12, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SRGGB12_1X12, 0, "V4L2_MBUS_FMT_SRGGB12_1X12", "SRGGB12_1X12", 12, FORMAT_MBUS},

    {V4L2_MBUS_FMT_SBGGR10_1X10, 0, "V4L2_MBUS_FMT_SBGGR10_1X10", "SBGGR10_1X10", 10, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGBRG10_1X10, 0, "V4L2_MBUS_FMT_SGBRG10_1X10", "SGBRG10_1X10", 10, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGRBG10_1X10, 0, "V4L2_MBUS_FMT_SGRBG10_1X10", "SGRBG10_1X10", 10, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SRGGB10_1X10, 0, "V4L2_MBUS_FMT_SRGGB10_1X10", "SRGGB10_1X10", 10, FORMAT_MBUS},

    {V4L2_MBUS_FMT_SBGGR8_1X8, 0, "V4L2_MBUS_FMT_SBGGR8_1X8", "SBGGR8_1X8", 8, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGBRG8_1X8, 0, "V4L2_MBUS_FMT_SGBRG8_1X8", "SGBRG8_1X8", 8, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SGRBG8_1X8, 0, "V4L2_MBUS_FMT_SGRBG8_1X8", "SGRBG8_1X8", 8, FORMAT_MBUS},
    {V4L2_MBUS_FMT_SRGGB8_1X8, 0, "V4L2_MBUS_FMT_SRGGB8_1X8", "SRGGB8_1X8", 8, FORMAT_MBUS},

    {V4L2_MBUS_FMT_UYVY8_1X16, 0, "V4L2_MBUS_FMT_UYVY8_1X16", "UYVY8_1X16", 16, FORMAT_MBUS},
    {V4L2_MBUS_FMT_YUYV8_1X16, 0, "V4L2_MBUS_FMT_YUYV8_1X16", "YUYV8_1X16", 16, FORMAT_MBUS},
    {V4L2_MBUS_FMT_UYVY8_2X8, 0, "V4L2_MBUS_FMT_UYVY8_2X8", "UYVY8_2X8", 8, FORMAT_MBUS},

    {MEDIA_BUS_FMT_RGB888_1X24, 0, "MEDIA_BUS_FMT_RGB888_1X24", "RGB888_1X24", 0, FORMAT_MBUS},
    {MEDIA_BUS_FMT_RGB565_1X16, 0, "MEDIA_BUS_FMT_RGB565_1X16", "RGB565_1X16", 0, FORMAT_MBUS},
    {MEDIA_BUS_FMT_YUYV12_1X24, 0, "MEDIA_BUS_FMT_YUYV12_1X24", "YUYV12_1X24", 0, FORMAT_MBUS},
    {MEDIA_BUS_FMT_SGRBG10_1X10, 0, "MEDIA_BUS_FMT_SGRBG10_1X10", "SGRBG10_1X10", 0, FORMAT_MBUS},

    {MEDIA_BUS_FMT_RGB888_1X32_PADHI, 0, "MEDIA_BUS_FMT_RGB888_1X32_PADHI", "RGB888_1X32_PADHI", 0,
     FORMAT_MBUS},

    {V4L2_FMT_IPU_ISYS_META, 0, "V4L2_FMT_IPU_ISYS_META", "META_DATA", 0, FORMAT_MBUS},

    {V4L2_PIX_FMT_YUYV420_V32, GET_FOURCC_FMT('y', '0', '3', '2'), "y032", "y032", 24,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_NV12, GET_FOURCC_FMT('N', 'V', '1', '2'), "YUV420_8_SP", "NV12", 12,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_NV12, GET_FOURCC_FMT('N', 'V', '2', '1'), "YUV420_8_SP_REV", "NV21", 12,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_UYVY, GET_FOURCC_FMT('U', 'Y', 'V', 'Y'), "UYVY", "UYVY", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_YUYV, GET_FOURCC_FMT('Y', 'U', 'Y', '2'), "YUV422_8_P64", "YUYV", 16,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_YUV420, GET_FOURCC_FMT('I', 'Y', 'U', 'V'), "YUV420_8_PL", "IYUV", 12,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_P010, GET_FOURCC_FMT('P', '0', '1', '0'), "YUV420_10_SP", "P010", 24,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG12V32, GET_FOURCC_FMT('b', 'V', '0', 'K'), "bV0K", "bV0K", 16,
     FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG10V32, GET_FOURCC_FMT('b', 'V', '0', 'G'), "bV0G", "bV0G", 16,
     FORMAT_FOURCC},
    {0, GET_FOURCC_FMT('V', '4', '2', '0'), "YUV420_10_PL", "V420", 24, FORMAT_FOURCC},
    {0, GET_FOURCC_FMT('C', 'S', 'L', '6'), "GRBG_12_LI", "CSL6", 12, FORMAT_FOURCC},
    {0, GET_FOURCC_FMT('C', 'S', '4', '2'), "YUV420_12_P64", "CS42", 18, FORMAT_FOURCC},
    {0, GET_FOURCC_FMT('C', 'S', '4', '0'), "YUV420_10_P64", "CS40", 15, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG8, GET_FOURCC_FMT('G', 'R', 'B', 'G'), "GRBG", "GRBG", 8, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SBGGR10, GET_FOURCC_FMT('B', 'G', '1', '0'), "BGGR10", "BG10", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGBRG10, GET_FOURCC_FMT('G', 'B', '1', '0'), "GBRG10", "GB10", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG10, GET_FOURCC_FMT('B', 'A', '1', '0'), "GRBG10", "BA10", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SRGGB10, GET_FOURCC_FMT('R', 'G', '1', '0'), "RGGB10", "RG10", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG10, GET_FOURCC_FMT('G', 'R', '1', '0'), "GRBG10", "GR10", 16, FORMAT_FOURCC},
    {V4L2_PIX_FMT_SGRBG12, GET_FOURCC_FMT('B', 'A', '1', '2'), "GRBG12", "BA12", 16, FORMAT_FOURCC},
};

struct TuningModeStringInfo {
    TuningMode mode;
    const char* str;
};

static const TuningModeStringInfo TuningModeStringInfoTable[] = {
    {TUNING_MODE_VIDEO, "VIDEO"},
    {TUNING_MODE_VIDEO_ULL, "VIDEO-ULL"},
    {TUNING_MODE_VIDEO_CUSTOM_AIC, "VIDEO-CUSTOM_AIC"},
    {TUNING_MODE_VIDEO_LL, "VIDEO-LL"},
    {TUNING_MODE_VIDEO_REAR_VIEW, "VIDEO-REAR-VIEW"},
    {TUNING_MODE_VIDEO_HITCH_VIEW, "VIDEO-HITCH-VIEW"},
    {TUNING_MODE_STILL_CAPTURE, "STILL_CAPTURE"},
};

const char* CameraUtils::tuningMode2String(TuningMode mode) {
    int size = ARRAY_SIZE(TuningModeStringInfoTable);
    for (int i = 0; i < size; i++) {
        if (TuningModeStringInfoTable[i].mode == mode) {
            return TuningModeStringInfoTable[i].str;
        }
    }
    LOGW("Invalid TuningMode %d, use string VIDEO as default", mode);
    return TuningModeStringInfoTable[0].str;
}

TuningMode CameraUtils::string2TuningMode(const char* str) {
    int size = ARRAY_SIZE(TuningModeStringInfoTable);
    for (int i = 0; i < size; i++) {
        if (strcmp(TuningModeStringInfoTable[i].str, str) == 0) {
            return TuningModeStringInfoTable[i].mode;
        }
    }
    LOGW("Invalid TuningMode string %s, use TUNING_MODE_VIDEO as default", str);
    return TuningModeStringInfoTable[0].mode;
}

const char* CameraUtils::pixelCode2String(int code) {
    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].v4l2Fmt == code || gFormatMapping[i].iaFourcc == code) {
            return gFormatMapping[i].fullName;
        }
    }

    LOGE("Invalid Pixel Format: %d", code);
    return "INVALID FORMAT";
}

int CameraUtils::string2PixelCode(const char* code) {
    CheckAndLogError(code == nullptr, -1, "Invalid null pixel format.");

    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].type != FORMAT_FOURCC) {
            if (!strcmp(gFormatMapping[i].fullName, code) ||
                !strcmp(gFormatMapping[i].shortName, code)) {
                return gFormatMapping[i].v4l2Fmt;
            }
        }
    }

    LOGE("Invalid Pixel Format: %s", code);
    return -1;
}

int CameraUtils::string2IaFourccCode(const char* code) {
    CheckAndLogError(code == nullptr, -1, "Invalid null pixel format.");

    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].type == FORMAT_FOURCC) {
            if (!strcmp(gFormatMapping[i].fullName, code) ||
                !strcmp(gFormatMapping[i].shortName, code)) {
                return gFormatMapping[i].iaFourcc;
            }
        }
    }

    LOGE("Invalid Pixel Format: %s", code);
    return -1;
}

const string CameraUtils::fourcc2String(int format4cc) {
    char fourccBuf[5];
    CLEAR(fourccBuf);
    snprintf(fourccBuf, sizeof(fourccBuf), "%c%c%c%c", (format4cc >> 24) & 0xff,
             (format4cc >> 16) & 0xff, (format4cc >> 8) & 0xff, format4cc & 0xff);

    return string(fourccBuf);
}

std::string CameraUtils::format2string(int format) {
    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].v4l2Fmt == format || gFormatMapping[i].iaFourcc == format) {
            return std::string(gFormatMapping[i].shortName);
        }
    }

    LOG2("%s, Not in our format list :%x", __func__, format);
    return fourcc2String(format);
}

unsigned int CameraUtils::fourcc2UL(char* str4cc) {
    CheckAndLogError(str4cc == nullptr, 0, "Invalid null string.");
    CheckAndLogError(strlen(str4cc) != 4, 0, "Invalid string %s, should be 4cc.", str4cc);

    return FOURCC_TO_UL(str4cc[0], str4cc[1], str4cc[2], str4cc[3]);
}

bool CameraUtils::isPlanarFormat(int format) {
    return (format == V4L2_PIX_FMT_NV12 || format == V4L2_PIX_FMT_NV21 ||
            format == V4L2_PIX_FMT_YUV420 || format == V4L2_PIX_FMT_YVU420 ||
            format == V4L2_PIX_FMT_YUV422P || format == V4L2_PIX_FMT_NV16 ||
            format == V4L2_PIX_FMT_P010);
}

bool CameraUtils::isRaw(int format) {
    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].v4l2Fmt == format) {
            // Both normal raw and vector raw treated as raw here.
            return gFormatMapping[i].type == FORMAT_RAW_VEC || gFormatMapping[i].type == FORMAT_RAW;
        }
    }

    return false;
}

int CameraUtils::getBpp(int format) {
    int size = ARRAY_SIZE(gFormatMapping);
    for (int i = 0; i < size; i++) {
        if (gFormatMapping[i].v4l2Fmt == format || gFormatMapping[i].iaFourcc == format) {
            return gFormatMapping[i].bpp;
        }
    }

    LOGE("There is no bpp supplied for format %s", pixelCode2String(format));
    return -1;
}

int CameraUtils::getPlanarByte(int format) {
    int planarBpp = 8;
    switch (format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV16:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_YUV422P:
            planarBpp = 8;
            break;
        case V4L2_PIX_FMT_P010:
            planarBpp = 10;
            break;
        default:
            planarBpp = 8;
            LOGW("planar bpp defaulting to 8 for format:%s", format2string(format).c_str());
            break;
    }

    return ceil(static_cast<double>(planarBpp) / 8);
}

/**
 * Get the stride which is also known as aligned bype per line in some context.
 * Mainly used for locate the start of next line.
 */
int CameraUtils::getStride(int format, int width) {
    int bpl = width * getBpp(format) / 8;
    if (isPlanarFormat(format)) {
        bpl = width * getPlanarByte(format);
    }
    return ALIGN_64(bpl);
}

/**
 * Calculate bytes per line(bpl) based on fourcc format.
 *
 * \param[in] format fourcc code in OS specific format
 * \return bpl bytes per line
 */
int32_t CameraUtils::getBpl(int32_t format, int32_t width) {
    int32_t bpl = 0;
    switch (format) {
        case GET_FOURCC_FMT('C', 'S', '4', '2'):  // YUV
            /*
             * Align based on UV planes, which have half the strides compared to y plane.
             * 42 whole pixels in each 64 byte word, rest 8 bits per word is padding.
             * The total bpl is double the UV-plane strides.
             */
            bpl = ceil((static_cast<double>(width) / 2) / 42) * 64 * 2;
            break;
        case GET_FOURCC_FMT('y', '0', '3', '2'):  // Y032
            bpl = width * 6;
            break;
        case GET_FOURCC_FMT('C', 'S', 'L', '6'):  // CSL6
            bpl = width * 4;
            break;
        case GET_FOURCC_FMT('b', 'V', '0', 'G'):  // BV0G
        case GET_FOURCC_FMT('V', '4', '2', '0'):  // V420
        case GET_FOURCC_FMT('b', 'V', '0', 'K'):  // BV0K
        case GET_FOURCC_FMT('B', 'A', '1', '0'):  // BA10
        case GET_FOURCC_FMT('G', 'R', '1', '0'):  // GR10
        case GET_FOURCC_FMT('B', 'A', '1', '2'):  // BA12
        case GET_FOURCC_FMT('P', '0', '1', '0'):  // P010
        case GET_FOURCC_FMT('P', '0', '1', 'L'):  // P010_LSB
        case GET_FOURCC_FMT('T', '0', '1', '0'):  // P010_msb_tail_y
        case GET_FOURCC_FMT('C', '0', '1', '0'):  // P010_msb_ceil_y
        case GET_FOURCC_FMT('Y', 'U', 'Y', '2'):  // YUY2
            bpl = width * 2;
            break;
        case GET_FOURCC_FMT('N', 'V', '1', '2'):  // NV12
        case GET_FOURCC_FMT('G', 'R', 'B', 'G'):  // GRBG
            bpl = width;
            break;
        default:
            bpl = width;
            LOGW("bpl defaulting to width for format:%s", format2string(format).c_str());
            break;
    }

    return bpl;
}

int32_t CameraUtils::getV4L2Format(const int32_t iaFourcc) {
    for (size_t i = 0; i < ARRAY_SIZE(gFormatMapping); i++) {
        if (gFormatMapping[i].iaFourcc == iaFourcc) return gFormatMapping[i].v4l2Fmt;
    }

    LOGE("Failed to find any V4L2 format with format %s", pixelCode2String(iaFourcc));

    return -1;
}

/*
 * Calc frame size for compression
 */
int CameraUtils::getCompressedFrameSize(int format, int width, int height) {
    int frameSize = 0;
    switch (format) {
        case V4L2_PIX_FMT_SBGGR8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SRGGB10: {
            int alignedBpl = getStride(format, width);
            alignedBpl = ALIGN(alignedBpl, ISYS_COMPRESSION_STRIDE_ALIGNMENT_BYTES);
            int alignedHeight = ALIGN(height, ISYS_COMPRESSION_HEIGHT_ALIGNMENT);
            int imageBufferSize = ALIGN(alignedBpl * alignedHeight, ISYS_COMPRESSION_PAGE_SIZE);
            int singlePlanarTileStatusSize =
                CAMHAL_CEIL_DIV(((alignedBpl * alignedHeight / ISYS_COMPRESSION_TILE_SIZE_BYTES) *
                                 ISYS_COMPRESSION_TILE_STATUS_BITS),
                                8);
            int singleTileStatusSize =
                ALIGN(singlePlanarTileStatusSize, ISYS_COMPRESSION_PAGE_SIZE);

            frameSize = imageBufferSize + singleTileStatusSize;
            break;
        }
        case GET_FOURCC_FMT('V', '4', '2', '0'):
        case GET_FOURCC_FMT('I', 'Y', 'U', 'V'): {
            int alignedBpl = format == GET_FOURCC_FMT('V', '4', '2', '0') ? width * 2 : width;
            alignedBpl = ALIGN(alignedBpl, PSYS_COMPRESSION_PSA_Y_STRIDE_ALIGNMENT);
            int alignedHeight = ALIGN(height, PSYS_COMPRESSION_PSA_HEIGHT_ALIGNMENT);
            int alignWidthUV = alignedBpl / 2;
            int alignHeightUV = alignedHeight / 2;

            int imageBufferSize =
                ALIGN((alignedBpl * alignedHeight + alignWidthUV * alignHeightUV * 2),
                      PSYS_COMPRESSION_PAGE_SIZE);

            int planarYTileStatus = CAMHAL_CEIL_DIV(
                (alignedBpl * alignedHeight / TILE_SIZE_YUV420_Y) * TILE_STATUS_BITS_YUV420_Y, 8);
            planarYTileStatus = ALIGN(planarYTileStatus, PSYS_COMPRESSION_PAGE_SIZE);
            int planarUVTileStatus = CAMHAL_CEIL_DIV(
                (alignWidthUV * alignHeightUV / TILE_SIZE_YUV420_UV) * TILE_STATUS_BITS_YUV420_UV,
                8);
            planarUVTileStatus = ALIGN(planarUVTileStatus, PSYS_COMPRESSION_PAGE_SIZE);

            frameSize = imageBufferSize + planarYTileStatus + planarUVTileStatus * 2;
            break;
        }
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_P010: {
            int bpl = 0, heightAlignment = 0, tsBit = 0, tileSize = 0;
            if (format == V4L2_PIX_FMT_NV12) {
                bpl = width;
                heightAlignment = PSYS_COMPRESSION_TNR_LINEAR_HEIGHT_ALIGNMENT;
                tsBit = TILE_STATUS_BITS_TNR_NV12_TILE_Y;
                tileSize = TILE_SIZE_TNR_NV12_Y;
            } else {
                bpl = width * 2;
                heightAlignment = PSYS_COMPRESSION_OFS_TILE_HEIGHT_ALIGNMENT;
                tsBit = TILE_STATUS_BITS_OFS_P010_TILE_Y;
                tileSize = TILE_SIZE_OFS10_12_TILEY;
            }
            int alignedBpl = ALIGN(bpl, PSYS_COMPRESSION_TNR_STRIDE_ALIGNMENT);
            int alignedHeight = ALIGN(height, heightAlignment);
            int alignedHeightUV = ALIGN(height / UV_HEIGHT_DIVIDER, heightAlignment);
            int imageBufferSize =
                ALIGN(alignedBpl * (alignedHeight + alignedHeightUV), PSYS_COMPRESSION_PAGE_SIZE);
            int planarYTileStatus =
                CAMHAL_CEIL_DIV((alignedBpl * alignedHeight / tileSize) * tsBit, 8);
            planarYTileStatus = ALIGN(planarYTileStatus, PSYS_COMPRESSION_PAGE_SIZE);
            int planarUVTileStatus =
                CAMHAL_CEIL_DIV((alignedBpl * alignedHeightUV / tileSize) * tsBit, 8);
            planarUVTileStatus = ALIGN(planarUVTileStatus, PSYS_COMPRESSION_PAGE_SIZE);

            LOG1("@%s: format: %s, stride:%d height:%d imageSize:%d, tile_status_Y:%d, "
                 "tile_status_UV:%d",
                 __func__, pixelCode2String(format), alignedBpl, alignedHeight, imageBufferSize,
                 planarYTileStatus, planarUVTileStatus);
            frameSize = imageBufferSize + planarYTileStatus + planarUVTileStatus;
            break;
        }
        default:
            LOGE("@%s: unexpected format 0x%x in string %s, unsupported compression format",
                 __func__, format, pixelCode2String(format));
            frameSize = 0;
            break;
    }

    return frameSize;
}

/*
 * Calc frame buffer size.
 *
 *  Why alignment is 64?
 *  The IPU DMA unit must transimit at leat 64 bytes one time.
 *
 *  Why need extra size? It's due to a hardware issue: the DMA unit is a power of
 *  two, and a line should be transferred as few units as possible.
 *  The result is that up to line length more data than the image size
 *  may be transferred to memory after the image.
 *
 *  Another limition is the GDA(Global Dynamic Allocator) allocation unit size(1024). For low
 *  resolution it gives a bigger number. Use larger one to avoid
 *  memory corruption.
 *  for example: 320x480 UVVY, which bpl is 640, less than 1024, in this case, driver will
 *  allocate 1024 bytes for the last line.
 */
int CameraUtils::getFrameSize(int format, int width, int height, bool needAlignedHeight,
                              bool needExtraSize, bool needCompression) {
    int alignedBpl = getStride(format, width);

    // Get frame size with aligned height taking in count for internal buffers.
    // To garantee PSYS kernel like GDC always get enough buffer size to process.
    // This is to satisfy the PSYS kernel, like GDC, input alignment requirement.
    if (needAlignedHeight) {
        height = ALIGN_64(height);
        LOG2("@%s buffer aligned height %d", __func__, height);
    }
    int bufferHeight =
        isPlanarFormat(format) ? ((height * getBpp(format) / 8) / getPlanarByte(format)) : height;

    if (!needExtraSize) {
        LOG2("%s: no need extra size, frame size is %d", __func__, alignedBpl * bufferHeight);
        return alignedBpl * bufferHeight;
    }

    if (needCompression) {
        int compressedFrameSize = getCompressedFrameSize(format, width, height);
        return compressedFrameSize;
    }

    // Extra size should be at least one alignedBpl
    int extraSize = isPlanarFormat(format) ?
                        (alignedBpl * getBpp(format) / 8 / getPlanarByte(format)) :
                        alignedBpl;
    extraSize = std::max(extraSize, 1024);

    return alignedBpl * bufferHeight + extraSize;
}

int CameraUtils::getNumOfPlanes(int format) {
    switch (format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_FMT_IPU_ISYS_META:
            return 1;
        // Add more when needed...
        default:
            return 1;
    }
}

void CameraUtils::getDeviceName(const char* entityName, string& deviceNodeName, bool isSubDev) {
    const char* filePrefix = "video";
    const char* dirPath = "/sys/class/video4linux/";
    if (isSubDev) filePrefix = "v4l-subdev";

    DIR* dp = opendir(dirPath);
    CheckAndLogError((dp == nullptr), VOID_VALUE, "@%s, Fail open : %s", __func__, dirPath);

    struct dirent* dirp = nullptr;
    while ((dirp = readdir(dp)) != nullptr) {
        if ((dirp->d_type == DT_LNK) &&
            (strncmp(dirp->d_name, filePrefix, strlen(filePrefix)) == 0)) {
            string subDeviceName = dirPath;
            subDeviceName += dirp->d_name;
            subDeviceName += "/name";
            int fd = open(subDeviceName.c_str(), O_RDONLY);
            CheckAndLogError((fd < 0), VOID_VALUE, "@%s, open file %s failed. err: %s", __func__,
                             subDeviceName.c_str(), strerror(errno));

            char buf[128] = {'\0'};
            int len = read(fd, buf, sizeof(buf));
            close(fd);
            len--;  // remove "\n"
            if (len == (int)strlen(entityName) && memcmp(buf, entityName, len) == 0) {
                deviceNodeName = "/dev/";
                deviceNodeName += dirp->d_name;
                break;
            }
        }
    }
    closedir(dp);
}

void CameraUtils::getSubDeviceName(const char* entityName, string& deviceNodeName) {
    getDeviceName(entityName, deviceNodeName, true);
}

int CameraUtils::getInterlaceHeight(int field, int height) {
    if (SINGLE_FIELD(field))
        return height / 2;
    else
        return height;
}

bool CameraUtils::isUllPsysPipe(TuningMode tuningMode) {
    return (tuningMode == TUNING_MODE_VIDEO_ULL || tuningMode == TUNING_MODE_VIDEO_CUSTOM_AIC);
}

ConfigMode CameraUtils::getConfigModeByName(const char* ConfigName) {
    ConfigMode configMode = CAMERA_STREAM_CONFIGURATION_MODE_END;

    if (ConfigName == nullptr) {
        LOGE("%s, the ConfigName is nullptr", __func__);
    } else if (strcmp(ConfigName, "AUTO") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_AUTO;
    } else if (strcmp(ConfigName, "ULL") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_ULL;
    } else if (strcmp(ConfigName, "NORMAL") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_NORMAL;
    } else if (strcmp(ConfigName, "HIGH_SPEED") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_CONSTRAINED_HIGH_SPEED;
    } else if (strcmp(ConfigName, "CUSTOM_AIC") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_CUSTOM_AIC;
    } else if (strcmp(ConfigName, "VIDEO_LL") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_VIDEO_LL;
    } else if (strcmp(ConfigName, "STILL_CAPTURE") == 0) {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_STILL_CAPTURE;
    } else if (strcmp(ConfigName, "NONE") == 0) {
        LOG1("%s, the detected internal 'NONE' ConfigName", __func__);
    } else {
        configMode = CAMERA_STREAM_CONFIGURATION_MODE_NORMAL;
        LOG2("%s, the ConfigName %s is not supported, use normal as default", __func__, ConfigName);
    }

    return configMode;
}

void CameraUtils::getConfigModeFromString(string str, std::vector<ConfigMode>& cfgModes) {
    bool split = true;
    ConfigMode mode;
    string resultStr, modeStr = str;

    while (split) {
        size_t pos = 0;
        if ((pos = modeStr.find(",")) == string::npos) {
            mode = getConfigModeByName(modeStr.c_str());
            split = false;
        } else {
            resultStr = modeStr.substr(0, pos);
            modeStr = modeStr.substr(pos + 1);
            mode = getConfigModeByName(resultStr.c_str());
        }
        cfgModes.push_back(mode);
    }
}

ConfigMode CameraUtils::getConfigModeBySceneMode(camera_scene_mode_t sceneMode) {
    ConfigMode configMode = CAMERA_STREAM_CONFIGURATION_MODE_END;

    switch (sceneMode) {
        case SCENE_MODE_NORMAL:
            configMode = CAMERA_STREAM_CONFIGURATION_MODE_NORMAL;
            break;
        case SCENE_MODE_ULL:
            configMode = CAMERA_STREAM_CONFIGURATION_MODE_ULL;
            break;
        case SCENE_MODE_CUSTOM_AIC:
            configMode = CAMERA_STREAM_CONFIGURATION_MODE_CUSTOM_AIC;
            break;
        case SCENE_MODE_VIDEO_LL:
            configMode = CAMERA_STREAM_CONFIGURATION_MODE_VIDEO_LL;
            break;
        default:
            // There is no corresponding ConfigMode for some scene.
            LOG2("there is no corresponding ConfigMode for scene %d", sceneMode);
            break;
    }
    return configMode;
}

camera_scene_mode_t CameraUtils::getSceneModeByName(const char* sceneName) {
    if (sceneName == nullptr)
        return SCENE_MODE_MAX;
    else if (strcmp(sceneName, "AUTO") == 0)
        return SCENE_MODE_AUTO;
    else if (strcmp(sceneName, "ULL") == 0)
        return SCENE_MODE_ULL;
    else if (strcmp(sceneName, "VIDEO_LL") == 0)
        return SCENE_MODE_VIDEO_LL;
    else if (strcmp(sceneName, "NORMAL") == 0)
        return SCENE_MODE_NORMAL;
    else if (strcmp(sceneName, "CUSTOM_AIC") == 0)
        return SCENE_MODE_CUSTOM_AIC;

    return SCENE_MODE_MAX;
}

camera_awb_mode_t CameraUtils::getAwbModeByName(const char* awbName) {
    if (awbName == nullptr)
        return AWB_MODE_MAX;
    else if (strcmp(awbName, "AUTO") == 0)
        return AWB_MODE_AUTO;
    else if (strcmp(awbName, "INCANDESCENT") == 0)
        return AWB_MODE_INCANDESCENT;
    else if (strcmp(awbName, "FLUORESCENT") == 0)
        return AWB_MODE_FLUORESCENT;
    else if (strcmp(awbName, "DAYLIGHT") == 0)
        return AWB_MODE_DAYLIGHT;
    else if (strcmp(awbName, "FULL_OVERCAST") == 0)
        return AWB_MODE_FULL_OVERCAST;
    else if (strcmp(awbName, "PARTLY_OVERCAST") == 0)
        return AWB_MODE_PARTLY_OVERCAST;
    else if (strcmp(awbName, "SUNSET") == 0)
        return AWB_MODE_SUNSET;
    else if (strcmp(awbName, "VIDEO_CONFERENCE") == 0)
        return AWB_MODE_VIDEO_CONFERENCE;
    else if (strcmp(awbName, "MANUAL_CCT_RANGE") == 0)
        return AWB_MODE_MANUAL_CCT_RANGE;
    else if (strcmp(awbName, "MANUAL_WHITE_POINT") == 0)
        return AWB_MODE_MANUAL_WHITE_POINT;
    else if (strcmp(awbName, "MANUAL_GAIN") == 0)
        return AWB_MODE_MANUAL_GAIN;
    else if (strcmp(awbName, "MANUAL_COLOR_TRANSFORM") == 0)
        return AWB_MODE_MANUAL_COLOR_TRANSFORM;

    return AWB_MODE_MAX;
}

unsigned int CameraUtils::getMBusFormat(int cameraId, unsigned int isysFmt) {
    unsigned int pixelCode = 0;

    switch (isysFmt) {
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_NV16:
            pixelCode = V4L2_MBUS_FMT_UYVY8_1X16;
            break;
        case V4L2_PIX_FMT_YUYV:
            pixelCode = V4L2_MBUS_FMT_YUYV8_1X16;
            break;
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_XBGR32:
            pixelCode = MEDIA_BUS_FMT_RGB888_1X24;
            break;
        case V4L2_PIX_FMT_RGB565:
        case V4L2_PIX_FMT_XRGB32:
            pixelCode = MEDIA_BUS_FMT_RGB565_1X16;
            break;
        case V4L2_PIX_FMT_SGRBG8:
            pixelCode = V4L2_MBUS_FMT_SGRBG8_1X8;
            break;
        default:
            LOGE("No input format to match the output: %s", pixelCode2String(isysFmt));
            break;
    }

    return pixelCode;
}

void* CameraUtils::dlopenLibrary(const char* name, int flags) {
    CheckAndLogError((name == nullptr), nullptr, "%s, invalid parameters", __func__);

    void* handle = dlopen(name, flags);

    const char* lError = dlerror();
    if (lError) {
        if (handle == nullptr) {
            LOGW("%s, handle is NULL", __func__);
        }
        LOGW("%s, dlopen Error: %s", __func__, lError);
        return nullptr;
    }

    LOG1("%s, handle %p, name %s has been opened", __func__, handle, name);
    return handle;
}

void* CameraUtils::dlsymLibrary(void* handle, const char* str) {
    CheckAndLogError((handle == nullptr || str == nullptr), nullptr, "%s, invalid parameters",
                     __func__);

    void* sym = dlsym(handle, str);

    const char* lError = dlerror();
    if (lError) {
        if (sym == nullptr) {
            LOGW("%s, symbol is nullptr", __func__);
        }
        LOGW("%s, dlopen Error: %s", __func__, lError);
        return nullptr;
    }

    LOG1("%s, handle %p, str %s has been found", __func__, handle, str);
    return sym;
}

int CameraUtils::dlcloseLibrary(void* handle) {
    CheckAndLogError((handle == nullptr), BAD_VALUE, "%s, invalid parameters", __func__);

    dlclose(handle);
    LOG1("%s, handle %p has been closed", __func__, handle);
    return OK;
}

std::vector<string> CameraUtils::splitString(const char* srcStr, char delim) {
    std::vector<string> tokens;
    std::stringstream ss(srcStr);
    string item;

    for (size_t i = 0; std::getline(ss, item, delim); i++) {
        tokens.push_back(item);
    }

    return tokens;
}

nsecs_t CameraUtils::systemTime() {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return nsecs_t(t.tv_sec) * 1000000000LL + t.tv_nsec;
}

}  // namespace icamera
