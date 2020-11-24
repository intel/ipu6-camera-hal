/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2016 - 2017 Intel Corporation.
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

#ifndef __IPU_DEVICE_DFM_PROPERTIES_H
#define __IPU_DEVICE_DFM_PROPERTIES_H

#include "storage_class.h"
#include "type_support.h"
#include "ipu_device_dfm_properties_defs.h" /* from the subsystem */
#include "ipu_fabrics.h"
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_base_addr(const unsigned dfm_id);

/**
 * Address of the dfm slave port within the Fabric it resides
 * dfm_id-> destionation dfm
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_local_addr(const unsigned dfm_id);

/**
 * ipu_device_dfm_addr
 * -> get the address of slave port of DFM (dfm_id)
 *  as seen from the fabric "fabric_id"
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_addr(enum ipu_fabrics_id fabric_id, const unsigned dfm_id);

/**
 * ipu_device_dfm_port_addr
 * -> get the address of the port "port" of the dfm "dfm_id"
 *  as seen from the fabric "fabric_id"
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_port_addr(enum ipu_fabrics_id src_fabric_id,
				const unsigned dfm_id,
				const unsigned port);

/**
 * ipu_device_dfm_local_port_addr
 * -> get the address of the port "port" of the dfm "dfm_id"
 *  within in the same fabric as initiator.
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_local_port_addr(const unsigned dfm_id,
			const unsigned port);

STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_bcpr_addr_for_isl_ps(enum ipu_fabrics_id src_fabric_id,
			 unsigned dfm_bcpr_reg_num);

#include "ipu_device_dfm_properties_func.h"

#endif /* __IPU_DEVICE_DFM_PROPERTIES_H */
