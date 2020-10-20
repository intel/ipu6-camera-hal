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

#define LOG_TAG "IntelMkn"

#include "modules/algowrapper/IntelMkn.h"

#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IntelMkn::IntelMkn() {
    LOG1("%s", __func__);
}

IntelMkn::~IntelMkn() {
    LOG1("%s", __func__);
}

ia_mkn* IntelMkn::init(ia_mkn_config_bits mkn_config_bits, size_t mkn_section_1_size,
                       size_t mkn_section_2_size) {
    LOG1("%s", __func__);

    return ia_mkn_init(mkn_config_bits, mkn_section_1_size, mkn_section_2_size);
}

int IntelMkn::enable(ia_mkn* pMkn, bool enable_data_collection) {
    LOG1("%s", __func__);
    CheckError(!pMkn, BAD_VALUE, "@%s, pMkn is null", __func__);

    ia_err err = ia_mkn_enable(pMkn, enable_data_collection);
    CheckError(err != ia_err_none, UNKNOWN_ERROR, "@%s, call ia_mkn_enable() fails", __func__);

    return OK;
}

void IntelMkn::deinit(ia_mkn* pMkn) {
    LOG1("%s", __func__);
    CheckError(!pMkn, VOID_VALUE, "@%s, pMkn is null", __func__);

    ia_mkn_uninit(pMkn);
}

int IntelMkn::prepare(ia_mkn* pMkn, ia_mkn_trg data_target, ia_binary_data* binaryData) {
    LOG1("%s", __func__);
    CheckError(!pMkn, BAD_VALUE, "@%s, pMkn is null", __func__);

    *binaryData = ia_mkn_prepare(pMkn, data_target);
    CheckError(binaryData->size == 0 || binaryData->data == nullptr, NO_MEMORY,
               "@%s, binaryData->size:%d, binaryData->data:%p, error!", __func__, binaryData->size,
               binaryData->data);

    return OK;
}
}  // namespace icamera
