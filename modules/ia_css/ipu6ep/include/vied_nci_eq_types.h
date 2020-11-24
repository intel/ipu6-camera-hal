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

#ifndef __VIED_NCI_EQ_TYPES_H
#define __VIED_NCI_EQ_TYPES_H

#include <type_support.h>
/* this header file is included to expose the device enumeration which is
 * part of the NCI interface */
#include "ipu_device_eq_devices.h"

/* EQ address offset corresponding to one increment in priority */
#define NCI_EQ_PRIORITY_ADDRESS_OFFSET 0x4

/**
 * Type for event queue token
 */
typedef uint32_t nci_eq_token_t;
/**
 * Type for event queue SID
 */
typedef uint32_t nci_eq_sid_t;
/**
 * Type for event queue PID
 */
typedef uint32_t nci_eq_pid_t;
/**
 * Type for event queue MSG
 */
typedef uint32_t nci_eq_msg_t;

/**
 * Type for event queue priority
 */
typedef uint32_t nci_eq_priority_t;

/**
 * Type for event queue queue
 */
typedef uint32_t nci_eq_queue_t;

/**
 * Event queue device configuration
 */
struct nci_eq_device_config
{
	nci_eq_queue_t num_queues;		/**< number of queues to be used */
	nci_eq_priority_t num_priorities;	/**< number of priorities to be used */
};

/**
 * Event queue, queue configuration
 */
struct nci_eq_queue_config
{
    nci_eq_pid_t endpid;		/**< highest PID mapped to this queue */
};

#endif /* __VIED_NCI_EQ_TYPES_H */
