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

#define LOG_TAG "IntelCmc"

#include "modules/algowrapper/IntelCmc.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelCmc::IntelCmc() : mHandle(nullptr) {
    LOG1("@%s", __func__);
}

IntelCmc::~IntelCmc() {
    LOG1("@%s", __func__);
}

bool IntelCmc::init(const ia_binary_data* aiqbData, const ia_binary_data* nvmData) {
    LOG1("@%s, aiqbData:%p, nvmData:%p", __func__, aiqbData, nvmData);
    CheckError(!aiqbData, false, "aiqbData is nullptr");
    CheckError(nvmData, false, "nvmData is not nullptr");  // it doesn't support nvmData currently.

    mHandle = ia_cmc_parser_init_v1(aiqbData, nvmData);
    LOG1("@%s, mHandle:%p", __func__, mHandle);

    return true;
}

ia_cmc_t* IntelCmc::getCmc() const {
    LOG1("@%s, mHandle:%p", __func__, mHandle);

    return mHandle;
}

uintptr_t IntelCmc::getCmcHandle() const {
    LOG1("@%s", __func__);

    return reinterpret_cast<uintptr_t>(mHandle);
}

void IntelCmc::deinit() {
    LOG1("@%s", __func__);
    CheckError(!mHandle, VOID_VALUE, "mHandle is nullptr");

    ia_cmc_parser_deinit(mHandle);
}
} /* namespace icamera */
