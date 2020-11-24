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

#ifndef __VIED_NCI_EQ_ACCESS_H
#define __VIED_NCI_EQ_ACCESS_H

#include "vied_nci_eq_types.h"

STORAGE_CLASS_INLINE unsigned int nci_eq_get_sid_size(enum nci_eq_device_id eq_id);

STORAGE_CLASS_INLINE unsigned int nci_eq_get_wakeup_prio(enum nci_eq_device_id eq_id, unsigned int queue_nr);

STORAGE_CLASS_INLINE void nci_eq_set_wake_prio(enum nci_eq_device_id eq_id, unsigned int queue_nr,
	unsigned int wakeup_prio);

STORAGE_CLASS_INLINE unsigned int nci_eq_get_timer_inc(enum nci_eq_device_id eq_id);

STORAGE_CLASS_INLINE void nci_eq_set_timer_inc(enum nci_eq_device_id eq_id, unsigned int timer);

STORAGE_CLASS_INLINE void nci_eq_set_sdp(enum nci_eq_device_id eq_id, unsigned int prio, unsigned int deadline);

STORAGE_CLASS_INLINE void nci_eq_set_pidend(enum nci_eq_device_id eq_id, unsigned int queue, unsigned int pidend);

STORAGE_CLASS_INLINE unsigned int nci_eq_get_wakup_stat_low(enum nci_eq_device_id eq_id);

STORAGE_CLASS_INLINE void nci_eq_enable_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit);

STORAGE_CLASS_INLINE void nci_eq_set_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit);

STORAGE_CLASS_INLINE void nci_eq_clear_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit);

STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_a(enum nci_eq_device_id eq_id, unsigned int addr);

STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_b(enum nci_eq_device_id eq_id, unsigned int addr);

STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_c(enum nci_eq_device_id eq_id, unsigned int addr);

STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_d(enum nci_eq_device_id eq_id, unsigned int addr);

STORAGE_CLASS_INLINE void nci_eq_enable_trace(enum nci_eq_device_id eq_id, unsigned int trace_enable);

STORAGE_CLASS_INLINE void nci_eq_set_trace_pc_timer(enum nci_eq_device_id eq_id, unsigned int timer);

STORAGE_CLASS_INLINE void nci_eq_set_trace_header(enum nci_eq_device_id eq_id, unsigned int header);

STORAGE_CLASS_INLINE void nci_eq_set_trace_mode(enum nci_eq_device_id eq_id, unsigned int mode);

STORAGE_CLASS_INLINE unsigned int nci_eq_get_lost_packets(enum nci_eq_device_id eq_id);

STORAGE_CLASS_INLINE void nci_eq_clear_lost_packets(enum nci_eq_device_id eq_id);

STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_first(enum nci_eq_device_id eq_id, unsigned int msg);

STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_middle(enum nci_eq_device_id eq_id, unsigned int msg);

STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_last(enum nci_eq_device_id eq_id, unsigned int msg);

/*we will always inline functions declared in this file */
#include "vied_nci_eq_access_inline.h"

#endif /* __VIED_NCI_EQ_ACCESS_H */
