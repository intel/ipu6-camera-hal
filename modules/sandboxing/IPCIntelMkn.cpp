/*
 * Copyright (C) 2019-2020 Intel Corporation
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

#define LOG_TAG "IPC_INTEL_MKN"

#include "modules/sandboxing/IPCIntelMkn.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"

namespace icamera {
IPCIntelMkn::IPCIntelMkn() {
    LOGIPC("@%s", __func__);
}

IPCIntelMkn::~IPCIntelMkn() {
    LOGIPC("@%s", __func__);
}

bool IPCIntelMkn::clientFlattenInit(ia_mkn_config_bits mkn_config_bits, size_t mkn_section_1_size,
                                    size_t mkn_section_2_size, MknInitParams* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(params == nullptr, false, "@%s, params is nullptr", __func__);

    params->mkn_config_bits = mkn_config_bits;
    params->mkn_section_1_size = mkn_section_1_size;
    params->mkn_section_2_size = mkn_section_2_size;

    return true;
}

bool IPCIntelMkn::clientFlattenPrepare(uintptr_t mkn, ia_mkn_trg data_target,
                                       MknPrepareParams* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(params == nullptr, false, "@%s, params is nullptr", __func__);

    params->mkn_handle = mkn;
    params->data_target = data_target;

    return true;
}

bool IPCIntelMkn::clientUnflattenPrepare(MknPrepareParams* params, ia_binary_data* mknData) {
    LOGIPC("@%s, mknData:%p", __func__, mknData);
    CheckError(params == nullptr, false, "@%s, params is nullptr", __func__);
    CheckError(mknData == nullptr, false, "@%s, mknData is nullptr", __func__);

    params->results.data = static_cast<void*>(params->data);
    *mknData = params->results;

    LOGIPC("@%s, mknData.size:%d", __func__, mknData->size);

    return true;
}

bool IPCIntelMkn::serverFlattenPrepare(const ia_binary_data& inData, MknPrepareParams* params) {
    LOGIPC("@%s, params:%p", __func__, params);
    CheckError(params == nullptr, false, "@%s, params is nullptr", __func__);
    CheckError(sizeof(params->data) < inData.size, false, "@%s, buffer is small", __func__);

    params->results.size = inData.size;
    MEMCPY_S(params->data, sizeof(params->data), inData.data, inData.size);

    return true;
}
} /* namespace icamera */
