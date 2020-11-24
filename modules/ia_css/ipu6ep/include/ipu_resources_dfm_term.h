/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __IPU_RESOURCES_DFM_TERM_H
#define __IPU_RESOURCES_DFM_TERM_H

#include "storage_class.h"
#include "dev_api_dfm_types.h"
#include "vied_nci_psys_resource_model.h"
#include "vied_nci_psys_resource_model_private.h"
#include "ipu_device_dma_devices.h"
#include "ipu_device_dma_properties.h"
#include "ia_css_psys_terminal_private_types.h"
#include "dfm_resource_types.h"
#include "pg_control_init_support.h"

/*!
 * (public) DFM descriptor configuration.
 */
struct ipu_resources_dfm_descriptor_config {
	dev_api_dfm_port_s port_config;
	dev_api_dfm_stream_s stream_config;
};

/*!
 * Fills in the program control init load section descriptor for DFM
 * port (it sets mem_offset, mem_size and device_descriptor_id).
 *
 * @param[in] prog_ctrl_init_prog_desc          program descriptor pointer
 * @param[in] dev            DFM device ID from the resource model.
 * @param[in] port_number    DFM port number
 * @param[in] section_id     Section ID
 * @param[in] mode_bitmask   Specifies bitmask of the processing modes of the section
 *
 *
 * @return                    Payload size in section in bytes.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_set_progctrlinit_section(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
	const vied_nci_dev_dfm_id_t  dev,
	const uint32_t               port_number,
	const vied_nci_dfm_section_id_t section_id,
	const uint32_t               mode_bitmask);

/*!
 * Fills in all the program control init load section descriptors for DFM each
 * port in a specified DFM resource (contiguous ports)
 *
 * @param[in] prog_ctrl_init_prog_desc           program descriptor pointer
 * @param[in] dfm_resource   DFM resource (contiguous ports)
 * @param[in] mode_bitmask   Specifies bitmask of the processing modes of the section
 *
 * @return                   Total payload size in bytes.
 */
STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_set_progctrlinit_sections_resource(
	ia_css_program_control_init_program_desc_t *prog_ctrl_init_prog_desc,
		dfm_port_resource_t dfm_resource,
		uint32_t mode_bitmask);

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_get_progctrlinit_sections_size(
		dfm_port_resource_t dfm_resource);

/*!
 * Fills in the program control init terminal payload DFM ports.
 *
 * @param[in]  dev                   DFM device ID from the resource model.
 * @param[in]  port_number           DFM port number
 * @param[in]  public_config         DFM public configuration
 * @param[in]  load_section_desc     Program control init terminal load section descriptor.
 * @param[out] buffer                Program control init terminal (payload) pointer.
 *
 * @return bytes written (0 on error)
 */

STORAGE_CLASS_INLINE uint32_t ipu_resources_dfm_fill_progctrlinit_payload(
	const vied_nci_dev_dfm_id_t     dev,
	const uint32_t                  port_number,
	struct ipu_resources_dfm_descriptor_config *public_config,
	load_section_desc_list_t		load_sec_desc_list,
	void                            *buffer);

/*!
 * Print the program control init terminal payload for DFM port.
 *
 * @param[in] dev                   DFM device ID from the resource model.
 * @param[in] port_number           DFM port number
 * @param[in] load_section_desc     Program control init terminal load section descriptor.
 * @param[in] buffer                Program control init terminal (payload) pointer.
 *
 */
STORAGE_CLASS_INLINE void ipu_resources_dfm_print_progctrlinit_payload(
	const vied_nci_dev_dfm_id_t    dev,
	const uint32_t                 port_number,
	load_section_desc_list_t		load_sec_desc_list,
	const void                     *buffer);

#include "ipu_resources_dfm_term_impl.h"

#endif /* __IPU_RESOURCES_DFM_TERM_H */
