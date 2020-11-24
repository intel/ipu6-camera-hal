/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2017 Intel Corporation.
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

#ifndef __VIED_NCI_S2V_H
#define __VIED_NCI_S2V_H

#include "vied_nci_s2v_types.h"          /* for nci_s2v_ack_cfg, nci_s2v_cfg */
#include "vied_nci_s2v_storage_class.h"  /* for NCI_S2V_STORAGE_CLASS_H      */
#include "ipu_device_s2v_devices.h"      /* for nci_s2v_device_id            */
#include <type_support.h>                /* for uint32_t                     */

/* Description : Declarations of S2V NCI functions
*/

NCI_S2V_STORAGE_CLASS_H
int nci_s2v_config(
		enum nci_s2v_device_id s2v_id,
		const struct nci_s2v_cfg *public_cfg,
		const struct nci_s2v_ack_cfg *public_ack_cfg,
		void *buffer);

NCI_S2V_STORAGE_CLASS_H
int nci_s2v_get_sizeof_section(enum nci_s2v_section_id id);

NCI_S2V_STORAGE_CLASS_H
int nci_s2v_get_sizeof_sections(void);

NCI_S2V_STORAGE_CLASS_H
int nci_s2v_get_numberof_sections(void);

NCI_S2V_STORAGE_CLASS_H
void nci_s2v_print_sections(void *buffer);

NCI_S2V_STORAGE_CLASS_H
int nci_s2v_get_offsetof_section(enum nci_s2v_section_id id);

NCI_S2V_STORAGE_CLASS_H
void nci_s2v_start(enum nci_s2v_device_id s2v_id);

NCI_S2V_STORAGE_CLASS_H
unsigned int nci_s2v_dump_reg_status(
	enum nci_s2v_device_id s2v_id,
	unsigned int reg);

NCI_S2V_STORAGE_CLASS_H
unsigned int nci_s2v_get_command_register_offset(void);

NCI_S2V_STORAGE_CLASS_H
unsigned int nci_s2v_get_process_N_vectors_command(unsigned int nof_vectors);

NCI_S2V_STORAGE_CLASS_H
void nci_s2v_run(
	enum nci_s2v_device_id s2v_id,
	uint32_t nof_vectors);

NCI_S2V_STORAGE_CLASS_H
void nci_s2v_cond_run(
	bool cond,
	enum nci_s2v_device_id s2v_id,
	uint32_t nof_vectors);

#ifdef __INLINE_NCI_S2V__
#include "vied_nci_s2v_inline.h"
#endif

#endif /* __VIED_NCI_S2V_H */
