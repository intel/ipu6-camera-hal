/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
 * All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or licensors. Title to the Material remains with Intel
 * Corporation or its licensors. The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * licensors. The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No License under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
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
