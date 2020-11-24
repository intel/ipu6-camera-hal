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

#define LOG_TAG "PGUtils"

#include <stdint.h>

#include <vector>

#include "iutils/Utils.h"
#include "iutils/CameraLog.h"

#include "PGUtils.h"

namespace icamera {

namespace PGUtils {

/* ************************************************************
 * Common definitions
 * ***********************************************************/

#define GET_FOURCC_FMT(a, b, c, d) ((uint32_t)(d) | ((uint32_t)(c) << 8) \
                                 | ((uint32_t)(b) << 16) | ((uint32_t)(a) << 24))

struct FormatMap {
    int v4l2Fmt;
    ia_css_frame_format_type cssFmt;

    int cssBpp;
};

static const FormatMap sFormatMapping[] = {
    { V4L2_PIX_FMT_YUYV,   IA_CSS_DATA_FORMAT_YUYV, 12 },
    { V4L2_PIX_FMT_UYVY,   IA_CSS_DATA_FORMAT_UYVY, 12 },
    { V4L2_PIX_FMT_YUV420, IA_CSS_DATA_FORMAT_YUV420, 16 },
    { V4L2_PIX_FMT_NV12,   IA_CSS_DATA_FORMAT_NV12, 8 },
    { V4L2_PIX_FMT_NV16,   IA_CSS_DATA_FORMAT_NV16, 12 },
    { V4L2_PIX_FMT_RGB565, IA_CSS_DATA_FORMAT_RGB565, 16 },
    { V4L2_PIX_FMT_RGB24,  IA_CSS_DATA_FORMAT_RGB888, 24 },
    { V4L2_PIX_FMT_RGB32,  IA_CSS_DATA_FORMAT_RGBA888, 24 },
    { V4L2_PIX_FMT_SGRBG12, IA_CSS_DATA_FORMAT_RAW, 16 },
    { V4L2_PIX_FMT_SGRBG10, IA_CSS_DATA_FORMAT_RAW, 16 }, // IA_CSS_DATA_FORMAT_BAYER_GRBG or IA_CSS_DATA_FORMAT_RAW ?
// IPU4_FEATURE_S
    { V4L2_PIX_FMT_YUYV420_V32, IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED, 16 },
// IPU4_FEATURE_E

    { GET_FOURCC_FMT('Y', 'U', 'Y', 'V'), IA_CSS_DATA_FORMAT_YUYV, 12 },
    { GET_FOURCC_FMT('U', 'Y', 'V', 'Y'), IA_CSS_DATA_FORMAT_UYVY, 12 },
    { GET_FOURCC_FMT('Y', 'U', '1', '2'), IA_CSS_DATA_FORMAT_YUV420, 16 },
    { GET_FOURCC_FMT('N', 'V', '1', '2'), IA_CSS_DATA_FORMAT_NV12, 8 },
    { GET_FOURCC_FMT('N', 'V', '1', '6'), IA_CSS_DATA_FORMAT_NV16, 12 },
    { GET_FOURCC_FMT('R', 'G', 'B', 'P'), IA_CSS_DATA_FORMAT_RGB565, 16 },
    { GET_FOURCC_FMT('R', 'G', 'B', '3'), IA_CSS_DATA_FORMAT_RGB888, 24 },
    { GET_FOURCC_FMT('R', 'G', 'B', '4'), IA_CSS_DATA_FORMAT_RGBA888, 24 },
    { GET_FOURCC_FMT('B', 'A', '1', '2'), IA_CSS_DATA_FORMAT_RAW, 16 },
    { GET_FOURCC_FMT('B', 'A', '1', '0'), IA_CSS_DATA_FORMAT_RAW, 16 }, // IA_CSS_DATA_FORMAT_BAYER_GRBG or IA_CSS_DATA_FORMAT_RAW ?
    { GET_FOURCC_FMT('y', '0', '3', '2'), IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED, 16 },
    { GET_FOURCC_FMT('V', '4', '2', '0'), IA_CSS_DATA_FORMAT_YUV420, 16 },
    { GET_FOURCC_FMT('b','V','0','K'),    IA_CSS_DATA_FORMAT_BAYER_VECTORIZED, 16 },
    { GET_FOURCC_FMT('C','S','L','6'),    IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED, 16},
    { GET_FOURCC_FMT('G','R','1','0'),    IA_CSS_DATA_FORMAT_BAYER_GRBG, 16 },
    { GET_FOURCC_FMT('I','Y','U','V'),    IA_CSS_DATA_FORMAT_YUV420, 12 },
};

static int getStride(int cssFmt, int width);

ia_css_frame_format_type getCssFmt(int v4l2Fmt) {
    int size = ARRAY_SIZE(sFormatMapping);
    for (int i = 0; i < size; i++) {
        if (sFormatMapping[i].v4l2Fmt == v4l2Fmt) {
            return sFormatMapping[i].cssFmt;
        }
    }

    LOG2("%s: unsupported v4l2 pixel format: %s", __func__,
         CameraUtils::format2string(v4l2Fmt).c_str());
    return IA_CSS_N_FRAME_FORMAT_TYPES;
}

int getCssStride(int v4l2Fmt, int width) {
    int stride = width;
    ia_css_frame_format_type cssFmt = getCssFmt(v4l2Fmt);
    switch (v4l2Fmt) {
        case GET_FOURCC_FMT('I','Y','U','V'):
            stride = width;
            break;
        default:
            stride = getStride(cssFmt, width);
            break;
    }
    return stride;
}

int getCssBpp(int v4l2Fmt) {
    int size = ARRAY_SIZE(sFormatMapping);
    for (int i = 0; i < size; i++) {
        if (sFormatMapping[i].v4l2Fmt == v4l2Fmt) {
            return sFormatMapping[i].cssBpp;
        }
    }

    LOG2("%s: unsupported v4l2 pixel format: 0x%x", __func__, v4l2Fmt);
    return 8;
}

int getStride(int cssFmt, int width) {
    int stride = width;
    switch (cssFmt) {
        case IA_CSS_DATA_FORMAT_BAYER_GRBG: // GR10
        case IA_CSS_DATA_FORMAT_RAW:        // BA10
            stride = ALIGN_64(width * 2);
            break;
        case IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED: // y032
            stride = width * 6;
            break;
        case IA_CSS_DATA_FORMAT_BAYER_VECTORIZED: // bv0k
        case IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED: // css_fourcc_grbg_12_li
            stride = width * 4;
            stride = ALIGN_64(stride);
            break;
        case IA_CSS_DATA_FORMAT_YUV420:
            stride = width * 2;
            stride = ALIGN_64(stride);
            break;
        case IA_CSS_DATA_FORMAT_NV12:
            stride = width;
            break;
        default:
            LOG2("TODO for format: %d", cssFmt);
            break;
    }
    return stride;
}

/* ************************************************************
 * Difference between PGs
 * ***********************************************************/
#define PG_PSYS_IPU6_ISA_LB 187
#define PG_PSYS_IPU6_BB 189
#define PG_PSYS_IPU6_ISL 198

// the below terminals belong to PG_PSYS_IPU6_BB
#define PG_BB_TERMINAL_ID_TNR_REF_IN 4       // data_terminal
#define PG_BB_TERMINAL_ID_TNR_REF_OUT 6      // data_terminal
#define PG_BB_TERMINAL_ID_TNR_SIM_REF_IN 5   // spetial_terminal
#define PG_BB_TERMINAL_ID_TNR_SIM_REF_OUT 7  // spetial_terminal

// the below terminals belong to PG_PSYS_IPU6_ISA_LB
#ifdef IPU_SYSVER_ipu6v5
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L0 20 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L1 21 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L2 22 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L0 23 // param_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L1 24 // param_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L2 25 // param_terminal
#else
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L0 21 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L1 22 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_IN_L2 23 // program_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L0 24 // param_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L1 25 // param_terminal
#define ISA_LB_TERMINAL_ID_DVS_FE_OUT_L2 26 // param_terminal
#endif

bool getTerminalPairs(int pgId, TERMINAL_PAIR_TYPE type, std::vector<TerminalPair>* pairs) {
    LOG2("@%s, pgId:%d, type:%d, pairs:%p", __func__, pgId, type, pairs);
    CheckError(!pairs, false, "@%s, pairs is nullptr", __func__);

    struct TerminalPairs {
        int pgId;
        TERMINAL_PAIR_TYPE type;
        std::vector<TerminalPair> pairs;
    };
    static const TerminalPairs tps[] = {
        {PG_PSYS_IPU6_BB, TERMINAL_PAIR_TNR,
            {{PG_BB_TERMINAL_ID_TNR_REF_IN, PG_BB_TERMINAL_ID_TNR_REF_OUT}}},
        {PG_PSYS_IPU6_BB, TERMINAL_PAIR_TNR_SIM,
            {{PG_BB_TERMINAL_ID_TNR_SIM_REF_IN, PG_BB_TERMINAL_ID_TNR_SIM_REF_OUT}}},
        {PG_PSYS_IPU6_ISA_LB, TERMINAL_PAIR_DVS,
            {{ISA_LB_TERMINAL_ID_DVS_FE_IN_L0, ISA_LB_TERMINAL_ID_DVS_FE_OUT_L0},
             {ISA_LB_TERMINAL_ID_DVS_FE_IN_L1, ISA_LB_TERMINAL_ID_DVS_FE_OUT_L1},
             {ISA_LB_TERMINAL_ID_DVS_FE_IN_L2, ISA_LB_TERMINAL_ID_DVS_FE_OUT_L2}}}
    };

    for (unsigned int i = 0; i < ARRAY_SIZE(tps); i++) {
        if (tps[i].pgId == pgId && tps[i].type == type) {
            *pairs = tps[i].pairs;
            return true;
        }
    }

    return false;
}

} // name space PGUtils
} // namespace icamera
