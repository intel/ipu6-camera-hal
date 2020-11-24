/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2015 - 2017 Intel Corporation.
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

#ifndef __IA_CSS_CLIENT_PKG_H
#define __IA_CSS_CLIENT_PKG_H

#include "type_support.h"
#include "ia_css_client_pkg_storage_class.h"
/* for ia_css_client_pkg_header_s (ptr only), ia_css_client_pkg_t */
#include "ia_css_client_pkg_types.h"

IA_CSS_CLIENT_PKG_STORAGE_CLASS_H
int ia_css_client_pkg_get_pg_manifest_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size);

IA_CSS_CLIENT_PKG_STORAGE_CLASS_H
int ia_css_client_pkg_get_prog_list_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size);

IA_CSS_CLIENT_PKG_STORAGE_CLASS_H
int ia_css_client_pkg_get_prog_desc_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size);

IA_CSS_CLIENT_PKG_STORAGE_CLASS_H
int ia_css_client_pkg_get_prog_bin_entry_offset_size(
	const ia_css_client_pkg_t *client_pkg,
	uint32_t program_id,
	uint32_t *offset,
	uint32_t *size);

IA_CSS_CLIENT_PKG_STORAGE_CLASS_H
int ia_css_client_pkg_get_indexed_prog_desc_entry_offset_size(
	const ia_css_client_pkg_t *client_pkg,
	uint32_t program_id,
	uint32_t program_index,
	uint32_t *offset,
	uint32_t *size);

#ifdef __INLINE_CLIENT_PKG__
#include "ia_css_client_pkg_impl.h"
#endif

#endif /* __IA_CSS_CLIENT_PKG_H */
