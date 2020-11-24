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

#ifndef __IPU_DEVICE_CELL_PROPERTIES_IMPL_H
#define __IPU_DEVICE_CELL_PROPERTIES_IMPL_H

#include "ipu_device_sp2600_control_properties_impl.h"
#include "ipu_device_cell_properties_defs.h"
#include "ipu_device_cell_type_properties.h"
#include "ipu_device_cell_devices.h"
#include "ipu_device_sp2600_proxy_properties_impl.h"
#ifdef HAS_ISP0
#include "ipu_device_idsp_properties_impl.h"
#endif

static const unsigned int
ipu_device_spc0_mem_address[IPU_DEVICE_SP2600_CONTROL_NUM_MEMORIES] = {
	SPC0_REGS_CBUS_ADDRESS,
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no pmem */
	SPC0_DMEM_CBUS_ADDRESS
};

static const unsigned int
ipu_device_spc0_mem_databus_address[IPU_DEVICE_SP2600_CONTROL_NUM_MEMORIES] = {
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no reg addr */
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no pmem */
	SPC0_DMEM_DBUS_ADDRESS
};

static const unsigned int
ipu_device_spp0_mem_address[IPU_DEVICE_SP2600_PROXY_NUM_MEMORIES] = {
	SPP0_REGS_CBUS_ADDRESS,
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no pmem */
	SPP0_DMEM_CBUS_ADDRESS
};

static const unsigned int
ipu_device_spp0_mem_databus_address[IPU_DEVICE_SP2600_PROXY_NUM_MEMORIES] = {
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no reg addr */
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no pmem */
	SPP0_DMEM_DBUS_ADDRESS
};

#ifdef HAS_ISP0
static const unsigned int
ipu_device_isp0_mem_address[IPU_DEVICE_IDSP_NUM_MEMORIES] = {
	ISP0_REGS_CBUS_ADDRESS, /* reg addr */
	ISP0_PMEM_CBUS_ADDRESS, /* pmem addr */
	ISP0_DMEM_CBUS_ADDRESS, /* dmem addr */
	ISP0_BMEM_CBUS_ADDRESS, /* bamem addr */
	ISP0_VMEM_CBUS_ADDRESS  /* vmem addr */
};

static const unsigned int
ipu_device_isp0_mem_databus_address[IPU_DEVICE_IDSP_NUM_MEMORIES] = {
	IPU_DEVICE_INVALID_MEM_ADDRESS, /* no reg addr */
	ISP0_PMEM_DBUS_ADDRESS,         /* pmem addr */
	ISP0_DMEM_DBUS_ADDRESS,         /* dmem addr */
	ISP0_BMEM_DBUS_ADDRESS,         /* bamem addr */
	ISP0_VMEM_DBUS_ADDRESS          /* vmem addr */
};
#endif

static const struct ipu_device_cell_properties_s
ipu_device_cell_properties[NUM_CELLS] = {
	{
		&ipu_device_sp2600_control_properties,
		ipu_device_spc0_mem_address,
		ipu_device_spc0_mem_databus_address
	},
	{
		&ipu_device_sp2600_proxy_properties,
		ipu_device_spp0_mem_address,
		ipu_device_spp0_mem_databus_address
	},
#ifdef HAS_ISP0
	{
		&ipu_device_idsp_properties,
		ipu_device_isp0_mem_address,
		ipu_device_isp0_mem_databus_address
	},
#endif
};

#ifdef C_RUN

/* Mapping between hrt_hive_processors enum and cell_id's used in FW
 * The mapping of CRUN cell ids can be checked in SDK at
 * systems/ipu_system/hrt/include/ipu_system_hrtx.h
 */
static const int ipu_device_map_cell_id_to_crun_proc_id[NUM_CELLS] = {
	1, /* SPC0 */
	2, /* SPP0 */
#ifdef HAS_ISP0
	0 /* ISP0 */
#endif
};

#endif
#endif /* __IPU_DEVICE_CELL_PROPERTIES_IMPL_H */
