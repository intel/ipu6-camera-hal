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

#ifndef __NCI_GDC_LOCAL_H
#define __NCI_GDC_LOCAL_H

#include "nci_gdc_types.h"
#include "nci_gdc_storage_class.h"

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_reg_offset(
		const uint32_t reg_id);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_reg_addr(
		const uint32_t reg_id);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_stride_reg_rel_offset(
		void);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_nof_sections(void);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_sizeof_section(
		enum nci_gdc_sections section);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_get_sizeof_blob(void);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_pack_bpp_control_register(
		uint32_t input_bpp,
		uint32_t output_bpp,
		uint32_t input_align,
		uint32_t output_align);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_pack_proc_mode_register(
		uint32_t mode,
		uint32_t op0,
		uint32_t op1,
		uint32_t op2);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_pack_gro_rpwc_register(
		uint32_t gro_en,
		uint32_t rpwc_dis);

NCI_GDC_STORAGE_CLASS_H uint32_t
nci_gdc_pack_coord_ratio_register(
		uint32_t coord_ratio_x,
		uint32_t coord_ratio_y);

#endif /* __NCI_GDC_LOCAL_H */
