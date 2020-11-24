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

#ifndef __NCI_V2S_H
#define __NCI_V2S_H

#include "nci_v2s_types.h"
#include "nci_v2s_storage_class.h"
#include "dai_ipu_top_devices.h"

NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_nof_sections(void);

NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_sizeof_section(unsigned int section_id);

NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_offsetof_section(
		unsigned int section_id);

NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_sizeof_blob(void);

NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_fill_config(
		dai_ipu_top_vec_to_str_v3_instances_t dev_id,
		const struct nci_v2s_cfg *cfg,
		void *buffer);

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_print_config(
	void *buffer);

NCI_V2S_STORAGE_CLASS_H
void nci_v2s_set_command(
	int32_t dev_id,
	const struct nci_v2s_cmd *cfg);

/**
 * @brief Get V2S procees N vectors command offset
 *
 * Get V2S process N vectors command offset relative to V2S base address.
 */
NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_command_offset(void);

/**
 * @brief Get V2S procees N vectors command
 *
 * @param[in] nof_vectors	Number of vectors per frame
 *
 * Get V2S process N vectors command value to be written by DFM sequencer to V2S command address.
 */
NCI_V2S_STORAGE_CLASS_H
uint32_t nci_v2s_get_command_token(unsigned nof_vectors);

#ifdef __INLINE_NCI_V2S__
#include "nci_v2s_impl.h"
#endif

#endif /* __NCI_V2S_H */
