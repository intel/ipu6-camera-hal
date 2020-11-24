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

#ifndef __DEV_API_V2S_INLINE_H
#define __DEV_API_V2S_INLINE_H

#include "dev_api_v2s.h"
#include "dev_api_v2s_trace.h"
#include "nci_v2s.h"
#include "vec_to_str_v3_properties.h"
#include "dai_ipu_top_devices.h"
#include "assert_support.h"

#define INIT_OPCODE		0x10
#define	INIT_ARGS		0x00
#define PROC_N_VEC_OPCODE	0x01

/* Each format needs that a certain number of
 * buffers are configured.
 * The information about how many buffers are needed
 * per format can be found on the MAS. */
STORAGE_CLASS_INLINE unsigned int
get_n_of_buffers_per_format(enum nci_v2s_format format)
{
	unsigned int n_buffers;

	n_buffers = 4; /* Bayer not interleaved and YUV 420 */

	if (format == V2S_FORMAT_BAYER_INTERLEAVED_4PPC) {
		n_buffers = 2;
	} else if (format == V2S_FORMAT_RGB_4PPC) {
		n_buffers = 6;
	} else if (format == V2S_FORMAT_BAYER_4PPC ||
			format == V2S_FORMAT_YUV420_4PPC ||
			format == V2S_FORMAT_BAYER_ON_YUV420_4PPC ||
			format == V2S_FORMAT_BAYER_INTERLEAVED_4PPC) {
		n_buffers = 4;
	} else {
		IA_CSS_TRACE_1(DEV_API_V2S, ERROR, "dev_api_v2s invalid format || {format = %u}\n", format);
		assert(0);
	}
	return n_buffers;
}

/* Configure V2S device with device/fragment parameters
 * and buffer configuration
 */
DEV_API_V2S_STORAGE_CLASS_C void
dev_api_v2s_configure_device(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		struct dev_api_v2s_config *cfg,
		void *payload_address)
{
	struct nci_v2s_cfg v2s_cfg;
	int i = 0;

	IA_CSS_TRACE_1(DEV_API_V2S, VERBOSE, "dev_api_v2s_configure_device || {dev_id = %u}\n", dev_id);

	assert(cfg != NULL);
	assert(cfg->format < NUM_V2S_FORMAT);
	assert(dev_id < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);

	/* The number of buffers supported by the API must be less or equal
	 * than the actual number of buffers supported by the device */
	assert(DEV_API_V2S_NUM_BUFFERS >= dai_vec_to_str_v3_get_hwp_nof_bufs(dev_id));

	v2s_cfg.format = cfg->format;
	v2s_cfg.dev_cfg = cfg->device_config;
	v2s_cfg.ack_cfg = cfg->ack_config;

	for (i = 0; i < V2S_NUM_BUFFERS; i++) {
		v2s_cfg.buf_cfg[i] = cfg->buffer_config[i];
	}

	nci_v2s_fill_config(dev_id, &v2s_cfg, payload_address);
}

/* Send a command to initialize the device */
DEV_API_V2S_STORAGE_CLASS_C void
dev_api_v2s_init_device(dai_ipu_top_vec_to_str_v3_instances_t dev_id)
{
	struct nci_v2s_cmd cmd = {INIT_OPCODE, INIT_ARGS};

	IA_CSS_TRACE_1(DEV_API_V2S, VERBOSE, "dev_api_v2s_init_device || {dev_id = %u}\n", dev_id);

	assert(dev_id < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);

	/* Set command encoding according to Section 1.5.1 in V2S v3 MAS */
	nci_v2s_set_command(dev_id, &cmd);
}

/* Send a PROCESS_N_VECTORS command.
 */
DEV_API_V2S_STORAGE_CLASS_C void
dev_api_v2s_process_n_vectors(dai_ipu_top_vec_to_str_v3_instances_t dev_id, unsigned int n_vectors)
{
	struct nci_v2s_cmd cmd = {PROC_N_VEC_OPCODE, n_vectors};

	IA_CSS_TRACE_2(DEV_API_V2S, VERBOSE, "dev_api_v2s_process_n_vectors || {dev_id = %u, n_vectors = %d}\n",
						dev_id, n_vectors);

	assert(dev_id < NUM_DAI_IPU_TOP_VEC_TO_STR_V3_INSTANCES);

	/* TODO Check that n_vectors is compatible with current format
	 * See MAS sect. 1.5.1.2.1 (command constraints)
	 * OPEN How can we get the format given the device id? */
	/* Example: how to implement the format() below?
	assert(
		(n_vectors % 4 &&
		 	format(dev_id) == V2S_FORMAT_BAYER_4PPC) ||
		(n_vectors % 2 &&
		 	format(dev_id) == V2S_FORMAT_BAYER_INTERLEAVED_4PPC) ||
		(n_vectors % 6 &&
		 	format(dev_id) == V2S_FORMAT_YUV420_4PPC) ||
		(n_vectors % 6 &&
		 	format(dev_id) == V2S_FORMAT_RGB_4PPC));
	*/

	/* TODO should we add some state to ensure that proc_n_vec is sent after init? */

	/* Set command encoding according to Section 1.5.1 in V2S v3 MAS */
	nci_v2s_set_command(dev_id, &cmd);
}

#endif /* __DEV_API_V2S_INLINE_H */
