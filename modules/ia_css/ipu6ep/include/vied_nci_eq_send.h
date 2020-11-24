/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2013 - 2016 Intel Corporation.
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

/*===========================================================================*/
/** @brief <b> The NCI interfaces will not do any error checking and so will not
 * return an error code. We use only ASSERTS to do range checking. For more
 * information please read the NCI specification document </b>
 */
/*===========================================================================*/
#ifndef __VIED_NCI_EQ_SEND_H
#define __VIED_NCI_EQ_SEND_H

#include "vied_nci_eq_storage_class.h"
#include "vied_nci_eq_types.h"
#include <type_support.h>

/** Try reserve a space in event queue
 *
 * Try to reserve empty space for one token to be sent to an event queue.
 * Returns '1' when space has been reserved, or '0' when space is not
 * available. The event queue protocol requires the caller to first
 * reserve the space before sending a token (to avoid deadlock on the CIO
 * interface). After reserving a space, the caller "must" send a token.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 *
 * @return 	uint32_t, (0 = nospace available, 1 = space available)
 */
NCI_EQ_STORAGE_CLASS_H uint32_t nci_eq_reserve(enum nci_eq_device_id eq_id);

/** Pack an event queue token
 *
 * Packs SID, PID and MSG fields into a token and returns the token
 *
 * @param	eq_id		nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	sid		nci_eq_sid_t, SID field of the token
 * @param[in]	pid		nci_eq_pid_t, PID field of the token
 * @param[in]	msg		nci_eq_msg_t, MSG field of the token
 *
 * @return	nci_eq_token_t, event queue token
 */
NCI_EQ_STORAGE_CLASS_H nci_eq_token_t nci_eq_pack(
	enum nci_eq_device_id eq_id,
	nci_eq_sid_t sid,
	nci_eq_pid_t pid,
	nci_eq_msg_t msg);

/** Send an event queue token
 *
 * Send a token to an event queue device, where the PID field of the token and
 * the event queue's PID map determines into which queue the token will go. Calling
 * this function without calling nci_eq_reserve can cause deadlock on the CIO interface.
 * So the caller "must" first call nci_eq_reserve to successfully reserve a space
 * for the token. Note that for the tokens sent to a specific queue (determined
 * by the PID map), the queuing order is determined by the send order and not by
 * the reserve order (assuming the tokens have the same priority)
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	prio		nci_eq_priority_t, priority level of the token
 * @param[in]	token		nci_eq_token_t, event queue token to be sent
 *
 * @return 	void
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_send(
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token);

/** Reserve and send an event queue token
 *
 * This function combines the implementation of the nci_eq_reserve and the nci_eq_send
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_reserve_send(
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token);

/** Conditional reserve and send an event queue token
 *
 * This function does the same as nci_eq_reserve_send, but only when the condition is true
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_cond_reserve_send(
	bool cond,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token);

/** Conditional reserve and send an event queue token
 *
 * This function does the same as nci_eq_reserve_send, but only when the condition is true
 * and if buffered is true, the events can buffered in a local token buffer.
 */
NCI_EQ_STORAGE_CLASS_H void nci_eq_cond_reserve_send_buffered(
	bool cond,
	bool buffered,
	enum nci_eq_device_id eq_id,
	nci_eq_priority_t prio,
	nci_eq_token_t token);

#ifdef __INLINE_NCI_EQ__
#include "vied_nci_eq_send_inline.h"
#endif

#endif /* __VIED_NCI_EQ_SEND_H */
