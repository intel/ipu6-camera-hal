/*
 * Copyright (C) 2019 Intel Corporation.
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

#include <ia_lard.h>

#include "modules/sandboxing/IPCCommon.h"

namespace icamera {

#define MAX_LARD_DATA_SIZE MAX_IA_BINARY_DATA_SIZE

struct lard_init_params {
    ia_binary_data base;
    unsigned char data[MAX_LARD_DATA_SIZE];

    // the below is returned
    uintptr_t results;
};

struct lard_get_tag_list_params {
    uintptr_t lard;
    unsigned int mode_tag;

    // the below is returned
    unsigned int num_cmc_tags;
    unsigned int num_aiq_tags;
    unsigned int num_isp_tags;
    unsigned int num_others_tags;
    unsigned int cmc_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int aiq_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int isp_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int others_tags_list[MAX_LARD_DATA_SIZE];
};

struct lard_run_params_results {
    ia_lard_results base;

    unsigned int cmc_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int aiq_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int isp_tags_list[MAX_LARD_DATA_SIZE];
    unsigned int others_tags_list[MAX_LARD_DATA_SIZE];
};

struct lard_run_params {
    uintptr_t lard;
    ia_lard_input_params inputParams;

    // the below is returned
    lard_run_params_results results;
};

struct lard_deinit_params {
    uintptr_t lard;
};

class IPCIntelLard {
 public:
    IPCIntelLard();
    virtual ~IPCIntelLard();

    // for ia_lard_init
    bool clientFlattenInit(void* pData, unsigned int size, const ia_binary_data* lard_data_ptr);
    bool serverUnflattenInit(void* pData, int size, ia_binary_data* lard_data_ptr);
    bool serverFlattenInit(void* pData, int size, ia_lard* lard);
    bool clientUnflattenInit(void* pData, unsigned int size, ia_lard** lard);

    // for ia_lard_get_tag_list
    bool clientFlattenGetTagList(void* pData, unsigned int size, ia_lard* lard,
                                 unsigned int mode_tag);
    bool serverUnflattenGetTagList(void* pData, unsigned int size, ia_lard** lard,
                                   unsigned int* mode_tag);
    bool serverFlattenGetTagList(void* pData, unsigned int size, unsigned int num_tags,
                                 const unsigned int* tags);
    bool clientUnflattenGetTagList(void* pData, unsigned int size, unsigned int mode_tag,
                                   unsigned int* num_tags, const unsigned int** tags);

    // ia_lard_run
    bool clientFlattenRun(void* pData, unsigned int size, ia_lard* lard,
                          ia_lard_input_params* inputParams);
    bool serverUnflattenRun(void* pData, unsigned int size, ia_lard** lard,
                            ia_lard_input_params** inputParams);
    bool serverFlattenRun(void* pData, unsigned int size, ia_lard_results* result);
    bool clientUnflattenRun(void* pData, unsigned int size, ia_lard_results** result);

    // ia_lard_deinit
    bool clientFlattenDeinit(void* pData, unsigned int size, ia_lard* lard);
    bool serverUnflattenDeinit(void* pData, unsigned int size, ia_lard** lard);
};
} /* namespace icamera */
