/*
 * Copyright (C) 2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
