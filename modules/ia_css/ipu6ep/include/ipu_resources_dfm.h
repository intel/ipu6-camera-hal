/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __IPU_RESOURCES_DFM_H
#define __IPU_RESOURCES_DFM_H

#include "storage_class.h"
#include "dev_api_dfm_types.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"
#include "dfm_resource_types.h"
#include "ipu_resources_token_format.h"

/*!
 * Returns the DFM dev_api (equivalent to dai id) device id given
 * a vied_nci_dev_dfm_id_t value (from resource_model).
 */
STORAGE_CLASS_INLINE enum ipu_device_dfm_id ipu_resources_dfm_get_nci_dev_id(
	const vied_nci_dev_dfm_id_t dev);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_nci_port_number(
	const vied_nci_dev_dfm_id_t dev, const uint32_t port_num);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_section_payload_size(
	const vied_nci_dev_dfm_id_t dev,
	const uint32_t port_num,
	const vied_nci_dfm_section_id_t sec_id);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_resource_payload_size(
	dfm_port_resource_t dfm_resource);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_resource_gather_multi_mask(
	dfm_port_resource_t dfm_resource);

/*!
 * Returns the start port number of a given vied_nci_dev_chn_ID_t value (from resource_model).
 */
STORAGE_CLASS_INLINE uint16_t ipu_resources_dfm_get_dev_port_num_start(
	const vied_nci_dev_dfm_id_t dev);

/*!
 * Returns true if the dfm port type is full for given vied_nci_dev_chn_ID_t value (from resource_model).
 */
STORAGE_CLASS_INLINE bool ipu_resources_dfm_is_dev_port_full_type(
	const vied_nci_dev_dfm_id_t dev);

STORAGE_CLASS_INLINE
dfm_port_resource_t ipu_resources_dfm_increment_offset(dfm_port_resource_t in, uint32_t inc);

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dfm_get_port_address(uint32_t fabric_id, dfm_port_resource_t res);

/*!
 * Returns the process id that is expected at SPC
 */
STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_dfm_encode_process_id(
		const vied_nci_dev_dfm_id_t dev, const uint32_t port_num);

/*!
 * Returns the evq token the is expected at the SPC
 */
STORAGE_CLASS_INLINE nci_eq_token_t ipu_resources_dfm_evq_token_encode(
		enum nci_eq_device_id eq_id,
		const vied_nci_dev_dfm_id_t dev,
		const uint32_t port_num);

#include "ipu_resources_dfm_impl.h"

#endif /* __IPU_RESOURCES_DFM_H */
