/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2019 - 2019 Intel Corporation.
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

#ifndef __IPU_DEVICE_DFM_PROPERTIES_FUNC_H
#define __IPU_DEVICE_DFM_PROPERTIES_FUNC_H

#include "type_support.h"
#include "ipu_device_dfm_properties_struct.h" /* from interface */
#include "dai_ipu_top_devices.h" /* from the dai */
#include "assert_support.h"
#include "misc_support.h"
#include "storage_class.h"
#include "ipu_fabrics.h"
#ifdef __VIED_CELL
#include "dai_route.h"
#else
#include <dai_device_access.h>
#include <dai_internal_routes_ipu_top_entry.h>
#endif
#include "dfm_address_map.h"

STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_base_addr(const unsigned dfm_id)
{
	const dai_device_route_t *route;
	assert(dfm_id < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	route = dai_device_get_route(DAI_TYPE_DFM, dfm_id, DAI_DFM_SL_X_C);
	return route->slave_address;
}

/**
 * Address of the dfm slave port within the Fabric it resides
 * dfm_id-> destionation dfm
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_local_addr(const unsigned dfm_id)
{
	assert(dfm_id < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	return ipu_device_dfm_base_addr(dfm_id);
}

/**
 * ipu_device_dfm_addr
 * -> get the address of slave port of DFM (dfm_id)
 *  as seen from the fabric "fabric_id"
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_addr(enum ipu_fabrics_id fabric_id, const unsigned dfm_id)
{
	uint32_t addr;

	assert(dfm_id < NUM_DAI_IPU_TOP_DFM_INSTANCES);
	assert(fabric_id < NUM_OF_IPU_FABRICS);

	addr = ipu_device_dfm_base_addr(dfm_id);

	return addr;
}

/**
 * ipu_device_dfm_port_addr
 * -> get the address of the port "port" of the dfm "dfm_id"
 *  as seen from the fabric "fabric_id"
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_port_addr(enum ipu_fabrics_id src_fabric_id,
			 const unsigned dfm_id,
			 const unsigned port)
{
	uint32_t port_addr;
	const dai_device_route_t *route;

	NOT_USED(src_fabric_id);

	route = dai_device_get_route(DAI_TYPE_DFM, dfm_id, DAI_DFM_SL_X_C);

	port_addr = _dai_get_address_in_bank(route,
		DAI_DFM_RB_DFM_DP_CMD_REGS, port, DAI_DFM_REG_DFM_DP_CMD_REGS_REG_DFM_DP_CMD_RATIO_CONV_EVENT_CMD_INFO);

	return port_addr;
}

/**
 * ipu_device_dfm_local_port_addr
 * -> get the address of the port "port" of the dfm "dfm_id"
 *  within in the same fabric as initiator.
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_local_port_addr(const unsigned dfm_id,
				const unsigned port)
{
	return ipu_device_dfm_port_addr((enum ipu_fabrics_id)0, dfm_id, port);
}

/**
 * ipu_device_dfm_bcpr_addr_for_isl_ps
 * -> get the address of the bcpr register of the dfm "dfm_id"
 *  as seen from the fabric "fabric_id"
 */
STORAGE_CLASS_INLINE uint32_t
ipu_device_dfm_bcpr_addr_for_isl_ps(enum ipu_fabrics_id src_fabric_id,
			 unsigned dfm_bcpr_reg_num)
{
	/* ISL PS has its own DFM instance and the dfm instance id is:
	 * DAI_IPU_TOP_ISL_CTRL_PS_TOP_I_ISL_PS_DFM */
	uint32_t base_addr = ipu_device_dfm_addr(src_fabric_id, DAI_IPU_TOP_ISL_CTRL_PS_TOP_I_ISL_PS_DFM);

	return MAKE_BCPR_ADDR(dfm_bcpr_reg_num, base_addr);
}

#endif /* __IPU_DEVICE_DFM_PROPERTIES_FUNC_H */
