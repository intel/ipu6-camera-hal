/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2018 Intel Corporation.
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

#ifndef __NCI_TNR_LOCAL_H
#define __NCI_TNR_LOCAL_H

#include "nci_tnr_storage_class.h"
#include "nci_tnr_types.h"

/* Function to retrieve TNR HW registerse using the DAI */
NCI_TNR_STORAGE_CLASS_H uint32_t nci_tnr_get_reg_addr(const uint32_t reg_id);

/*
 * This is a trigger signal to TNR meaning that there is a block arrived from GDC.
 * Note, TNR does 3:1 compression here:
 * It waits for the three triggers to arrive in order.
 */
NCI_TNR_STORAGE_CLASS_H uint32_t nci_tnr_get_ip_buff_enq_cmd_addr(void);

/*
 * Get the address for the output buffer release signal.
 * Typically, OFS sends this signal.
 */
NCI_TNR_STORAGE_CLASS_H uint32_t nci_tnr_get_op_buff_rel_cmd_addr(void);

/* Get the address where the DMA HB dequeue ack is being sent after a Slim Next Instruction is taken up. */
NCI_TNR_STORAGE_CLASS_H uint32_t nci_tnr_get_de_queue_ack_addr_dma_hb(void);

/*
 * Get the address where the DMA HB ack is being sent after it is done writing the reference frame to DDR.
 * DMA HB writes 0x1 to this address.
 */
NCI_TNR_STORAGE_CLASS_H uint32_t nci_tnr_get_op_ref_buff_rel_cmd_addr(void);

/*
 * Get the address where MBR signals TNR that a new reference block has arrived.
 * Note: TNR waits for three such signals (Y, U/V and rec. sim.) to arrive in order to start processing it.
 * This is 6-entry fifo (to accommodate double buffering)
 */
NCI_TNR_STORAGE_CLASS_C uint32_t nci_tnr_get_ip_ref_buff_enq_cmd_addr(void);

#endif /* __NCI_TNR_LOCAL_H */
