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

#ifndef __NCI_PIXEL_STR2MMIO_PRIVATE_TYPES_H
#define __NCI_PIXEL_STR2MMIO_PRIVATE_TYPES_H

#include "nci_pixel_str2mmio_types.h"
#include <stream2mmio_defs.h>               /* for _STREAM2MMIO_COMMAND_REG_ID, ... */
#include "type_support.h"

/*
 * Register map from one example str2mmio instance.
 * This nci module needs an update/versioning in case device address
 * map goes through a change!
-------------------------------------------------------------------
-- Device ipu_isl_ctrl_ps_top_pixel_s2m_sis : registers accessible from port sl_in
--Address	--	Rd only		Register_name
--------------------------------------------------------------------
  36E000..40				Skipped.. Only for MIPI str2mmio related registers
  36E040				register stream2mmio_snd_cmd_sid0 is SKIPPED on purpose
  36E044		 0		register : stream2mmio_pix_width_id_sid0
  36E048		 0		register : stream2mmio_start_address_sid0
  36E04C		 0		register : stream2mmio_end_address_sid0
  36E050		 0		register : stream2mmio_stride_sid0
  36E054		 0		register : stream2mmio_num_items_sid0
  36E058		 0		register : stream2mmio_block_when_no_cmd_sid0
  36E05C		 0		register : stream2mmio_ack_base_addr_sid0
  36E060		 0		register : stream2mmio_sidpid_addr_sid0
  36E064				register stream2mmio_datafield_remap_cfg_sid0 is SKIPPED on purpose
  36E068				register stream2mmio_rpwc_enable_reg_id_sid0 is SKIPPED on purpose
-------------------------------------------------------------------
*/
#define NCI_PIXEL_STR2MMIO_COMMAND_REG_OFFSET	(_STREAM2MMIO_REG_ALIGN * _MIPI_STREAM2MMIO_COMMON_REG_OFFSET + _STREAM2MMIO_COMMAND_REG_ID)
/* Configuration registers lay just after command register which has size _STREAM2MMIO_REG_ALIGN */
#define NCI_PIXEL_STR2MMIO_SECTION0_OFFSET	(NCI_PIXEL_STR2MMIO_COMMAND_REG_OFFSET + _STREAM2MMIO_REG_ALIGN)

/*Bit Masks for str2mmio registers */
#define STR2MMIO_PIX_WIDTH_REG_MASK			(0x3)
#define STR2MMIO_MPORT_AW_MASK				(0xFFFFFFFF)
#define STR2MMIO_NUM_ITEMS_REG_MASK			(0x003FFFFF)
#define STR2MMIO_BLOCK_WHEN_NO_CMD_REG_MASK		(0x1)
#define STR2MMIO_ACK_BASE_ADDR_REG_MASK			(0xFFFFFFFF)
#define STR2MMIO_RPWC_ENABLE_REG_MASK			(0x1)
#define STR2MMIO_SIDPID_REG_MASK			(0xFFF)

struct nci_pixel_str2mmio_private_cfg_section0 {
	uint32_t pix_width;
	uint32_t start_address;
	uint32_t end_address;
	uint32_t stride;
	uint32_t num_items;
	uint32_t block_when_no_cmd;
	uint32_t ack_base_addr;
	uint32_t sidpid;
	uint32_t datafield_remap_cfg;
	uint32_t rpwc_enable;
};

#endif /* __NCI_PIXEL_STR2MMIO_PRIVATE_TYPES_H */
