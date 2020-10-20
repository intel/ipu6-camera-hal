/*
 * Copyright (C) 2020 Intel Corporation.
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

#ifndef __PG_CONTROL_INIT_FRAMEWORK_H
#define __PG_CONTROL_INIT_FRAMEWORK_H

#include <type_support.h>
#include <ia_css_psys_process_types.h>
#include <ia_css_terminal_types.h>
#include <ia_css_kernel_user_param_types.h>

#pragma GCC visibility push(default)

enum pg_control_init_ret_type {
    PG_CONTROL_INIT_SUCCESS  = 0,
    PG_CONTROL_INIT_FAILURE = (1 << 0)
};

/*
 * @brief  Fill the program control init desc.
 *
 * @param[in]  process_group Process group.
 * @param[out] terminal      Program control init terminal.
 *
 * @retval  0  Successful.
 * @retval  1  Error.
 */

int pg_control_init_terminal_init(
        ia_css_process_group_t *process_group,
        ia_css_program_control_init_terminal_t *terminal);

/**
 * @brief  Fill the program control init payload.
 *
 * @param[in]  process_group    Process group.
 * @param[in]  params           Kernel user parameters object.
 * @param[in]  payload_address  Host address in payload.
 *
 * @retval 0   Successful.
 * @retval 1  Error.
 */

int pg_control_init_fill_payload(
    const ia_css_process_group_t *process_group,
    const ia_css_kernel_user_param_t *params,
    void *payload_address);

/**
 * @brief  Get the payload size
 *
 * @param[in]   process_group Process group
 * @param[out]  payload_size  Payload size to be allocated for the process.
 *
 * @retval 0   Successful.
 * @retval 1  Error.
 */

int pg_control_init_get_payload_size(
    const ia_css_process_group_t *process_group,
    unsigned int *payload_size);

#pragma GCC visibility pop

#endif /*__PG_CONTROL_INIT_FRAMEWORK_H*/
