/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __IA_CSS_CLIENT_PKG_IMPL_H
#define __IA_CSS_CLIENT_PKG_IMPL_H

#include "ia_css_client_pkg.h"
#include "ia_css_client_pkg_types.h"

#include "error_support.h"

IA_CSS_CLIENT_PKG_STORAGE_CLASS_C
int ia_css_client_pkg_get_pg_manifest_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size)
{
	int ret_val = -1;

	verifjmpexit(NULL != client_pkg_header);
	verifjmpexit(NULL != offset);
	verifjmpexit(NULL != size);

	*(offset) = client_pkg_header->pg_manifest_offset;
	*(size) = client_pkg_header->pg_manifest_size;
	ret_val = 0;
EXIT:
	return ret_val;
}

IA_CSS_CLIENT_PKG_STORAGE_CLASS_C
int ia_css_client_pkg_get_prog_list_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size)
{
	int ret_val = -1;

	verifjmpexit(NULL != client_pkg_header);
	verifjmpexit(NULL != offset);
	verifjmpexit(NULL != size);

	*(offset) = client_pkg_header->prog_list_offset;
	*(size) = client_pkg_header->prog_list_size;
	ret_val = 0;
EXIT:
	return ret_val;
}

IA_CSS_CLIENT_PKG_STORAGE_CLASS_C
int ia_css_client_pkg_get_prog_desc_offset_size(
	const struct ia_css_client_pkg_header_s *client_pkg_header,
	uint32_t *offset,
	uint32_t *size)
{
	int ret_val = -1;

	verifjmpexit(NULL != client_pkg_header);
	verifjmpexit(NULL != offset);
	verifjmpexit(NULL != size);

	*(offset) = client_pkg_header->prog_desc_offset;
	*(size) = client_pkg_header->prog_desc_size;
	ret_val = 0;
EXIT:
	return ret_val;
}

IA_CSS_CLIENT_PKG_STORAGE_CLASS_C
int ia_css_client_pkg_get_prog_bin_entry_offset_size(
	const ia_css_client_pkg_t *client_pkg,
	uint32_t program_id,
	uint32_t *offset,
	uint32_t *size)
{
	uint8_t i;
	int ret_val = -1;
	struct ia_css_client_pkg_header_s *client_pkg_header = NULL;
	const struct ia_css_client_pkg_prog_list_s *pkg_prog_list = NULL;
	const struct ia_css_client_pkg_prog_s *pkg_prog_bin_entry = NULL;

	verifjmpexit(NULL != client_pkg);
	verifjmpexit(NULL != offset);
	verifjmpexit(NULL != size);

	client_pkg_header =
		(struct ia_css_client_pkg_header_s *)((uint8_t *)client_pkg);
	pkg_prog_list =
		(struct ia_css_client_pkg_prog_list_s *)((uint8_t *)client_pkg +
		client_pkg_header->prog_list_offset);
	pkg_prog_bin_entry =
		(struct ia_css_client_pkg_prog_s *)((uint8_t *)pkg_prog_list +
		sizeof(struct ia_css_client_pkg_prog_list_s));
	pkg_prog_bin_entry += pkg_prog_list->prog_desc_count;

	for (i = 0; i < pkg_prog_list->prog_bin_count; i++) {
		if (program_id == pkg_prog_bin_entry->prog_id) {
			*(offset) = pkg_prog_bin_entry->prog_offset;
			*(size) = pkg_prog_bin_entry->prog_size;
			ret_val = 0;
			break;
		} else if (0 == pkg_prog_bin_entry->prog_size) {
			/* We can have a variable number of program descriptors.
			 * The first non-valid one will have size set to 0
			*/
			break;
		}
		pkg_prog_bin_entry++;
	}
EXIT:
	return ret_val;
}

IA_CSS_CLIENT_PKG_STORAGE_CLASS_C
int ia_css_client_pkg_get_indexed_prog_desc_entry_offset_size(
	const ia_css_client_pkg_t *client_pkg,
	uint32_t program_id,
	uint32_t program_index,
	uint32_t *offset,
	uint32_t *size)
{
	int ret_val = -1;
	struct ia_css_client_pkg_header_s *client_pkg_header = NULL;
	const struct ia_css_client_pkg_prog_list_s *pkg_prog_list = NULL;
	const struct ia_css_client_pkg_prog_s *pkg_prog_desc_entry = NULL;

	verifjmpexit(NULL != client_pkg);
	verifjmpexit(NULL != offset);
	verifjmpexit(NULL != size);

	client_pkg_header =
		(struct ia_css_client_pkg_header_s *)((uint8_t *)client_pkg);
	pkg_prog_list =
		(struct ia_css_client_pkg_prog_list_s *)((uint8_t *)client_pkg +
		client_pkg_header->prog_list_offset);
	pkg_prog_desc_entry =
		(struct ia_css_client_pkg_prog_s *)((uint8_t *)pkg_prog_list +
		sizeof(struct ia_css_client_pkg_prog_list_s));

	verifjmpexit(program_index < pkg_prog_list->prog_desc_count);
	verifjmpexit(program_id == pkg_prog_desc_entry[program_index].prog_id);
	verifjmpexit(pkg_prog_desc_entry[program_index].prog_size > 0);
	*(offset) = pkg_prog_desc_entry[program_index].prog_offset;
	*(size) = pkg_prog_desc_entry[program_index].prog_size;
	ret_val = 0;

EXIT:
	return ret_val;
}

#endif /* __IA_CSS_CLIENT_PKG_IMPL_H */
