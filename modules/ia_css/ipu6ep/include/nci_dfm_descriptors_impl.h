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

#include "nci_dfm_descriptors.h"
#include "nci_dfm_trace.h"
#include "assert_support.h"
#include "misc_support.h"
#include "ipu_nci_dfm_struct.h"
#include "ipu_device_subsystem_ids.h"
#ifndef USE_DFM_DAI
#include "ipu_device_dfm_properties.h"
#include "ipu_device_dfm_port_properties.h"
#endif
#include "nci_dfm_reg_pack_unpack.h"
#include "nci_dfm_private_types.h"
#include "ipu_device_dfm_port_properties.h"

/* Include local implementation: */
#include "nci_dfm_descriptors_impl_local.h"
#include "nci_dfm_descriptors_impl_dep.h"

/***********/
/* Private */
/***********/
STORAGE_CLASS_INLINE
void ipu_nci_dfm_port_fill_cmdbank_seq(const struct nci_dfm_sequence *seq, void *buffer);

/***********/
/* Public  */
/***********/
STORAGE_CLASS_INLINE
void ipu_nci_dfm_port_fill_cmdbank_seq(const struct nci_dfm_sequence *seq, void *buffer)
{
	uint32_t *output = (uint32_t *)buffer;
	uint32_t i = 0;
	uint32_t output_size = 0;

	assert(NULL != seq);
	assert(NULL != buffer);
	assert(seq->size <= NCI_DFM_DESC_SEQ_MAX_NUM_CMDS); /* Number of commands per sequence */

	output[output_size++] = seq->agent_address;
	for (i = 0; i < seq->size; i++) {
		output[output_size++] = seq->cmd[i];
	}
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_fill_section0(uint32_t dev_id,
		const struct nci_dfm_public_cmdbank_config *cmdbank_config,
		void *buffer)
{
	uint32_t *ack_done;
	uint32_t offset = 0;

	assert(cmdbank_config->port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));
	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(NULL != cmdbank_config);
	assert(NULL != buffer);
	assert(cmdbank_config->port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));

	if (cmdbank_config->begin.size > 0) {
		/* Verify that according to the properties, this sequence exists. */
		assert(ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
				DFM_SEQ_TYPE_IDX_BEGIN) > 0);
		ipu_nci_dfm_port_fill_cmdbank_seq(&cmdbank_config->begin, buffer);
	}

	offset += ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
			DFM_SEQ_TYPE_IDX_BEGIN);
	if (cmdbank_config->middle.size > 0) {
		/* Verify that according to the properties, this sequence exists. */
		assert(ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
				DFM_SEQ_TYPE_IDX_MIDDLE) > 0);
		assert(cmdbank_config->begin.size > 0);
		ipu_nci_dfm_port_fill_cmdbank_seq(&cmdbank_config->middle, (uint8_t *)buffer + offset);
	}

	offset += ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
			DFM_SEQ_TYPE_IDX_MIDDLE);
	if (cmdbank_config->end.size > 0) {
		/* Verify that according to the properties, this sequence exists. */
		assert(ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
				DFM_SEQ_TYPE_IDX_END) > 0);
		assert(cmdbank_config->middle.size > 0);
		ipu_nci_dfm_port_fill_cmdbank_seq(&cmdbank_config->end, (uint8_t *)buffer + offset);
	}

	offset += ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
			DFM_SEQ_TYPE_IDX_END);

	/* Empty device ports will have additional entry for the ack_done. */
	if (ipu_device_dfm_port_properties_is_empty_port(cmdbank_config->port_num) &&
		(ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, cmdbank_config->port_num,
			DFM_SEQ_TYPE_IDX_BEGIN))) {
		ack_done = (uint32_t *) ((uint8_t *)buffer + offset);
		ack_done[0] = cmdbank_config->ack_done_addr;
		ack_done[1] = cmdbank_config->ack_done_cmd;
		offset += (IPU_DEVICE_DFM_CF_NEXT_CMD_TOKEN_OFFSET); /* Ack Addr and Ack done */
	}

	/* Sanity check: */
	assert(offset == ipu_nci_dfm_port_get_section0_size(dev_id, cmdbank_config->port_num));
	return offset; /* This is equal to the config size. */
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
void ipu_nci_dfm_port_print_section0(uint32_t dev_id, uint32_t port_num, const void *buffer)
{
/* Prevent unnecessary workload when no pretty print is required */
#if (NCI_DFM_TRACE_CONFIG == NCI_DFM_TRACE_LOG_LEVEL_DEBUG)
	uint32_t *loc_to_print;
	uint32_t size_of_begin_seq = 0;
	uint32_t size_of_middle_seq = 0;
	uint32_t size_of_end_seq = 0;
	uint32_t i = 0;

	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));
	assert(NULL != buffer);

	size_of_begin_seq = ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_BEGIN);
	size_of_middle_seq = ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_MIDDLE);
	size_of_end_seq = ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_END);

	IA_CSS_TRACE_0(NCI_DFM, DEBUG,
		"ipu_nci_dfm_port_print_section0:\n");
	loc_to_print = (uint32_t *)buffer;
	for (i = 0; i < (size_of_begin_seq/(sizeof(uint32_t))); i++) {
		IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t begin         = 0x%08x\n", *(loc_to_print++));
	}
	for (i = 0; i < (size_of_middle_seq/(sizeof(uint32_t))); i++) {
		IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t middle        = 0x%08x\n", *(loc_to_print++));
	}
	for (i = 0; i < (size_of_end_seq/(sizeof(uint32_t))); i++) {
		IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t end           = 0x%08x\n", *(loc_to_print++));
	}
	/* Empty device ports will have additional entry for the ack_done. */
	if (ipu_device_dfm_port_properties_is_empty_port(port_num) && size_of_begin_seq) {
		IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t ack_done.addr = 0x%08x\n", *(loc_to_print++));
		IA_CSS_TRACE_1(NCI_DFM, DEBUG, "\t ack_done.cmd  = 0x%08x\n", *(loc_to_print++));
	}
#else
	NOT_USED(dev_id);
	NOT_USED(port_num);
	NOT_USED(buffer);
#endif
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_section0_size(uint32_t dev_id, uint32_t port_num)
{
	uint32_t size;

	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));

	size = ipu_device_dfm_port_properties_get_cmdbank_config_size(dev_id, port_num);

	return size;
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_all_sections_size(uint32_t dev_id, uint32_t port_num)
{
	assert(ipu_device_dfm_port_properties_is_port_instantiated(dev_id, port_num));

	return ipu_nci_dfm_port_get_section0_size(dev_id, port_num) +
			ipu_nci_dfm_port_get_section1_size();
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_get_otf_ack_data_offset(uint32_t dev_id, uint32_t port_num, bool is_on_middle)
{
	uint32_t offset = 0;

	assert(port_num < (IPU_DEVICE_DFM_NUM_MAX_PORTS_PAIRS * IPU_DEVICE_DFM_NUM_PORTS_IN_PAIR));
	assert(dev_id < IPU_DEVICE_DFM_NUM_OF_DEVICES);
	/* Verify that according to the properties, this sequence exists. */
	assert(ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_BEGIN) > 0);

	if (is_on_middle) {
		offset +=  ipu_device_dfm_port_properties_get_cmd_seq_size(dev_id, port_num, DFM_SEQ_TYPE_IDX_BEGIN);
	}

	offset += 3 * sizeof(uint32_t); /*This DFM command is resides in memory as: address, cmd1, cmd2, cmd3. cmd3 is the ack data that we modify*/

	return offset;
}
