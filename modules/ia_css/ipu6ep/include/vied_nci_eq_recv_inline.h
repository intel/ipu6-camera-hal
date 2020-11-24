/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2016 Intel Corporation.
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

#ifndef __VIED_NCI_EQ_RECV_INLINE_H
#define __VIED_NCI_EQ_RECV_INLINE_H

#include "vied_nci_eq_recv.h"
#include "vied_nci_eq_reg_access.h" /* for event_queue_op_reg_load */
#include "vied_nci_eq_storage_class.h" /* for NCI_EQ_STORAGE_CLASS_C */
#include "ipu_device_eq_properties.h" /* for ipu_device_eq_num_devices, ipu_device_eq_msg_size, ipu_device_eq_... */
#include "assert_support.h"
#include "storage_class.h"

STORAGE_CLASS_INLINE unsigned int
mask_value(unsigned int bits)
{
	return (1 << bits) - 1;
}

/* Get number of tokens in a queue */
NCI_EQ_STORAGE_CLASS_C uint32_t nci_eq_available(
	enum nci_eq_device_id eq_id,
	nci_eq_queue_t queue_nr)
{
	assert(eq_id < ipu_device_eq_num_devices());
	assert(queue_nr < ipu_device_eq_nr_queues(eq_id));
	return event_queue_op_reg_load(eq_id, EVENT_QUEUE_IP_QSTAT_BASE + queue_nr*0x4);
}

/* Get the total number of tokens in all queues */
NCI_EQ_STORAGE_CLASS_C uint32_t nci_eq_all_available(enum nci_eq_device_id eq_id)
{
	assert(eq_id < ipu_device_eq_num_devices());
	return event_queue_op_reg_load(eq_id, EVENT_QUEUE_IP_TOT_QSTAT);
}

/* Read a token from the event queue via output port */
NCI_EQ_STORAGE_CLASS_C nci_eq_token_t nci_eq_recv(
	enum nci_eq_device_id eq_id,
	nci_eq_queue_t queue_nr)
{
	assert(eq_id < ipu_device_eq_num_devices());
	assert(queue_nr < ipu_device_eq_nr_queues(eq_id));
	return event_queue_op_reg_load(eq_id, EVENT_QUEUE_OP_QUEUE_BASE+(queue_nr*0x4));
}

/* Token functions */
NCI_EQ_STORAGE_CLASS_C nci_eq_msg_t  nci_eq_get_msg(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token)
{
	assert(eq_id < ipu_device_eq_num_devices());
	return token & mask_value(ipu_device_eq_msg_size(eq_id));
}

NCI_EQ_STORAGE_CLASS_C nci_eq_pid_t  nci_eq_get_pid(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token)
{
	assert(eq_id < ipu_device_eq_num_devices());
	return (token >> ipu_device_eq_msg_size(eq_id)) & mask_value(ipu_device_eq_pid_size(eq_id));
}

NCI_EQ_STORAGE_CLASS_C nci_eq_sid_t  nci_eq_get_sid(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token)
{
	assert(eq_id < ipu_device_eq_num_devices());
	return token >> (ipu_device_eq_msg_size(eq_id) + ipu_device_eq_pid_size(eq_id));
}

#endif /* __VIED_NCI_EQ_RECV_INLINE_H */
