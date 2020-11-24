/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2017 - 2017 Intel Corporation.
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

#ifndef __IPU_RESOURCES_TOKEN_FORMAT_IMPL_H
#define __IPU_RESOURCES_TOKEN_FORMAT_IMPL_H

#include "misc_support.h"
#include "vied_nci_eq_types.h"
#include "vied_nci_eq_recv.h"
#include "vied_nci_eq_send.h"
#include "ipu_device_eq_properties.h"
#include "ipu_resources_token_format.h"

/* Generic PID calculation for tokens sent by a device on process completion */
/*
Each process can be identified by the resources used. The resources can be used to define 12 bit
value which can be used to fill the SID and PID of event done token. The 12 bit field is defined as follows,

Encoding for devices except ACB:
|<--------------PID-------------------->|<------SID---------->|
|<--Device type--->|<--Device instance->|<--Device channel--->|
|<----4 bits------>|<----2 bits-------->|<----6 bits--------->|

Encoding for ACB:
|<--------------PID-------------------->|<------SID---------->|
|<--Device type--->|<------------------>|<--Device instance-->|
|<----4 bits------>|<----2 bits-------->|<----6 bits--------->|

 dev_type identifies which device sends back the event (DFM, ACC, VP, SP)
 dev_instance identifies which instance of the device (VP 0/1/2, SP0/1, DFM - LB/BB etc), the different instance of
 ACBs are modeled in SID field due to the large number of ACB instances.
 dev_ch identifies which channel of the device (for DFM this translates to port number)

 Exceptions/reservations :
    dev_type 0xF reserved for special FW events
	dev_instance 0 (PID - 0x3C) - Client/dma tokens to dispatcher queue handled in dispatcher itself (legacy)
		dev_ch 0 (SID 0) - Client
		dev_ch 1 (SID 1) - DMA
		dev_ch 2 (SID 2) - XCOM
	dev_instance 1 (PID - 0x3D) - Identify tokens to FW load queue
	dev_instance 2 (PID - 0x3E) - Identify tokens to terminal load queue
	dev_instance 3 - Available
*/
#define N_BITS_DEVICE_TYPE			(4)
#define N_BITS_DEVICE_INSTANCE			(2)
#define N_BITS_DEVICE_CHANNEL			(6)

#define DEVICE_TYPE_START_OFFSET	 	(N_BITS_DEVICE_INSTANCE + N_BITS_DEVICE_CHANNEL)
#define DEVICE_TYPE_MASK			((1 << N_BITS_DEVICE_TYPE) - 1)

#define DEVICE_INSTANCE_START_OFFSET_NON_ACB 	(N_BITS_DEVICE_CHANNEL)
#define DEVICE_INSTANCE_MASK_NON_ACB 		((1 << N_BITS_DEVICE_INSTANCE) - 1)

#define DEVICE_CHANNEL_START_OFFSET_NON_ACB 	(0)
#define DEVICE_CHANNEL_MASK_NON_ACB 		((1 << N_BITS_DEVICE_CHANNEL) - 1)

#define DEVICE_INSTANCE_START_OFFSET_ACB 	(0)
#define DEVICE_INSTANCE_MASK_ACB 		((1 << N_BITS_DEVICE_CHANNEL) - 1)

#define DEVICE_CHANNEL_START_OFFSET_ACB 	(0)	/* invalid */
#define DEVICE_CHANNEL_MASK_ACB 		(0)	/* invalid */

#define ENCODE_PROCESS_ID_NON_ACB(dev_type, dev_instance, dev_channel) \
	(dev_type << DEVICE_TYPE_START_OFFSET | dev_instance << DEVICE_INSTANCE_START_OFFSET_NON_ACB | dev_channel)

#define ENCODE_PROCESS_ID_ACB(dev_type, dev_instance) \
	(dev_type << DEVICE_TYPE_START_OFFSET | dev_instance << DEVICE_INSTANCE_START_OFFSET_ACB)

STORAGE_CLASS_INLINE
nci_eq_token_t ipu_resources_encode_evq_token(
		enum nci_eq_device_id eq_id,
		ia_css_process_id_t process_id)
{
	uint8_t pid_size = ipu_device_eq_pid_size(eq_id);
	uint16_t pid_mask = (1 << pid_size) - 1;
	uint8_t sid_size = ipu_device_eq_sid_size(eq_id);
	uint16_t sid_mask = (1 << sid_size) - 1;
	nci_eq_msg_t msg = 0;
	nci_eq_pid_t pid = (process_id >> sid_size) & (pid_mask);
	nci_eq_sid_t sid = process_id & (sid_mask);

	return nci_eq_pack(eq_id, sid, pid, msg);
}

STORAGE_CLASS_INLINE
ia_css_process_id_t ipu_resources_decode_evq_token(
		enum nci_eq_device_id eq_id,
		nci_eq_token_t token)
{
	nci_eq_pid_t pid = nci_eq_get_pid(eq_id, token);
	nci_eq_sid_t sid = nci_eq_get_sid(eq_id, token);
	return ipu_resources_process_id_from_sid_pid(eq_id, sid, pid);
}

STORAGE_CLASS_INLINE
ia_css_process_id_t ipu_resources_process_id_from_sid_pid(
		enum nci_eq_device_id eq_id,
		nci_eq_sid_t sid,
		nci_eq_pid_t pid)
{
	uint8_t sid_size = ipu_device_eq_sid_size(eq_id);

	return ((pid << sid_size)| sid);
}

#include "ipu_resources_token_format_impl_dep.h"

#endif /* __IPU_RESOURCES_TOKEN_FORMAT_IMPL_H */
