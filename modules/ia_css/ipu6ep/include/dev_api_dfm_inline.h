/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2019 Intel Corporation.
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

#ifndef __DEV_API_DFM_INLINE_H
#define __DEV_API_DFM_INLINE_H

#include "ipu_device_subsystem_ids.h"
#include "dev_api_dfm.h"
#include "dev_api_dfm_types.h"
#include "dev_api_dfm_storage_class.h" /* for DEV_API_DFM_STORAGE_CLASS_C */
#include "nci_dfm_types.h"
#include "ipu_device_dfm_properties.h"
#include "nci_dfm_descriptors.h"
#include "ipu_device_dfm_port_properties.h"
#include "ipu_device_dfm_properties_defs.h"
#include "type_support.h"
#include "assert_support.h"
#include "dev_api_dfm_dep.h"

DEV_API_DFM_STORAGE_CLASS_C
uint32_t dev_api_dfm_config_port(const struct dev_api_dfm_stream * const stream,
		struct dev_api_dfm_port * const port)
{
	uint32_t config_size;
	struct dev_api_dfm_agent_property *agent;
	struct nci_dfm_public_cmdbank_config cmdbank_config; /* DFM cmd bank config */

	assert(NULL != stream);
	assert(NULL != port);
	assert(ipu_device_dfm_port_properties_is_port_instantiated(stream->dev_id, port->port_num));

	port->port_config.rc_cnt.iter = 0;
	port->port_config.rc_cnt.in_event = 0;
	if ((ipu_device_dfm_port_properties_is_empty_port(port->port_num)) && (port->mode == DEV_API_DFM_ACTIVE)) {
		port->port_config.rc_cnt.iter = stream->seq_iter;
		port->port_config.rc_cnt.in_event = port->priming;
	}

	/* Begin iteration should always be > 0 */
	assert(port->port_config.bc_iter.begin_iter >= 1);

	port->port_config.bc_seq_type.dfm_seq_type_init_type   =
		(ipu_device_dfm_init_type_t)dev_api_get_init_seq(port->agent[DFM_SEQ_TYPE_IDX_BEGIN].type);
	port->port_config.bc_seq_type.dfm_seq_type_begin_type  =
		dev_api_get_seq_type(port->agent[DFM_SEQ_TYPE_IDX_BEGIN].type);
	port->port_config.bc_seq_type.dfm_seq_type_middle_type =
		(ipu_device_dfm_seq_type)(port->port_config.bc_iter.middle_iter ?
		dev_api_get_seq_type(port->agent[DFM_SEQ_TYPE_IDX_MIDDLE].type) : 0);
	port->port_config.bc_seq_type.dfm_seq_type_end_type    =
		(ipu_device_dfm_seq_type)(port->port_config.bc_iter.end_iter ?
		dev_api_get_seq_type(port->agent[DFM_SEQ_TYPE_IDX_END].type) : 0);

	assert((port->port_config.bcpr_config.buff_chasing_en == 0) ||
		(ipu_device_dfm_port_properties_is_empty_port(port->port_num) && (port->mode == DEV_API_DFM_ACTIVE)));

	/*****************************************************/
	/* Prepare the command bank configuration...         */
	/*****************************************************/

	/* Configuration */
	cmdbank_config.port_num = port->port_num;
	cmdbank_config.ack_done_addr = stream->ack_addr + dev_api_dfm_get_fabric_offset(stream->dev_id);
	cmdbank_config.ack_done_cmd = stream->ack_data;
	/* BEGIN */
	agent = &(port->agent[DFM_SEQ_TYPE_IDX_BEGIN]);
	cmdbank_config.begin.agent_address = get_agent_address(stream->dev_id, agent);
	cmdbank_config.begin.cmd = agent->cmd;
	cmdbank_config.begin.size = agent->seq_num_cmds;
	/* MIDDLE */
	agent = &(port->agent[DFM_SEQ_TYPE_IDX_MIDDLE]);
	if (port->port_config.bc_iter.middle_iter) {
		cmdbank_config.middle.agent_address = get_agent_address(stream->dev_id, agent);
		cmdbank_config.middle.cmd = agent->cmd;
		cmdbank_config.middle.size = agent->seq_num_cmds;
	} else {
		cmdbank_config.middle.agent_address = 0;
		cmdbank_config.middle.cmd = 0;
		cmdbank_config.middle.size = 0;
	}
	/* END */
	agent = &(port->agent[DFM_SEQ_TYPE_IDX_END]);
	if (port->port_config.bc_iter.end_iter) {
		cmdbank_config.end.agent_address = get_agent_address(stream->dev_id, agent);
		cmdbank_config.end.cmd = agent->cmd;
		cmdbank_config.end.size = agent->seq_num_cmds;
	} else {
		cmdbank_config.end.agent_address = 0;
		cmdbank_config.end.cmd = 0;
		cmdbank_config.end.size = 0;
	}

	/* The following function will disable the port before configuration.			*/
	/* This should happen ONLY in ISP flow. The host implementation should be empty.	*/
	/* For VP-less this is being done from the SPC.
	*/
	ipu_nci_dfm_disable_port(stream->dev_id, port->port_num);

	config_size = ipu_nci_dfm_port_fill_all_sections(stream->dev_id, port->port_num,
			&port->port_config,	/* Device port config */
			&cmdbank_config,	/* Command bank config */
			port->config_buffer);
	/* The following function will enable passive ports after configuration.		*/
	/* This should happen ONLY in ISP flow. The host implementation should be empty.	*/
	/* For VP-less this is being done from the SPC.						*/
	ipu_nci_dfm_enable_passive_port(stream->dev_id, port->port_num, (port->mode == DEV_API_DFM_PASSIVE));

	return config_size;
}

DEV_API_DFM_STORAGE_CLASS_C
void dev_api_dfm_config_ports(struct dev_api_dfm_stream *stream, struct dev_api_dfm_port *port)
{
	unsigned p;

	for (p = 0; p < (stream->num_prod + stream->num_cons); p++) {
		dev_api_dfm_config_port(stream, &port[p]);
	}
}

#endif /* __DEV_API_DFM_INLINE_H */
