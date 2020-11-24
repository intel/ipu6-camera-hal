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

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
void ipu_nci_dfm_enable_passive_port(uint32_t dev_id, uint32_t port_num, bool is_passive_mode)
{
	/* NOTE1: The following function will enable passive ports after configuration	*/
	/* ONLY in ISP flow. The host implementation should be empty.			*/
	/* For VP-less this is being done from the SPC.					*/
	/* NOTE2 : the buffer_ctrl has already been enabled as				*/
	/* part of the configuration (last register of section1)			*/
	NOT_USED(dev_id);
	NOT_USED(port_num);
	NOT_USED(is_passive_mode);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
void ipu_nci_dfm_disable_port(uint32_t dev_id, uint32_t port_num)
{
	/* NOTE: This function will disable the port before configuration	*/
	/* ONLY in ISP flow.							*/
	/* The host implementation should be empty.				*/
	/* For VP-less this is being done from the SPC.				*/
	NOT_USED(dev_id);
	NOT_USED(port_num);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint32_t ipu_nci_dfm_port_fill_all_sections(uint32_t dev_id, uint32_t port_num,
		const struct nci_dfm_port_config *dfm_config,
		const struct nci_dfm_public_cmdbank_config *cmdbank_config,
		void *buffer)
{
	uint32_t offset;
	/* Size of the two blobs combined */
	uint32_t total_size = 0;

	/* Verify we are receiving a valid pointer */
	assert(NULL != buffer);

	assert(ipu_device_dfm_port_properties_is_port_instantiated(dev_id, port_num));

	/********************************************************/
	/* The command bank config. must be section0 because    */
	/* section1 (device port regs) will enable the buffer   */
	/* control, and after this is done, we cannot write in  */
	/* these registers anymore.                             */
	/********************************************************/

	offset = total_size;
	/* Generate section0 (cmd bank) and get offset */
	total_size += ipu_nci_dfm_port_fill_section0(dev_id, cmdbank_config, (uint8_t *)buffer + offset);
	/* Pretty print (only when using NCI_DFM_TRACE_LOG_LEVEL_DEBUG) */
	ipu_nci_dfm_port_print_section0(dev_id, port_num, (uint8_t *)buffer + offset);

	offset = total_size;
	/* Generate section1 (device port) and get offset */
	total_size += ipu_nci_dfm_port_fill_section1(dfm_config, (uint8_t *)buffer + offset);
	/* Pretty print (only when using NCI_DFM_TRACE_LOG_LEVEL_DEBUG) */
	ipu_nci_dfm_port_print_section1((uint8_t *)buffer + offset);

	return total_size;
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
void ipu_nci_dfm_port_print_all_sections_from_registers(uint32_t dev_id, uint32_t port_num)
{
	/* Valid only from cells */
	NOT_USED(dev_id);
	NOT_USED(port_num);
}

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_C
uint8_t ipu_nci_dfm_port_buff_ctrl_en_default_value(void)
{
	/* For VP-less the buff_ctrl_en field for each port should be 0 by default.		*/
	/* The Process Manager of PSYS Server will enable all ports after the program control	*/
	/* and connect sections have been loaded.						*/
	return 0;
}

