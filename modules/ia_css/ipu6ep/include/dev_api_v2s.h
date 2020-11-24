/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __DEV_API_V2S_H
#define __DEV_API_V2S_H

#include "dai_device_access_types.h" /* Should be included from NCI */
#include "dai_ipu_top_devices.h"
#include "nci_v2s_types.h"
#include "dev_api_v2s_storage_class.h"

#define DEV_API_V2S_NUM_BUFFERS 6

struct dev_api_v2s_config {
	enum nci_v2s_format	format;
	struct nci_v2s_dev_cfg	device_config;
	struct nci_v2s_ack_cfg	ack_config;
	struct nci_v2s_buf_cfg	buffer_config[DEV_API_V2S_NUM_BUFFERS];
};

/**
 * @brief Configure device
 *
 * @param[in] dev_id		ID of the device to configure
 * @param[in] cfg		configuration to be stored in the device
 * @param[in] payload_address	buffer where the configuration should be stored (only used in host)
 *
 * Configure V2S device with device/fragment parameters
 * and buffer configuration
 */
DEV_API_V2S_STORAGE_CLASS_H void
dev_api_v2s_configure_device(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		struct dev_api_v2s_config *cfg,
		void *payload_address
	);

/**
 * @brief Send an INIT command.
 *
 * @param[in] dev_id	ID of the device to init
 *
 * Send an INIT command to the specified device.
 * Must be called before beginning the processing of each fragment.
 */
DEV_API_V2S_STORAGE_CLASS_H void
dev_api_v2s_init_device(dai_ipu_top_vec_to_str_v3_instances_t dev_id);

/**
 * @brief Send a PROCESS_N_VECTORS command.
 *
 * @param[in] dev_id	ID of the device receiving the command
 * @param[in] n_vectors	# of vectors to process by the device.
 *
 * Send a PROCESS_N_VECTORS command to the specified device.
 * Restictions apply to n_vectors. Consult MAS.
 *
 * Must be called AFTER dev_api_v2s_init_device().
 */
DEV_API_V2S_STORAGE_CLASS_H void
dev_api_v2s_process_n_vectors(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		unsigned int n_vectors);

/**
 * @brief Set ack_k_vectors of the device
 *
 * @param[in] dev_id		ID of the device to reconfigure
 * @param[in] ack_k_vectors	# of vectors processed per ack sent
 *
 * Set ack_k_vectors.
 *
 * This functions reads the current configuration of the device
 * reconfigures the ack_k_vec field and stores it back.
 */
DEV_API_V2S_STORAGE_CLASS_H void
dev_api_v2s_set_ack_k_vectors(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		unsigned int ack_k_vectors);

#ifdef __INLINE_DEV_API_V2S__
#include "dev_api_v2s_inline.h"
#endif

#endif /* __DEV_API_V2S_H */
