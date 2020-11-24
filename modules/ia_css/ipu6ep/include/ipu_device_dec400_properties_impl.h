/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_DEC400_PROPERTIES_IMPL_H
#define __IPU_DEVICE_DEC400_PROPERTIES_IMPL_H

/* Properties of the DEC400 devices in PSYS */
#include "ipu_device_dec400_properties.h"
#include "ipu_device_dec400_properties_struct.h" /* from interface */
#include "ipu_device_dec400_properties_defs.h" /* from the subsystem */
#include "ipu_device_dec400_devices.h" /* from the subsystem */
#include "ipu_device_dec400_properties_storage_class.h"

#if (HAS_DEC400 == 1)
DEC400_PROPERTIES_DATA_STORAGE_CLASS_C struct ipu_device_dec400_properties_s
	ipu_device_dec400_properties[NCI_DEC400_NUM_DEVICES] = {
	{ /* NCI_DEC400_DECODER */
		IPU_DEVICE_DEC400_DECODER_SP, /* regs_base_address */
		DEC400_REG_GCREG_AHBDECREAD_CONFIG0, /* reg_config_offset */
		DEC400_REG_GCREG_AHBDECREAD_EX_CONFIG0, /* reg_exConfig_offset */
		DEC400_REG_GCREG_AHBDECREAD_BUFFER_BASE0, /* reg_buffer_base_offset */
		DEC400_REG_GCREG_AHBDECREAD_CACHE_BASE0, /* reg_buffer_cache_offset */
		DEC400_REG_GCREG_AHBDECREAD_BUFFER_END0, /* reg_buffer_end_offset */
		DEC400_REG_GCREG_AHBDECREAD_STRIDE0, /* reg_stride_offset */
		DEC400_REG_GCREG_AHBDECSTATE_COMMIT, /* Commit state to shadow registers. */
		IPU_DEVICE_DEC400_GP_REGS_SP, /* gp_regs_base_address */
		DEC400_GP_C2_REG_DCMP_AXI_ID, /* gp_reg_axi_id_offset */
		DEC400_GP_C2_REG_DCMP_PATH_EN, /* gp_reg_path_en_offset */
		IPU_DEVICE_DEC400_IRQ_REGS_SP /* irq_regs_base_address */
	},
	{ /* NCI_DEC400_ENCODER */
		IPU_DEVICE_DEC400_ENCODER_SP, /* regs_base_address */
		DEC400_REG_GCREG_AHBDECWRITE_CONFIG0, /* reg_config_offset */
		DEC400_REG_GCREG_AHBDECWRITE_EX_CONFIG0, /* reg_exConfig_offset */
		DEC400_REG_GCREG_AHBDECWRITE_BUFFER_BASE0, /* reg_buffer_base_offset */
		DEC400_REG_GCREG_AHBDECWRITE_CACHE_BASE0, /* reg_buffer_cache_offset */
		DEC400_REG_GCREG_AHBDECWRITE_BUFFER_END0, /* reg_buffer_end_offset */
		DEC400_REG_GCREG_AHBDECWRITE_STRIDE0, /* reg_stride_offset */
		DEC400_REG_GCREG_AHBDECSTATE_COMMIT, /* Commit state to shadow registers. */
		IPU_DEVICE_DEC400_GP_REGS_SP, /* gp_regs_base_address */
		DEC400_GP_C2_REG_CMP_AXI_ID, /* gp_reg_axi_id_offset */
		DEC400_GP_C2_REG_CMP_PATH_EN, /* gp_reg_path_en_offset */
		IPU_DEVICE_DEC400_IRQ_REGS_SP /* irq_regs_base_address */
	}
};
#endif

#endif /* __IPU_DEVICE_DEC400_PROPERTIES_IMPL_H */
