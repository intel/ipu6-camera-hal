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

#ifndef __NCI_DFM_DESCRIPTORS_H_
#define __NCI_DFM_DESCRIPTORS_H_

#include "nci_dfm_types.h"
#include "type_support.h"
#include "nci_dfm_descriptors_storage_class.h"

/**
 * @brief Fills a buffer with the DFM device port descriptor.
 * @param[in] dfm_config	Configuration of the DFM port
 * @param[out] buffer		Buffer to write the DFM port descriptor (i.e. configuration blob to be
 *				transferred to registers).
 * @return			The size of the config written in the buffer
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_fill_section1(const struct nci_dfm_port_config *dfm_config,
		void *buffer);

/**
 * @brief Retrieves the size of the buffer that the function ipu_nci_dfm_port_fill_section1
 * will be filling.
 * @return the size of the DFM port descriptor
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_section1_size(void);

/**
 * @brief Calculates the device port config. offset (from the base address of a DFM device) for the given port
 * @param port_num	Port number
 * @return the device port config. offset (from the base address of a DFM device) for the given port
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_section1_offset(uint32_t port_num);

/**
 * @brief Fills a buffer with the command bank port descriptor.
 * @param dev_id		Device id
 * @param[in] cmdbank_config	Configuration of the command bank for one port.
 * @param[out] buffer		Buffer where the configuration blob is stored.
 * @return			The size of the config written in the buffer
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_fill_section0(uint32_t dev_id,
		const struct nci_dfm_public_cmdbank_config *cmdbank_config,
		void *buffer);

/**
 * @brief Retrieves the size of the buffer that the function ipu_nci_dfm_port_fill_section0
 * will be filling.
 * @param dev_id	Device id
 * @param port_num	Port number
 * @return the size of the command bank port descriptor
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_section0_size(uint32_t dev_id, uint32_t port_num);

/**
 * @brief Calculates the command bank port config. offset (from the base address of a DFM device) for the given port
 * @param dev_id	Device id (the properties of the device are needed to calculate the offset of the port)
 * @param port_num	Port number
 * @return the command bank port config. offset (from the base address of a DFM device) for the given port
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_section0_offset(uint32_t dev_id, uint32_t port_num);

/**
 * @brief Makes a pretty print of the device port configuration
 * @param[in] buffer	The configuration blob to unpack and print.
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
void ipu_nci_dfm_port_print_section1(const void *buffer);

/**
 * @brief Makes a pretty print of the command bank port configuration
 * @param dev_id	Device id
 * @param port_num	Port number
 * @param[in] buffer	The configuration blob to print.
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
void ipu_nci_dfm_port_print_section0(uint32_t dev_id, uint32_t port_num, const void *buffer);

/**
 * @brief Fill all configuration sections in the provided output buffer.
 * @param dev_id		Device id
 * @param port_num		Port number
 * @param[in] dfm_config	Configuration of the DFM port
 * @param[in] cmdbank_config	Configuration of the command bank for one port.
 * @param[out] buffer		Buffer where the configuration blob is stored.
 * @return Size of the total configuration buffer (all sections)
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_fill_all_sections(uint32_t dev_id, uint32_t port_num,
		const struct nci_dfm_port_config *dfm_config,
		const struct nci_dfm_public_cmdbank_config *cmdbank_config,
		void *buffer);

/**
 * @brief Get size of the total configuration buffer (all sections).
 * Useful for allocating a buffer from the host.
 * @param dev_id	Device id
 * @param port_num	Port number
 * @return Size of the total configuration buffer (all sections).
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_all_sections_size(uint32_t dev_id, uint32_t port_num);

/**
 * @brief This function will enable passive ports after configuration.
 * This should happen ONLY in ISP flow. The host implementation should be empty.
 * For VP-less this is being done from the SPC.
 * @param dev_id	Device id
 * @param port_num	Port number
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
void ipu_nci_dfm_enable_passive_port(uint32_t dev_id, uint32_t port_num, bool is_passive_mode);

/**
 * @brief This function will disable the port before configuration.
 * This should happen ONLY in ISP flow. The host implementation should be empty.
 * For VP-less this is being done from the SPC.
 * @param dev_id	Device id
 * @param port_num	Port number
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
void ipu_nci_dfm_disable_port(uint32_t dev_id, uint32_t port_num);

/**
 * @brief This function prints DFM configurations in registers.
 * This function is available only for cells.
 *
 * @param dev_id	Device id
 * @param port_num	Port number
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
void ipu_nci_dfm_port_print_all_sections_from_registers(uint32_t dev_id, uint32_t port_num);

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_rc_buf_chasing_offset(uint32_t port_num);

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint8_t ipu_nci_dfm_port_buff_ctrl_en_default_value(void);

NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_gather_multicast_offset(uint32_t port_num);
/**
 * Get the offset to ack data field that is sent during OTF on passive DFM port.
 * Ack data field is inside begin or middle command.
 */
NCI_DFM_DESCRIPTORS_STORAGE_CLASS_H
uint32_t ipu_nci_dfm_port_get_otf_ack_data_offset(uint32_t dev_id, uint32_t port_num, bool is_on_middle);

#ifdef __INLINE_NCI_DFM_DESCRIPTORS__
#include "nci_dfm_descriptors_impl.h"
#endif

#endif /* __NCI_DFM_DESCRIPTORS_H_ */
