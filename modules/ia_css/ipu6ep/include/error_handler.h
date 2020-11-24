/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2019 Intel Corporation.
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

/**
 * @file error_handler.h
 * @brief This file contains error enums and handler for the fwapi testbench.
 */

#ifndef __ERROR_HANDLER_H
#define __ERROR_HANDLER_H

/* Bit position signifies the expected_error_code
 * for example: HOST_DATA_MISMATCH has expected_error_code as 2 */
#define HOST_MAX_STATUS 31
enum host_ret_type {
	HOST_SUCCESS            = 0,
	HOST_FAILURE            = (1 << 0),
	HOST_DATA_MISMATCH      = (1 << 1),
	HOST_PARAMOUT_MISMATCH  = (1 << 2),
	HOST_PGS_NOT_CONCURRENT = (1 << 3),
	HOST_PARSER_ERROR       = (1 << 4),
	HOST_PSYS_EVENT_HANDLER = (1 << 5),
	HOST_ISYS_EVENT_HANDLER = (1 << 6),
	HOST_MALLOC_FAILURE     = (1 << 7),
	HOST_ISYS_ERROR_FW_INTERNAL_CONSISTENCY             = (1 << 8),
	HOST_ISYS_ERROR_HW_CONSISTENCY                      = (1 << 9),
	HOST_ISYS_ERROR_DRIVER_INVALID_COMMAND_SEQUENCE     = (1 << 10),
	HOST_ISYS_ERROR_DRIVER_INVALID_DEVICE_CONFIGURATION = (1 << 11),
	HOST_ISYS_ERROR_DRIVER_INVALID_STREAM_CONFIGURATION = (1 << 12),
	HOST_ISYS_ERROR_DRIVER_INVALID_FRAME_CONFIGURATION  = (1 << 13),
	HOST_ISYS_ERROR_INSUFFICIENT_RESOURCES              = (1 << 14),
	HOST_ISYS_ERROR_HW_REPORTED_STR2MMIO                = (1 << 15),
	HOST_ISYS_ERROR_HW_REPORTED_SIG2CIO                 = (1 << 16),
	HOST_ISYS_ERROR_SENSOR_FW_SYNC                      = (1 << 17),
	HOST_ISYS_ERROR_STREAM_IN_SUSPENSION                = (1 << 18),
	HOST_ISYS_ERROR_RESPONSE_QUEUE_FULL                 = (1 << 19),
	HOST_ERROR_OPEN_STREAM                              = (1 << 20), /* Temporal error type */
	HOST_STIMULI_ERROR                                  = (1 << 21),
	HOST_CHECKER_ERROR                                  = (1 << 22),
	HOST_PSYS_POWER_STATUS_IS_NOT_DOWN                  = (1 << 23),
	HOST_PSYS_ERROR_INVALID_FRAME                       = (1 << 24),
	HOST_PSYS_INPUT_DATA_HANDLER_ERROR					= (1 << 25),
	HOST_PSYS_INPUT_PARAM_HANDLER_ERROR					= (1 << 26),
	HOST_PSYS_OUTPUT_DUMP_HANDLER_ERROR					= (1 << 27),
	HOST_PSYS_POWER_STATUS_IS_NOT_UP					= (1 << 28),
	/* This is a mechanism to initiate stream2gen thread end
	 * It is not an error, Will be moving this once the complete
	 * streaming buffer functionalities are supported.
	 */
	HOST_PSYS_STREAM2GEN_END							= (1 << 29),
	/* New error codes to be added here */
	HOST_N_STATES           = (1 << HOST_MAX_STATUS)
};

/**
 * @brief Compare accumulated error with expected error.
 *
 * This function maps user given expected_error_code to host_ret_type enum
 * and checks if it matches accumulated error
 *
 * @param[in]   error                 Accumulated error
 * @param[in]   expected_error_code   Bit in host_ret_type enum; range in [0 .. HOST_MAX_STATUS]
 *
 * @retval HOST_SUCCESS     if (expected_error_code != 0) and expected error matches
 * @retval HOST_FAILURE     if (expected_error_code != 0) and expected error does not match
 * @retval error            if (expected_error_code == 0)
 */
enum host_ret_type
check_expected_error(int error, int expected_error_code);

/**
 * @brief Get psys threads error
 *
 * @param[in]	void
 *
 * @retval		int		pg_psys_threads_error value
 */
int get_psys_threads_error(void);

/**
 * @brief Set psys threads error
 *
 * @param[in]	int		error value to set
 *
 * @retval		void
 */
void set_psys_threads_error(int error);

#endif /* __ERROR_HANDLER_H */
