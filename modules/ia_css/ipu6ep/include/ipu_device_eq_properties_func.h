/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2018 Intel Corporation.
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

#ifndef __IPU_DEVICE_EQ_PROPERTIES_FUNC_H
#define __IPU_DEVICE_EQ_PROPERTIES_FUNC_H

#include "ipu_device_eq_properties_struct.h" /* from interface */
#include "ipu_device_eq_devices.h" /* from the subsystem */
#include "assert_support.h"
#include "storage_class.h"

#ifdef __EXTERN_DATA_EQ_PROPERTIES__
#include "ipu_device_eq_properties_decl.h" /* declaration only */
#else
#include "ipu_device_eq_properties_impl.h" /* definition */
#endif

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_num_devices(void)
{
	return NCI_EQ_NUM_DEVS;
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_nr_prio(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_nr_prio[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_nr_queues(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_nr_queues[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_queue_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_queue_size[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_sid_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	assert(ipu_device_eq_properties_sid_size[eq_id] == ipu_device_eq_properties_sid_size[0]);
	(void)eq_id;
	/* (assume that) All event queues have same pid/sid/msg size,
	 * this improves performance by letting constant propagation do the job in case eq_id
         * is not compile-time known.
	 */
	return ipu_device_eq_properties_sid_size[0];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_pid_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	assert(ipu_device_eq_properties_pid_size[eq_id] == ipu_device_eq_properties_pid_size[0]);
	(void)eq_id;
	/* (assume that) All event queues have same pid/sid/msg size,
	 * this improves performance by letting constant propagation do the job in case eq_id
         * is not compile-time known.
	 */
	return ipu_device_eq_properties_pid_size[0];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_gap_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_gap_size[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_msg_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	assert(ipu_device_eq_properties_msg_size[eq_id] == ipu_device_eq_properties_msg_size[0]);
	(void)eq_id;
	/* (assume that) All event queues have same pid/sid/msg size,
	 * this improves performance by letting constant propagation do the job in case eq_id
         * is not compile-time known.
	 */
	return ipu_device_eq_properties_msg_size[0];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_tim_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_tim_size[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_nr_block_queues(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_nr_block_queues[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_block_cntr_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_block_cntr_size[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_tr_tim_size(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_tr_tim_size[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_trace_entry_depth(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_trace_entry_depth[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_trace_depth(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_trace_depth[eq_id];
}

STORAGE_CLASS_INLINE unsigned int
ipu_device_eq_ip_address(const unsigned int eq_id)
{
	assert(eq_id < NCI_EQ_NUM_DEVS);
	return ipu_device_eq_properties_address[eq_id];
}

#endif /* __IPU_DEVICE_EQ_PROPERTIES_FUNC_H */
