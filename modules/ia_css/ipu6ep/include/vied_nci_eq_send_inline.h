/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2014 - 2019 Intel Corporation.
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

#ifndef __VIED_NCI_EQ_SEND_INLINE_H
#define __VIED_NCI_EQ_SEND_INLINE_H

#include "vied_nci_eq_send.h"
#include "vied_nci_eq_reg_access.h" /* for event_queue_ip_reg_load, event_queue_ip_cond_reg_store, event_que... */
#include "vied_nci_eq_storage_class.h" /* for NCI_EQ_STORAGE_CLASS_C */
#include "ipu_device_eq_properties.h" /* for ipu_device_eq_num_devices, ipu_device_eq_msg_size, ipu_device_eq_... */
#include "assert_support.h"
#include "platform_support.h" /* for ia_css_sleep() */

#ifdef DEBUG_EQ
#include "vied_nci_eq_trace.h" /* for event queue trace messages */
#endif

/*
 * Note about HAS_EQC_IN_VP:
 * If the VP has an EQ space checker in HW, then we must not check in FW. The
 * check (HW or FW) has a side effect: it reserves a location in the EQ. If
 * both HW and FW check, then there are two locations reserved for every
 * event, but only one location is released when the event is written. This
 * will cause a hang when the EQ runs out of reservations.
 */

NCI_EQ_STORAGE_CLASS_C uint32_t nci_eq_reserve(enum nci_eq_device_id eq_id)
{
	assert(eq_id < ipu_device_eq_num_devices());
#ifdef DEBUG_EQ
	IA_CSS_TRACE_1(VIED_NCI_EQ, VERBOSE, "EQC nci_eq_reserve() for %d: Enter\n", eq_id);
#endif
#if defined(__ISP) && HAS_EQC_IN_VP
	return 1;
#else
	return event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_QRESERVE);
#endif
}

NCI_EQ_STORAGE_CLASS_C nci_eq_token_t nci_eq_pack(
	enum nci_eq_device_id eq_id,
	nci_eq_sid_t sid,
	nci_eq_pid_t pid,
	nci_eq_msg_t msg)
{
	assert(eq_id < ipu_device_eq_num_devices());
	assert(sid < (1u << ipu_device_eq_sid_size(eq_id)));
	assert(pid < (1u << ipu_device_eq_pid_size(eq_id)));
	assert(msg < (1u << ipu_device_eq_msg_size(eq_id)));

	return (((sid << ipu_device_eq_pid_size(eq_id)) + pid) << ipu_device_eq_msg_size(eq_id)) + msg;
}

NCI_EQ_STORAGE_CLASS_C void nci_eq_send(
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
	assert(eq_id < ipu_device_eq_num_devices());
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_QUEUE_BASE+(prio * 0x4), token);
}

NCI_EQ_STORAGE_CLASS_C void nci_eq_reserve_send(
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
#if defined(__ISP) && HAS_EQC_IN_VP
	assert(eq_id < ipu_device_eq_num_devices());
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_QUEUE_BASE +
		(prio * NCI_EQ_PRIORITY_ADDRESS_OFFSET), token);
#else
	bool reserved = false;
	assert(eq_id < ipu_device_eq_num_devices());
	while (!reserved) {
		reserved = event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_QRESERVE) != 0;
		event_queue_ip_cond_reg_store(reserved, eq_id, EVENT_QUEUE_IP_QUEUE_BASE +
			(prio * NCI_EQ_PRIORITY_ADDRESS_OFFSET), token);
		ia_css_sleep();
	}
#endif

}

#ifndef NCI_EQ_USE_LOCAL_SEND_BUFFER
#define NCI_EQ_USE_LOCAL_SEND_BUFFER 0
#endif

#if NCI_EQ_USE_LOCAL_SEND_BUFFER

#define NCI_EQ_LOCAL_SEND_BUFFER_SIZE 15
#define NCI_EQ_DEFAULT_EVENT_PRIO 0
#define eq_buffer CAT(eq_buffer_, ISP_ID)
#define eq_buffer_head CAT(eq_buffer_head_, ISP_ID)
#define eq_buffer_tail CAT(eq_buffer_tail_, ISP_ID)

#define NCI_EQ_LOCAL_SEND_BUFFER_VMEM 1

#if NCI_EQ_LOCAL_SEND_BUFFER_VMEM
#if NCI_EQ_LOCAL_SEND_BUFFER_SIZE * 2 > ISP_NWAY
#error "buffer size has to fit in one vector, taking two vector elements per token"
#endif

typedef struct {
	tvector token;
} nci_eq_buffer;
#else
typedef struct {
	nci_eq_token_t token[NCI_EQ_LOCAL_SEND_BUFFER_SIZE];
} nci_eq_buffer;
#endif
static nci_eq_buffer eq_buffer [3];

static unsigned eq_buffer_head [3];
static unsigned eq_buffer_tail [3];

/* todo
 * - cond reserve
 * - merge the two while loops.
 * - merge some sends into main block
 * - SYNC instead of VOLATILE in trace
 */
NCI_EQ_STORAGE_CLASS_C void
nci_eq_push_to_buffer(
	bool cond,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
	OP___dump(__LINE__, eq_id);
	assert(eq_buffer_head[eq_id] < NCI_EQ_LOCAL_SEND_BUFFER_SIZE);
	assert(prio == NCI_EQ_DEFAULT_EVENT_PRIO); /* not buffering the prio, assuming it is always default */
	(void) prio;
#if NCI_EQ_LOCAL_SEND_BUFFER_VMEM
	/* put the buffer in vmem to reduce loads and stores on the dmem lsu */
	eq_buffer[eq_id].token = OP_vec_set(eq_buffer[eq_id].token, eq_buffer_head[eq_id] * 2 + 0, token); /* no need to mask with 0xFFFF because op_vec_set will only take 16bit.*/
	eq_buffer[eq_id].token = OP_vec_set(eq_buffer[eq_id].token, eq_buffer_head[eq_id] * 2 + 1, token >> ISP_VEC_ELEMBITS);
#else
	eq_buffer[eq_id].token[eq_buffer_head[eq_id]] = token;
#endif

	/* only increment the head if condition is true */
	eq_buffer_head[eq_id] += cond;
	/* other option is to always increment the head, and pack the condition
	 * together with the eq_id. this way the compiler could could calculate
	 * the store addresses
	 */
}

NCI_EQ_STORAGE_CLASS_C void
nci_eq_send_from_buffer(bool cond, enum nci_eq_device_id eq_id)
{
	bool reserved;
#if NCI_EQ_LOCAL_SEND_BUFFER_VMEM
	nci_eq_token_t token = OP_vec_get32(eq_buffer[eq_id].token, eq_buffer_tail[eq_id]);
#else
	nci_eq_token_t token = eq_buffer[eq_id].token[eq_buffer_tail[eq_id]];
#endif
	nci_eq_priority_t prio = NCI_EQ_DEFAULT_EVENT_PRIO; /* not buffering the prio, assuming it is always default */

	assert((eq_buffer_head[eq_id] != eq_buffer_tail[eq_id]) | !cond);

#if __ISP && HAS_EQC_IN_VP
	reserved = true;
	event_queue_ip_reg_store(eq_id, EVENT_QUEUE_IP_QUEUE_BASE+(prio * 0x4), token);
#else
	reserved = event_queue_ip_cond_reg_load(cond, eq_id, EVENT_QUEUE_IP_QRESERVE) != 0;
	event_queue_ip_cond_reg_store(reserved, eq_id, EVENT_QUEUE_IP_QUEUE_BASE+(prio * 0x4), token);
#endif
#ifdef DEBUG_EQ
	IA_CSS_TRACE_1(VIED_NCI_EQ, VERBOSE, "EQC nci_eq_send_from_buffer() for %d: Exit\n", eq_id);
#endif
	/* increment the tail ptr when the event is reserved and sent successfully */
	eq_buffer_tail[eq_id] += reserved;
}

NCI_EQ_STORAGE_CLASS_C unsigned
nci_eq_token_buffer_count(unsigned id)
{
	return eq_buffer_head[id] - eq_buffer_tail[id];
}

NCI_EQ_STORAGE_CLASS_C unsigned
nci_eq_token_buffer_empty(unsigned id)
{
	return eq_buffer_head[id] == eq_buffer_tail[id];
}

NCI_EQ_STORAGE_CLASS_C void
nci_eq_send_all_from_buffer(void)
{
	while (!nci_eq_token_buffer_empty(NCI_EQ_SPP_1)) {
		nci_eq_send_from_buffer(true, NCI_EQ_SPP_1);
	}
	/* reset buffer counters because we don't use circular buffer */
	eq_buffer_head[NCI_EQ_SPP_1] = 0;
	eq_buffer_tail[NCI_EQ_SPP_1] = 0;

	while (!nci_eq_token_buffer_empty(NCI_EQ_SPP_0)) {
		nci_eq_send_from_buffer(true, NCI_EQ_SPP_0);
	}
	/* reset buffer counters because we don't use circular buffer */
	eq_buffer_head[NCI_EQ_SPP_0] = 0;
	eq_buffer_tail[NCI_EQ_SPP_0] = 0;

}

NCI_EQ_STORAGE_CLASS_C void nci_eq_cond_reserve_send2buffer(
	bool cond,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
	assert(eq_id < ipu_device_eq_num_devices());

	nci_eq_push_to_buffer(cond, eq_id, prio, token);

	/* at this point we could already start sending out events from the token buffer */
	/* nci_eq_send_from_buffer(eq_buffer_head != eq_buffer_tail, eq_id);*/
}
#endif

NCI_EQ_STORAGE_CLASS_C void nci_eq_cond_reserve_send(
	bool cond,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
#if defined(__ISP) && HAS_EQC_IN_VP
	assert(eq_id < ipu_device_eq_num_devices());
	event_queue_ip_cond_reg_store(cond, eq_id, EVENT_QUEUE_IP_QUEUE_BASE+(prio * 0x4), token);
#else
	bool reserved = false;
	assert(eq_id < ipu_device_eq_num_devices());
	while (cond) {
		reserved = event_queue_ip_reg_load(eq_id, EVENT_QUEUE_IP_QRESERVE) != 0;
		event_queue_ip_cond_reg_store(reserved, eq_id, EVENT_QUEUE_IP_QUEUE_BASE+(prio * 0x4), token);
		cond = !reserved;
		ia_css_sleep();
	}
#endif
#ifdef DEBUG_EQ
	IA_CSS_TRACE_1(VIED_NCI_EQ, VERBOSE, "EQC nci_eq_cond_reserve_send() for %d: Exit\n", eq_id);
#endif
}

NCI_EQ_STORAGE_CLASS_C void nci_eq_cond_reserve_send_buffered(
	bool cond,
	bool buffered,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token)
{
#if NCI_EQ_USE_LOCAL_SEND_BUFFER
	if (buffered) {
		nci_eq_cond_reserve_send2buffer(cond, eq_id, prio, token);
	} else
#endif
	{
		nci_eq_cond_reserve_send(cond, eq_id, prio, token);
	}
	(void) buffered;
}
#endif /* __VIED_NCI_EQ_SEND_INLINE_H */

