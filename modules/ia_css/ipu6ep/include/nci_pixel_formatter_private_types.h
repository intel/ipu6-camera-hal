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

#ifndef __NCI_PIXEL_FORMATTER_PRIVATE_TYPES_H
#define __NCI_PIXEL_FORMATTER_PRIVATE_TYPES_H

#include "type_support.h"

/*
 * Layout of registers of pixel formatters derived from .hsd file. Table needs to be
 * updated in case of Pixel Formatter update.
 *				reg_id	reg_name	access	reg_width	Configured through (origin)
 *				0	configuration	RW		11			Cached parameter terminal (P2P)
 *				1	flush			WO		1			Not set
 * 				2	status			RO		32			Not set
 *				4	interrrupt_set	WO		32			Not set
 *				5	interrupt_clear	WO		32			Not set
 *				6	interrupt_status RO		32			Not set
 *				7	interrupt_enable RW		32			Not set
 *				8	start_set0		RW		32			Program init terminal (P2P)
 *				9	size_set0		RW		32			Program init terminal (P2P)
 *				10	out_set0		RW		4			Program Control Init terminal (FW)
 *				11	pad_set0		RW		2			Program Control Init terminal (FW)
 *				12	start_set1		RW		32			Not set
 *				13	size_set1		RW		32			Not set
 *				14	out_set1		RW		4			Not set
 *				15	pad_set1		RW		2			Not set
 *				16	start_set2		RW		32			Not set
 *				17	size_set2		RW		32			Not set
 *				18	out_set2		RW		4			Not set
 *				19	pad_set2		RW		2			Not set
 *				20	start_set3		RW		32			Not set
 *				21	size_set3		RW		32			Not set
 *				22	out_set3		RW		4			Not set
 *				23	pad_set3		RW		2			Not set
 *
 * In this module, NCIs are used to configure the following registers that are calculated
 * by firmware. Rest of the registers calculated by P2P. NCI provides support to configure
 * only set0, remaining three sets are not yet implemented in this NCI.
 *				10	out_set0		RW		4			Program Control Init terminal (FW)
 *				11	pad_set0		RW		2			Program Control Init terminal (FW)
 */

#define NCI_PF_OUT_REG_MODE_OFFSET	(0)
#define NCI_PF_OUT_REG_MODE_SIZE	(2)
#define NCI_PF_OUT_MUXA_SEL_OFFSET	(NCI_PF_OUT_REG_MODE_OFFSET + NCI_PF_OUT_REG_MODE_SIZE)
#define NCI_PF_OUT_MUXA_SEL_SIZE	(1)
#define NCI_PF_OUT_MUXB_SEL_OFFSET	(NCI_PF_OUT_MUXA_SEL_OFFSET + NCI_PF_OUT_MUXA_SEL_SIZE)
#define NCI_PF_OUT_MUXB_SEL_SIZE	(1)
#define NCI_PF_BITMASK(a)			((1<<a)-1)

#define NCI_PF_PACK_OUT_REG(reg_mode, muxA_sel, muxB_sel)  \
							((muxB_sel& NCI_PF_BITMASK(NCI_PF_OUT_MUXB_SEL_SIZE)) << NCI_PF_OUT_MUXB_SEL_OFFSET) |	\
							((muxA_sel & NCI_PF_BITMASK(NCI_PF_OUT_MUXA_SEL_SIZE)) << NCI_PF_OUT_MUXA_SEL_OFFSET) |	\
							((reg_mode & NCI_PF_BITMASK(NCI_PF_OUT_REG_MODE_SIZE)) << NCI_PF_OUT_REG_MODE_OFFSET)

struct nci_pixel_formatter_private_cfg {
	uint32_t out_param;
	uint32_t pad_param;
};

#endif /* __NCI_PIXEL_FORMATTER_PRIVATE_TYPES_H */
