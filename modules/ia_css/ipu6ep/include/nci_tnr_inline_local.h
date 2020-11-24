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

#ifndef __NCI_TNR_INLINE_LOCAL_H
#define __NCI_TNR_INLINE_LOCAL_H

#include "nci_tnr_storage_class.h"
#include "type_support.h"
#include <dai/dai_device_access.h>

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_reg_addr(const uint32_t reg_id)
{

	const dai_device_route_t *dai_handle;
	dai_handle = dai_device_get_route(DAI_TYPE_BBTNR, DAI_IPU_TOP_TNR_TOP_I_TNR6, DAI_BBTNR_C_SL);
	assert(dai_handle != NULL);
	return (dai_handle->slave_address + dai_handle->address_map[reg_id]);
}

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_ip_buff_enq_cmd_addr(void)
{
	return(nci_tnr_get_reg_addr(DAI_BBTNR_REG_TNR_REGS_REG_IP_BUFF_ENQ_CMD_INFO));
}

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_ip_ref_buff_enq_cmd_addr(void)
{
	return(nci_tnr_get_reg_addr(DAI_BBTNR_REG_TNR_REGS_REG_IP_REF_BUFF_ENQ_CMD_INFO));
}

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_op_buff_rel_cmd_addr(void)
{
	return(nci_tnr_get_reg_addr(DAI_BBTNR_REG_TNR_REGS_REG_OP_BUFF_REL_CMD_INFO));
}

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_de_queue_ack_addr_dma_hb(void)
{
	return nci_tnr_get_reg_addr(DAI_BBTNR_REG_TNR_REGS_REG_OP_REF_DMA_DE_QUEUE_ACK_INFO);
}

NCI_TNR_STORAGE_CLASS_C uint32_t
nci_tnr_get_op_ref_buff_rel_cmd_addr(void)
{
	return nci_tnr_get_reg_addr(DAI_BBTNR_REG_TNR_REGS_REG_OP_REF_BUFF_REL_CMD_INFO);
}

#endif /* __NCI_TNR_INLINE_LOCAL_H */
