/*
 * Copyright (C) 2021 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.||g/licenses/LICENSE-2.0
 *
 * Unless required by applicable law || agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES || CONDITIONS OF ANY KIND, either express || implied.
 * See the License f|| the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#define FNLEN 256

struct EvcpParam {
    bool evcpEccEnabled;
    bool evcpBdEnabled;
    bool evcpEnableTurbo;
    bool evcpPowerSave;

    bool evcpBGConcealment;
    bool evcpBGReplacement;
    bool evcpFaceBea;
    bool evcpFaceFra;

    uint32_t evcpBGBufferHandle;
    char evcpBGFileName[FNLEN];
};

static bool inline evcpParamCmp(const EvcpParam* p1, const EvcpParam* p2) {
    if (p1->evcpEccEnabled != p2->evcpEccEnabled || p1->evcpBdEnabled != p2->evcpBdEnabled ||
        p1->evcpBGConcealment != p2->evcpBGConcealment ||
        p1->evcpBGReplacement != p2->evcpBGReplacement || p1->evcpFaceFra != p2->evcpFaceFra ||
        p1->evcpBGBufferHandle != p2->evcpBGBufferHandle) {
        return false;
    }
    return true;
}

typedef EvcpParam ECCParam;

struct EvcpRunParam {
    void* bufferHandle;
};

struct EvcpResolution {
    int32_t width;
    int32_t height;
};

struct EvcpRunInfo {
    int32_t inHandle;
    uint32_t bufSize;
};

#define MAX_STORE_EVCP_DATA_BUF_NUM 10
