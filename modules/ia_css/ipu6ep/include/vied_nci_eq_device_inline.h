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

#ifndef __VIED_NCI_EQ_DEVICE_INLINE_H
#define __VIED_NCI_EQ_DEVICE_INLINE_H

#include "vied_nci_eq_device.h"
#include "vied_nci_eq_access.h" /* for nci_eq_set_pidend, nci_eq_set_sdp, nci_eq_set_timer_inc */
#include "vied_nci_eq_storage_class.h" /* for NCI_EQ_STORAGE_CLASS_C */
#include "ipu_device_eq_properties.h" /* for ipu_device_eq_num_devices, ipu_device_eq_nr_prio, ipu_device_eq_n... */
#include "assert_support.h"

NCI_EQ_STORAGE_CLASS_C
void
nci_eq_device_configure(
	enum nci_eq_device_id eq_id,
	const struct nci_eq_device_config *dconf,
	const struct nci_eq_queue_config *qconf
	/* const struct nci_eq_priority_config *pconf */)
{
	unsigned int i;

	assert(eq_id < ipu_device_eq_num_devices());
	assert(dconf->num_queues >= 1);
	assert(dconf->num_queues <= ipu_device_eq_nr_queues(eq_id));
	assert(dconf->num_priorities > 0);
	assert(dconf->num_priorities <= ipu_device_eq_nr_prio(eq_id));

	/* configure the pidmap, endpid of last queue is not used */
	for (i = 0; i < dconf->num_queues; i++) {
		nci_eq_set_pidend(eq_id, i, qconf[i].endpid);
	}

	/* Set timer_inc and SDP to maximum to avoid priority promotion */
	/* Set timer_inc to maximum */
	nci_eq_set_timer_inc(eq_id, (1u << ipu_device_eq_tim_size(eq_id)) - 1);
	/* Set deadlines to maximum */
	for (i = 1; i < dconf->num_priorities; i++) {
		nci_eq_set_sdp(eq_id, i, (1u << EVENT_QUEUE_SDP_BITS) - 1);
	}
}

/* Flush all queues, both blocking and non-blocking (WO) */
NCI_EQ_STORAGE_CLASS_C
void nci_eq_device_flush(enum nci_eq_device_id eq_id)
{
	assert(eq_id < ipu_device_eq_num_devices());
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TOT_QCFG, 0x1);
}

#endif /* __VIED_NCI_EQ_DEVICE_INLINE_H */
