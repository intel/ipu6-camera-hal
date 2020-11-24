/**
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
#ifndef __VIED_NCI_S2V_INLINE_LOCAL_H
#define __VIED_NCI_S2V_INLINE_LOCAL_H

#include "vied_nci_s2v_types.h"             /* for nci_s2v_ack_cfg, nci_s2v_ack_cfg */
#include "vied_nci_s2v_storage_class.h"     /* for NCI_S2V_STORAGE_CLASS_C          */
#include "ipu_device_s2v_devices.h"
#include "assert_support.h"                 /* for assert                           */
#include "type_support.h"                   /* for NULL, uint32_t                   */

/*
 * Fill S2V configuration buffer
 */
NCI_S2V_STORAGE_CLASS_C
int nci_s2v_config(
		enum nci_s2v_device_id s2v_id,
		const struct nci_s2v_cfg *public_cfg,
		const struct nci_s2v_ack_cfg *public_ack_cfg,
		void *buffer)
{
	int size_sec0 = -1;
	int size_sec1 = -1;

	(void) s2v_id;
	assert(public_cfg != NULL);
	assert(public_ack_cfg != NULL);
	assert(buffer != NULL);

	size_sec0 = nci_s2v_fill_section0(public_cfg, buffer);
	buffer = (uint8_t *) buffer + size_sec0;
	size_sec1 = nci_s2v_fill_section1(public_ack_cfg, buffer);

	return size_sec0 + size_sec1;
}

/*
 * Functions below have no implementation for host as host can't
 * access S2V registers directly.
 */
NCI_S2V_STORAGE_CLASS_C
void nci_s2v_start(enum nci_s2v_device_id s2v_id)
{
	(void) s2v_id;
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_run(
	enum nci_s2v_device_id s2v_id,
	uint32_t nof_vectors)
{
	(void) s2v_id;
	(void) nof_vectors;
}

NCI_S2V_STORAGE_CLASS_C
void nci_s2v_cond_run(
	bool cond,
	enum nci_s2v_device_id s2v_id,
	uint32_t nof_vectors)
{
	(void) cond;
	(void) s2v_id;
	(void) nof_vectors;
}

NCI_S2V_STORAGE_CLASS_C
unsigned int nci_s2v_dump_reg_status(
	enum nci_s2v_device_id s2v_id,
	unsigned int reg)
{
	(void) s2v_id;
	(void) reg;

	return 0;
}

#endif /* __VIED_NCI_S2V_INLINE_LOCAL_H */
