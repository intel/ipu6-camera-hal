/*
 * Copyright (C) 2019-2021 Intel Corporation.
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

#include "PGUtils.h"

// CIPF backends
extern "C" {
#include <ia_cipf_css/ia_cipf_css.h>
}

#include <stdint.h>
#include <vector>
#include "iutils/Utils.h"
#include "iutils/CameraLog.h"

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
    int cssBpe;
};

static const FormatMap sFormatMapping[] = {
    { V4L2_PIX_FMT_YUYV,    IA_CSS_DATA_FORMAT_YUYV, 16, 8 },
    { V4L2_PIX_FMT_UYVY,    IA_CSS_DATA_FORMAT_UYVY, 16, 8 },
    { V4L2_PIX_FMT_YUV420,  IA_CSS_DATA_FORMAT_YUV420, 12, 8 },
    { V4L2_PIX_FMT_NV12,    IA_CSS_DATA_FORMAT_NV12, 12, 8 },
    { V4L2_PIX_FMT_NV16,    IA_CSS_DATA_FORMAT_NV16, 16, 8 },
    { V4L2_PIX_FMT_RGB565,  IA_CSS_DATA_FORMAT_RGB565, 16, 16 },
    { V4L2_PIX_FMT_RGB24,   IA_CSS_DATA_FORMAT_RGB888, 24, 24 },
    { V4L2_PIX_FMT_RGB32,   IA_CSS_DATA_FORMAT_RGBA888, 24, 24 },
    { V4L2_PIX_FMT_SGRBG12, IA_CSS_DATA_FORMAT_RAW, 16, 16 },
    { V4L2_PIX_FMT_SGRBG10, IA_CSS_DATA_FORMAT_RAW, 16, 16 },
    { V4L2_PIX_FMT_SGRBG8,  IA_CSS_DATA_FORMAT_RAW, 8, 8 },

    { GET_FOURCC_FMT('Y', 'U', 'Y', 'V'), IA_CSS_DATA_FORMAT_YUYV, 16, 8 },
    { GET_FOURCC_FMT('Y', 'U', 'Y', '2'), IA_CSS_DATA_FORMAT_YUYV, 16, 8 },
    { GET_FOURCC_FMT('U', 'Y', 'V', 'Y'), IA_CSS_DATA_FORMAT_UYVY, 16, 8 },
    { GET_FOURCC_FMT('Y', 'U', '1', '2'), IA_CSS_DATA_FORMAT_YUV420, 12, 8 },
    { GET_FOURCC_FMT('Y', 'V', '1', '2'), IA_CSS_DATA_FORMAT_YUV420, 12, 8 },
    { GET_FOURCC_FMT('N', 'V', '1', '2'), IA_CSS_DATA_FORMAT_NV12, 12, 8 },
    { GET_FOURCC_FMT('N', 'V', '2', '1'), IA_CSS_DATA_FORMAT_NV21, 12, 8 },
    { GET_FOURCC_FMT('T', 'I', 'L', 'E'), IA_CSS_DATA_FORMAT_NV12_TILEY, 12, 8 },
    { GET_FOURCC_FMT('N', 'V', '1', '6'), IA_CSS_DATA_FORMAT_NV16, 16, 8 },
    { GET_FOURCC_FMT('R', 'G', 'B', 'P'), IA_CSS_DATA_FORMAT_RGB565, 16, 16},
    { GET_FOURCC_FMT('R', 'G', 'B', '3'), IA_CSS_DATA_FORMAT_RGB888, 24, 24},
    { GET_FOURCC_FMT('R', 'G', 'B', '4'), IA_CSS_DATA_FORMAT_RGBA888, 24, 24 },
    { GET_FOURCC_FMT('B', 'A', '1', '2'), IA_CSS_DATA_FORMAT_RAW, 16, 16 },
    { GET_FOURCC_FMT('B', 'A', '1', '0'), IA_CSS_DATA_FORMAT_RAW, 16, 16 },
    { GET_FOURCC_FMT('y', '0', '3', '2'), IA_CSS_DATA_FORMAT_YYUVYY_VECTORIZED, 24, 16 },
    { GET_FOURCC_FMT('V', '4', '2', '0'), IA_CSS_DATA_FORMAT_YUV420, 24, 16 },
    { GET_FOURCC_FMT('b', 'V', '0', 'K'), IA_CSS_DATA_FORMAT_BAYER_VECTORIZED, 16, 16 },
    { GET_FOURCC_FMT('b', 'V', '0', 'G'), IA_CSS_DATA_FORMAT_BAYER_VECTORIZED, 16, 16 },
    { GET_FOURCC_FMT('C', 'S', 'L', '6'), IA_CSS_DATA_FORMAT_BAYER_LINE_INTERLEAVED, 12, 10 },
    { GET_FOURCC_FMT('C', 'S', '4', '2'), IA_CSS_DATA_FORMAT_YUV420, 18, 12 },
    { GET_FOURCC_FMT('G', 'R', '1', '0'), IA_CSS_DATA_FORMAT_BAYER_GRBG, 16, 16 },
    { GET_FOURCC_FMT('R', 'G', '1', '0'), IA_CSS_DATA_FORMAT_BAYER_RGGB, 16, 16 },
    { GET_FOURCC_FMT('G', 'B', '1', '0'), IA_CSS_DATA_FORMAT_BAYER_GBRG, 16, 16 },
    { GET_FOURCC_FMT('B', 'G', '1', '0'), IA_CSS_DATA_FORMAT_BAYER_BGGR, 16, 16 },
    { GET_FOURCC_FMT('B', 'A', '1', '0'), IA_CSS_DATA_FORMAT_RAW, 16, 16 },
    { GET_FOURCC_FMT('I', 'Y', 'U', 'V'), IA_CSS_DATA_FORMAT_YUV420, 12, 8 },
    { GET_FOURCC_FMT('P', '0', '1', '0'), IA_CSS_DATA_FORMAT_P010, 24, 16 },
    { GET_FOURCC_FMT('P', '4', '1', '2'), IA_CSS_DATA_FORMAT_YUV420, 24, 16 },
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
        case GET_FOURCC_FMT('Y','U','Y','2'):
            stride = ALIGN_64(width * 2);
            break;
        default:
            stride = getStride(cssFmt, width);
            break;
    }
    return stride;
}

int getCssBpp(int v4l2Fmt, bool compression) {
    int size = ARRAY_SIZE(sFormatMapping);
    int bpp = 0;
    for (int i = 0; i < size; i++) {
        if (sFormatMapping[i].v4l2Fmt == v4l2Fmt) {
            bpp = sFormatMapping[i].cssBpp;
        }
    }

    if (compression) {
        ia_css_frame_format_type cssFmt = getCssFmt(v4l2Fmt);
        switch (cssFmt) {
            case IA_CSS_DATA_FORMAT_BAYER_GRBG:
            case IA_CSS_DATA_FORMAT_BAYER_RGGB:
            case IA_CSS_DATA_FORMAT_BAYER_BGGR:
            case IA_CSS_DATA_FORMAT_BAYER_GBRG:
                bpp = 10;
                break;
            case IA_CSS_DATA_FORMAT_YUV420:
            case IA_CSS_DATA_FORMAT_NV12:
                bpp = 8;
                break;
            default:
                LOGW("%s format %d compress not supported", __func__, v4l2Fmt);
                break;
        }
    }

    if (bpp == 0) {
        LOG2("%s: unsupported v4l2 format: 0x%x, compressed %d", __func__, v4l2Fmt, compression);
        bpp = 8;
    }
    return bpp;
}

int getCssBpe(int v4l2Fmt, bool compression) {
    int size = ARRAY_SIZE(sFormatMapping);
    int bpe = 0;
    for (int i = 0; i < size; i++) {
        if (sFormatMapping[i].v4l2Fmt == v4l2Fmt) {
            bpe = sFormatMapping[i].cssBpe;
        }
    }

    if (compression) {
        ia_css_frame_format_type cssFmt = getCssFmt(v4l2Fmt);
        switch (cssFmt) {
            case IA_CSS_DATA_FORMAT_BAYER_GRBG:
            case IA_CSS_DATA_FORMAT_BAYER_RGGB:
            case IA_CSS_DATA_FORMAT_BAYER_BGGR:
            case IA_CSS_DATA_FORMAT_BAYER_GBRG:
                bpe = 16;
                break;
            case IA_CSS_DATA_FORMAT_YUV420:
            case IA_CSS_DATA_FORMAT_NV12:
                bpe = 8;
                break;
            default:
                LOGW("%s format %d compress not supported", __func__, v4l2Fmt);
                break;
        }
    }

    if (bpe == 0) {
        LOG2("%s: unsupported v4l2 format: 0x%x, compressed %d", __func__, v4l2Fmt, compression);
        bpe = getCssBpp(v4l2Fmt, compression);
    }
    return bpe;
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
        case IA_CSS_DATA_FORMAT_YUYV:
            stride = ALIGN_64(width * 2);
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

bool isCompressionTerminal(int terminalId) {
    bool cmp = false;
    /* only pg 187 terminal 3/7 and pg 189 terminal 0 support compression
    ** tnr compression 189 terminal 4/6 not enabled in software
    */
    if (terminalId == psys_ipu6_isa_lb_input_high_uid ||
        terminalId == psys_ipu6_isa_lb_output_uid ||
        terminalId == psys_ipu6_bb_input_uid ||
        terminalId == psys_ipu6_bb_tnr_ref_in_uid ||
        terminalId == psys_ipu6_bb_tnr_ref_out_uid) {
        cmp = true;
    }

    return cmp;
}
} // name space PGUtils
} // namespace icamera
