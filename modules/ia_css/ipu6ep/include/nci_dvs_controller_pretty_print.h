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

#ifndef __NCI_DVS_CONTROLLER_PRETTY_PRINT_H
#define __NCI_DVS_CONTROLLER_PRETTY_PRINT_H

#include "nci_dvs_controller.h"
#include "nci_dvs_controller_trace.h"
#include "misc_support.h"

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
void nci_dvs_controller_print_section_ack_cfg(
	void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_H
void nci_dvs_controller_print_section3(
	void *buffer);

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
void nci_dvs_controller_print_section_ack_cfg(
	void *buffer)
{
	struct nci_dvs_controller_private_section_ack_cfg *priv = (struct nci_dvs_controller_private_section_ack_cfg *) buffer;

	assert(buffer != NULL);

	IA_CSS_TRACE_1(NCI_DVS_CONTROLLER, DEBUG, "    ack_data = %d\n", priv->ack_data);
	IA_CSS_TRACE_1(NCI_DVS_CONTROLLER, DEBUG, "    ack_addr = %d\n", priv->ack_addr);

	/* Prevent compilation warning (=error) when not tracing in debug level */
	NOT_USED(priv);
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
void nci_dvs_controller_print_section3(
	void *buffer)
{
	struct nci_dvs_controller_private_section3 *priv = (struct nci_dvs_controller_private_section3 *) buffer;

	assert(buffer != NULL);

	IA_CSS_TRACE_1(NCI_DVS_CONTROLLER, DEBUG, "    dvs_type = %d\n", priv->dvs_type);
	IA_CSS_TRACE_1(NCI_DVS_CONTROLLER, DEBUG, "    acb_addr = %d\n", priv->acb_addr);

	/* Prevent compilation warning (=error) when not tracing in debug level */
	NOT_USED(priv);
}

NCI_DVS_CONTROLLER_STORAGE_CLASS_C
void nci_dvs_controller_print_config(
	void *buffer)
{
	char *buffer_char = (char *)buffer;
	uint32_t size_sec0 = nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION0);
	uint32_t size_sec1 = nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION1);
	uint32_t size_sec2 = nci_dvs_controller_get_sizeof_section(NCI_DVS_CONTROLLER_SECTION2);

	assert(buffer_char != NULL);

	IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, DEBUG, "=== NCI_DVS_CONTROLLER configuration ===\n");
	IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, DEBUG, "=== NCI_DVS_CONTROLLER_SECTION0 ===\n");
	nci_dvs_controller_print_section_ack_cfg(buffer_char);
	buffer_char += size_sec0;

	IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, DEBUG, "=== NCI_DVS_CONTROLLER_SECTION1 ===\n");
	nci_dvs_controller_print_section_ack_cfg(buffer_char);
	buffer_char += size_sec1;

	IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, DEBUG, "=== NCI_DVS_CONTROLLER_SECTION2 ===\n");
	nci_dvs_controller_print_section_ack_cfg(buffer_char);
	buffer_char += size_sec2;

	IA_CSS_TRACE_0(NCI_DVS_CONTROLLER, DEBUG, "=== NCI_DVS_CONTROLLER_SECTION3 ===\n");
	nci_dvs_controller_print_section3(buffer_char);
}

#endif /* __NCI_DVS_CONTROLLER_PRETTY_PRINT_H */
