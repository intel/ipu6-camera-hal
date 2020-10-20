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

#include "modules/algowrapper/IntelLard.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
IntelLard::IntelLard() {
    LOG1("@%s", __func__);
}

IntelLard::~IntelLard() {
    LOG1("@%s", __func__);
}

ia_lard* IntelLard::init(const ia_binary_data* lard_data_ptr) {
    LOG1("@%s", __func__);
    CheckError(!lard_data_ptr, nullptr, "lard_data_ptr is nullptr");
    LOG1("@%s, lard_data_ptr, data:%p, size:%d", __func__, lard_data_ptr->data,
         lard_data_ptr->size);

    ia_lard* lard = ia_lard_init(lard_data_ptr);
    LOG1("@%s, lard:%p", __func__, lard);

    return lard;
}

ia_err IntelLard::getTagList(ia_lard* ia_lard_ptr, unsigned int mode_tag, unsigned int* num_tags,
                             const unsigned int** tags) {
    LOG1("@%s", __func__);
    CheckError(!ia_lard_ptr, ia_err_general, "ia_lard_ptr is nullptr");
    CheckError(!num_tags, ia_err_general, "num_tags is nullptr");
    CheckError(!tags, ia_err_general, "tags is nullptr");

    return ia_lard_get_tag_list(ia_lard_ptr, mode_tag, num_tags, tags);
}

ia_err IntelLard::run(ia_lard* ia_lard_ptr, ia_lard_input_params* lard_input_params_ptr,
                      ia_lard_results** lard_results_ptr) {
    LOG1("@%s", __func__);
    CheckError(!ia_lard_ptr, ia_err_general, "ia_lard_ptr is nullptr");
    CheckError(!lard_input_params_ptr, ia_err_general, "lard_input_params_ptr is nullptr");
    CheckError(!lard_results_ptr, ia_err_general, "lard_results_ptr is nullptr");

    return ia_lard_run(ia_lard_ptr, lard_input_params_ptr, lard_results_ptr);
}

void IntelLard::deinit(ia_lard* ia_lard_ptr) {
    LOG1("@%s", __func__);
    CheckError(!ia_lard_ptr, VOID_VALUE, "ia_lard_ptr is nullptr");

    ia_lard_deinit(ia_lard_ptr);
}
} /* namespace icamera */
