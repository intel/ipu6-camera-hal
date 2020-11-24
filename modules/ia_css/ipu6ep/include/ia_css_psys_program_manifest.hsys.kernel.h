/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2019 Intel Corporation.
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

#ifndef __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_KERNEL_H
#define __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_KERNEL_H

/*! \file */

/** @file ia_css_psys_program_manifest.hsys.kernel.h
 *
 * Define the methods on the program manifest object: Hsys kernel interface
 */

#include <ia_css_psys_manifest_types.h>

#include <vied_nci_psys_system_global.h>

#include <type_support.h>					/* uint8_t */
#include "ia_css_psys_static_storage_class.h"

/*
 * Resources needs
 */

/*! Get the cell ID from the program manifest object

 @param	manifest[in]			program manifest object

 Note: If the cell ID is specified, the program this manifest belongs to
 must be mapped on that instance. If the cell ID is invalid (limit value)
 then the cell type ID must be specified instead

 @return cell ID, limit value if not specified
 */
extern
vied_nci_cell_ID_t ia_css_program_manifest_get_cell_ID(
	const ia_css_program_manifest_t			*manifest);

/*! Get the cell type ID from the program manifest object

 @param	manifest[in]			program manifest object

 Note: If the cell type ID is specified, the program this manifest belongs
 to can be mapped on any instance of this clee type. If the cell type ID is
 invalid (limit value) then a specific cell ID must be specified instead

 @return cell ID, limit value if not specified
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_cell_type_ID_t ia_css_program_manifest_get_cell_type_ID(
	const ia_css_program_manifest_t			*manifest);

/*! Get the memory resource (size) specification for a memory
 that belongs to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type ID

 @return 0 when not applicable
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_int_mem_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id);

/*! Get the memory resource (size) specification for a memory
 that does not belong to the cell where the program will be mapped

 @param	manifest[in]			program manifest object
 @param	mem_type_id[in]			mem type ID

 @return 0 when not applicable
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_ext_mem_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_mem_type_ID_t			mem_type_id);

#if HAS_DFM
/*! Get a device channel resource (size) specification

 @param	manifest[in]			program manifest object
 @param	dev_chn_id[in]			device channel ID

 @return 0 when not applicable
 */
IA_CSS_PSYS_STATIC_STORAGE_CLASS_H
vied_nci_resource_size_t ia_css_program_manifest_get_dev_chn_size(
	const ia_css_program_manifest_t			*manifest,
	const vied_nci_dev_chn_ID_t				dev_chn_id);
#endif
#endif /* __IA_CSS_PSYS_PROGRAM_MANIFEST_HSYS_KERNEL_H */
