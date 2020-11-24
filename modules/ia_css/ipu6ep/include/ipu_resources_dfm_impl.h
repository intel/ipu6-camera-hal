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

#ifndef __IPU_RESOURCES_DFM_IMPL_H
#define __IPU_RESOURCES_DFM_IMPL_H

#include "ipu_resources_dfm.h"
#include "ipu_resources_dep.h"
#include "ipu_device_dfm_port_properties.h"
#include "nci_dfm_descriptors.h"
#include "ipu_device_dfm_properties.h"
#include "ia_css_base_types.h"

STORAGE_CLASS_INLINE enum ipu_device_dfm_id ipu_resources_dfm_get_nci_dev_id(
	const vied_nci_dev_dfm_id_t dev)
{
	enum ipu_device_dfm_id nci_dfm_device_id;

	assert(dev < VIED_NCI_N_DEV_DFM_ID);
	nci_dfm_device_id = resource_model_dfm_dev_2_nci_dfm_dev[dev];
	assert(nci_dfm_device_id < IPU_DEVICE_DFM_NUM_DEVICES);
	return nci_dfm_device_id;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_nci_port_number(
	const vied_nci_dev_dfm_id_t dev, const uint32_t port_num)
{
	uint32_t nci_port_num;

	assert(dev < VIED_NCI_N_DEV_DFM_ID);
	assert(port_num < IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS);
	nci_port_num = port_num + resource_model_dfm_dev_port_num_start[dev];
	assert(nci_port_num < IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR);
	return nci_port_num;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_section_payload_size(
	const vied_nci_dev_dfm_id_t dev,
	const uint32_t port_num,
	const vied_nci_dfm_section_id_t section_id)
{
	uint32_t size;
	enum ipu_device_dfm_id dai_dev_id = ipu_resources_dfm_get_nci_dev_id(dev);
	uint32_t nci_port_number = ipu_resources_dfm_get_nci_port_number(dev, port_num);

	switch (section_id) {
	case VIED_NCI_DFM_SECTION_ID_CMDBANK:
		size = ipu_nci_dfm_port_get_section0_size(dai_dev_id, nci_port_number);
		break;
	case VIED_NCI_DFM_SECTION_ID_PORT:
		size = ipu_nci_dfm_port_get_section1_size();
		break;
	default:
		assert(0);
		size = 0;
	}
	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_payload_size(
	const vied_nci_dev_dfm_id_t dev,
	const uint32_t port_num)
{
	uint32_t size = 0;
	uint32_t section;

	for (section = VIED_NCI_DFM_SECTION_ID_CMDBANK;
			section < VIED_NCI_DFM_SECTION_ID_N;
			++section)
	{
		size += ipu_resources_dfm_get_section_payload_size(dev, port_num,
			(const vied_nci_dfm_section_id_t)section);
	}
	assert(size > 0);
	return size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_resource_payload_size(
	dfm_port_resource_t dfm_resource)
{
	int payload_size = 0;
	uint32_t dfm_size = 0;
	unsigned int i;

	for (i = 0; i < dfm_resource.value; i++, dfm_resource.offset++) {
		dfm_size = ipu_resources_dfm_get_payload_size(
			dfm_resource.id,
			dfm_resource.offset);
		assert(dfm_size != 0);
		payload_size += dfm_size;
	}

	assert(payload_size > 0);
	return payload_size;
}

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_resource_gather_multi_mask(
	dfm_port_resource_t dfm_resource)
{
	unsigned int i;
	uint32_t nci_port_num;
	uint32_t gather_multi_mask = 0;

	for (i = 0; i < dfm_resource.value; i++, dfm_resource.offset++) {
		nci_port_num = ipu_resources_dfm_get_nci_port_number(dfm_resource.id,
				dfm_resource.offset);
		gather_multi_mask |= (ipu_device_dfm_port_properties_is_empty_port(nci_port_num) ?
				(1 << (nci_port_num)) : (1 << ((nci_port_num) - IPU_DEVICE_DFM_FULL_PORT_START_ID)));
	}

	return gather_multi_mask;
}

STORAGE_CLASS_INLINE uint16_t ipu_resources_dfm_get_dev_port_num_start(
	const vied_nci_dev_dfm_id_t dev)
{
	uint16_t port_num_start;

	assert(dev < VIED_NCI_N_DEV_DFM_ID);
	port_num_start = resource_model_dfm_dev_port_num_start[dev];
	return port_num_start;
}

STORAGE_CLASS_INLINE bool ipu_resources_dfm_is_dev_port_full_type(
	const vied_nci_dev_dfm_id_t dev)
{
	assert(dev < VIED_NCI_N_DEV_DFM_ID);

	return resource_model_dfm_dev_is_full_port_type[dev];
}

STORAGE_CLASS_INLINE
dfm_port_resource_t ipu_resources_dfm_increment_offset(dfm_port_resource_t in, uint32_t inc)
{
	dfm_port_resource_t out = in;

	out.offset = in.offset + inc;

	assert(out.offset < IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS);
	assert(inc < out.value);

	/* Update the size of output resource struct */
	out.value -= inc;

	return out;
}

STORAGE_CLASS_INLINE
uint32_t ipu_resources_dfm_get_port_address(uint32_t fabric_id, dfm_port_resource_t res)
{
	enum ipu_device_dfm_id nci_dev_id = ipu_resources_dfm_get_nci_dev_id(res.id);
	uint32_t nci_port_num = ipu_resources_dfm_get_nci_port_number(res.id, res.offset);
	return ipu_device_dfm_port_addr((enum ipu_fabrics_id)fabric_id, nci_dev_id, nci_port_num);
}

STORAGE_CLASS_INLINE ia_css_process_id_t ipu_resources_dfm_encode_process_id(
		const vied_nci_dev_dfm_id_t dev, const uint32_t port_num)
{
	enum ipu_device_dfm_id dai_dev_id = ipu_resources_dfm_get_nci_dev_id(dev);
	uint32_t nci_port_number = ipu_resources_dfm_get_nci_port_number(dev, port_num);

	return ipu_resources_encode_process_id(
				VIED_NCI_DEVICE_ID_DFM,
				dai_dev_id,
				nci_port_number);
}

STORAGE_CLASS_INLINE nci_eq_token_t ipu_resources_dfm_evq_token_encode(
		enum nci_eq_device_id eq_id,
		const vied_nci_dev_dfm_id_t dev,
		const uint32_t port_num)
{
	ia_css_process_id_t process_id;
	nci_eq_token_t token;
	enum ipu_device_dfm_id dai_dev_id = ipu_resources_dfm_get_nci_dev_id(dev);
	uint32_t nci_port_number = ipu_resources_dfm_get_nci_port_number(dev, port_num);

	process_id = ipu_resources_encode_process_id(
					VIED_NCI_DEVICE_ID_DFM,
					dai_dev_id,
					nci_port_number);

	token = ipu_resources_encode_evq_token(
					eq_id,
					process_id);
	return token;
}

#endif /* __IPU_RESOURCES_DFM_IMPL_H */
