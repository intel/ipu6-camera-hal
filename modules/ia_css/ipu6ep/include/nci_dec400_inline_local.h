
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

#ifndef __NCI_DEC400_IMPL_LOCAL_H
#define __NCI_DEC400_IMPL_LOCAL_H

#include "nci_dec400.h"
#include "nci_dec400_struct.h"
#include "nci_dec400_vpless_defs.h"
#include "assert_support.h"
#include "misc_support.h"
#include "ipu_device_dec400_devices.h" /* for ipu_device_dec400_get_type */

NCI_DEC400_STORAGE_CLASS_C
void nci_dec400_frame_config_fill_sections(
	nci_dec400_device_id dev_id,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	uint32_t *fill_buffer = (uint32_t *)buffer;

	assert(cfg != NULL);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);

	nci_dec400_fill_section0_config(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_CONFIG]);
	nci_dec400_fill_section1_exconfig(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_EX_CONFIG]);
	nci_dec400_fill_section5_stride(cfg, &fill_buffer[NCI_DEC400_FILL_SECTIONS_STRIDE]);
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_channel_config(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	assert(cfg != NULL);
	assert(buffer);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	nci_dec400_channel_config_fill_sections(dev_id, cfg, buffer);

	return nci_dec400_get_sizeof_channel_blob();
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_buffer_config(
	nci_dec400_device_id dev_id,
	uint8_t channel,
	const struct nci_dec400_stream_cfg_t *cfg,
	void *buffer)
{
	assert(cfg != NULL);
	assert(buffer);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	nci_dec400_buffer_config_fill_sections(dev_id, channel, cfg, buffer);
	return nci_dec400_get_sizeof_buffer_blob();
}

NCI_DEC400_STORAGE_CLASS_C
uint32_t nci_dec400_frame_config(
		nci_dec400_device_id dev_id,
		uint8_t channel,
		const struct nci_dec400_stream_cfg_t *cfg,
		void *buffer)
{
	assert(cfg != NULL);
	assert(buffer);
	assert(dev_id < NCI_DEC400_NUM_DEVICES);
	assert(channel < IPU_DEVICE_DEC400_NUM_OF_CHANNELS);

	/* not used in host implementation */
	NOT_USED(dev_id);

	nci_dec400_frame_config_fill_sections(dev_id, cfg, buffer);

	return nci_dec400_get_sizeof_connected_blob_per_stream();
}

#endif /* __NCI_DEC400_IMPL_LOCAL_H */
