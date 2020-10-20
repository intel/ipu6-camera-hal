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

#pragma once

#include <ia_mkn_encoder.h>

#include "IPCCommon.h"
#include "iutils/Utils.h"

struct MknInitParams {
    ia_mkn_config_bits mkn_config_bits;
    size_t mkn_section_1_size;
    size_t mkn_section_2_size;
    uintptr_t results;
};

struct MknPrepareParams {
    uintptr_t mkn_handle;
    ia_mkn_trg data_target;
    ia_binary_data results;
    char data[MAKERNOTE_SECTION1_SIZE + MAKERNOTE_SECTION2_SIZE];
};

struct MknDeinitParams {
    uintptr_t mkn_handle;
};

struct MknEnableParams {
    uintptr_t mkn_handle;
    bool enable_data_collection;
};

namespace icamera {
class IPCIntelMkn {
 public:
    IPCIntelMkn();
    virtual ~IPCIntelMkn();

    // for init
    bool clientFlattenInit(ia_mkn_config_bits mkn_config_bits, size_t mkn_section_1_size,
                           size_t mkn_section_2_size, MknInitParams* params);

    // for prepare
    bool clientFlattenPrepare(uintptr_t mkn, ia_mkn_trg data_target, MknPrepareParams* params);
    bool clientUnflattenPrepare(MknPrepareParams* params, ia_binary_data* mknData);
    bool serverFlattenPrepare(const ia_binary_data& inData, MknPrepareParams* results);
};
}  // namespace icamera
