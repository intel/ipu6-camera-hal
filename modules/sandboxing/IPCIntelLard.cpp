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

#define LOG_TAG "IPC_INTEL_LARD"

#include "modules/sandboxing/IPCIntelLard.h"

#include <ia_types.h>

#include "CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
IPCIntelLard::IPCIntelLard() {
    LOGIPC("@%s", __func__);
}

IPCIntelLard::~IPCIntelLard() {
    LOGIPC("@%s", __func__);
}

bool IPCIntelLard::clientFlattenInit(void* pData, unsigned int size,
                                     const ia_binary_data* lard_data_ptr) {
    LOGIPC("@%s, pData:%p, size:%d, lard_data_ptr:%p", __func__, pData, size, lard_data_ptr);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard_data_ptr, false, "@%s, lard_data_ptr is nullptr", __func__);

    lard_init_params* params = static_cast<lard_init_params*>(pData);
    params->base = *lard_data_ptr;

    CheckError(params->base.size > MAX_LARD_DATA_SIZE, false, "@%s, data:%d is too small", __func__,
               params->base.size);
    MEMCPY_S(params->data, MAX_LARD_DATA_SIZE, params->base.data, params->base.size);

    return true;
}

bool IPCIntelLard::serverUnflattenInit(void* pData, int size, ia_binary_data* lard_data_ptr) {
    LOGIPC("@%s, pData:%p, size:%d, binaryData:%p", __func__, pData, size, lard_data_ptr);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard_data_ptr, false, "@%s, binaryData is nullptr", __func__);

    lard_init_params* params = static_cast<lard_init_params*>(pData);
    params->base.data = static_cast<void*>(params->data);
    *lard_data_ptr = params->base;

    return true;
}

bool IPCIntelLard::serverFlattenInit(void* pData, int size, ia_lard* lard) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p", __func__, pData, size, lard);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);

    lard_init_params* params = static_cast<lard_init_params*>(pData);

    params->results = reinterpret_cast<uintptr_t>(lard);

    return true;
}

bool IPCIntelLard::clientUnflattenInit(void* pData, unsigned int size, ia_lard** lard) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p", __func__, pData, size, lard);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_init_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);

    lard_init_params* params = static_cast<lard_init_params*>(pData);
    *lard = reinterpret_cast<ia_lard*>(params->results);

    return true;
}

bool IPCIntelLard::clientFlattenGetTagList(void* pData, unsigned int size, ia_lard* lard,
                                           unsigned int mode_tag) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p, mode_tag:%d", __func__, pData, size, lard, mode_tag);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_get_tag_list_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);

    lard_get_tag_list_params* params = static_cast<lard_get_tag_list_params*>(pData);
    params->lard = reinterpret_cast<uintptr_t>(lard);
    params->mode_tag = mode_tag;

    return true;
}

bool IPCIntelLard::serverUnflattenGetTagList(void* pData, unsigned int size, ia_lard** lard,
                                             unsigned int* mode_tag) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p, mode_tag:%p", __func__, pData, size, lard, mode_tag);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_get_tag_list_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);
    CheckError(!mode_tag, false, "@%s, mode_tag is nullptr", __func__);

    lard_get_tag_list_params* params = static_cast<lard_get_tag_list_params*>(pData);
    *lard = reinterpret_cast<ia_lard*>(params->lard);
    *mode_tag = params->mode_tag;

    return true;
}

bool IPCIntelLard::serverFlattenGetTagList(void* pData, unsigned int size, unsigned int num_tags,
                                           const unsigned int* tags) {
    LOGIPC("@%s, pData:%p, size:%d, num_tags:%d, tags:%p", __func__, pData, size, num_tags, tags);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_get_tag_list_params), false, "@%s, buffer is small", __func__);
    CheckError(!tags, false, "@%s, tags is nullptr", __func__);

    lard_get_tag_list_params* params = static_cast<lard_get_tag_list_params*>(pData);
    int tagsSize = num_tags * sizeof(unsigned int);

    CheckError(tagsSize > MAX_LARD_DATA_SIZE, false, "@%s, tags:%d is small", __func__, tagsSize);
    switch (params->mode_tag) {
        case LCMC_TAG:
            params->num_cmc_tags = num_tags;
            MEMCPY_S(params->cmc_tags_list, tagsSize, tags, tagsSize);
            break;
        case LAIQ_TAG:
            params->num_aiq_tags = num_tags;
            MEMCPY_S(params->aiq_tags_list, tagsSize, tags, tagsSize);
            break;
        case LISP_TAG:
            params->num_isp_tags = num_tags;
            MEMCPY_S(params->isp_tags_list, tagsSize, tags, tagsSize);
            break;
        case LTHR_TAG:
            params->num_others_tags = num_tags;
            MEMCPY_S(params->others_tags_list, tagsSize, tags, tagsSize);
            break;
        default:
            return false;
    }

    return true;
}

bool IPCIntelLard::clientUnflattenGetTagList(void* pData, unsigned int size, unsigned int mode_tag,
                                             unsigned int* num_tags, const unsigned int** tags) {
    LOGIPC("@%s, pData:%p, size:%d, mode_tag:%d, num_tags:%p, tags:%p", __func__, pData, size,
           mode_tag, num_tags, tags);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_get_tag_list_params), false, "@%s, buffer is small", __func__);
    CheckError(!num_tags, false, "@%s, num_tags is nullptr", __func__);
    CheckError(!tags, false, "@%s, tags is nullptr", __func__);

    lard_get_tag_list_params* params = static_cast<lard_get_tag_list_params*>(pData);

    switch (mode_tag) {
        case LCMC_TAG:
            *num_tags = params->num_cmc_tags;
            *tags = params->cmc_tags_list;
            break;
        case LAIQ_TAG:
            *num_tags = params->num_aiq_tags;
            *tags = params->aiq_tags_list;
            break;
        case LISP_TAG:
            *num_tags = params->num_isp_tags;
            *tags = params->isp_tags_list;
            break;
        case LTHR_TAG:
            *num_tags = params->num_others_tags;
            *tags = params->others_tags_list;
            break;
        default:
            return false;
    }

    return true;
}

bool IPCIntelLard::clientFlattenRun(void* pData, unsigned int size, ia_lard* lard,
                                    ia_lard_input_params* inputParams) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p, inputParams:%p", __func__, pData, size, lard,
           inputParams);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_run_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);
    CheckError(!inputParams, false, "@%s, inputParams is nullptr", __func__);

    lard_run_params* params = static_cast<lard_run_params*>(pData);
    params->lard = reinterpret_cast<uintptr_t>(lard);
    params->inputParams = *inputParams;

    return true;
}

bool IPCIntelLard::serverUnflattenRun(void* pData, unsigned int size, ia_lard** lard,
                                      ia_lard_input_params** inputParams) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p, inputParams:%p", __func__, pData, size, lard,
           inputParams);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_run_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);
    CheckError(!inputParams, false, "@%s, inputParams is nullptr", __func__);

    lard_run_params* params = static_cast<lard_run_params*>(pData);
    *lard = reinterpret_cast<ia_lard*>(params->lard);
    *inputParams = &params->inputParams;

    return true;
}

bool IPCIntelLard::serverFlattenRun(void* pData, unsigned int size, ia_lard_results* result) {
    LOGIPC("@%s, pData:%p, size:%d, result:%p", __func__, pData, size, result);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_run_params), false, "@%s, buffer is small", __func__);
    CheckError(!result, false, "@%s, result is nullptr", __func__);

    lard_run_params* params = static_cast<lard_run_params*>(pData);

    lard_run_params_results* results = &params->results;
    ia_lard_results* base = &results->base;
    *base = *result;

    CheckError(base->aiqb_cmc_data.size > MAX_LARD_DATA_SIZE, false, "@%s, cmc:%d is too big",
               __func__, base->aiqb_cmc_data.size);
    CheckError(base->aiqb_aiq_data.size > MAX_LARD_DATA_SIZE, false, "@%s, aiq:%d is too big",
               __func__, base->aiqb_aiq_data.size);
    CheckError(base->aiqb_isp_data.size > MAX_LARD_DATA_SIZE, false, "@%s, isp:%d is too big",
               __func__, base->aiqb_isp_data.size);
    CheckError(base->aiqb_other_data.size > MAX_LARD_DATA_SIZE, false, "@%s, other:%d is too big",
               __func__, base->aiqb_other_data.size);

    MEMCPY_S(results->cmc_tags_list, MAX_LARD_DATA_SIZE, base->aiqb_cmc_data.data,
             base->aiqb_cmc_data.size);
    MEMCPY_S(results->aiq_tags_list, MAX_LARD_DATA_SIZE, base->aiqb_aiq_data.data,
             base->aiqb_aiq_data.size);
    MEMCPY_S(results->isp_tags_list, MAX_LARD_DATA_SIZE, base->aiqb_isp_data.data,
             base->aiqb_isp_data.size);
    MEMCPY_S(results->others_tags_list, MAX_LARD_DATA_SIZE, base->aiqb_other_data.data,
             base->aiqb_other_data.size);

    return true;
}

bool IPCIntelLard::clientUnflattenRun(void* pData, unsigned int size, ia_lard_results** result) {
    LOGIPC("@%s, pData:%p, size:%d, result:%p", __func__, pData, size, result);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_run_params), false, "@%s, buffer is small", __func__);
    CheckError(!result, false, "@%s, result is nullptr", __func__);

    lard_run_params* params = static_cast<lard_run_params*>(pData);
    lard_run_params_results* results = &params->results;
    ia_lard_results* base = &results->base;

    base->aiqb_cmc_data.data = results->cmc_tags_list;
    base->aiqb_aiq_data.data = results->aiq_tags_list;
    base->aiqb_isp_data.data = results->isp_tags_list;
    base->aiqb_other_data.data = results->others_tags_list;

    *result = base;

    return true;
}

bool IPCIntelLard::clientFlattenDeinit(void* pData, unsigned int size, ia_lard* lard) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p", __func__, pData, size, lard);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_deinit_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);

    lard_deinit_params* params = static_cast<lard_deinit_params*>(pData);
    params->lard = reinterpret_cast<uintptr_t>(lard);

    return true;
}

bool IPCIntelLard::serverUnflattenDeinit(void* pData, unsigned int size, ia_lard** lard) {
    LOGIPC("@%s, pData:%p, size:%d, lard:%p", __func__, pData, size, lard);
    CheckError(!pData, false, "@%s, pData is nullptr", __func__);
    CheckError(size < sizeof(lard_deinit_params), false, "@%s, buffer is small", __func__);
    CheckError(!lard, false, "@%s, lard is nullptr", __func__);

    lard_deinit_params* params = static_cast<lard_deinit_params*>(pData);
    *lard = reinterpret_cast<ia_lard*>(params->lard);

    return true;
}
} /* namespace icamera */
