/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_DFM_PORT_PROPERTIES_FUNC_H
#define __IPU_DEVICE_DFM_PORT_PROPERTIES_FUNC_H
#include "ipu_device_dfm_port_properties.h"
#include "assert_support.h"
#include "misc_support.h"
#include "type_support.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE bool ipu_device_dfm_port_properties_is_port_instantiated(
		const uint32_t dev_id,
		const uint32_t port_num)
{
	bool ret_val;
	uint32_t num_port_pairs = ipu_device_dfm_get_num_ports(dev_id);

	ret_val = ipu_device_dfm_port_properties_is_empty_port(port_num) ?
			(port_num < num_port_pairs) :
			(port_num < (num_port_pairs + IPU_DEVICE_DFM_FULL_PORT_START_ID));

	return ret_val;
}

STORAGE_CLASS_INLINE uint8_t ipu_device_dfm_port_properties_get_cmd_seq_size(const uint32_t dev_id,
		const uint32_t port_num,
		const ipu_device_dfm_seq_type_idx seq)
{
	uint8_t size = 0;
	NOT_USED(seq);

	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));

	if (ipu_device_dfm_port_properties_is_port_instantiated(dev_id, port_num))
	{
		/* In IPU6EP all ports have same capability of 2 64-bit command tokens for each sequence */
		size = IPU_DEVICE_DFM_MAX_NUM_CMD_TOKENS * IPU_DEVICE_DFM_CF_NEXT_CMD_TOKEN_OFFSET;
	}
	return size;
}

STORAGE_CLASS_INLINE uint8_t ipu_device_dfm_port_properties_get_cmdbank_config_size(const uint32_t dev_id,
		const uint32_t port_num)
{
	uint8_t size = 0;

	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));

	/* In IPU6EP all ports have same capabilities and support 3 command sequences*/
	if (ipu_device_dfm_port_properties_is_port_instantiated(dev_id, port_num)) {
		size = ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_BEGIN) * IPU_DEVICE_DFM_MAX_NUM_SEQUENCE_TYPES;
	}

	/* Empty device ports will have additional entry for the ack_done. */
	if (ipu_device_dfm_port_properties_is_empty_port(port_num) &&
		ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_BEGIN)) {
		size += (IPU_DEVICE_DFM_CF_NEXT_CMD_TOKEN_OFFSET); /* Ack Addr and Ack done */
	}
	return size;
}

STORAGE_CLASS_INLINE bool ipu_device_dfm_port_properties_is_empty_port(const uint32_t port_num)
{
	return (port_num < IPU_DEVICE_DFM_FULL_PORT_START_ID);
}

#endif /* __IPU_DEVICE_DFM_PORT_PROPERTIES_FUNC_H */
