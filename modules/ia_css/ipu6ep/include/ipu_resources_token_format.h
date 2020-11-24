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

#ifndef _IPU_RESOURCES_TOKEN_FORMAT_H_
#define _IPU_RESOURCES_TOKEN_FORMAT_H_
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_eq_types.h"
#include "vied_nci_psys_resource_model_private.h"
#include "ia_css_base_types.h"

enum ipu_resources_ack_type {
	IPU_RESOURCES_INIT_ACK,
	IPU_RESOURCES_DONE_ACK,
	IPU_RESOURCES_PRE_COMPRESSION_DONE_ACK, /* Process done, trigger compression engine */
	IPU_RESOURCES_ACK_NONE
};

/*
 * Derive process id from device type, device instance and device channel
 */
STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_encode_process_id(
		vied_nci_device_type_id_t dev_type,
		uint8_t dev_instance,
		uint8_t dev_channel);

/*
 * Create process id from combination of
 * dev_type, dev_instance and dev_channel
 */
STORAGE_CLASS_INLINE nci_eq_token_t ipu_resources_encode_evq_token(
		enum nci_eq_device_id eq_id,
		ia_css_process_id_t process_id);

/*
 * Decode evq token to extract the process id
 */
STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_decode_evq_token(
		enum nci_eq_device_id eq_id,
		nci_eq_token_t token);

/*
 * Decode process id to get back dev_type, dev_instance
 * and dev_channel
 */
STORAGE_CLASS_INLINE void ipu_resources_decode_process_id(
		ia_css_process_id_t process_id,
		vied_nci_device_type_id_t *dev_type,
		uint8_t *dev_instance,
		uint8_t *dev_channel);

STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_process_id_from_sid_pid(
		enum nci_eq_device_id eq_id,
		nci_eq_sid_t sid,
		nci_eq_pid_t pid);

#include "ipu_resources_token_format_impl.h"

#endif /* _IPU_RESOURCES_TOKEN_FORMAT_H_ */
