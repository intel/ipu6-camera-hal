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

#ifndef __VIED_NCI_EQ_REG_ACCESS_H
#define __VIED_NCI_EQ_REG_ACCESS_H

#include <type_support.h>
#include "vied_nci_eq_storage_class.h"

/** Store 32 bit value to an event queue register through s_ip slave port
 *
 * This function is used only in the implementation of the event queue NCI.
 * The function used to write to the event queue registers accessible through
 * the s_ip slave port.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	reg		uint32_t, address of the register
 * @param[in]	value		uint32_t, value to store
 *
 * @return 	void
 */
NCI_EQ_STORAGE_CLASS_H void event_queue_ip_reg_store(
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value);

/** Store 32 bit value to an event queue register through s_ip slave port
 *
 * Same behavior as event_queue_ip_reg_store, but only when condition is true
 */
NCI_EQ_STORAGE_CLASS_H
void event_queue_ip_cond_reg_store(
	bool cond,
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value);

/** Load 32 bit value from an event queue register through s_ip slave port
 *
 * This function is used only in the implementation of the event queue NCI.
 * The function is used to read from the event queue registers accessible through
 * the s_ip slave port.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	reg		uint32_t, address of the register
 *
 * @return 	uint32_t, value read from the register
 */
NCI_EQ_STORAGE_CLASS_H uint32_t event_queue_ip_reg_load(
	enum nci_eq_device_id eq_id,
	uint32_t reg);

/** Load 32 bit value from an event queue register through s_ip slave port
 *
 * Same behavior as event_queue_ip_reg_store, but only when condition is true
 *
 * @param[in]   cond    the condition whether to execute the load
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	reg		uint32_t, address of the register
 *
 * @return	uint32_t, value read from the register
 */
NCI_EQ_STORAGE_CLASS_H uint32_t event_queue_ip_cond_reg_load(
	bool cond,
	enum nci_eq_device_id eq_id,
	uint32_t reg);

/** Store 32 bit value to an event queue register through s_op slave port
 *
 * This function is used only in the implementation of the event queue NCI.
 * The function is used to write to the event queue registers accessible through
 * the s_ip slave port.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	reg		uint32_t, address of the register
 * @param[in]	value		uint32_t, value to store
 *
 * @return 	void
 */
NCI_EQ_STORAGE_CLASS_H void event_queue_op_reg_store(
	enum nci_eq_device_id eq_id,
	uint32_t reg,
	uint32_t value);

/** Load 32 bit value from an event queue register through s_op slave port
 *
 * This function is used only in the implementation of the event queue NCI.
 * The function is used to read from the event queue registers accessible through
 * the s_ip slave port.
 *
 * @param[in]	eq_id		enum nci_eq_device_id, device ID of the event queue
 * instance
 * @param[in]	reg		uint32_t, address of the register
 *
 * @return 	uint32_t, value read from the register
 */
NCI_EQ_STORAGE_CLASS_H uint32_t event_queue_op_reg_load(
	enum nci_eq_device_id eq_id,
	uint32_t reg);

#ifdef __INLINE_NCI_EQ__
#include "vied_nci_eq_reg_access_inline.h"
#endif

#endif /* __VIED_NCI_EQ_REG_ACCESS_H */
