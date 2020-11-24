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

#ifndef __IPU_DEVICE_EQ_PROPERTIES_IMPL_H
#define __IPU_DEVICE_EQ_PROPERTIES_IMPL_H

/* Properties of the EQ devices in PSYS */
#include "ipu_device_eq_properties.h"
#include "ipu_device_eq_properties_defs.h" /* from the subsystem */
#include "ipu_device_eq_devices.h" /* from the subsystem */
#include "ipu_device_eq_properties_storage_class.h"
#include "ipu_device_eq_properties_impl_dep.h"
#include "type_support.h"

EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_nr_prio[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(NR_PRIOS)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_nr_queues[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(NR_QUEUES)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_queue_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(QUEUE_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_sid_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(SID_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_pid_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(PID_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_gap_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(GAP_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_msg_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(MSG_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_tim_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(TIM_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_nr_block_queues[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(NR_BLOCK_QUEUES)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_block_cntr_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(BLOCK_CNTR_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_tr_tim_size[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(TR_TIM_SIZE)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_trace_entry_depth[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(TRACE_ENTRY_DEPTH)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint8_t ipu_device_eq_properties_trace_depth[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(TRACE_DEPTH)
};
EQ_PROPERTIES_DATA_STORAGE_CLASS_C uint32_t ipu_device_eq_properties_address[NCI_EQ_NUM_DEVS] = {
	EQ_INSTANCES_PROPERTY(ADDRESS)
};

#endif /* __IPU_DEVICE_EQ_PROPERTIES_IMPL_H */
