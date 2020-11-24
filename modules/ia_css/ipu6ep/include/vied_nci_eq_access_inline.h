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

#ifndef __VIED_NCI_EQ_ACCESS_INLINE_H
#define __VIED_NCI_EQ_ACCESS_INLINE_H

#include "vied_nci_eq_access.h"
#include "vied_nci_eq_device.h"
#include "vied_nci_eq_reg_access.h"
#include "ipu_device_eq_properties.h"
#include "assert_support.h"
#include "misc_support.h"

/** Get event queue properties **/

STORAGE_CLASS_INLINE unsigned int nci_eq_get_sid_size(enum nci_eq_device_id eq_id)
{
	return ipu_device_eq_sid_size(eq_id);
}

/***************************Queue Internal Configure Interface**********************/

/* 0x300
 * Get/Set wakeup priority level for queue = queue_nr*/
STORAGE_CLASS_INLINE unsigned int nci_eq_get_wakeup_prio(enum nci_eq_device_id eq_id, unsigned int queue_nr)
{
	unsigned int read_val;

	assert(queue_nr < ipu_device_eq_nr_queues(eq_id));
	read_val  = event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_QCFG_BASE + queue_nr*0x4);
	return read_val;
}

STORAGE_CLASS_INLINE void nci_eq_set_wake_prio(enum nci_eq_device_id eq_id, unsigned int queue_nr,
	unsigned int wakeup_prio)
{
	assert(queue_nr < ipu_device_eq_nr_queues(eq_id));
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_QCFG_BASE + queue_nr*0x4, wakeup_prio);
}

/* 0x800
 * Get/Set timer increment value */
STORAGE_CLASS_INLINE unsigned int nci_eq_get_timer_inc(enum nci_eq_device_id eq_id)
{
	return event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_TIMER_INC);
}

STORAGE_CLASS_INLINE void nci_eq_set_timer_inc(enum nci_eq_device_id eq_id, unsigned int timer)
{
	assert(timer < (1u << ipu_device_eq_tim_size(eq_id)));

	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TIMER_INC, timer);
}

STORAGE_CLASS_INLINE void nci_eq_set_sdp(enum nci_eq_device_id eq_id, unsigned int prio, unsigned int deadline)
{
	assert(prio < ipu_device_eq_nr_prio(eq_id));
	assert(deadline < (1u << EVENT_QUEUE_SDP_BITS));

	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_SDP_BASE + prio*4, deadline);
}

STORAGE_CLASS_INLINE void nci_eq_set_pidend(enum nci_eq_device_id eq_id, unsigned int queue, unsigned int pidend)
{
	assert(queue < ipu_device_eq_nr_queues(eq_id));
	assert(pidend < (1u << ipu_device_eq_pid_size(eq_id)));

	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_PIDMAP_BASE + queue*4, pidend);
}

/****************** Queue Wake Up Interface **********************/
/* Only valid for the output port */

STORAGE_CLASS_INLINE unsigned int nci_eq_get_wakup_stat_low(enum nci_eq_device_id eq_id)
{
	return event_queue_op_reg_load(eq_id, EVENT_QUEUE_OP_WAKEUP_STAT_LOW);
}

STORAGE_CLASS_INLINE void nci_eq_enable_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_WAKEUP_ENAB_LOW, wakeup_bit);
}

STORAGE_CLASS_INLINE void nci_eq_set_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_WAKEUP_SET_LOW, wakeup_bit);
}

STORAGE_CLASS_INLINE void nci_eq_clear_wakeup_low(enum nci_eq_device_id eq_id, unsigned int wakeup_bit)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_WAKEUP_CLR_LOW, wakeup_bit);
}

/*************** Tracing Interface *************************/

/* Address of the FW first trace packet */
STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_a(enum nci_eq_device_id eq_id, unsigned int addr)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_ADDR_FIRST, addr);
}

/* Address of the FW middle trace packet */
STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_b(enum nci_eq_device_id eq_id, unsigned int addr)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_ADDR_MIDDLE, addr);
}

/* Address of the FW last trace packet */
STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_c(enum nci_eq_device_id eq_id, unsigned int addr)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_ADDR_LAST, addr);
}

/* Address of all event queue and pc trace packets. */
STORAGE_CLASS_INLINE void nci_eq_set_trace_addr_d(enum nci_eq_device_id eq_id, unsigned int addr)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_ADDR_ALL, addr);
}

/* Individual trace enable.
 * Bit 0 : Enable flush tracing (non-block queue)
 * Bit 1 : Enable entry tracing (non-block queue)
 * Bit 2 : Enable exit tracing (non-block queue)
 * Bit 3 : Enable promotion tracing (non-block queue)
 * Bit 4 : Enable entry tracing (block queue)
 * Bit 5 : Enable exit tracing (block queue)
 * Bit 6 : Enable blocked-exit tracing (block queue)
 * Bit 7 : Enable periodic PC tracing
 * Bit 8 : Enable (I)SP tracing
 */
STORAGE_CLASS_INLINE void nci_eq_enable_trace(enum nci_eq_device_id eq_id, unsigned int trace_enable)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_ENABLE, trace_enable);
}

/* The periodic PC trace timer will count upto timer. It will then reset
 * and send a PC trace packet.
 */
STORAGE_CLASS_INLINE void nci_eq_set_trace_pc_timer(enum nci_eq_device_id eq_id, unsigned int timer)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_PER_PC, timer);
}

/* Bit[3:0] : SVEN header
 */
STORAGE_CLASS_INLINE void nci_eq_set_trace_header(enum nci_eq_device_id eq_id, unsigned int header)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_HEADER, header);
}

/* Mode of tracing :
 * Bit 0 : 0 = lossy tracing
 *         1 = lossless tracing
 */
STORAGE_CLASS_INLINE void nci_eq_set_trace_mode(enum nci_eq_device_id eq_id, unsigned int mode)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_MODE, mode);
}

/* Get the lost packets */
STORAGE_CLASS_INLINE unsigned int nci_eq_get_lost_packets(enum nci_eq_device_id eq_id)
{
	return event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_TRACE_LOST_PACKET);
}

/* Clear lost packets */
STORAGE_CLASS_INLINE void nci_eq_clear_lost_packets(enum nci_eq_device_id eq_id)
{
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_TRACE_LP_CLEAR, 0x1);
}

/* Message of the FIRST FW debug write message */
STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_first(enum nci_eq_device_id eq_id, unsigned int msg)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_FW_TRACE_ADDR_FIRST, msg);
}

/* Message of the any other than FIRST or LAST FW debug write message */
STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_middle(enum nci_eq_device_id eq_id, unsigned int msg)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_FW_TRACE_ADDR_MIDDLE, msg);
}

/* Message of the LAST FW debug write message */
STORAGE_CLASS_INLINE void nci_eq_set_fwtrace_last(enum nci_eq_device_id eq_id, unsigned int msg)
{
	event_queue_op_reg_store(eq_id, EVENT_QUEUE_OP_FW_TRACE_ADDR_LAST, msg);
}

#endif /* __VIED_NCI_EQ_ACCESS_INLINE_H */
