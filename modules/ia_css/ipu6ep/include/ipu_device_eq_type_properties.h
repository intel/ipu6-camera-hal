/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2018 Intel Corporation.
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

#ifndef __IPU_DEVICE_EQ_TYPE_PROPERTIES_H
#define __IPU_DEVICE_EQ_TYPE_PROPERTIES_H

#include "ipu_device_eq_type_properties_dep.h"

/* constants and data type definitions for event queues */

/* Input port register offsets */
enum ipu_device_eq_ip_registers {
	EVENT_QUEUE_IP_QUEUE_BASE		= 0x000,
	EVENT_QUEUE_IP_SDP_BASE			= 0x100,
	EVENT_QUEUE_IP_PIDMAP_BASE		= 0x200,
	EVENT_QUEUE_IP_QCFG_BASE		= 0x300,
	EVENT_QUEUE_IP_QSTAT_BASE		= 0x400, /* status of each queue */
	EVENT_QUEUE_IP_TOT_QCFG			= 0x500, /* flush */
	EVENT_QUEUE_IP_TOT_QSTAT		= 0x600,
	EVENT_QUEUE_IP_QRESERVE			= 0x700,
	EVENT_QUEUE_IP_TIMER_INC		= 0x800,

	/* Tracing */
	EVENT_QUEUE_IP_TRACE_ADDR_FIRST		= 0x900,
	EVENT_QUEUE_IP_TRACE_ADDR_MIDDLE	= 0x904,
	EVENT_QUEUE_IP_TRACE_ADDR_LAST		= 0x908,
	EVENT_QUEUE_IP_TRACE_ADDR_ALL		= 0x90c,
	EVENT_QUEUE_IP_TRACE_ENABLE		= 0x910,
	EVENT_QUEUE_IP_TRACE_PER_PC		= 0x914,
	EVENT_QUEUE_IP_TRACE_HEADER		= 0x918,
	EVENT_QUEUE_IP_TRACE_MODE		= 0x91C,
	EVENT_QUEUE_IP_TRACE_LOST_PACKET	= 0x920,
	EVENT_QUEUE_IP_TRACE_LP_CLEAR		= 0x924,
};

#define EVENT_QUEUE_SDP_BITS 16

#endif /* __IPU_DEVICE_EQ_TYPE_PROPERTIES_H */
