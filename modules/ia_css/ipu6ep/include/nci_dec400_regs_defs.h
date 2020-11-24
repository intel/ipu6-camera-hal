/**
* INTEL CONFIDENTIAL
*
* Copyright (C) 2017 - 2019 Intel Corporation.
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

#ifndef __NCI_DEC400_REGS_DEFS_H
#define __NCI_DEC400_REGS_DEFS_H

#include <ipu_device_eq_devices.h>
#include "type_support.h"

union nci_dec400_register_ex_config {
	struct {
		uint32_t spare0 : 3; /*[0:2]*/
		uint32_t cbsr_width: 13; /*[3:15]*/
		uint32_t bit_depth: 3;  /*[16:18]*/
		uint32_t tile_y: 1; /*[19:19]*/
		uint32_t stream_mode: 5; /*[20:24]*/
		uint32_t spare1 : 1; /*[25:25]*/
		uint32_t ts_cache_mode: 1; /*[26:26]*/
		uint32_t pixel_cache_replacement: 1; /*[27:27]*/
		uint32_t intel_p010: 1; /*[28:28]*/
		uint32_t ts_cache_replacement: 1; /*[29:29]*/
		uint32_t spare2 : 2; /*[30:31]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_config {
	struct {
		uint32_t compression_enable: 1; /*[0:0]*/
		uint32_t spare0: 2; /*[1:2]*/
		uint32_t compression_format: 5; /*[7:3]*/
		uint32_t spare1: 8; /*[8:15]*/
		uint32_t compression_align_mode: 2; /*[17:16]*/
		uint32_t spare2: 4; /*[18:21]*/
		uint32_t tile_align_mode: 3; /*[24:22]*/
		uint32_t tile_mode: 5; /*[29:25]*/
		uint32_t spare3: 2; /*[31:30]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_control {
	struct {
		uint32_t flush: 1; /*[0:0]*/
		uint32_t disable_compression: 1; /*[1:1]*/
		uint32_t disable_ram_clock_gating: 1; /*[2:2]*/
		uint32_t disable_debug_registers: 1; /*[3:3]*/
		uint32_t soft_reset: 1; /*[4:4]*/
		uint32_t spare0: 1; /*[5:5]*/
		uint32_t tile_status_read_id: 5; /*[10:6]*/
		uint32_t tile_status_write_id: 5; /*[15:11]*/
		uint32_t disable_hw_flush: 1; /*[16:16]*/
		uint32_t clk_dis: 1; /*[17:17]*/
		uint32_t sw_flush_id: 6; /*[23:18]*/
		uint32_t spare1: 4; /*[27:24]*/
		uint32_t hw_update_shadow_reg_mode: 1; /*[28:28] for C1 only*/
		uint32_t soft_update_shadow_reg: 1; /*[29:29] for C1 only*/
		uint32_t disable_module_clock_gating: 1; /*[30:30]*/
		uint32_t spare2: 1; /*[31:31]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_control_ex {
	struct {
		uint32_t axuser: 12; /*[11:0]*/
		uint32_t axqos: 4; /*[15:12]*/
		uint32_t enable_burst_split: 1; /*[16:16]*/
		uint32_t enable_end_address_range_check: 1; /*[17:17]*/
		uint32_t spare0: 14; /*[31:18]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_product_id {
	struct {
		uint32_t grade_level: 4; /*[3:0]*/
		uint32_t product_num: 20; /*[23:4]*/
		uint32_t product_type: 4; /*[27:24]*/
		uint32_t spare: 4; /*[31:28]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_chip_info {
	struct {
		uint32_t spare0: 3; /*[2:0]*/
		uint32_t engine_dec: 1; /* [3:3]*/
		uint32_t spare1: 4; /*[7:4]*/
		uint32_t one_set_bus: 1; /* [8:8]*/
		uint32_t two_set_bus: 1; /* [9:9]*/
		uint32_t spare2: 2; /*[11:10]*/
		uint32_t interface_ahb: 1; /* [12:12]*/
		uint32_t spare3: 3; /*[15:13]*/
		uint32_t one_gpu: 1; /* [16:16]*/
		uint32_t spare4: 3; /*[19:17]*/
		uint32_t bus64_bits: 1; /* [20:20]*/
		uint32_t bus128_bits: 1; /* [21:21]*/
		uint32_t spare5: 2; /*[23:22]*/
		uint32_t dec: 1; /* [24:24] */
		uint32_t dec_d: 1; /* [25:25]*/
		uint32_t dec_e: 1; /* [26:26]*/
		uint32_t spare6: 5; /*[31:27]*/
	} data_bits;
	uint32_t data;
};

union nci_dec400_register_debug_stream_counter_select {
	struct {
		uint32_t select_stream: 5; /*[4:0]*/
		uint32_t select_counter: 4; /*[8:5]*/
		uint32_t spare0: 23; /*[31:9]*/
	} data_bits;
	uint32_t data;
};

#endif /* __NCI_DEC400_REGS_DEFS_H */
