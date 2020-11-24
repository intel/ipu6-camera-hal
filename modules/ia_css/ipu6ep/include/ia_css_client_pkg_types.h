/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2018 Intel Corporation.
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

#ifndef __IA_CSS_CLIENT_PKG_TYPES_H
#define __IA_CSS_CLIENT_PKG_TYPES_H

#include "type_support.h"

typedef void ia_css_client_pkg_t;

/** Structure will addtional data for "client" program groups
 *
 *  Client program groups actually do some processing or data transfer
 *  as opposed to the firmware servers that provide control
 *
 *  @sa ia_css_client_pkg_prog_list_s
 */
struct ia_css_client_pkg_header_s {
	uint32_t prog_list_offset; 	/**< Offset from this structure to an extension structure, ia_css_client_pkg_prog_list_s  */
	uint32_t prog_list_size;   	/**< Size of the structure at prog_list_offset */
	uint32_t prog_desc_offset;	/**< Offset from this structure to a linked list of program descriptor structures of type, ia_css_progdesc_t */
	uint32_t prog_desc_size;	/**< Size of the entire list of program descriptors at prog_desc_offset */
	uint32_t pg_manifest_offset; /**< Offset from this structure to the program group manifest, ia_css_program_group_manifest_t */
	uint32_t pg_manifest_size;	/**< Size of the structure at program group manifest */
	uint32_t prog_bin_offset; /**< Offset from this structure to an array of ia_css_client_pkg_prog_s  */
	uint32_t prog_bin_size;	  /**< Size of the entire array of programs starting at prog_bin_offset */
};

struct ia_css_client_pkg_prog_s {
	uint32_t prog_id;       /**< Unique identifier of the program in the scope of the program group. */
	uint32_t prog_offset;   /**< Offset from this structure to structure of ia_css_cell_program_s,
							 * which contains more detailed information about a program in the group. */
	uint32_t prog_size;     /**< Size of the program.  Set to zero for the last program? */
};

/** This structure extends ia_css_client_pkg_header_s.
 *
 *  @note Unclear why a separate structure is necesssary and this fields are not included in
 *  ia_css_client_pkg_prog_list_s directly.
 */
struct ia_css_client_pkg_prog_list_s {
	uint32_t prog_desc_count;    /**< Number of entries inside the structure at ia_css_client_pkg_header_s::prog_desc_offset */
	uint32_t prog_bin_count;     /**< Number of entries inside the structure at ia_css_client_pkg_header_s::prog_bin_offset */
};

#endif /* __IA_CSS_CLIENT_PKG_TYPES_H */
