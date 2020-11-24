/*
* INTEL CONFIDENTIAL
*
* Copyright (C) 2010 - 2017 Intel Corporation.
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
/* Generated file - please do not edit. */

#ifndef _IPU_DEVICE_CELL_PROPERTIES_DEFS_H_
#define _IPU_DEVICE_CELL_PROPERTIES_DEFS_H_
#ifdef XTENSA
#define SPC0_REGS_CBUS_ADDRESS 0x118000
#define SPC0_DMEM_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_uc_tile_ps_cioat_dm_sp
#else
#define SPC0_REGS_CBUS_ADDRESS 0x118000
#define SPC0_DMEM_CBUS_ADDRESS 0x100000
#endif
#define SPC0_DMEM_DBUS_ADDRESS SPC0_DMEM_CBUS_ADDRESS
#define SPP0_REGS_CBUS_ADDRESS 0x130000
#define SPP0_DMEM_CBUS_ADDRESS 0x120000
#define SPP0_DMEM_DBUS_ADDRESS SPP0_DMEM_CBUS_ADDRESS
#ifdef HAS_ISP0
#define ISP0_REGS_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_par_idsp_idsp_tile_top_idsp_sl_stat_ip
#define ISP0_PMEM_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_par_idsp_idsp_tile_top_idsp_sl_pmem_ip
#define ISP0_DMEM_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_par_idsp_idsp_tile_top_idsp_sl_dmem_ip
#define ISP0_BMEM_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_par_idsp_idsp_tile_top_idsp_sl_bmem_ip
#define ISP0_VMEM_CBUS_ADDRESS _hrt_master_to_slave_address_host_op0_to_ipu_par_idsp_idsp_tile_top_idsp_sl_vmem_ip
#define ISP0_PMEM_DBUS_ADDRESS ISP0_PMEM_CBUS_ADDRESS
#define ISP0_DMEM_DBUS_ADDRESS ISP0_PMEM_CBUS_ADDRESS
#define ISP0_BMEM_DBUS_ADDRESS ISP0_BMEM_CBUS_ADDRESS
#define ISP0_VMEM_DBUS_ADDRESS ISP0_VMEM_CBUS_ADDRESS
#endif
#define SPCX_REGS_CBUS_ADDRESS 0x210000
#define SPCX_DMEM_CBUS_ADDRESS 0x200000
#endif /* _IPU_DEVICE_CELL_PROPERTIES_DEFS_H_ */

