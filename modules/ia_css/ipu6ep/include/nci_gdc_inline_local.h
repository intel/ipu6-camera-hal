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

#ifndef __NCI_GDC_INLINE_LOCAL_H
#define __NCI_GDC_INLINE_LOCAL_H

#include "nci_gdc_types.h"
#include "nci_gdc_storage_class.h"

#include "type_support.h"
#include "misc_support.h"
#include "assert_support.h"
#include <ipu_device_subsystem_ids.h> /* for SSID values */
#include <dai/dai_device_access.h>

#define NCI_GDC_MASK(val, bits)                            ((val) & ((1 << (bits)) -1))

/* bpp_control:
 *
 *       [16]         |       [15]         | [14-13]  |    [12-8]    |  [7-5]   |    [4-0]
 *  output MSB align  |  input MSB align   |  unused  |  output bpp  |  unused  |  input bpp
 */
#define NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BITS             5
#define NCI_GDC_BPP_CONTROL_REG_UNUSED_0_BITS              3
#define NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BITS            5
#define NCI_GDC_BPP_CONTROL_REG_UNUSED_1_BITS              2
#define NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BITS           1
#define NCI_GDC_BPP_CONTROL_REG_OUTPUT_ALIGN_BITS          1

#define NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BIT_OFFSET       0
#define NCI_GDC_BPP_CONTROL_REG_UNUSED_0_BIT_OFFSET        (NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BIT_OFFSET + NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BITS)
#define NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BIT_OFFSET      (NCI_GDC_BPP_CONTROL_REG_UNUSED_0_BIT_OFFSET + NCI_GDC_BPP_CONTROL_REG_UNUSED_0_BITS)
#define NCI_GDC_BPP_CONTROL_REG_UNUSED_1_BIT_OFFSET        (NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BIT_OFFSET + NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BITS)
#define NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BIT_OFFSET     (NCI_GDC_BPP_CONTROL_REG_UNUSED_1_BIT_OFFSET + NCI_GDC_BPP_CONTROL_REG_UNUSED_1_BITS)
#define NCI_GDC_BPP_CONTROL_REG_OUTPUT_ALIGN_BIT_OFFSET    (NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BIT_OFFSET + NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BITS)

/* proc mode:
 *
 *  [7-6]   |   [5-4]   |   [3-2]   |  [1-0]
 *   Op2    |    Op1    |    Op0    |  mode
 */
#define NCI_GDC_PROC_MODE_REG_MODE_BITS                    2
#define NCI_GDC_PROC_MODE_REG_OP_0_BITS                    2
#define NCI_GDC_PROC_MODE_REG_OP_1_BITS                    2
#define NCI_GDC_PROC_MODE_REG_OP_2_BITS                    2

#define NCI_GDC_PROC_MODE_REG_MODE_BIT_OFFSET              0
#define NCI_GDC_PROC_MODE_REG_OP_0_BIT_OFFSET              (NCI_GDC_PROC_MODE_REG_MODE_BIT_OFFSET + NCI_GDC_PROC_MODE_REG_MODE_BITS)
#define NCI_GDC_PROC_MODE_REG_OP_1_BIT_OFFSET              (NCI_GDC_PROC_MODE_REG_OP_0_BIT_OFFSET + NCI_GDC_PROC_MODE_REG_OP_0_BITS)
#define NCI_GDC_PROC_MODE_REG_OP_2_BIT_OFFSET              (NCI_GDC_PROC_MODE_REG_OP_1_BIT_OFFSET + NCI_GDC_PROC_MODE_REG_OP_1_BITS)

/*
 * GRO_RPWC:
 *
 *      [1]       |     [0]
 *  RWPC DISABLE  |  GRO_ENABLE
 */
#define NCI_GDC_GRO_RPWC_REG_GRO_EN_BITS                   1
#define NCI_GDC_GRO_RPWC_REG_RPWC_DIS_BITS                 1

#define NCI_GDC_GRO_RPWC_REG_GRO_EN_BIT_OFFSET             0
#define NCI_GDC_GRO_RPWC_REG_RPWC_DIS_BIT_OFFSET           (NCI_GDC_GRO_RPWC_REG_GRO_EN_BIT_OFFSET + NCI_GDC_GRO_RPWC_REG_GRO_EN_BITS)

/* coord_ratio :
 *
 *        [7-4]     |      [3-0]
 *   coord_ratio_Y  |  coord_ratio_X
 */
#define NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BITS         4
#define NCI_GDC_COORD_RATIO_REG_COORD_RATIO_Y_BITS         4

#define NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BIT_OFFSET   0
#define NCI_GDC_COORD_RATIO_REG_COORD_RATIO_Y_BIT_OFFSET   (NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BIT_OFFSET + NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BITS)

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_reg_offset(const uint32_t reg_id)
{
	const dai_device_route_t *dai_handle;

	assert(NUM_DAI_BBGDC4_IN_CFG_REGISTERS > reg_id);

	dai_handle = dai_device_get_route(DAI_TYPE_BBGDC4, DAI_IPU_TOP_GDC_TOP_GDC0, DAI_BBGDC4_IN_CFG);

	assert(dai_handle != NULL);
	/* assert that the value in address_map table is an offset */
	assert(dai_handle->address_map[DAI_BBGDC4_REG_STATUS_REG_CMD_INFO] == 0);

	return (dai_handle->address_map[reg_id]);
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_reg_addr(const uint32_t reg_id)
{
	const dai_device_route_t *dai_handle;

	assert(NUM_DAI_BBGDC4_IN_CFG_REGISTERS > reg_id);

	dai_handle = dai_device_get_route(DAI_TYPE_BBGDC4, DAI_IPU_TOP_GDC_TOP_GDC0, DAI_BBGDC4_IN_CFG);

	assert(dai_handle != NULL);

	return (dai_handle->slave_address + dai_handle->address_map[reg_id]);
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_stride_reg_rel_offset(void)
{
	/* gdc_stride_reg_offset should be set to the register ID (not byte offset) of "src_stride" register in GDC
	 * The offset must be relative to the first channel register.
	 */
	return DAI_BBGDC4_REG_STATUS_REG_CH0_SRC_STRIDE_INFO;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_nof_sections(void)
{
	return NCI_GDC_NUM_SECTIONS;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_sizeof_section(
		enum nci_gdc_sections section)
{
	uint32_t size = 0;

	switch (section) {
	case NCI_GDC_GLOBAL_SECTION:
		size = sizeof(struct gdc_global_cfg);
		break;
	case NCI_GDC_CHAN0_SECTION_A:
	case NCI_GDC_CHAN1_SECTION_A:
	case NCI_GDC_CHAN2_SECTION_A:
		size = sizeof(struct gdc_sectionA_cfg);
		break;
	case NCI_GDC_CHAN0_SECTION_B:
	case NCI_GDC_CHAN1_SECTION_B:
	case NCI_GDC_CHAN2_SECTION_B:
		size = sizeof(struct gdc_sectionB_cfg);
		break;
	default:
		assert(0);
		break;
	}

	return size;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_get_sizeof_blob(void)
{
	return sizeof(struct nci_gdc_cfg);
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_pack_bpp_control_register(
		uint32_t input_bpp,
		uint32_t output_bpp,
		uint32_t input_align,
		uint32_t output_align)
{
	uint32_t retval = 0;

	retval |= (NCI_GDC_MASK(input_bpp, NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BITS)
			<< NCI_GDC_BPP_CONTROL_REG_INPUT_BPP_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(output_bpp, NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BITS)
			<< NCI_GDC_BPP_CONTROL_REG_OUTPUT_BPP_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(input_align, NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BITS)
			<< NCI_GDC_BPP_CONTROL_REG_INPUT_ALIGN_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(output_align, NCI_GDC_BPP_CONTROL_REG_OUTPUT_ALIGN_BITS)
			<< NCI_GDC_BPP_CONTROL_REG_OUTPUT_ALIGN_BIT_OFFSET);

	return retval;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_pack_proc_mode_register(
		uint32_t mode,
		uint32_t op0,
		uint32_t op1,
		uint32_t op2)
{
	uint32_t retval = 0;

	retval |= (NCI_GDC_MASK(mode, NCI_GDC_PROC_MODE_REG_MODE_BITS)
			<< NCI_GDC_PROC_MODE_REG_MODE_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(op0, NCI_GDC_PROC_MODE_REG_OP_0_BITS)
			<< NCI_GDC_PROC_MODE_REG_OP_0_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(op1, NCI_GDC_PROC_MODE_REG_OP_1_BITS)
			<< NCI_GDC_PROC_MODE_REG_OP_1_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(op2, NCI_GDC_PROC_MODE_REG_OP_2_BITS)
			<< NCI_GDC_PROC_MODE_REG_OP_2_BIT_OFFSET);

	return retval;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_pack_gro_rpwc_register(
		uint32_t gro_en,
		uint32_t rpwc_dis)
{
	uint32_t retval = 0;

	retval |= (NCI_GDC_MASK(gro_en, NCI_GDC_GRO_RPWC_REG_GRO_EN_BITS)
			<< NCI_GDC_GRO_RPWC_REG_GRO_EN_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(rpwc_dis, NCI_GDC_GRO_RPWC_REG_RPWC_DIS_BITS)
			<< NCI_GDC_GRO_RPWC_REG_RPWC_DIS_BIT_OFFSET);

	return retval;
}

NCI_GDC_STORAGE_CLASS_C uint32_t
nci_gdc_pack_coord_ratio_register(
		uint32_t coord_ratio_x,
		uint32_t coord_ratio_y)
{
	uint32_t retval = 0;

	retval |= (NCI_GDC_MASK(coord_ratio_x, NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BITS)
			<< NCI_GDC_COORD_RATIO_REG_COORD_RATIO_X_BIT_OFFSET);

	retval |= (NCI_GDC_MASK(coord_ratio_y, NCI_GDC_COORD_RATIO_REG_COORD_RATIO_Y_BITS)
			<< NCI_GDC_COORD_RATIO_REG_COORD_RATIO_Y_BIT_OFFSET);

	return retval;
}

#endif /* __NCI_GDC_INLINE_LOCAL_H */
