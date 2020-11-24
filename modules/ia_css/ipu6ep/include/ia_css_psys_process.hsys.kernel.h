/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2018 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_PROCESS_HSYS_KERNEL_H
#define __IA_CSS_PSYS_PROCESS_HSYS_KERNEL_H

/**
 * @file ia_css_psys_process.hsys.kernel.h
 *
 * Define the methods on the process object: Hsys kernel interface
 */

#include <ia_css_psys_process_types.h>

#include <vied_nci_psys_system_global.h>

/*
 * Internal resources
 */

/*!
 * @brief Clear all resource (offset) specifications
 *
 * @param[in]	process				process object
 *
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_clear_all(ia_css_process_t *process);

/*!
 * @brief Set the cell ID resource specification
 *
 * @param[in]	process				process object
 * @param[in]	cell_id				cell ID
 *
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_set_cell(
	ia_css_process_t					*process,
	const vied_nci_cell_ID_t				cell_id);

/*!
 * @brief Clear cell ID resource specification
 *
 * @param[in]	process				process object
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_clear_cell(ia_css_process_t *process);

/*!
 * @brief Set the memory resource (offset) specification for a memory
 * that belongs to the cell that is assigned to the process.
 *
 * @param[in]	process				process object
 * @param[in]	mem_type_id			mem type ID
 * @param[in]	offset				offset
 *
 * @pre The cell ID must be set
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_set_int_mem(
	ia_css_process_t		*process,
	const	vied_nci_mem_type_ID_t	mem_type_id,
	const vied_nci_resource_size_t	offset);

/*!
 * @brief Clear the memory resource (offset) specification for a memory
 * type that belongs to the cell that is assigned to the process.
 *
 * @param[in]	process				process object
 * @param[in]	mem_id				mem ID
 *
 * @pre The cell ID must be set
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_clear_int_mem(
	ia_css_process_t		*process,
	const vied_nci_mem_type_ID_t	mem_type_id);

/*!
 * @brief Set the memory resource (offset) specification for a memory
 * that does not belong to the cell that is assigned to the process.
 *
 * @param[in]	process				process object
 * @param[in]	mem_type_id				mem type ID
 * @param[in]	offset				offset
 *
 * @pre The cell ID must be set
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_set_ext_mem(
	ia_css_process_t		*process,
	const vied_nci_mem_ID_t		mem_id,
	const vied_nci_resource_size_t	offset);

/*!
 * @brief Clear the memory resource (offset) specification for a memory
 * type that does not belong to the cell that is assigned to the process.
 *
 * @param[in]	process				process object
 * @param[in]	mem_id				mem ID
 *
 * Precondition: The cell ID must be set
 *
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_clear_ext_mem(
	ia_css_process_t		*process,
	const vied_nci_mem_type_ID_t	mem_type_id);

/*!
 * @brief Set a device channel resource (offset) specification.
 *
 * @param[in]	process				process object
 * @param[in]	dev_chn_id			device channel ID
 * @param[in]	offset				offset
 *
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_set_dev_chn(
	ia_css_process_t		*process,
	const vied_nci_dev_chn_ID_t	dev_chn_id,
	const vied_nci_resource_size_t	offset);

/*!
 * @brief Clear a device channel resource (offset) specification
 *
 * @param[in]	process				process object
 * @param[in]	dev_chn_id			device channel ID
 *
 * @return < 0 on error
 * @ingroup group_psysapi_process_host_kernel
 */
extern int ia_css_process_clear_dev_chn(
	ia_css_process_t		*process,
	const vied_nci_dev_chn_ID_t	dev_chn_id);

#endif /* __IA_CSS_PSYS_PROCESS_HSYS_KERNEL_H */
