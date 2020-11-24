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

#ifndef __VIED_NCI_EQ_RECV_H
#define __VIED_NCI_EQ_RECV_H

#include "vied_nci_eq_storage_class.h"
#include "vied_nci_eq_types.h"
#include <type_support.h>

/* value returned by nci_eq_recv() when no token is available */
#define EQ_INVALID_TOKEN 0xFFFFFFFF

/** Get the number of tokens that can be received from the event queue
 * device without blocking.
 *
 * Note that this function returns the total number of tokens available from
 * all the queues of the event queue device.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue.
 *
 * @return 	uint32_t, number of tokens in event queue device
 */
NCI_EQ_STORAGE_CLASS_H uint32_t nci_eq_all_available(enum nci_eq_device_id eq_id);

/** Returns the number of tokens that can be received from a specific queue
 * of the event queue device without blocking
 *
 * Get the number of tokens that can be received from the queue specified by the
 * queue number without blocking. Note that this is a lower bound: depending on
 * the hardware capabilities the actual number of tokens in the queue may be
 * larger, but not smaller than the returned number.
 *
 * @param[in]	eq_id		eunm nci_eq_device_id, device ID of the event queue.
 * @param[in]	queue_nr	nci_eq_queue_t, queue number to check for number of
 * available tokens.
 *
 * @return 	uint32_t, number of tokens in queue
 */
NCI_EQ_STORAGE_CLASS_H uint32_t nci_eq_available(
	enum nci_eq_device_id eq_id,
	nci_eq_queue_t queue_nr);

/** Receive a token from an event queue
 *
 * The event queue device consists of number of queues (the total number of
 * queues is an hardware property and is specific to event queue identfied by eq_id).
 * This function returns the token from a specific queue identified by the
 * queue_nr argument.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue.
 * @param[in]	queue_nr	nci_eq_queue_t, queue number from which msg has to be
 * received.
 *
 * @return 	nci_eq_token_t, event queue token
 */
NCI_EQ_STORAGE_CLASS_H nci_eq_token_t nci_eq_recv(
	enum nci_eq_device_id eq_id,
	nci_eq_queue_t queue_nr);

/** Get the message field of a token
 *
 * A token consists of SID + PID + MSG. The number of bits allocated to these
 * fields is specific to the event queue identified by eq_id. This function
 * extracts the MSG field from the token.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue.
 * @param[in]	token		nci_eq_token_t, event queue token
 *
 * @return 	nci_eq_msg_t, MSG field of token
 */
NCI_EQ_STORAGE_CLASS_H nci_eq_msg_t  nci_eq_get_msg(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token);

/** Get the PID field of a token
 *
 * A token consists of SID + PID + MSG. The number of bits allocated to these
 * fields is specific to the event queue identified by eq_id. This function
 * extracts the PID field from the token.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue.
 * @param[in]	token		nci_eq_token_t, event queue token
 *
 * @return 	nci_eq_pid_t, PID field of token
 */
NCI_EQ_STORAGE_CLASS_H nci_eq_pid_t  nci_eq_get_pid(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token);

/** Get the SID field of a token
 *
 * A token consists of SID + PID + MSG. The number of bits allocated to these
 * fields is specific to the event queue identified by eq_id. This function
 * extracts the SID field from the token.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue.
 * @param[in]	token		nci_eq_token_t, event queue token
 *
 * @return 	nci_eq_sid_t, SID field of token
 */
NCI_EQ_STORAGE_CLASS_H nci_eq_sid_t  nci_eq_get_sid(
	enum nci_eq_device_id eq_id,
	nci_eq_token_t token);

#ifdef __INLINE_NCI_EQ__
#include "vied_nci_eq_recv_inline.h"
#endif

#endif /* __VIED_NCI_EQ_RECV_H */
