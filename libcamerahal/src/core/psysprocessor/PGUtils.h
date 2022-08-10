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

#pragma once

extern "C" {
#include <ia_css_program_group_data.h>
#include <ia_css_psys_process_group.h>
#include <ia_css_psys_program_group_manifest.h>
#include <ia_p2p.h>
}

#include <vector>

namespace icamera {
#define PG_PAGE_SIZE 4096
#define PSYS_MAX_KERNELS_PER_PG IA_CSS_KERNEL_BITMAP_BITS
#define IPU_MAX_TERMINAL_COUNT 40

struct PgFrameDesc {
    PgFrameDesc() {
        width = 0;
        height = 0;
        bpe = 0;
    }
    int width;
    int height;
    int bpe;
};

struct PgConfiguration {
    ia_css_program_group_manifest_t* pgManifest;
    int pgManifestSize;
    std::vector<int> disableDataTermials;
    uint8_t fragmentCount;

    // New API, for desc calculation by itself, instead of fragmentDesc
    PgFrameDesc inputMainFrame;
    PgFrameDesc outputMainFrame;
};

enum { PAIR_BUFFER_IN_INDEX = 0, PAIR_BUFFER_OUT_INDEX };
#define PAIR_BUFFER_COUNT (PAIR_BUFFER_OUT_INDEX + 1)

struct TerminalPair {
    int inId;
    int outId;
};

namespace PGUtils {
/* ************************************************************
 * Common definitions
 * ***********************************************************/

#define IS_DATA_TERMINAL(terminalType) (terminalType == IA_CSS_TERMINAL_TYPE_DATA_IN || \
                                        terminalType == IA_CSS_TERMINAL_TYPE_DATA_OUT)

ia_css_frame_format_type getCssFmt(int v4l2Fmt);
int getCssBpp(int v4l2Fmt, bool compression = false);
int getCssBpe(int v4l2Fmt, bool compression = false);
int getCssStride(int v4l2Fmt, int width);
bool isCompressionTerminal(int terminalId);

/* ************************************************************
 * Difference between PGs
 * ***********************************************************/
enum TERMINAL_PAIR_TYPE { TERMINAL_PAIR_TNR, TERMINAL_PAIR_TNR_SIM, TERMINAL_PAIR_DVS };

bool getTerminalPairs(int pgId, TERMINAL_PAIR_TYPE type, std::vector<TerminalPair>* pairs);
}  // namespace PGUtils
}  // namespace icamera
