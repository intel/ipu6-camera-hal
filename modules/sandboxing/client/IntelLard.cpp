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

#define LOG_TAG "IntelLard"

#include "modules/sandboxing/client/IntelLard.h"

#include <string>

#include "CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLard::IntelLard() : mInitialized(false) {
    LOGIPC("@%s", __func__);

    uintptr_t personal = reinterpret_cast<uintptr_t>(this);
    std::string initName = "/lardInit" + std::to_string(personal) + SHM_NAME;
    std::string getTagListName = "/lardGetTagList" + std::to_string(personal) + SHM_NAME;
    std::string runName = "/lardRun" + std::to_string(personal) + SHM_NAME;
    std::string deinitName = "/lardDeinit" + std::to_string(personal) + SHM_NAME;

    mMems = {{initName.c_str(), sizeof(lard_init_params), &mMemInit, false},
             {getTagListName.c_str(), sizeof(lard_get_tag_list_params), &mMemGetTagList, false},
             {runName.c_str(), sizeof(lard_run_params), &mMemRun, false},
             {deinitName.c_str(), sizeof(lard_deinit_params), &mMemDeinit, false}};

    bool success = mCommon.allocateAllShmMems(&mMems);
    if (!success) {
        mCommon.releaseAllShmMems(mMems);
        return;
    }

    LOGIPC("@%s, done", __func__);
    mInitialized = true;
}

IntelLard::~IntelLard() {
    LOGIPC("@%s", __func__);
    mCommon.releaseAllShmMems(mMems);
}

ia_lard* IntelLard::init(const ia_binary_data* lard_data_ptr) {
    LOGIPC("@%s, binaryData:%p", __func__, lard_data_ptr);

    CheckError(mInitialized == false, nullptr, "@%s, mInitialized is false", __func__);
    CheckError(!lard_data_ptr, nullptr, "@%s, lard_data_ptr is nullptr", __func__);
    CheckError(!lard_data_ptr->data, nullptr, "@%s, lard_data_ptr->data is nullptr", __func__);
    CheckError(lard_data_ptr->size == 0, nullptr, "@%s, lard_data_ptr->size is 0", __func__);

    bool ret = mIpc.clientFlattenInit(mMemInit.mAddr, mMemInit.mSize, lard_data_ptr);
    CheckError(ret == false, nullptr, "@%s, clientFlattenInit fails", __func__);

    ret = mCommon.requestSync(IPC_LARD_INIT, mMemInit.mHandle);
    CheckError(ret == false, nullptr, "@%s, requestSync fails", __func__);

    ia_lard* lard = nullptr;
    ret = mIpc.clientUnflattenInit(mMemInit.mAddr, mMemInit.mSize, &lard);
    CheckError(ret == false, nullptr, "@%s, clientUnflattenInit fails", __func__);

    return lard;
}

ia_err IntelLard::getTagList(ia_lard* ia_lard_ptr, unsigned int mode_tag, unsigned int* num_tags,
                             const unsigned int** tags) {
    LOGIPC("@%s, ia_lard_ptr:%p, mode_tag:%d", __func__, ia_lard_ptr, mode_tag);

    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(ia_lard_ptr == nullptr, ia_err_general, "@%s, ia_lard_ptr is nullptr", __func__);
    CheckError(num_tags == nullptr, ia_err_general, "@%s, num_tags is nullptr", __func__);
    CheckError(tags == nullptr, ia_err_general, "@%s, tags is nullptr", __func__);

    bool ret = mIpc.clientFlattenGetTagList(mMemGetTagList.mAddr, mMemGetTagList.mSize, ia_lard_ptr,
                                            mode_tag);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenGetTagList fails", __func__);

    ret = mCommon.requestSync(IPC_LARD_GET_TAG_LIST, mMemGetTagList.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenGetTagList(mMemGetTagList.mAddr, mMemGetTagList.mSize, mode_tag,
                                         num_tags, tags);
    CheckError(ret == false, ia_err_general, "@%s, clientUnflattenGetTagList fails", __func__);

    return ia_err_none;
}

ia_err IntelLard::run(ia_lard* ia_lard_ptr, ia_lard_input_params* lard_input_params_ptr,
                      ia_lard_results** lard_results_ptr) {
    LOGIPC("@%s, ia_lard_ptr:%p, lard_input_params_ptr:%p", __func__, ia_lard_ptr,
           lard_input_params_ptr);

    CheckError(mInitialized == false, ia_err_general, "@%s, mInitialized is false", __func__);
    CheckError(ia_lard_ptr == nullptr, ia_err_general, "@%s, ia_lard_ptr is nullptr", __func__);
    CheckError(lard_input_params_ptr == nullptr, ia_err_general,
               "@%s, lard_input_params_ptr is nullptr", __func__);
    CheckError(lard_results_ptr == nullptr, ia_err_general, "@%s, lard_results_ptr is nullptr",
               __func__);

    bool ret =
        mIpc.clientFlattenRun(mMemRun.mAddr, mMemRun.mSize, ia_lard_ptr, lard_input_params_ptr);
    CheckError(ret == false, ia_err_general, "@%s, clientFlattenRun fails", __func__);

    ret = mCommon.requestSync(IPC_LARD_RUN, mMemRun.mHandle);
    CheckError(ret == false, ia_err_general, "@%s, requestSync fails", __func__);

    ret = mIpc.clientUnflattenRun(mMemRun.mAddr, mMemRun.mSize, lard_results_ptr);
    CheckError(ret == false, ia_err_general, "@%s, clientUnflattenRun fails", __func__);

    return ia_err_none;
}

void IntelLard::deinit(ia_lard* ia_lard_ptr) {
    LOGIPC("@%s, ia_lard_ptr:%p", __func__, ia_lard_ptr);

    CheckError(mInitialized == false, VOID_VALUE, "@%s, mInitialized is false", __func__);
    CheckError(ia_lard_ptr == nullptr, VOID_VALUE, "@%s, ia_lard_ptr is nullptr", __func__);

    bool ret = mIpc.clientFlattenDeinit(mMemDeinit.mAddr, mMemDeinit.mSize, ia_lard_ptr);
    CheckError(ret == false, VOID_VALUE, "@%s, clientFlattenDeinit fails", __func__);

    ret = mCommon.requestSync(IPC_LARD_DEINIT, mMemDeinit.mHandle);
    CheckError(ret == false, VOID_VALUE, "@%s, requestSync fails", __func__);
}
} /* namespace icamera */
